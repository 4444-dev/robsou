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

	inline void wipe_pe_headers( std::uint64_t module_base );

	inline std::uint64_t g_self_base = 0;

	// Wiping the PE headers destroys the export table, so it cannot happen while
	// the loader may still resolve "DllMain", g_obs_slot or dpc_entry. It is
	// therefore deferred: DllMain records the base, and the wipe runs from Present
	// a few seconds in, once nothing can still be reading the headers.
	inline void late_wipe_headers( ) {
		static int s_frames = 0;
		if ( g_self_base == 0 ) return;
		if ( ++s_frames != 180 ) return;
		__try {
			wipe_pe_headers( g_self_base );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
	}

	// Not a security boundary, but it keeps this thread out of a debugger's thread
	// list, which is the first place anything looks for a foreign thread.
	inline void hide_current_thread( ) {
		HMODULE ntdll = GetModuleHandleA( "ntdll.dll" );
		if ( !ntdll ) return;
		using ntsit_fn = LONG( __stdcall* )( HANDLE, ULONG, void*, ULONG );
		auto f_set_info = reinterpret_cast< ntsit_fn >( GetProcAddress( ntdll, "NtSetInformationThread" ) );
		if ( !f_set_info ) return;
		constexpr ULONG thread_hide_from_debugger = 17;
		f_set_info( GetCurrentThread( ), thread_hide_from_debugger, nullptr, 0 );
	}

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

	// Only real failures burn the retry budget. "GameAssembly not loaded yet" and
	// "Cinemachine not loaded yet" are normal during loading screens and must not
	// consume attempts, otherwise the hook is abandoned ten seconds after the
	// process starts.
	//
	// State: 0 idle, 1 installing, 2 installed, 3 failed for this session. There is
	// no re-arm: after a failure the scene flag stays false (the LateUpdate hook is
	// what updates it), so re-arming on "scene not ready" would retry forever.
	inline void try_install_main_hook( ) {
		if ( InterlockedCompareExchange( &g_late_update_state, 0, 0 ) != 0 ) return;

		static ULONGLONG s_last_try = 0;
		const ULONGLONG now = GetTickCount64( );
		if ( s_last_try != 0 && now - s_last_try < late_update_retry_ms ) return;
		s_last_try = now;

		if ( InterlockedCompareExchange( &g_late_update_state, 1, 0 ) != 0 ) return;

		bool hard_fail = false;

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
				if ( !unity::str_ieq( f_name( img ), "Cinemachine.dll" ) ) continue;

				void* brain_cls = f_cls( img, "Cinemachine", "CinemachineBrain" );
				if ( !brain_cls ) break;

				void* mi = f_meth( brain_cls, "LateUpdate", 0 );
				if ( !is_method_pointer_valid( mi ) ) {
					// Assembly present but the method pointer does not check out:
					// that is a genuine offset mismatch (game update).
					hard_fail = true;
					break;
				}

				void** slot = reinterpret_cast< void** >( mi );
				if ( *slot == reinterpret_cast< void* >( &hk_late_update ) ) {
					g_late_update_slot = slot;
					InterlockedExchange( &g_late_update_attempts, 0 );
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
					InterlockedExchange( &g_late_update_attempts, 0 );
					InterlockedExchange( &g_late_update_state, 2 );
					config::il2cpp_hook_ready = true;
					config::il2cpp_hook_failed = false;
					return;
				}
				hard_fail = true;
				break;
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			hard_fail = true;
		}

		if ( hard_fail && InterlockedIncrement( &g_late_update_attempts ) > late_update_max_attempts ) {
			InterlockedExchange( &g_late_update_state, 3 );
			config::il2cpp_hook_ready = false;
			config::il2cpp_hook_failed = true;
			return;
		}
		InterlockedExchange( &g_late_update_state, 0 );
	}

	inline HRESULT __stdcall hk_present( IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags, void* present_params ) {
		__try {
			// Installing the IL2CPP hook must not depend on the UI being sized.
			try_install_main_hook( );
			if ( d3d11_ui::g_width > 0.0f && d3d11_ui::g_height > 0.0f ) {
				__try { esp::render( ); } __except ( EXCEPTION_EXECUTE_HANDLER ) {}
				menu::tick( );
			}
			d3d11_ui::render_frame( swap_chain );
			late_wipe_headers( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			d3d11_ui::clear_batches( );
		}

		return o_present ? o_present( swap_chain, sync_interval, flags, present_params ) : S_OK;
	}

	inline HRESULT __stdcall hk_resize_buffers( IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags ) {
		// The render target and the cached size both survive ResizeBuffers: drop
		// them so render_frame rebuilds against the new back buffer.
		d3d11_ui::invalidate_back_buffer( );
		if ( !o_resize_buffers ) return E_FAIL;
		return o_resize_buffers( swap_chain, buffer_count, width, height, new_format, swap_chain_flags );
	}

	// A throwaway window + device whose only purpose is to hand out the shared
	// DXGI swap chain vtable (and the real Present address).
	inline bool make_temp_swap_chain( IDXGISwapChain** out_sc, ID3D11Device** out_dev, ID3D11DeviceContext** out_ctx ) {
		*out_sc = nullptr;
		*out_dev = nullptr;
		*out_ctx = nullptr;

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

		D3D_FEATURE_LEVEL fl{};
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			nullptr, 0, D3D11_SDK_VERSION, &sd, out_sc, out_dev, &fl, out_ctx );

		DestroyWindow( tmp_hwnd );
		UnregisterClassA( wc.lpszClassName, wc.hInstance );

		if ( FAILED( hr ) || !*out_sc ) {
			if ( *out_ctx ) { ( *out_ctx )->Release( ); *out_ctx = nullptr; }
			if ( *out_dev ) { ( *out_dev )->Release( ); *out_dev = nullptr; }
			return false;
		}
		return true;
	}

	// OBS keeps a copy of the original Present pointer in one of the writable
	// sections of graphics-hook64.dll. Matching "any pointer that lands inside
	// dxgi.dll" hooks the wrong slot (an import thunk, a cached ResizeBuffers,
	// ...), so the candidate has to be the exact Present address.
	inline uintptr_t find_obs_present_slot( HMODULE gh, void* want_present ) {
		if ( !gh || !want_present ) return 0;

		auto* gh_dos = reinterpret_cast< PIMAGE_DOS_HEADER >( gh );
		if ( !gh_dos || gh_dos->e_magic != IMAGE_DOS_SIGNATURE ) return 0;
		auto* gh_nt = reinterpret_cast< PIMAGE_NT_HEADERS64 >( reinterpret_cast< uintptr_t >( gh ) + gh_dos->e_lfanew );
		if ( !gh_nt || gh_nt->Signature != IMAGE_NT_SIGNATURE ) return 0;

		const uintptr_t want = reinterpret_cast< uintptr_t >( want_present );
		auto* sec = IMAGE_FIRST_SECTION( gh_nt );

		for ( WORD i = 0; i < gh_nt->FileHeader.NumberOfSections; ++i ) {
			const bool writable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
			const bool executable = ( sec[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
			if ( !writable || executable ) continue;

			const uintptr_t sec_start = reinterpret_cast< uintptr_t >( gh ) + sec[ i ].VirtualAddress;
			const DWORD sec_size = sec[ i ].Misc.VirtualSize ? sec[ i ].Misc.VirtualSize : sec[ i ].SizeOfRawData;
			if ( sec_size < sizeof( uintptr_t ) ) continue;

			for ( DWORD off = 0; off + sizeof( uintptr_t ) <= sec_size; off += sizeof( uintptr_t ) ) {
				// Per-word probe rather than a whole-section one: the tail page of a
				// section is not always committed.
				if ( !unity::is_mem_readable( sec_start + off, sizeof( uintptr_t ) ) ) continue;
				if ( *reinterpret_cast< uintptr_t* >( sec_start + off ) == want ) {
					return sec_start + off;
				}
			}
		}
		return 0;
	}

	inline bool hook_swap_chain_vtable( void** vtable ) {
		if ( !vtable || !vtable[ 8 ] ) return false;
		if ( vtable[ 8 ] == reinterpret_cast< void* >( &hk_present ) ) return true;

		DWORD old_prot = 0;
		if ( !VirtualProtect( &vtable[ 8 ], sizeof( void* ) * 6, PAGE_EXECUTE_READWRITE, &old_prot ) )
			return false;

		g_swapchain_vtable = vtable;
		o_present = reinterpret_cast< present_fn >( vtable[ 8 ] );
		InterlockedExchangePointer( &vtable[ 8 ], reinterpret_cast< PVOID >( &hk_present ) );
		if ( vtable[ 13 ] && vtable[ 13 ] != reinterpret_cast< void* >( &hk_resize_buffers ) ) {
			o_resize_buffers = reinterpret_cast< resize_fn >( vtable[ 13 ] );
			InterlockedExchangePointer( &vtable[ 13 ], reinterpret_cast< PVOID >( &hk_resize_buffers ) );
		}
		VirtualProtect( &vtable[ 8 ], sizeof( void* ) * 6, old_prot, &old_prot );
		return true;
	}

	// Validates both candidate bases and picks ours: a readable PE header that is
	// not the host executable.
	inline std::uint64_t resolve_self_base( std::uint64_t arg1, std::uint64_t arg3 ) {
		const std::uint64_t host = reinterpret_cast< std::uint64_t >( GetModuleHandleA( nullptr ) );
		const std::uint64_t candidates[ 2 ] = { arg1, arg3 };
		for ( int i = 0; i < 2; ++i ) {
			const std::uint64_t c = candidates[ i ];
			if ( !c || c == host ) continue;
			if ( ( c & 0xFFF ) != 0 ) continue;
			if ( !unity::is_mem_readable( c, 0x40 ) ) continue;
			const auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( c );
			if ( dos->e_magic != IMAGE_DOS_SIGNATURE ) continue;
			const std::uint64_t nt_addr = c + static_cast< std::uint64_t >( dos->e_lfanew );
			if ( !unity::is_mem_readable( nt_addr, sizeof( IMAGE_NT_HEADERS64 ) ) ) continue;
			const auto* nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( nt_addr );
			if ( nt->Signature != IMAGE_NT_SIGNATURE ) continue;
			return c;
		}
		return 0;
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

		// One throwaway device is enough: it yields both the shared swap chain
		// vtable and the real Present address used to identify the OBS slot.
		IDXGISwapChain* sc = nullptr;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* ctx = nullptr;
		if ( !make_temp_swap_chain( &sc, &dev, &ctx ) ) return false;

		void** vtable = *reinterpret_cast< void*** >( sc );
		bool ok = false;
		if ( vtable ) {
			const uintptr_t slot = find_obs_present_slot( GetModuleHandleA( "graphics-hook64.dll" ), vtable[ 8 ] );
			if ( slot ) {
				if ( *reinterpret_cast< void** >( slot ) != reinterpret_cast< void* >( &hk_present ) ) {
					o_present = reinterpret_cast< present_fn >( *reinterpret_cast< uintptr_t* >( slot ) );
					InterlockedExchangePointer(
						reinterpret_cast< volatile PVOID* >( slot ),
						reinterpret_cast< PVOID >( &hk_present )
					);
				}
				ok = true;
			}
			else {
				ok = hook_swap_chain_vtable( vtable );
			}
		}

		sc->Release( );
		if ( ctx ) ctx->Release( );
		if ( dev ) dev->Release( );
		return ok;
	}

	// Runs on its own thread: creating a D3D11 device (and hooking vtables) from
	// DllMain happens under the loader lock and can deadlock the process.
	inline DWORD WINAPI init_thread( void* ) {
		__try {
			hide_current_thread( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}

		for ( int i = 0; i < 40; ++i ) {
			if ( GetModuleHandleA( "dxgi.dll" ) ) break;
			Sleep( 50 );
		}

		for ( int attempt = 0; attempt < 20; ++attempt ) {
			__try {
				if ( install( ) ) return 0;
			}
			__except ( EXCEPTION_EXECUTE_HANDLER ) {
			}
			Sleep( 250 );
		}
		return 0;
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

// The two callers disagree on the arguments. reflective_loader calls
// dllmain( m_dll_base, DLL_PROCESS_ATTACH, host_base ), but the injector's
// shellcode calls it directly with rcx = host base, rdx = DLL_PROCESS_ATTACH and
// r8 = our image base. Taking arg1 as our base therefore registered the SEH table
// of the host executable a second time, which corrupts unwinding for the whole
// process. Resolve our base from whichever argument actually points at a PE that
// is not the host module.
extern "C" __declspec( dllexport ) BOOL APIENTRY DllMain(
	std::uint64_t module_base,
	std::uint32_t reason,
	std::uint64_t reserved
) {
	if ( reason != DLL_PROCESS_ATTACH ) return TRUE;

	const std::uint64_t self = dx11_hook::resolve_self_base( module_base, reserved );
	__try {
		if ( self ) dx11_hook::register_seh_pdata( self );
		if ( self ) dx11_hook::g_self_base = self;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
	}

	// The injector fills g_obs_slot with the graphics-hook64 slot before calling
	// us, so that path only rewrites a pointer and has to complete before we
	// return. Without a slot install() would have to create a throwaway D3D11
	// device, which cannot happen under the loader lock: that is retried on its
	// own thread instead.
	if ( ::g_obs_slot.slot_addr && ::g_obs_slot.orig_present ) {
		__try {
			dx11_hook::install( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
		return TRUE;
	}

	HANDLE thread = CreateThread( nullptr, 0, &dx11_hook::init_thread, nullptr, 0, nullptr );
	if ( thread ) CloseHandle( thread );
	return TRUE;
}
