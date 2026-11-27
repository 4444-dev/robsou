#include <dependencies/impl/includes.h>

namespace dx_hook {
	using present_fn = HRESULT( __stdcall* )( IDXGISwapChain*, UINT, UINT, void* );
	using exec_cmd_lists_fn = void( __stdcall* )( ID3D12CommandQueue*, UINT, ID3D12CommandList* const* );

	using pfn_d3d11on12_create = HRESULT( WINAPI* )(
		IUnknown*, UINT, const D3D_FEATURE_LEVEL*, UINT,
		IUnknown* const*, UINT, UINT,
		ID3D11Device**, ID3D11DeviceContext**, D3D_FEATURE_LEVEL*
	);

	struct obs_slot_t {
		std::uint64_t slot_addr;
		std::uint64_t orig_present;
	};

	inline present_fn o_present = nullptr;
	inline exec_cmd_lists_fn o_exec_cmd_lists = nullptr;

	inline ID3D12CommandQueue* g_dx12_queue = nullptr;
	inline ID3D11On12Device* g_d3d11on12 = nullptr;
	inline constexpr int k_max_buffers = 8;
	inline ID3D12Resource* g_dx12_buffers[ k_max_buffers ]{};
	inline ID3D11Resource* g_wrapped_buffers[ k_max_buffers ]{};
	inline ID3D11RenderTargetView* g_dx12_rtvs[ k_max_buffers ]{};
	inline UINT g_dx12_buf_count = 0;
	inline bool g_dx12_init_tried = false;
	inline bool g_dx12_queue_hook_tried = false;
	inline bool g_is_dx12 = false;
	inline IDXGISwapChain* g_dx12_sc = nullptr;

	inline bool is_addr_in_module( std::uintptr_t addr, HMODULE mod ) {
		if ( !mod || !addr )
			return false;
		auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( mod );
		if ( dos->e_magic != IMAGE_DOS_SIGNATURE )
			return false;
		auto* nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( reinterpret_cast< std::uintptr_t >( mod ) + dos->e_lfanew );
		if ( nt->Signature != IMAGE_NT_SIGNATURE )
			return false;
		std::uintptr_t start = reinterpret_cast< std::uintptr_t >( mod );
		std::uintptr_t end = start + nt->OptionalHeader.SizeOfImage;
		return addr >= start && addr < end;
	}

	inline bool is_trampoline_for_module( std::uintptr_t code_ptr, HMODULE target_mod ) {
		if ( is_addr_in_module( code_ptr, target_mod ) )
			return true;
		if ( code_ptr < 0x10000 || code_ptr > 0x00007FFFFFFFFFFFULL )
			return false;

		MEMORY_BASIC_INFORMATION mbi{};
		if ( !VirtualQuery( reinterpret_cast< LPCVOID >( code_ptr ), &mbi, sizeof( mbi ) ) )
			return false;
		if ( mbi.State != MEM_COMMIT || ( mbi.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) ) )
			return false;
		constexpr DWORD k_exec_mask = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
		if ( !( mbi.Protect & k_exec_mask ) )
			return false;

		std::uintptr_t reg_end = reinterpret_cast< std::uintptr_t >( mbi.BaseAddress ) + mbi.RegionSize;
		std::size_t avail = reg_end - code_ptr;
		if ( avail > 48 ) avail = 48;
		const auto* p = reinterpret_cast< const std::uint8_t* >( code_ptr );

