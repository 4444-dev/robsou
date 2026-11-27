#include <impl/includes.h>
#include "resource.h"
#include <tlhelp32.h>
#include <dpapi.h>
#include <fstream>
#include <iterator>
#pragma comment(lib, "crypt32.lib")

namespace {
	static std::vector< std::uint8_t > g_cached_drv, g_cached_map, g_cached_dll;
	static volatile bool g_res_cached = false;

	constexpr std::uint32_t k_res_magic = 0x4E585231u;

	bool load_embedded_resource( WORD id, std::vector< std::uint8_t >& out ) {
		HRSRC resource = FindResourceW( nullptr, MAKEINTRESOURCEW( id ), MAKEINTRESOURCEW( 10 ) );
		if ( !resource )
			return false;
		HGLOBAL handle = LoadResource( nullptr, resource );
		if ( !handle )
			return false;
		const DWORD size = SizeofResource( nullptr, resource );
		const auto* data = static_cast< const std::uint8_t* >( LockResource( handle ) );
		if ( size <= 8 || !data )
			return false;

		std::uint32_t magic = 0;
		std::uint32_t seed = 0;
		std::memcpy( &magic, data, 4 );
		std::memcpy( &seed, data + 4, 4 );
		if ( magic != k_res_magic ) {
			out.assign( data, data + size );
			return true;
		}

		const std::size_t payload_size = size - 8;
		out.resize( payload_size );
		std::uint32_t state = seed ^ 0xA5F19C3Du;
		for ( std::size_t i = 0; i < payload_size; ++i ) {
			state = state * 1664525u + 1013904223u;
			const std::uint8_t k = static_cast< std::uint8_t >( ( state >> 16 ) ^ ( ( i * 0x45u ) & 0xFFu ) );
			out[ i ] = data[ 8 + i ] ^ k;
		}
		return true;
	}

	void preload_resources() {
		if ( g_res_cached ) return;
		load_embedded_resource( IDR_ORELIA_DRIVER, g_cached_drv );
		load_embedded_resource( IDR_ORELIA_MAPPER, g_cached_map );
		load_embedded_resource( IDR_ORELIA_DLL, g_cached_dll );
		g_res_cached = true;
	}

	bool write_temp_file( const std::string& path, const std::vector< std::uint8_t >& bytes ) {
		HANDLE h = CreateFileA( path.c_str( ), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
				FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN, nullptr );
		if ( h == INVALID_HANDLE_VALUE )
			return false;

		DWORD written = 0;
		BOOL ok = WriteFile( h, bytes.data( ), static_cast< DWORD >( bytes.size( ) ), &written, nullptr );

		if ( !ok || written != bytes.size( ) ) {
			CloseHandle( h );
			return false;
		}

		CloseHandle( h );
		return true;
	}

	bool is_driver_active( ) {
		auto dev_path = encrypt( L"\\\\.\\{9C4E2A1B-7D3F-48E0-91A2-5B6C8D0E4F12}" );
		HANDLE h = CreateFileW( dev_path.decrypt( ), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE )
			return false;
		CloseHandle( h );
		return true;
	}

	DWORD read_registry_dword( HKEY root, const wchar_t* path, const wchar_t* name ) {
		HKEY key = nullptr;
		if ( RegOpenKeyExW( root, path, 0, KEY_READ | KEY_WOW64_64KEY, &key ) != ERROR_SUCCESS )
			return 0;
		DWORD value = 0;
		DWORD size = sizeof( value );
		const LONG status = RegQueryValueExW( key, name, nullptr, nullptr, reinterpret_cast< LPBYTE >( &value ), &size );
		RegCloseKey( key );
		return status == ERROR_SUCCESS ? value : 0;
	}

	bool write_registry_dword( HKEY root, const wchar_t* path, const wchar_t* name, DWORD value ) {
		HKEY key = nullptr;
		if ( RegCreateKeyExW( root, path, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, nullptr, &key, nullptr ) != ERROR_SUCCESS )
			return false;
		const LONG status = RegSetValueExW( key, name, 0, REG_DWORD, reinterpret_cast< const BYTE* >( &value ), sizeof( value ) );
		RegCloseKey( key );
		return status == ERROR_SUCCESS;
	}

	struct sec_block_info_t {
		bool hvci_reg = false;
		bool hvci_live = false;
		bool vbs = false;
		bool blocklist = false;
		bool any( ) const { return hvci_reg || hvci_live || vbs || blocklist; }
	};

	sec_block_info_t detect_security_blocks( ) {
		sec_block_info_t info{};
		constexpr const wchar_t* ci_path = L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\Scenarios\\HypervisorEnforcedCodeIntegrity";
		constexpr const wchar_t* dg_path = L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard";
		constexpr const wchar_t* bl_path = L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config";

		info.hvci_reg = read_registry_dword( HKEY_LOCAL_MACHINE, ci_path, L"Enabled" ) != 0;
		info.vbs = read_registry_dword( HKEY_LOCAL_MACHINE, dg_path, L"EnableVirtualizationBasedSecurity" ) != 0;
		info.blocklist = read_registry_dword( HKEY_LOCAL_MACHINE, bl_path, L"VulnerableDriverBlocklistEnable" ) != 0;

		static auto fn_nt_qsi = LAZY_FN( L"ntdll.dll", NtQuerySystemInformation );
		if ( fn_nt_qsi ) {
			struct { ULONG Length; ULONG CodeIntegrityOptions; } ci_info{ 8, 0 };
			ULONG ret = 0;
			if ( NT_SUCCESS( fn_nt_qsi( static_cast< SYSTEM_INFORMATION_CLASS >( 103 ), &ci_info, sizeof( ci_info ), &ret ) ) ) {
				if ( ( ci_info.CodeIntegrityOptions & 0x00000400u ) != 0 )
					info.hvci_live = true;
			}
		}
		return info;
	}

