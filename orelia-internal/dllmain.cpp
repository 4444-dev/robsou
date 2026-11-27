// orelia-internal/dllmain.cpp
#include <dependencies/impl/includes.h>
#include <workspace/game/features/esp/esp.hxx>

namespace dx11_hook {
	using present_fn = HRESULT( __stdcall* )( IDXGISwapChain*, UINT, UINT, void* );
	using resize_fn = HRESULT( __stdcall* )( IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT );
	using late_update_fn = void( * )( void*, void* );

	struct obs_slot_t {
		std::uint64_t slot_addr;
		std::uint64_t orig_present;
	};

	inline present_fn o_present = nullptr;
	inline resize_fn o_resize_buffers = nullptr;
	inline late_update_fn o_late_update = nullptr;
	inline void** g_late_update_slot = nullptr;
	inline void** g_swapchain_vtable = nullptr;

	inline volatile LONG g_late_update_state = 0;
	inline volatile LONG g_late_update_attempts = 0;
	constexpr LONG late_update_max_attempts = 20;
	constexpr DWORD late_update_retry_ms = 500;

	inline void hk_late_update( void* self, void* method ) {
		if ( o_late_update ) {
			o_late_update( self, method );
		}
		__try {
			esp::g_last_late_update_tick = GetTickCount64( );
			if ( unity::init( ) ) {
				unity::ensure_thread_attached( );
				unity::is_scene_ready( );
				{
					static ULONGLONG s_last_fault_tick = 0;
					const LONG faults = InterlockedCompareExchange( &config::diag_seh_faults, 0, 0 );
					const ULONGLONG now = GetTickCount64( );
					if ( faults <= 20 || ( now - s_last_fault_tick ) >= 500 ) {
						__try {
							esp::collect( );
						}
						__except ( EXCEPTION_EXECUTE_HANDLER ) {
							InterlockedIncrement( &config::diag_seh_faults );
							s_last_fault_tick = now;
						}
					}
				}
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
	}

	using method_get_pointer_fn = void*( * )( void* );

	inline bool is_method_pointer_valid( void* mi ) {
		if ( !mi || !unity::is_mem_readable( reinterpret_cast< uintptr_t >( mi ), 0x18 ) )
			return false;

		void* slot_fn = *reinterpret_cast< void** >( mi );
		void* fn = slot_fn;
		if ( HMODULE ga = GetModuleHandleA( "GameAssembly.dll" ) ) {
			auto f_get_pointer = reinterpret_cast< method_get_pointer_fn >( GetProcAddress( ga, "il2cpp_method_get_pointer" ) );
			if ( f_get_pointer ) {
				void* api_fn = f_get_pointer( mi );
				if ( api_fn ) {
					if ( slot_fn != api_fn ) return false;
					fn = api_fn;
				}
			}
		}

		uintptr_t address = reinterpret_cast< uintptr_t >( fn );
		if ( address < 0x10000 || address > 0x00007FFFFFFFFFFF || ( address & 0x7 ) != 0 )
			return false;

		MEMORY_BASIC_INFORMATION mbi{};
		if ( VirtualQuery( reinterpret_cast< LPCVOID >( address ), &mbi, sizeof( mbi ) ) != sizeof( mbi ) )
			return false;
		if ( mbi.State != MEM_COMMIT ) return false;
		const DWORD executable = mbi.Protect & 0xFF;
		return executable == PAGE_EXECUTE
			|| executable == PAGE_EXECUTE_READ
			|| executable == PAGE_EXECUTE_READWRITE
			|| executable == PAGE_EXECUTE_WRITECOPY;
	}

	inline void try_install_main_hook( ) {
		const LONG state = InterlockedCompareExchange( &g_late_update_state, 0, 0 );
		if ( state == 1 || state == 2 || state == 3 ) return;
		if ( g_late_update_slot ) return;

		static ULONGLONG s_last_try = 0;
		const ULONGLONG now = GetTickCount64( );
		if ( s_last_try != 0 && now - s_last_try < late_update_retry_ms ) return;
		s_last_try = now;

		const LONG attempts = InterlockedIncrement( &g_late_update_attempts );
		if ( attempts > late_update_max_attempts ) {
			InterlockedExchange( &g_late_update_state, 3 );
			config::il2cpp_hook_ready = false;
			config::il2cpp_hook_failed = true;
			return;
		}

		if ( InterlockedCompareExchange( &g_late_update_state, 1, 0 ) != 0 ) return;

		__try {
			HMODULE ga = GetModuleHandleA( "GameAssembly.dll" );
			if ( !ga ) {
				InterlockedExchange( &g_late_update_state, 0 );
				return;
			}

			auto f_dom = reinterpret_cast< unity::domain_get_fn >( GetProcAddress( ga, "il2cpp_domain_get" ) );
			auto f_asms = reinterpret_cast< unity::domain_get_assemblies_fn >( GetProcAddress( ga, "il2cpp_domain_get_assemblies" ) );
			auto f_img = reinterpret_cast< unity::assembly_get_image_fn >( GetProcAddress( ga, "il2cpp_assembly_get_image" ) );
			auto f_name = reinterpret_cast< unity::image_get_name_fn >( GetProcAddress( ga, "il2cpp_image_get_name" ) );
			auto f_cls = reinterpret_cast< unity::class_from_name_fn >( GetProcAddress( ga, "il2cpp_class_from_name" ) );
			auto f_meth = reinterpret_cast< unity::class_get_method_fn >( GetProcAddress( ga, "il2cpp_class_get_method_from_name" ) );
			if ( !f_dom || !f_asms || !f_img || !f_name || !f_cls || !f_meth ) {
				InterlockedExchange( &g_late_update_state, 0 );
				return;
			}

			void* dom = f_dom( );
			if ( !dom ) {
				InterlockedExchange( &g_late_update_state, 0 );
				return;
			}

			size_t count = 0;
			void** asms = f_asms( dom, &count );
			if ( !asms || count == 0 ) {
				InterlockedExchange( &g_late_update_state, 0 );
				return;
			}

			for ( size_t i = 0; i < count; ++i ) {
				void* img = f_img( asms[ i ] );
				if ( !img ) continue;
				if ( !unity::str_eq( f_name( img ), "Cinemachine.dll" ) ) continue;

				void* brain_cls = f_cls( img, "Cinemachine", "CinemachineBrain" );
				if ( !brain_cls ) break;

				void* mi = f_meth( brain_cls, "LateUpdate", 0 );
				if ( !is_method_pointer_valid( mi ) ) break;

				void** slot = reinterpret_cast< void** >( mi );
				if ( *slot == reinterpret_cast< void* >( &hk_late_update ) ) {
					g_late_update_slot = slot;
					InterlockedExchange( &g_late_update_state, 2 );
					config::il2cpp_hook_ready = true;
					config::il2cpp_hook_failed = false;
					return;
				}

				o_late_update = reinterpret_cast< late_update_fn >( *slot );
				DWORD old_prot = 0;
				if ( VirtualProtect( slot, sizeof( void* ), PAGE_EXECUTE_READWRITE, &old_prot ) ) {
					InterlockedExchangePointer( slot, reinterpret_cast< void* >( &hk_late_update ) );
					VirtualProtect( slot, sizeof( void* ), old_prot, &old_prot );
					g_late_update_slot = slot;
					InterlockedExchange( &g_late_update_state, 2 );
					config::il2cpp_hook_ready = true;
					config::il2cpp_hook_failed = false;
					return;
				}
				break;
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
		InterlockedExchange( &g_late_update_state, 0 );
	}

	inline HRESULT __stdcall hk_present( IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags, void* present_params ) {
		__try {
			if ( d3d11_ui::g_width > 0.0f && d3d11_ui::g_height > 0.0f ) {
				try_install_main_hook( );
				__try { esp::render( ); } __except ( EXCEPTION_EXECUTE_HANDLER ) {}
				menu::tick( );
			}
			d3d11_ui::render_frame( swap_chain );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			d3d11_ui::clear_batches( );
		}

		return o_present ? o_present( swap_chain, sync_interval, flags, present_params ) : S_OK;
	}

	inline HRESULT __stdcall hk_resize_buffers( IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags ) {
		if ( d3d11_ui::g_rtv ) {
			d3d11_ui::g_rtv->Release( );
			d3d11_ui::g_rtv = nullptr;
		}
		return o_resize_buffers ? o_resize_buffers( swap_chain, buffer_count, width, height, new_format, swap_chain_flags ) : S_OK;
	}

	inline uintptr_t find_obs_present_slot( HMODULE gh, HMODULE dxgi ) {
		if ( !gh || !dxgi ) return 0;

		auto* dxgi_dos = reinterpret_cast< PIMAGE_DOS_HEADER >( dxgi );
		auto* dxgi_nt = reinterpret_cast< PIMAGE_NT_HEADERS64 >( reinterpret_cast< uintptr_t >( dxgi ) + dxgi_dos->e_lfanew );
		uintptr_t dxgi_start = reinterpret_cast< uintptr_t >( dxgi );
		uintptr_t dxgi_end = dxgi_start + dxgi_nt->OptionalHeader.SizeOfImage;

		auto* gh_dos = reinterpret_cast< PIMAGE_DOS_HEADER >( gh );
		auto* gh_nt = reinterpret_cast< PIMAGE_NT_HEADERS64 >( reinterpret_cast< uintptr_t >( gh ) + gh_dos->e_lfanew );
		auto* sec = IMAGE_FIRST_SECTION( gh_nt );

		for ( WORD i = 0; i < gh_nt->FileHeader.NumberOfSections; ++i ) {
			bool writable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
			bool executable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
			if ( !writable || executable ) continue;

			uintptr_t sec_start = reinterpret_cast< uintptr_t >( gh ) + sec[ i ].VirtualAddress;
			DWORD sec_size = sec[ i ].Misc.VirtualSize ? sec[ i ].Misc.VirtualSize : sec[ i ].SizeOfRawData;
			for ( DWORD off = 0; off + sizeof( uintptr_t ) <= sec_size; off += sizeof( uintptr_t ) ) {
				uintptr_t val = *reinterpret_cast< uintptr_t* >( sec_start + off );
				if ( val >= dxgi_start && val < dxgi_end ) {
					return sec_start + off;
				}
			}
		}
		return 0;
	}

	inline bool install_dxgi_vtable_hook( ) {
		WNDCLASSEXA wc{};
		wc.cbSize = sizeof( wc );
		wc.lpfnWndProc = DefWindowProcA;
		wc.hInstance = GetModuleHandleA( nullptr );
		wc.lpszClassName = "OreliaDxgiTmpWnd";
		RegisterClassExA( &wc );
		HWND tmp_hwnd = CreateWindowExA( 0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW,
			0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr );
		if ( !tmp_hwnd ) {
			UnregisterClassA( wc.lpszClassName, wc.hInstance );
			return false;
		}

		DXGI_SWAP_CHAIN_DESC sd{};
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = tmp_hwnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		IDXGISwapChain* sc = nullptr;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* ctx = nullptr;
		D3D_FEATURE_LEVEL fl{};
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			nullptr, 0, D3D11_SDK_VERSION, &sd, &sc, &dev, &fl, &ctx );
		if ( FAILED( hr ) || !sc ) {
			if ( ctx ) ctx->Release( );
			if ( dev ) dev->Release( );
			DestroyWindow( tmp_hwnd );
			UnregisterClassA( wc.lpszClassName, wc.hInstance );
			return false;
		}

		void** vtable = *reinterpret_cast< void*** >( sc );
		g_swapchain_vtable = vtable;
		DWORD old_prot = 0;
		bool ok = false;
		if ( vtable && vtable[ 8 ] == reinterpret_cast< void* >( &hk_present ) ) {
			ok = true;
		}
		else if ( vtable && VirtualProtect( &vtable[ 8 ], sizeof( void* ) * 6, PAGE_EXECUTE_READWRITE, &old_prot ) ) {
			o_present = reinterpret_cast< present_fn >( vtable[ 8 ] );
			InterlockedExchangePointer( &vtable[ 8 ], reinterpret_cast< PVOID >( &hk_present ) );
			if ( vtable[ 13 ] && vtable[ 13 ] != reinterpret_cast< void* >( &hk_resize_buffers ) ) {
				o_resize_buffers = reinterpret_cast< resize_fn >( vtable[ 13 ] );
				InterlockedExchangePointer( &vtable[ 13 ], reinterpret_cast< PVOID >( &hk_resize_buffers ) );
			}
			VirtualProtect( &vtable[ 8 ], sizeof( void* ) * 6, old_prot, &old_prot );
			ok = true;
		}

		sc->Release( );
		if ( ctx ) ctx->Release( );
		if ( dev ) dev->Release( );
		DestroyWindow( tmp_hwnd );
		UnregisterClassA( wc.lpszClassName, wc.hInstance );
		return ok;
	}

	inline void register_seh_pdata( std::uint64_t module_base ) {
		auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( module_base );
		if ( !dos || dos->e_magic != IMAGE_DOS_SIGNATURE ) return;
		auto* nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( module_base + dos->e_lfanew );
		if ( !nt || nt->Signature != IMAGE_NT_SIGNATURE ) return;
		const auto& dir = nt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXCEPTION ];
		if ( dir.VirtualAddress && dir.Size >= sizeof( RUNTIME_FUNCTION ) ) {
			RtlAddFunctionTable(
				reinterpret_cast< PRUNTIME_FUNCTION >( module_base + dir.VirtualAddress ),
				dir.Size / sizeof( RUNTIME_FUNCTION ),
				module_base
			);
		}
	}

	inline void wipe_pe_headers( std::uint64_t module_base ) {
		auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( module_base );
		if ( !dos || dos->e_magic != IMAGE_DOS_SIGNATURE ) return;
		auto* nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( module_base + dos->e_lfanew );
		if ( !nt || nt->Signature != IMAGE_NT_SIGNATURE ) return;
		SIZE_T hdr_size = nt->OptionalHeader.SizeOfHeaders;
		if ( hdr_size == 0 || hdr_size > 0x2000 ) hdr_size = 0x1000;
		DWORD old_prot = 0;
		if ( VirtualProtect( reinterpret_cast< LPVOID >( module_base ), hdr_size, PAGE_READWRITE, &old_prot ) ) {
			volatile unsigned char* p = reinterpret_cast< volatile unsigned char* >( module_base );
			for ( SIZE_T i = 0; i < hdr_size; ++i ) {
				p[ i ] = 0;
			}
			VirtualProtect( reinterpret_cast< LPVOID >( module_base ), hdr_size, old_prot, &old_prot );
		}
	}
}

extern "C" __declspec( dllexport ) dx11_hook::obs_slot_t g_obs_slot{};

namespace dx11_hook {
	inline bool install( ) {
		if ( g_obs_slot.slot_addr && g_obs_slot.orig_present ) {
			if ( *reinterpret_cast< void** >( g_obs_slot.slot_addr ) == reinterpret_cast< void* >( &hk_present ) )
				return true;
			o_present = reinterpret_cast< present_fn >( g_obs_slot.orig_present );
			InterlockedExchangePointer(
				reinterpret_cast< volatile PVOID* >( g_obs_slot.slot_addr ),
				reinterpret_cast< PVOID >( &hk_present )
			);
			return true;
		}

		HMODULE gh = GetModuleHandleA( "graphics-hook64.dll" );
		HMODULE dxgi = GetModuleHandleA( "dxgi.dll" );
		uintptr_t slot = find_obs_present_slot( gh, dxgi );
		if ( slot ) {
			if ( *reinterpret_cast< void** >( slot ) == reinterpret_cast< void* >( &hk_present ) )
				return true;
			o_present = reinterpret_cast< present_fn >( *reinterpret_cast< uintptr_t* >( slot ) );
			InterlockedExchangePointer(
				reinterpret_cast< volatile PVOID* >( slot ),
				reinterpret_cast< PVOID >( &hk_present )
			);
			return true;
		}

		return install_dxgi_vtable_hook( );
	}
}

extern "C" __declspec( dllexport ) void dpc_entry( void*, void* context, void*, void* ) {
	if ( !context ) return;
	struct dpc_context_t {
		std::uint64_t image_base;
		std::uint64_t entry_point;
		std::uint32_t image_size;
		volatile std::uint32_t status;
	};
	auto* ctx = reinterpret_cast< dpc_context_t* >( context );
	ctx->status = ( ctx->image_base && ctx->image_size ) ? 0u : 2u;
}

extern "C" __declspec( dllexport ) BOOL APIENTRY DllMain(
	std::uint64_t module_base,
	std::uint32_t reason,
	std::uint64_t reserved
) {
	( void )reserved;
	if ( reason == DLL_PROCESS_ATTACH ) {
		if ( !module_base ) return FALSE;
		dx11_hook::register_seh_pdata( module_base );
		__try {
			dx11_hook::install( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
		dx11_hook::wipe_pe_headers( module_base );
	}
	return TRUE;
}