		for ( std::size_t i = 0; i + 6 <= avail; ++i ) {
			// FF 25 disp32 -> jmp [rip + disp32] (Detours trampoline tail is FF 25 00 00 00 00 <qword>)
			if ( p[ i ] == 0xFF && p[ i + 1 ] == 0x25 && i + 6 + 8 <= avail ) {
				std::int32_t disp = 0;
				std::memcpy( &disp, p + i + 2, sizeof( disp ) );
				std::uintptr_t ptr_loc = code_ptr + i + 6 + disp;
				if ( ptr_loc >= code_ptr && ptr_loc + 8 <= reg_end ) {
					std::uintptr_t target = *reinterpret_cast< const std::uintptr_t* >( ptr_loc );
					if ( is_addr_in_module( target, target_mod ) )
						return true;
				}
			}
			// E9 rel32 -> jmp rel32
			if ( p[ i ] == 0xE9 && i + 5 <= avail ) {
				std::int32_t disp = 0;
				std::memcpy( &disp, p + i + 1, sizeof( disp ) );
				std::uintptr_t target = code_ptr + i + 5 + disp;
				if ( is_addr_in_module( target, target_mod ) )
					return true;
			}
		}
		return false;
	}

	inline std::uintptr_t find_obs_module_slot( HMODULE gh, HMODULE target_mod, std::uintptr_t exclude_slot, bool allow_trampoline ) {
		if ( !gh || !target_mod )
			return 0;

		auto* gh_dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( gh );
		if ( gh_dos->e_magic != IMAGE_DOS_SIGNATURE )
			return 0;
		auto* gh_nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( reinterpret_cast< std::uintptr_t >( gh ) + gh_dos->e_lfanew );
		if ( gh_nt->Signature != IMAGE_NT_SIGNATURE )
			return 0;

		auto* sec = IMAGE_FIRST_SECTION( gh_nt );
		std::uintptr_t fallback_slot = 0;

		for ( WORD i = 0; i < gh_nt->FileHeader.NumberOfSections; ++i ) {
			bool writable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
			bool executable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
			if ( !writable || executable )
				continue;

			std::uintptr_t sec_start = reinterpret_cast< std::uintptr_t >( gh ) + sec[ i ].VirtualAddress;
			DWORD sec_size = sec[ i ].Misc.VirtualSize ? sec[ i ].Misc.VirtualSize : sec[ i ].SizeOfRawData;
			for ( DWORD off = 0; off + sizeof( std::uintptr_t ) <= sec_size; off += sizeof( std::uintptr_t ) ) {
				std::uintptr_t slot_va = sec_start + off;
				if ( slot_va == exclude_slot )
					continue;
				std::uintptr_t val = *reinterpret_cast< const std::uintptr_t* >( slot_va );
				if ( val < 0x7FF000000000ULL || val > 0x00007FFFFFFFFFFFULL || ( val & 0xFFFULL ) == 0 )
					continue;
				if ( is_addr_in_module( val, target_mod ) )
					return slot_va;
				if ( allow_trampoline && !fallback_slot && is_trampoline_for_module( val, target_mod ) )
					fallback_slot = slot_va;
			}
		}
		return fallback_slot;
	}

	inline void release_dx12_targets( ) {
		for ( UINT i = 0; i < k_max_buffers; ++i ) {
			if ( g_dx12_rtvs[ i ] ) { g_dx12_rtvs[ i ]->Release( ); g_dx12_rtvs[ i ] = nullptr; }
			if ( g_wrapped_buffers[ i ] ) { g_wrapped_buffers[ i ]->Release( ); g_wrapped_buffers[ i ] = nullptr; }
			if ( g_dx12_buffers[ i ] ) { g_dx12_buffers[ i ]->Release( ); g_dx12_buffers[ i ] = nullptr; }
		}
		d3d11_ui::g_rtv = nullptr;
		g_dx12_buf_count = 0;
	}

	inline void __stdcall hk_exec_cmd_lists( ID3D12CommandQueue* q, UINT num, ID3D12CommandList* const* lists ) {
		if ( !g_dx12_queue && q ) {
			D3D12_COMMAND_QUEUE_DESC desc = q->GetDesc( );
			if ( desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT )
				g_dx12_queue = q;
		}
		if ( o_exec_cmd_lists )
			o_exec_cmd_lists( q, num, lists );
	}

	inline void try_capture_dx12_queue( IDXGISwapChain* sc ) {
		if ( g_dx12_queue )
			return;

		// Check SwapChain internal pointer slots safely under SEH before hooking ExecuteCommandLists
		__try {
			const auto* sc_qwords = reinterpret_cast< const std::uintptr_t* >( sc );
			HMODULE d3d12 = GetModuleHandleA( "d3d12.dll" );
			if ( d3d12 ) {
				for ( int i = 4; i < 64; ++i ) {
					std::uintptr_t cand = sc_qwords[ i ];
					if ( cand < 0x10000 || cand > 0x00007FFFFFFFFFFFULL || ( cand & 0x7 ) != 0 )
						continue;
					MEMORY_BASIC_INFORMATION mbi{};
					if ( !VirtualQuery( reinterpret_cast< LPCVOID >( cand ), &mbi, sizeof( mbi ) ) ||
						 mbi.State != MEM_COMMIT || ( mbi.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) ) )
						continue;
					std::uintptr_t vt = *reinterpret_cast< const std::uintptr_t* >( cand );
					if ( is_addr_in_module( vt, d3d12 ) ) {
						ID3D12CommandQueue* q = nullptr;
						if ( SUCCEEDED( reinterpret_cast< IUnknown* >( cand )->QueryInterface(
							__uuidof( ID3D12CommandQueue ), reinterpret_cast< void** >( &q ) ) ) && q ) {
							D3D12_COMMAND_QUEUE_DESC qd = q->GetDesc( );
							if ( qd.Type == D3D12_COMMAND_LIST_TYPE_DIRECT ) {
								g_dx12_queue = q;
								q->Release( );
								return;
							}
							q->Release( );
						}
					}
				}
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}

		if ( g_dx12_queue_hook_tried )
			return;
		g_dx12_queue_hook_tried = true;

		HMODULE gh = GetModuleHandleA( "graphics-hook64.dll" );
		HMODULE d3d12 = GetModuleHandleA( "d3d12.dll" );
		std::uintptr_t qslot = find_obs_module_slot( gh, d3d12, 0, true );
		if ( qslot ) {
			std::uintptr_t orig_exec = *reinterpret_cast< const std::uintptr_t* >( qslot );
			o_exec_cmd_lists = reinterpret_cast< exec_cmd_lists_fn >( orig_exec );
			InterlockedExchangePointer(
				reinterpret_cast< volatile PVOID* >( qslot ),
				reinterpret_cast< PVOID >( &hk_exec_cmd_lists )
			);
		}
	}

	inline bool init_dx12_on_present( IDXGISwapChain* sc ) {
		if ( !g_dx12_queue )
			try_capture_dx12_queue( sc );
		if ( !g_dx12_queue )
			return false;

		DXGI_SWAP_CHAIN_DESC sd{};
		if ( SUCCEEDED( sc->GetDesc( &sd ) ) && sd.BufferDesc.Width > 0 && sd.BufferDesc.Height > 0 ) {
			if ( g_dx12_buf_count > 0 &&
				 ( g_dx12_sc != sc ||
				   static_cast< float >( sd.BufferDesc.Width ) != d3d11_ui::g_width ||
				   static_cast< float >( sd.BufferDesc.Height ) != d3d11_ui::g_height ) ) {
				release_dx12_targets( );
			}
		}

		if ( g_d3d11on12 && g_dx12_buf_count > 0 )
			return true;

		ID3D12Device* d3d12_dev = nullptr;
		if ( FAILED( sc->GetDevice( __uuidof( ID3D12Device ), reinterpret_cast< void** >( &d3d12_dev ) ) ) || !d3d12_dev )
			return false;

		if ( !g_d3d11on12 ) {
			if ( g_dx12_init_tried ) {
				d3d12_dev->Release( );
				return false;
			}
			g_dx12_init_tried = true;

			HMODULE h_d3d11 = GetModuleHandleA( "d3d11.dll" );
			if ( !h_d3d11 )
				h_d3d11 = LoadLibraryA( "d3d11.dll" );
			auto fn_11on12 = h_d3d11 ? reinterpret_cast< pfn_d3d11on12_create >( GetProcAddress( h_d3d11, "D3D11On12CreateDevice" ) ) : nullptr;
			if ( !fn_11on12 ) {
				d3d12_dev->Release( );
				return false;
			}

			IUnknown* queues[ 1 ] = { g_dx12_queue };
			ID3D11Device* dev11 = nullptr;
			ID3D11DeviceContext* ctx11 = nullptr;
			HRESULT hr = fn_11on12(
				d3d12_dev,
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,
				nullptr, 0,
				queues, 1,
				0,
				&dev11, &ctx11, nullptr
			);
			if ( FAILED( hr ) || !dev11 || !ctx11 ) {
				d3d12_dev->Release( );
				return false;
			}

			if ( FAILED( dev11->QueryInterface( __uuidof( ID3D11On12Device ), reinterpret_cast< void** >( &g_d3d11on12 ) ) ) || !g_d3d11on12 ) {
				ctx11->Release( );
				dev11->Release( );
				d3d12_dev->Release( );
				return false;
			}

			d3d11_ui::g_device = dev11;
			d3d11_ui::g_context = ctx11;
		}

		g_dx12_sc = sc;
		d3d11_ui::g_swap_chain = sc;
		d3d11_ui::g_hwnd = sd.OutputWindow;
		d3d11_ui::g_width = static_cast< float >( sd.BufferDesc.Width );
		d3d11_ui::g_height = static_cast< float >( sd.BufferDesc.Height );
		if ( d3d11_ui::g_width <= 0.f ) d3d11_ui::g_width = 1920.f;
		if ( d3d11_ui::g_height <= 0.f ) d3d11_ui::g_height = 1080.f;

		g_dx12_buf_count = sd.BufferCount;
		if ( g_dx12_buf_count > k_max_buffers )
			g_dx12_buf_count = k_max_buffers;

		D3D11_RESOURCE_FLAGS d3d11_flags{};
		d3d11_flags.BindFlags = D3D11_BIND_RENDER_TARGET;

		for ( UINT i = 0; i < g_dx12_buf_count; ++i ) {
			if ( SUCCEEDED( sc->GetBuffer( i, __uuidof( ID3D12Resource ), reinterpret_cast< void** >( &g_dx12_buffers[ i ] ) ) ) && g_dx12_buffers[ i ] ) {
				g_d3d11on12->CreateWrappedResource(
					g_dx12_buffers[ i ],
					&d3d11_flags,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PRESENT,
					__uuidof( ID3D11Resource ),
					reinterpret_cast< void** >( &g_wrapped_buffers[ i ] )
				);
				if ( g_wrapped_buffers[ i ] ) {
					d3d11_ui::g_device->CreateRenderTargetView( g_wrapped_buffers[ i ], nullptr, &g_dx12_rtvs[ i ] );
				}
			}
		}

		d3d12_dev->Release( );
		return g_dx12_rtvs[ 0 ] != nullptr;
	}

	inline HRESULT __stdcall hk_present( IDXGISwapChain* sc, UINT sync, UINT flags, void* present_params ) {
		__try {
			apex_sdk::init( );

			if ( !d3d11_ui::g_device ) {
				ID3D11Device* d11 = nullptr;
				if ( SUCCEEDED( sc->GetDevice( __uuidof( ID3D11Device ), reinterpret_cast< void** >( &d11 ) ) ) && d11 ) {
					d11->Release( );
					g_is_dx12 = false;
				}
				else {
					ID3D12Device* d12 = nullptr;
					if ( SUCCEEDED( sc->GetDevice( __uuidof( ID3D12Device ), reinterpret_cast< void** >( &d12 ) ) ) && d12 ) {
						d12->Release( );
						g_is_dx12 = true;
					}
				}
			}

			if ( g_is_dx12 ) {
				if ( init_dx12_on_present( sc ) ) {
					IDXGISwapChain3* sc3 = nullptr;
					UINT idx = 0;
					if ( SUCCEEDED( sc->QueryInterface( __uuidof( IDXGISwapChain3 ), reinterpret_cast< void** >( &sc3 ) ) ) && sc3 ) {
						idx = sc3->GetCurrentBackBufferIndex( );
						sc3->Release( );
					}
					if ( idx < g_dx12_buf_count && g_wrapped_buffers[ idx ] && g_dx12_rtvs[ idx ] ) {
						g_d3d11on12->AcquireWrappedResources( &g_wrapped_buffers[ idx ], 1 );
						d3d11_ui::g_rtv = g_dx12_rtvs[ idx ];
						visuals::draw_render_thread( );
						menu::render( );
						d3d11_ui::render_frame( sc, false );
						d3d11_ui::g_rtv = nullptr;
						g_d3d11on12->ReleaseWrappedResources( &g_wrapped_buffers[ idx ], 1 );
						d3d11_ui::g_context->Flush( );
					}
				}
			}
			else {
				visuals::draw_render_thread( );
				menu::render( );
				d3d11_ui::render_frame( sc );
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
		return o_present ? o_present( sc, sync, flags, present_params ) : S_OK;
	}
}

extern "C" __declspec( dllexport ) dx_hook::obs_slot_t g_obs_slot{};

namespace dx_hook {
	inline bool install( ) {
		if ( g_obs_slot.slot_addr && g_obs_slot.orig_present ) {
			o_present = reinterpret_cast< present_fn >( g_obs_slot.orig_present );
			InterlockedExchangePointer(
				reinterpret_cast< volatile PVOID* >( g_obs_slot.slot_addr ),
				reinterpret_cast< PVOID >( &hk_present )
			);
			return true;
		}

		HMODULE gh = GetModuleHandleA( "graphics-hook64.dll" );
		HMODULE dxgi = GetModuleHandleA( "dxgi.dll" );
		std::uintptr_t slot = find_obs_module_slot( gh, dxgi, 0, false );
		if ( slot ) {
			o_present = reinterpret_cast< present_fn >( *reinterpret_cast< std::uintptr_t* >( slot ) );
			InterlockedExchangePointer(
				reinterpret_cast< volatile PVOID* >( slot ),
				reinterpret_cast< PVOID >( &hk_present )
			);
			return true;
		}

		return false;
	}
}

extern "C" __declspec( dllexport ) BOOL APIENTRY DllMain(
	std::uint64_t module_base,
	std::uint32_t reason,
	std::uint64_t reserved
) {
	( void )reserved;
	if ( reason == DLL_PROCESS_ATTACH ) {
		if ( !module_base )
			return FALSE;

		__try {
			pe::g_module_base = reinterpret_cast< std::uintptr_t >( GetModuleHandleA( nullptr ) );
			dx_hook::install( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
	}
	return TRUE;
}