	bool apply_security_fix( ) {
		constexpr const wchar_t* ci_path = L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard\\Scenarios\\HypervisorEnforcedCodeIntegrity";
		constexpr const wchar_t* dg_path = L"SYSTEM\\CurrentControlSet\\Control\\DeviceGuard";
		constexpr const wchar_t* bl_path = L"SYSTEM\\CurrentControlSet\\Control\\CI\\Config";

		bool ok1 = write_registry_dword( HKEY_LOCAL_MACHINE, ci_path, L"Enabled", 0 );
		bool ok2 = write_registry_dword( HKEY_LOCAL_MACHINE, dg_path, L"EnableVirtualizationBasedSecurity", 0 );
		bool ok3 = write_registry_dword( HKEY_LOCAL_MACHINE, bl_path, L"VulnerableDriverBlocklistEnable", 0 );
		return ok1 || ok2 || ok3;
	}

	void prompt_and_handle_security_block( const sec_block_info_t& blk, std::string& out_error ) {
		std::string details = "Securite Windows activee detectee :\n";
		if ( blk.hvci_reg || blk.hvci_live ) details += "- Isolation du noyau / Integrite de la memoire (HVCI)\n";
		if ( blk.vbs ) details += "- Securite basee sur la virtualisation (VBS)\n";
		if ( blk.blocklist ) details += "- Liste de blocage des pilotes vulnerables Microsoft\n";
		details += "\nCes protections bloquent le chargement automatique du pilote.\n"
				   "Voulez-vous que Nexus desactive ces protections automatiquement ?\n"
				   "(Un redemarrage du PC sera necessaire)";

		int res = MessageBoxA( nullptr, details.c_str( ), "Nexus - Protection Windows detectee", MB_YESNO | MB_ICONWARNING | MB_TOPMOST );
		if ( res == IDYES ) {
			if ( apply_security_fix( ) ) {
				int reb = MessageBoxA(
					nullptr,
					"Modifications appliquees avec succes !\n\nVoulez-vous redemarrer votre PC maintenant pour finaliser ?",
					"Nexus - Redemarrage requis",
					MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST
				);
				if ( reb == IDYES ) {
					HANDLE tok = nullptr;
					if ( OpenProcessToken( GetCurrentProcess( ), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tok ) ) {
						TOKEN_PRIVILEGES tp{};
						tp.PrivilegeCount = 1;
						if ( LookupPrivilegeValueA( nullptr, "SeShutdownPrivilege", &tp.Privileges[ 0 ].Luid ) ) {
							tp.Privileges[ 0 ].Attributes = SE_PRIVILEGE_ENABLED;
							AdjustTokenPrivileges( tok, FALSE, &tp, sizeof( tp ), nullptr, nullptr );
						}
						CloseHandle( tok );
					}
					InitiateSystemShutdownExA( nullptr, nullptr, 0, TRUE, TRUE, SHTDN_REASON_MAJOR_APPLICATION );
					out_error = "Redemarrage en cours...";
					return;
				}
				out_error = "Fix applique - redemarrez votre PC puis relancez Nexus.";
				return;
			}
			out_error = "Echec du fix auto (desactivez Tamper Protection dans Securite Windows).";
			return;
		}
		out_error = "Desactivez l'Isolation du noyau (HVCI) dans Securite Windows et redemarrez.";
	}

	bool ensure_driver_loaded( std::string& out_error ) {
		if ( is_driver_active( ) ) {
			if ( g_driver->setup( ) )
				return true;
			out_error = "Driver handle failed (err " + std::to_string( GetLastError( ) ) + ").";
			return false;
		}

		const sec_block_info_t blk = detect_security_blocks( );
		if ( blk.hvci_reg || blk.hvci_live || blk.vbs ) {
			prompt_and_handle_security_block( blk, out_error );
			return false;
		}

		char temp_dir[ MAX_PATH ]{};
		if ( !GetTempPathA( MAX_PATH, temp_dir ) ) {
			out_error = "Failed to resolve temp directory.";
			return false;
		}

		const DWORD pid = GetCurrentProcessId( );
		const std::string drv_path = std::string( temp_dir ) + "~nx_" + std::to_string( pid ) + "_d.sys";
		const std::string map_path = std::string( temp_dir ) + "~nx_" + std::to_string( pid ) + "_m.exe";

		if ( !g_res_cached ) preload_resources( );
		if ( g_cached_drv.empty( ) || g_cached_map.empty( ) ) {
			out_error = "Missing embedded driver or mapper resources.";
			return false;
		}

		const bool wrote = write_temp_file( drv_path, g_cached_drv ) && write_temp_file( map_path, g_cached_map );
		if ( !wrote ) {
			out_error = "Failed to write embedded driver files to the temp directory.";
			return false;
		}

		DWORD mapper_exit = 0;
		bool mapper_ran = false;
		std::string mapper_out;

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof( sa );
		sa.bInheritHandle = TRUE;

		HANDLE read_pipe = nullptr;
		HANDLE write_pipe = nullptr;
		CreatePipe( &read_pipe, &write_pipe, &sa, 0 );
		if ( read_pipe )
			SetHandleInformation( read_pipe, HANDLE_FLAG_INHERIT, 0 );

		std::string cmd = "\"" + map_path + "\" \"" + drv_path + "\" --silent";
		STARTUPINFOA si{};
		si.cb = sizeof( si );
		si.dwFlags = STARTF_USESHOWWINDOW | ( write_pipe ? STARTF_USESTDHANDLES : 0 );
		si.wShowWindow = SW_HIDE;
		si.hStdOutput = write_pipe;
		si.hStdError = write_pipe;
		PROCESS_INFORMATION pi{};
		if ( CreateProcessA( nullptr, cmd.data( ), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, temp_dir, &si, &pi ) ) {
			mapper_ran = true;
			if ( write_pipe ) {
				CloseHandle( write_pipe );
				write_pipe = nullptr;
			}
			if ( read_pipe ) {
				char buf[ 512 ];
				DWORD bytes_read = 0;
				while ( ReadFile( read_pipe, buf, sizeof( buf ) - 1, &bytes_read, nullptr ) && bytes_read > 0 ) {
					buf[ bytes_read ] = '\0';
					mapper_out += buf;
				}
			}
			WaitForSingleObject( pi.hProcess, 30000 );
			GetExitCodeProcess( pi.hProcess, &mapper_exit );
			if ( mapper_exit != 0 ) {
				if ( !mapper_out.empty( ) )
					logging::print( encrypt( "Mapper output:\n%s\n" ), mapper_out.c_str( ) );
				logging::print( encrypt( "Mapper exited with code: %lu\n" ), mapper_exit );
			}
			CloseHandle( pi.hThread );
			CloseHandle( pi.hProcess );
		}
		else {
			DWORD err = GetLastError( );
			out_error = "Failed to launch embedded mapper (err " + std::to_string( err ) + ").";
		}
		if ( write_pipe ) CloseHandle( write_pipe );
		if ( read_pipe ) CloseHandle( read_pipe );

		DeleteFileA( map_path.c_str( ) );
		DeleteFileA( drv_path.c_str( ) );

		if ( g_driver->setup( ) )
			return true;

		DWORD setup_err = GetLastError( );
		if ( mapper_ran && mapper_exit != 0 ) {
			if ( blk.blocklist ) {
				prompt_and_handle_security_block( blk, out_error );
				return false;
			}
			out_error = "Mapper failed (exit " + std::to_string( mapper_exit ) + ") - disable HVCI/Blocklist/AV.";
		}
		else
			out_error = "Driver handle failed (err " + std::to_string( setup_err ) + ").";
		return false;
	}
}

	struct ui_state_t {
		HWND hwnd = nullptr;
		HWND edit_key = nullptr;
		HWND btn_inject = nullptr;
		std::atomic<bool> busy{ false };
		std::atomic<int> progress{ 0 };
		std::atomic<int> state_kind{ 0 }; // 0=idle, 1=working, 2=ok, 3=err
		CRITICAL_SECTION cs{};
		char status_text[ 256 ]{ "Ready - Enter your license key" };
		bool game_running = false;
		bool obs_running = false;
		HBRUSH br_bg = nullptr;
		HBRUSH br_card = nullptr;
		HBRUSH br_edit = nullptr;
		HFONT fnt_title = nullptr;
		HFONT fnt_body = nullptr;
		HFONT fnt_bold = nullptr;
		bool hover_inject = false;
		bool hover_close = false;
		bool hover_min = false;
	};
	ui_state_t g_ui{};

	void set_status( int kind, int prog, const char* msg ) {
		EnterCriticalSection( &g_ui.cs );
		g_ui.state_kind = kind;
		g_ui.progress = prog;
		if ( msg ) {
			strncpy_s( g_ui.status_text, msg, sizeof( g_ui.status_text ) - 1 );
		}
		LeaveCriticalSection( &g_ui.cs );
		if ( g_ui.hwnd ) {
			InvalidateRect( g_ui.hwnd, nullptr, FALSE );
		}
	}

	std::string get_saved_key_path( ) {
		char appdata[ MAX_PATH ]{};
		if ( GetEnvironmentVariableA( "APPDATA", appdata, MAX_PATH ) > 0 ) {
			return std::string( appdata ) + "\\nexus_license.txt";
		}
		return "nexus_license.txt";
	}

	std::string load_saved_key( ) {
		const std::string p = get_saved_key_path( );
		std::ifstream file( p, std::ios::binary );
		if ( !file ) return {};
		std::vector<unsigned char> encrypted(
			(std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() );
		DATA_BLOB input{ static_cast<DWORD>( encrypted.size( ) ), encrypted.data( ) };
		DATA_BLOB output{};
		if ( !CryptUnprotectData( &input, nullptr, nullptr, nullptr, nullptr, 0, &output ) )
			return {};
		std::string key( reinterpret_cast<char*>( output.pbData ), output.cbData );
		LocalFree( output.pbData );
		std::fill( encrypted.begin( ), encrypted.end( ), 0 );
		return key;
	}

	void save_key( const std::string& key ) {
		const std::string p = get_saved_key_path( );
		DATA_BLOB input{ static_cast<DWORD>( key.size( ) ), reinterpret_cast<BYTE*>( const_cast<char*>( key.data( ) ) ) };
		DATA_BLOB output{};
		if ( !CryptProtectData( &input, L"Nexus license", nullptr, nullptr, nullptr, 0, &output ) )
			return;
		HANDLE h = CreateFileA( p.c_str( ), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h != INVALID_HANDLE_VALUE ) {
			DWORD written = 0;
			WriteFile( h, output.pbData, output.cbData, &written, nullptr );
			CloseHandle( h );
		}
		LocalFree( output.pbData );
	}

	bool is_running_elevated( ) {
		HANDLE token = nullptr;
		if ( !OpenProcessToken( GetCurrentProcess( ), TOKEN_QUERY, &token ) )
			return false;

		TOKEN_ELEVATION elevation{};
		DWORD size = 0;
		const BOOL ok = GetTokenInformation( token, TokenElevation, &elevation, sizeof( elevation ), &size );
		CloseHandle( token );
		return ok && elevation.TokenIsElevated;
	}

	bool is_process_running( const wchar_t* exe_name ) {
		if ( !exe_name )
			return false;

		const std::size_t target_chars = wcslen( exe_name );
		ULONG buf_size = 0x40000;
		std::vector< std::uint8_t > buffer( buf_size );
		NTSTATUS status = STATUS_INFO_LENGTH_MISMATCH;

		for ( int attempt = 0; attempt < 8; ++attempt ) {
			ULONG returned = 0;
			status = NtQuerySystemInformation(
				static_cast< SYSTEM_INFORMATION_CLASS >( 5 ),
				buffer.data( ),
				buf_size,
				&returned
			);
			if ( status != STATUS_INFO_LENGTH_MISMATCH )
				break;
			buf_size = ( returned > buf_size ? returned : buf_size * 2 ) + 0x10000;
			buffer.resize( buf_size );
		}

		if ( !NT_SUCCESS( status ) )
			return false;

		const auto* cur = reinterpret_cast< const SYSTEM_PROCESS_INFORMATION* >( buffer.data( ) );
		while ( cur ) {
			if ( cur->ImageName.Buffer &&
				 cur->ImageName.Length == target_chars * sizeof( wchar_t ) &&
				 _wcsnicmp( cur->ImageName.Buffer, exe_name, target_chars ) == 0 ) {
				return true;
			}
			if ( !cur->NextEntryOffset )
				break;
			cur = reinterpret_cast< const SYSTEM_PROCESS_INFORMATION* >(
				reinterpret_cast< const std::uint8_t* >( cur ) + cur->NextEntryOffset
			);
		}
		return false;
	}

	bool has_debugger_process_running( ) {
		const std::uint64_t name_hashes[] = {
			sec::hash_w_lower( L"x64dbg.exe" ), sec::hash_w_lower( L"x32dbg.exe" ),
			sec::hash_w_lower( L"windbg.exe" ), sec::hash_w_lower( L"ollydbg.exe" ),
			sec::hash_w_lower( L"ida.exe" ), sec::hash_w_lower( L"ida64.exe" ),
			sec::hash_w_lower( L"cheatengine-i386.exe" ), sec::hash_w_lower( L"cheatengine-x86_64.exe" ),
			sec::hash_w_lower( L"dnspy.exe" ), sec::hash_w_lower( L"dnspy-x86.exe" ),
			sec::hash_w_lower( L"ghidra.exe" ), sec::hash_w_lower( L"r2.exe" ),
			sec::hash_w_lower( L"megadumper.exe" ), sec::hash_w_lower( L"scylla_x64.exe" ),
			sec::hash_w_lower( L"scylla_x86.exe" )
		};

		static auto fn_nt_qsi = LAZY_FN( L"ntdll.dll", NtQuerySystemInformation );
		if ( !fn_nt_qsi ) return false;

		ULONG buf_size = 0x40000;
		std::vector< std::uint8_t > buffer( buf_size );
		NTSTATUS status = STATUS_INFO_LENGTH_MISMATCH;
		for ( int attempt = 0; attempt < 8; ++attempt ) {
			ULONG returned = 0;
			status = fn_nt_qsi(
				static_cast< SYSTEM_INFORMATION_CLASS >( 5 ),
				buffer.data( ), buf_size, &returned );
			if ( status != STATUS_INFO_LENGTH_MISMATCH )
				break;
			buf_size = ( returned > buf_size ? returned : buf_size * 2 ) + 0x10000;
			buffer.resize( buf_size );
		}
		if ( !NT_SUCCESS( status ) )
			return false;

		const auto* cur = reinterpret_cast< const SYSTEM_PROCESS_INFORMATION* >( buffer.data( ) );
		while ( cur ) {
			if ( cur->ImageName.Buffer && cur->ImageName.Length > 0 ) {
				const std::uint64_t h = sec::rt_hash_w_lower( cur->ImageName.Buffer, cur->ImageName.Length );
				for ( std::uint64_t target_h : name_hashes ) {
					if ( h == target_h )
						return true;
				}
			}
			if ( !cur->NextEntryOffset )
				break;
			cur = reinterpret_cast< const SYSTEM_PROCESS_INFORMATION* >(
				reinterpret_cast< const std::uint8_t* >( cur ) + cur->NextEntryOffset );
		}
		return false;
	}

	bool is_being_analyzed( );

	bool is_vm_environment( ) {
		int cpu_info[ 4 ]{};
		__cpuid( cpu_info, 1 );
		if ( cpu_info[ 2 ] & ( 1 << 31 ) ) {
			int hv_info[ 4 ]{};
			__cpuid( hv_info, 0x40000000 );
			char vendor[ 13 ]{};
			std::memcpy( vendor + 0, &hv_info[ 1 ], 4 );
			std::memcpy( vendor + 4, &hv_info[ 2 ], 4 );
			std::memcpy( vendor + 8, &hv_info[ 3 ], 4 );
			const std::uint64_t vh = sec::rt_hash_a( vendor );
			if ( vh == sec::hash_a( "VMwareVMware" ) ||
				 vh == sec::hash_a( "VBoxVBoxVBox" ) ||
				 vh == sec::hash_a( "KVMKVMKVM" ) ||
				 vh == sec::hash_a( "TCGTCGTCGTCG" ) ||
				 vh == sec::hash_a( "XenVMMXenVMM" ) ) {
				return true;
			}
		}

		auto s_p1 = skCrypt( L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation" );
		auto s_p2 = skCrypt( L"HARDWARE\\DESCRIPTION\\System" );
		const wchar_t* paths[] = { s_p1.decrypt( ), s_p2.decrypt( ) };
		auto s_k1 = skCrypt( L"SystemProductName" );
		auto s_k2 = skCrypt( L"SystemManufacturer" );
		auto s_k3 = skCrypt( L"SystemBiosVersion" );
		auto s_k4 = skCrypt( L"VideoBiosVersion" );
		const wchar_t* keys[] = { s_k1.decrypt( ), s_k2.decrypt( ), s_k3.decrypt( ), s_k4.decrypt( ) };
		auto s_v1 = skCrypt( L"VMWARE" );
		auto s_v2 = skCrypt( L"VIRTUALBOX" );
		auto s_v3 = skCrypt( L"KVM" );
		auto s_v4 = skCrypt( L"QEMU" );
		for ( const wchar_t* path : paths ) {
			HKEY key = nullptr;
			if ( RegOpenKeyExW( HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key ) == ERROR_SUCCESS ) {
				wchar_t value[ 128 ]{};
				DWORD size = sizeof( value );
				for ( const wchar_t* name : keys ) {
					if ( RegQueryValueExW( key, name, nullptr, nullptr, reinterpret_cast<BYTE*>( value ), &size ) == ERROR_SUCCESS ) {
						_wcsupr( value );
						if ( wcsstr( value, s_v1.decrypt( ) ) || wcsstr( value, s_v2.decrypt( ) ) ||
							 wcsstr( value, s_v3.decrypt( ) ) || wcsstr( value, s_v4.decrypt( ) ) ) {
							RegCloseKey( key );
							return true;
						}
					}
					size = sizeof( value );
					value[ 0 ] = L'\0';
				}
				RegCloseKey( key );
			}
		}
		return false;
	}

	DWORD WINAPI inject_worker( LPVOID param ) {
		sec::hide_thread( );
		if ( !sec::opaque_true( ) || is_being_analyzed( ) ) {
			ExitProcess( 0 );
		}
		auto* key_ptr = reinterpret_cast< std::string* >( param );
		std::string license_key = key_ptr ? *key_ptr : std::string{};
		delete key_ptr;

		while ( !license_key.empty( ) && ( license_key.front( ) == ' ' || license_key.front( ) == '\t' || license_key.front( ) == '\r' || license_key.front( ) == '\n' ) )
			license_key.erase( license_key.begin( ) );
		while ( !license_key.empty( ) && ( license_key.back( ) == ' ' || license_key.back( ) == '\t' || license_key.back( ) == '\r' || license_key.back( ) == '\n' ) )
			license_key.pop_back( );

		if ( license_key.empty( ) ) {
			set_status( 3, 0, "Please enter a license key." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 15, "Connecting to server..." );
		std::string name = skCrypt( "RobRun" ).decrypt( );
		std::string ownerid = skCrypt( "ZOhORJsXc1" ).decrypt( );
		std::string version = skCrypt( "1.0" ).decrypt( );
		std::string url = skCrypt( "https://keyauth.win/api/1.3/" ).decrypt( );
		std::string path = skCrypt( "" ).decrypt( );

		keyauth::c_api auth( name, ownerid, version, url, path );
		if ( !auth.init( ) ) {
			const char* err = auth.response.message.empty( ) ? "Failed to connect to auth server." : auth.response.message.c_str( );
			set_status( 3, 0, err );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 30, "Verifying license key..." );
		if ( !auth.license( license_key ) ) {
			const char* err = auth.response.message.empty( ) ? "Invalid or expired license key." : auth.response.message.c_str( );
			set_status( 3, 0, err );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}
		save_key( license_key );

		if ( !is_running_elevated( ) ) {
			set_status( 3, 0, "Administrator rights required - restart Nexus as Administrator." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 45, "Loading driver..." );
		std::string drv_error;
		if ( !ensure_driver_loaded( drv_error ) ) {
			set_status( 3, 0, drv_error.empty( ) ? "Driver load failed." : drv_error.c_str( ) );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 62, "Waiting for ROB RUN (go to lobby)..." );
		while ( !g_driver->attach( target_process ) ) {
			char msg[ 128 ]{};
			if ( !g_driver->m_eprocess )
				sprintf_s( msg, "Attach retry: eproc=0 err=0x%X io=0x%X info=%llu pid=%u",
					static_cast< unsigned >( g_driver->m_last_error ),
					static_cast< unsigned >( g_driver->m_last_io_result ),
					static_cast< unsigned long long >( g_driver->m_last_io_info ),
					g_driver->m_process_id );
			else if ( !g_driver->m_base_address )
				sprintf_s( msg, "Attach retry: base=0 (err 0x%X)", static_cast< unsigned >( g_driver->m_last_error ) );
			else if ( !g_driver->m_dtb )
				sprintf_s( msg, "Attach retry: dtb=0 (err 0x%X)", static_cast< unsigned >( g_driver->m_last_error ) );
			else
				sprintf_s( msg, "Waiting for ROB RUN (go to lobby)..." );
			set_status( 1, 62, msg );
			Sleep( 500 );
		}

		if ( is_process_running( L"obs64.exe" ) ) {
			set_status( 1, 70, "Checking OBS Game Capture hook..." );
			for ( int wait_i = 0; wait_i < 6; ++wait_i ) {
				if ( g_driver->get_process_module( encrypt( L"graphics-hook64.dll" ) ) )
					break;
				Sleep( 500 );
			}
		}
		else {
			set_status( 1, 70, "Using direct DXGI VTable hook..." );
		}

		set_status( 1, 82, "Mapping module into memory..." );
		auto injector = std::make_shared< injector::c_injector >( false );
		std::vector< std::uint8_t > payload;
		if ( !load_embedded_resource( IDR_ORELIA_DLL, payload ) ) {
			injector->cleanup( );
			g_driver->unload( );
			set_status( 3, 0, "Embedded module resource is missing." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}
		bool mapped = injector->map_memory( payload.data( ), payload.size( ) );
		std::fill( payload.begin( ), payload.end( ), 0 );

		if ( !mapped ) {
			std::string err_msg = injector->m_map_error.empty( ) ? "Failed to map module into memory." : ( "Map fail: " + injector->m_map_error );
			injector->cleanup( );
			g_driver->unload( );
			set_status( 3, 0, err_msg.c_str( ) );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 92, "Initializing in-game hooks..." );
		if ( !injector->hook( ) ) {
			injector->cleanup( );
			g_driver->unload( );
			set_status( 3, 0, "Failed to initialize in-game." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		injector->wipe_headers( );
		g_driver->unload( );
		set_status( 2, 100, "Injected! Press INS or F8 in-game." );
		Sleep( 2200 );
		PostMessageA( g_ui.hwnd, WM_CLOSE, 0, 0 );
		return 0;
	}

	constexpr RECT k_rc_close = { 442, 8, 470, 34 };
	constexpr RECT k_rc_min   = { 412, 8, 440, 34 };
	constexpr RECT k_rc_btn   = { 28, 246, 452, 290 };

	bool pt_in( const RECT& r, int x, int y ) {
		return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
	}

	LRESULT CALLBACK wnd_proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {
		switch ( msg ) {
		case WM_CREATE: {
			g_ui.br_bg   = CreateSolidBrush( RGB( 11, 13, 20 ) );
			g_ui.br_card = CreateSolidBrush( RGB( 16, 19, 30 ) );
			g_ui.br_edit = CreateSolidBrush( RGB( 20, 24, 38 ) );
			g_ui.fnt_title = CreateFontA( 19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );
			g_ui.fnt_bold  = CreateFontA( 15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );
			g_ui.fnt_body  = CreateFontA( 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );

			g_ui.edit_key = CreateWindowExA( 0, "EDIT", "",
				WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
				40, 114, 400, 22, hwnd, reinterpret_cast< HMENU >( 101 ), nullptr, nullptr );
			SendMessageA( g_ui.edit_key, WM_SETFONT, reinterpret_cast< WPARAM >( g_ui.fnt_body ), TRUE );
			std::string saved = load_saved_key( );
			if ( !saved.empty( ) ) {
				SetWindowTextA( g_ui.edit_key, saved.c_str( ) );
			}
			g_ui.game_running = is_process_running( target_process );
			g_ui.obs_running = is_process_running( L"obs64.exe" );
			SetTimer( hwnd, 1, 800, nullptr );
			return 0;
		}
		case WM_TIMER: {
			if ( wp == 1 ) {
				if ( is_being_analyzed( ) || has_debugger_process_running( ) || is_vm_environment( ) ) {
					PostMessageA( hwnd, WM_CLOSE, 0, 0 );
					return 0;
				}
				bool gr = is_process_running( target_process );
				bool orun = is_process_running( L"obs64.exe" );
				if ( gr != g_ui.game_running || orun != g_ui.obs_running ) {
					g_ui.game_running = gr;
					g_ui.obs_running = orun;
					InvalidateRect( hwnd, nullptr, FALSE );
				}
			}
			return 0;
		}
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC: {
			HDC hdc = reinterpret_cast< HDC >( wp );
			SetTextColor( hdc, RGB( 235, 238, 248 ) );
			SetBkColor( hdc, RGB( 20, 24, 38 ) );
			return reinterpret_cast< LRESULT >( g_ui.br_edit );
		}
		case WM_NCHITTEST: {
			POINT pt{ static_cast< short >( LOWORD( lp ) ), static_cast< short >( HIWORD( lp ) ) };
			ScreenToClient( hwnd, &pt );
			if ( pt.y <= 40 && !pt_in( k_rc_close, pt.x, pt.y ) && !pt_in( k_rc_min, pt.x, pt.y ) ) {
				return HTCAPTION;
			}
			return HTCLIENT;
		}
		case WM_MOUSEMOVE: {
			int x = static_cast< short >( LOWORD( lp ) );
			int y = static_cast< short >( HIWORD( lp ) );
			bool hc = pt_in( k_rc_close, x, y );
			bool hm = pt_in( k_rc_min, x, y );
			bool hb = pt_in( k_rc_btn, x, y );
			if ( hc != g_ui.hover_close || hm != g_ui.hover_min || hb != g_ui.hover_inject ) {
				g_ui.hover_close = hc;
				g_ui.hover_min = hm;
				g_ui.hover_inject = hb;
				InvalidateRect( hwnd, nullptr, FALSE );
			}
			TRACKMOUSEEVENT tme{ sizeof( tme ), TME_LEAVE, hwnd, 0 };
			TrackMouseEvent( &tme );
			return 0;
		}
		case WM_MOUSELEAVE: {
			g_ui.hover_close = g_ui.hover_min = g_ui.hover_inject = false;
			InvalidateRect( hwnd, nullptr, FALSE );
			return 0;
		}
		case WM_LBUTTONUP: {
			int x = static_cast< short >( LOWORD( lp ) );
			int y = static_cast< short >( HIWORD( lp ) );
			if ( pt_in( k_rc_close, x, y ) ) {
				PostMessageA( hwnd, WM_CLOSE, 0, 0 );
				return 0;
			}
			if ( pt_in( k_rc_min, x, y ) ) {
				ShowWindow( hwnd, SW_MINIMIZE );
				return 0;
			}
			if ( pt_in( k_rc_btn, x, y ) && !g_ui.busy ) {
				char buf[ 256 ]{};
				GetWindowTextA( g_ui.edit_key, buf, sizeof( buf ) );
				g_ui.busy = true;
				EnableWindow( g_ui.edit_key, FALSE );
				set_status( 1, 5, "Starting..." );
				auto* key_arg = new std::string( buf );
				HANDLE th = CreateThread( nullptr, 0, inject_worker, key_arg, 0, nullptr );
				if ( !th ) {
					delete key_arg;
					g_ui.busy = false;
					EnableWindow( g_ui.edit_key, TRUE );
					set_status( 3, 0, "Failed to start worker thread." );
				}
				else {
					CloseHandle( th );
				}
				return 0;
			}
			return 0;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps{};
			HDC hdc = BeginPaint( hwnd, &ps );
			RECT rc{};
			GetClientRect( hwnd, &rc );

			HDC mem = CreateCompatibleDC( hdc );
			HBITMAP bmp = CreateCompatibleBitmap( hdc, rc.right, rc.bottom );
			HGDIOBJ old_bmp = SelectObject( mem, bmp );

			FillRect( mem, &rc, g_ui.br_bg );

			// Header bar + top accent strip
			RECT hdr{ 0, 0, rc.right, 44 };
			FillRect( mem, &hdr, g_ui.br_card );
			RECT top_acc{ 0, 0, rc.right, 2 };
			HBRUSH br_acc = CreateSolidBrush( RGB( 99, 102, 241 ) );
			FillRect( mem, &top_acc, br_acc );
			DeleteObject( br_acc );

			// Outer border
			HPEN pen_border = CreatePen( PS_SOLID, 1, RGB( 34, 39, 60 ) );
			HGDIOBJ old_pen = SelectObject( mem, pen_border );
			HGDIOBJ old_br = SelectObject( mem, GetStockObject( NULL_BRUSH ) );
			Rectangle( mem, 0, 0, rc.right, rc.bottom );
			MoveToEx( mem, 0, 44, nullptr );
			LineTo( mem, rc.right, 44 );

			SetBkMode( mem, TRANSPARENT );
			SelectObject( mem, g_ui.fnt_title );
			SetTextColor( mem, RGB( 245, 247, 255 ) );
			TextOutA( mem, 24, 13, "O R E L I A", 11 );

			RECT badge{ 138, 13, 214, 31 };
			HBRUSH br_badge = CreateSolidBrush( RGB( 30, 34, 64 ) );
			FillRect( mem, &badge, br_badge );
			DeleteObject( br_badge );
			SelectObject( mem, g_ui.fnt_body );
			SetTextColor( mem, RGB( 56, 210, 238 ) );
			DrawTextA( mem, "ROB RUN", 7, &badge, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			// Window buttons
			if ( g_ui.hover_min ) {
				HBRUSH b = CreateSolidBrush( RGB( 32, 36, 56 ) );
				FillRect( mem, &k_rc_min, b );
				DeleteObject( b );
			}
			SetTextColor( mem, RGB( 175, 182, 210 ) );
			DrawTextA( mem, "_", 1, const_cast< RECT* >( &k_rc_min ), DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			if ( g_ui.hover_close ) {
				HBRUSH b = CreateSolidBrush( RGB( 195, 48, 58 ) );
				FillRect( mem, &k_rc_close, b );
				DeleteObject( b );
				SetTextColor( mem, RGB( 255, 255, 255 ) );
			}
			else {
				SetTextColor( mem, RGB( 175, 182, 210 ) );
			}
			DrawTextA( mem, "X", 1, const_cast< RECT* >( &k_rc_close ), DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			// Pre-flight status pills (Game & OBS)
			auto draw_pill = [&]( int x, int y, int w, const char* txt, bool ok ) {
				RECT pr{ x, y, x + w, y + 22 };
				HBRUSH bp = CreateSolidBrush( RGB( 18, 22, 34 ) );
				FillRect( mem, &pr, bp );
				DeleteObject( bp );
				HPEN pp = CreatePen( PS_SOLID, 1, ok ? RGB( 42, 130, 96 ) : RGB( 130, 48, 56 ) );
				HGDIOBJ op = SelectObject( mem, pp );
				Rectangle( mem, pr.left, pr.top, pr.right, pr.bottom );
				SelectObject( mem, op );
				DeleteObject( pp );
				HBRUSH bd = CreateSolidBrush( ok ? RGB( 52, 211, 153 ) : RGB( 248, 113, 113 ) );
				RECT dot{ x + 8, y + 8, x + 14, y + 14 };
				FillRect( mem, &dot, bd );
				DeleteObject( bd );
				SetTextColor( mem, ok ? RGB( 167, 243, 208 ) : RGB( 254, 202, 202 ) );
				RECT tr{ x + 18, y, x + w - 4, y + 22 };
				DrawTextA( mem, txt, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE );
			};
			draw_pill( 266, 62, 90, g_ui.game_running ? "GAME: OK" : "GAME: OFF", g_ui.game_running );
			draw_pill( 364, 62, 88, g_ui.obs_running ? "OBS: OK" : "OBS: OFF", g_ui.obs_running );

			// License label & input frame
			SelectObject( mem, g_ui.fnt_bold );
			SetTextColor( mem, RGB( 148, 158, 188 ) );
			TextOutA( mem, 28, 66, "LICENSE KEY", 11 );

			RECT edit_box{ 28, 102, 452, 146 };
			FillRect( mem, &edit_box, g_ui.br_edit );
			Rectangle( mem, edit_box.left, edit_box.top, edit_box.right, edit_box.bottom );

			// Status card + progress bar
			RECT stat_box{ 28, 160, 452, 228 };
			FillRect( mem, &stat_box, g_ui.br_card );
			Rectangle( mem, stat_box.left, stat_box.top, stat_box.right, stat_box.bottom );

			char st_buf[ 256 ]{};
			int kind = 0, prog = 0;
			EnterCriticalSection( &g_ui.cs );
			strncpy_s( st_buf, g_ui.status_text, sizeof( st_buf ) - 1 );
			kind = g_ui.state_kind;
			prog = g_ui.progress;
			LeaveCriticalSection( &g_ui.cs );

			COLORREF st_col = RGB( 205, 212, 232 );
			if ( kind == 1 ) st_col = RGB( 148, 168, 255 );
			else if ( kind == 2 ) st_col = RGB( 52, 211, 153 );
			else if ( kind == 3 ) st_col = RGB( 248, 113, 113 );

			SelectObject( mem, g_ui.fnt_body );
			SetTextColor( mem, st_col );
			RECT st_txt{ 42, 168, 396, 194 };
			DrawTextA( mem, st_buf, -1, &st_txt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );

			char pct_buf[ 16 ]{};
			snprintf( pct_buf, sizeof( pct_buf ), "%d%%", prog );
			SetTextColor( mem, RGB( 130, 140, 170 ) );
			RECT pct_txt{ 398, 168, 438, 194 };
			DrawTextA( mem, pct_buf, -1, &pct_txt, DT_RIGHT | DT_VCENTER | DT_SINGLELINE );

			RECT pbar_bg{ 42, 204, 438, 212 };
			HBRUSH br_pbg = CreateSolidBrush( RGB( 28, 32, 48 ) );
			FillRect( mem, &pbar_bg, br_pbg );
			DeleteObject( br_pbg );
			if ( prog > 0 ) {
				int pw = ( ( pbar_bg.right - pbar_bg.left ) * prog ) / 100;
				RECT pbar_fg{ pbar_bg.left, pbar_bg.top, pbar_bg.left + pw, pbar_bg.bottom };
				HBRUSH br_pfg = CreateSolidBrush( kind == 2 ? RGB( 52, 211, 153 ) : RGB( 99, 102, 241 ) );
				FillRect( mem, &pbar_fg, br_pfg );
				DeleteObject( br_pfg );
			}

			// Inject button
			COLORREF btn_rgb = g_ui.busy
				? RGB( 45, 48, 72 )
				: ( g_ui.hover_inject ? RGB( 116, 120, 255 ) : RGB( 99, 102, 241 ) );
			HBRUSH br_btn = CreateSolidBrush( btn_rgb );
			FillRect( mem, &k_rc_btn, br_btn );
			DeleteObject( br_btn );

			SelectObject( mem, g_ui.fnt_bold );
			SetTextColor( mem, g_ui.busy ? RGB( 155, 162, 190 ) : RGB( 255, 255, 255 ) );
			const char* btxt = g_ui.busy ? "INJECTING..." : "LOGIN & INJECT";
			DrawTextA( mem, btxt, -1, const_cast< RECT* >( &k_rc_btn ), DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			BitBlt( hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY );

			SelectObject( mem, old_pen );
			SelectObject( mem, old_br );
			DeleteObject( pen_border );
			SelectObject( mem, old_bmp );
			DeleteObject( bmp );
			DeleteDC( mem );
			EndPaint( hwnd, &ps );
			return 0;
		}
		case WM_DESTROY:
			KillTimer( hwnd, 1 );
			if ( g_ui.br_bg ) DeleteObject( g_ui.br_bg );
			if ( g_ui.br_card ) DeleteObject( g_ui.br_card );
			if ( g_ui.br_edit ) DeleteObject( g_ui.br_edit );
			if ( g_ui.fnt_title ) DeleteObject( g_ui.fnt_title );
			if ( g_ui.fnt_bold ) DeleteObject( g_ui.fnt_bold );
			if ( g_ui.fnt_body ) DeleteObject( g_ui.fnt_body );
			PostQuitMessage( 0 );
			return 0;
		}
		return DefWindowProcA( hwnd, msg, wp, lp );
	}

bool is_being_analyzed( ) {
		if ( !sec::opaque_true( ) ) return true;

		static auto fn_is_dbg = LAZY_FN( L"kernel32.dll", IsDebuggerPresent );
		if ( fn_is_dbg && fn_is_dbg( ) ) return true;

		static auto fn_rem_dbg = LAZY_FN( L"kernel32.dll", CheckRemoteDebuggerPresent );
		BOOL remote_debugger = FALSE;
		if ( fn_rem_dbg && fn_rem_dbg( GetCurrentProcess( ), &remote_debugger ) && remote_debugger )
			return true;

		static auto fn_nt_qip = LAZY_FN( L"ntdll.dll", NtQueryInformationProcess );
		if ( fn_nt_qip ) {
			ULONG_PTR debug_port = 0;
			ULONG returned = 0;
			if ( NT_SUCCESS( fn_nt_qip( GetCurrentProcess( ), ProcessDebugPort,
				&debug_port, sizeof( debug_port ), &returned ) ) && debug_port != 0 )
				return true;

			HANDLE debug_object = nullptr;
			if ( NT_SUCCESS( fn_nt_qip( GetCurrentProcess( ), static_cast< PROCESSINFOCLASS >( 30 ),
				&debug_object, sizeof( debug_object ), &returned ) ) && debug_object ) {
				CloseHandle( debug_object );
				return true;
			}
		}

#ifdef _M_X64
		const uintptr_t peb = __readgsqword( 0x60 );
		if ( peb ) {
			if ( *reinterpret_cast< const unsigned char* >( peb + 2 ) ) return true;
			const std::uint32_t nt_global_flag = *reinterpret_cast< const std::uint32_t* >( peb + 0xBC );
			if ( ( nt_global_flag & 0x70u ) != 0 ) return true;
		}
#endif

		static auto fn_get_ctx = LAZY_FN( L"kernel32.dll", GetThreadContext );
		CONTEXT context{};
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if ( fn_get_ctx && fn_get_ctx( GetCurrentThread( ), &context ) ) {
			if ( context.Dr0 || context.Dr1 || context.Dr2 || context.Dr3 )
				return true;
		}

		return false;
	}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int ) {
	sec::hide_thread( );
	if ( !sec::opaque_true( ) || is_being_analyzed( ) || has_debugger_process_running( ) || is_vm_environment( ) ) {
		return 0;
	}
	preload_resources( );
	sec::scrub_pe_sections( );
	SetUnhandledExceptionFilter( exception::exception_filter );
	InitializeCriticalSection( &g_ui.cs );

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof( wc );
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursorA( nullptr, reinterpret_cast< LPCSTR >( 32512 ) );
	wc.lpszClassName = "NexusLoaderWnd";
	RegisterClassExA( &wc );

	const int w = 480, h = 316;
	const int sx = ( GetSystemMetrics( SM_CXSCREEN ) - w ) / 2;
	const int sy = ( GetSystemMetrics( SM_CYSCREEN ) - h ) / 2;
	g_ui.hwnd = CreateWindowExA( WS_EX_APPWINDOW, wc.lpszClassName, "Nexus Internal",
		WS_POPUP | WS_VISIBLE, sx, sy, w, h, nullptr, nullptr, hInst, nullptr );

	MSG msg{};
	while ( GetMessageA( &msg, nullptr, 0, 0 ) > 0 ) {
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
	return 0;
}
