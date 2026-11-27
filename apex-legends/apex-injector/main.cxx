#include <impl/includes.h>
#include <workspace/embedded_assets.hxx>
#include <atomic>

namespace {
	bool download_url_to_bytes( const wchar_t* url, std::vector< std::uint8_t >& out ) {
		out.clear( );

		wchar_t host[ 256 ]{};
		wchar_t path[ 1024 ]{};
		URL_COMPONENTSW uc{};
		uc.dwStructSize = sizeof( uc );
		uc.lpszHostName = host;
		uc.dwHostNameLength = static_cast< DWORD >( sizeof( host ) / sizeof( host[ 0 ] ) );
		uc.lpszUrlPath = path;
		uc.dwUrlPathLength = static_cast< DWORD >( sizeof( path ) / sizeof( path[ 0 ] ) );

		if ( !WinHttpCrackUrl( url, 0, 0, &uc ) )
			return false;

		HINTERNET session = WinHttpOpen( L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
		if ( !session )
			return false;

		HINTERNET connect = WinHttpConnect( session, host, uc.nPort, 0 );
		if ( !connect ) {
			WinHttpCloseHandle( session );
			return false;
		}

		DWORD flags = ( uc.nScheme == INTERNET_SCHEME_HTTPS ) ? WINHTTP_FLAG_SECURE : 0;
		HINTERNET request = WinHttpOpenRequest( connect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags );
		if ( !request ) {
			WinHttpCloseHandle( connect );
			WinHttpCloseHandle( session );
			return false;
		}

		DWORD redirect_policy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
		WinHttpSetOption( request, WINHTTP_OPTION_REDIRECT_POLICY, &redirect_policy, sizeof( redirect_policy ) );

		bool ok = false;
		if ( WinHttpSendRequest( request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 ) &&
			 WinHttpReceiveResponse( request, nullptr ) ) {
			DWORD status = 0;
			DWORD status_size = sizeof( status );
			if ( WinHttpQueryHeaders( request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size, WINHTTP_NO_HEADER_INDEX ) &&
				 status == 200 ) {
				DWORD avail = 0;
				while ( WinHttpQueryDataAvailable( request, &avail ) && avail > 0 ) {
					const size_t offset = out.size( );
					out.resize( offset + avail );
					DWORD read = 0;
					if ( !WinHttpReadData( request, out.data( ) + offset, avail, &read ) ) {
						out.clear( );
						break;
					}
					out.resize( offset + read );
				}
				ok = !out.empty( );
			}
		}

		WinHttpCloseHandle( request );
		WinHttpCloseHandle( connect );
		WinHttpCloseHandle( session );
		return ok;
	}

	bool write_temp_file( const std::string& path, const std::vector< std::uint8_t >& bytes ) {
		HANDLE h = CreateFileA( path.c_str( ), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr );
		if ( h == INVALID_HANDLE_VALUE )
			return false;

		DWORD written = 0;
		BOOL ok = WriteFile( h, bytes.data( ), static_cast< DWORD >( bytes.size( ) ), &written, nullptr );
		CloseHandle( h );
		return ok && written == bytes.size( );
	}

	bool is_driver_active( ) {
		auto dev_path = encrypt( L"\\\\.\\{1A2B3C4D-5E6F-7890-ABCD-EF1234567890}" );
		HANDLE h = CreateFileW( dev_path.decrypt( ), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE )
			return false;
		CloseHandle( h );
		return true;
	}

	bool ensure_driver_loaded( ) {
		if ( is_driver_active( ) )
			return g_driver->setup( );

		char temp_dir[ MAX_PATH ]{};
		if ( !GetTempPathA( MAX_PATH, temp_dir ) )
			return false;

		DWORD pid = GetCurrentProcessId( );
		auto file_exists = [ ] ( const std::string& p ) -> bool {
			DWORD attr = GetFileAttributesA( p.c_str( ) );
			return attr != INVALID_FILE_ATTRIBUTES && !( attr & FILE_ATTRIBUTE_DIRECTORY );
		};

		char exe_dir[ MAX_PATH ]{};
		GetModuleFileNameA( nullptr, exe_dir, MAX_PATH );
		if ( char* slash = strrchr( exe_dir, '\\' ) )
			*( slash + 1 ) = '\0';

		std::string local_drv;
		std::string local_map;
		const std::string candidates[ ] = {
			std::string( exe_dir ),
			std::string( exe_dir ) + "..\\",
			".\\"
		};
		for ( const auto& dir : candidates ) {
			std::string d = dir + "orelia-ioctl.sys";
			std::string m = dir + "orelia-mapper.exe";
			if ( file_exists( d ) && file_exists( m ) ) {
				local_drv = d;
				local_map = m;
				break;
			}
		}

		bool using_local = !local_drv.empty( ) && !local_map.empty( );
		std::string drv_path = using_local ? local_drv : ( std::string( temp_dir ) + "~nx_" + std::to_string( pid ) + "_d.sys" );
		std::string map_path = using_local ? local_map : ( std::string( temp_dir ) + "~nx_" + std::to_string( pid ) + "_m.exe" );

		bool wrote = using_local;
		if ( !using_local ) {
			std::vector< std::uint8_t > drv_bytes;
			std::vector< std::uint8_t > map_bytes;
			if ( !download_url_to_bytes( L"https://github.com/4444-dev/robrun-int/raw/refs/heads/main/orelia-ioctl.sys", drv_bytes ) ||
				 !download_url_to_bytes( L"https://github.com/4444-dev/robrun-int/raw/refs/heads/main/orelia-mapper.exe", map_bytes ) ) {
				return false;
			}

			wrote = write_temp_file( drv_path, drv_bytes ) && write_temp_file( map_path, map_bytes );
			std::fill( drv_bytes.begin( ), drv_bytes.end( ), 0 );
			std::fill( map_bytes.begin( ), map_bytes.end( ), 0 );
		}

		if ( wrote ) {
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
				if ( write_pipe ) {
					CloseHandle( write_pipe );
					write_pipe = nullptr;
				}
				if ( read_pipe ) {
					char buf[ 512 ];
					DWORD bytes_read = 0;
					while ( ReadFile( read_pipe, buf, sizeof( buf ) - 1, &bytes_read, nullptr ) && bytes_read > 0 ) {
					}
				}
				WaitForSingleObject( pi.hProcess, 30000 );
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
			}
			if ( write_pipe ) CloseHandle( write_pipe );
			if ( read_pipe ) CloseHandle( read_pipe );
		}

		if ( !using_local ) {
			DeleteFileA( map_path.c_str( ) );
			DeleteFileA( drv_path.c_str( ) );
		}

		return g_driver->setup( );
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

	const wchar_t* find_apex_process( ) {
		if ( is_process_running( L"r5apex_dx12.exe" ) ) return L"r5apex_dx12.exe";
		if ( is_process_running( L"r5apex.exe" ) ) return L"r5apex.exe";
		return nullptr;
	}

	struct ui_state_t {
		HWND hwnd = nullptr;
		HWND edit_key = nullptr;
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
			return std::string( appdata ) + "\\nexus_apex_license.txt";
		}
		return "nexus_apex_license.txt";
	}

	std::string load_saved_key( ) {
		std::string p = get_saved_key_path( );
		HANDLE h = CreateFileA( p.c_str( ), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE ) return {};
		char buf[ 256 ]{};
		DWORD rd = 0;
		ReadFile( h, buf, sizeof( buf ) - 1, &rd, nullptr );
		CloseHandle( h );
		std::string s( buf, rd );
		while ( !s.empty( ) && ( s.back( ) == '\r' || s.back( ) == '\n' || s.back( ) == ' ' ) ) s.pop_back( );
		return s;
	}

	void save_key( const std::string& key ) {
		std::string p = get_saved_key_path( );
		HANDLE h = CreateFileA( p.c_str( ), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE ) return;
		DWORD wr = 0;
		WriteFile( h, key.data( ), static_cast< DWORD >( key.size( ) ), &wr, nullptr );
		CloseHandle( h );
	}

	DWORD WINAPI inject_worker( LPVOID param ) {
		auto* key_ptr = reinterpret_cast< std::string* >( param );
		std::string license_key = key_ptr ? *key_ptr : std::string{};
		delete key_ptr;

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
			set_status( 3, 0, "Failed to connect to auth server." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 30, "Verifying license key..." );
		if ( !auth.license( license_key ) ) {
			set_status( 3, 0, "Invalid or expired license key." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}
		save_key( license_key );

		set_status( 1, 45, "Loading driver..." );
		if ( !ensure_driver_loaded( ) ) {
			set_status( 3, 0, "Driver blocked - run as Administrator." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		if ( !is_process_running( L"obs64.exe" ) ) {
			set_status( 1, 55, "Waiting for OBS (start obs64.exe as Admin)..." );
			while ( !is_process_running( L"obs64.exe" ) ) {
				Sleep( 500 );
			}
		}

		set_status( 1, 62, "Waiting for Apex Legends (go to lobby)..." );
		const wchar_t* apex_exe = nullptr;
		while ( !( apex_exe = find_apex_process( ) ) ) {
			Sleep( 500 );
		}

		if ( !g_driver->attach( apex_exe ) ) {
			g_driver->unload( );
			set_status( 3, 0, "Failed to attach to game process." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 70, "Checking OBS Game Capture hook..." );
		std::uint64_t obs_mod = 0;
		for ( int wait_i = 0; wait_i < 30; ++wait_i ) {
			obs_mod = g_driver->get_process_module( encrypt( L"graphics-hook64.dll" ) );
			if ( obs_mod ) break;
			set_status( 1, 70, "OBS not hooked - add Game Capture in OBS..." );
			Sleep( 500 );
		}
		if ( !obs_mod ) {
			g_driver->unload( );
			set_status( 3, 0, "OBS hook missing (graphics-hook64.dll)." );
			g_ui.busy = false;
			EnableWindow( g_ui.edit_key, TRUE );
			return 0;
		}

		set_status( 1, 82, "Mapping module into memory..." );
		auto injector = std::make_shared< injector::c_injector >( false );
		auto payload = embedded_assets::decrypt( embedded_assets::payload_data, embedded_assets::payload_size );
		bool mapped = injector->map_memory( payload.data( ), payload.size( ) );
		std::fill( payload.begin( ), payload.end( ), 0 );

		if ( !mapped ) {
			injector->cleanup( );
			g_driver->unload( );
			set_status( 3, 0, "Failed to map module into memory." );
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

	constexpr RECT k_rc_close = { 404, 8, 430, 32 };
	constexpr RECT k_rc_min   = { 376, 8, 402, 32 };
	constexpr RECT k_rc_btn   = { 28, 214, 412, 256 };

	bool pt_in( const RECT& r, int x, int y ) {
		return x >= r.left && x <= r.right && y >= r.top && y <= r.bottom;
	}

	LRESULT CALLBACK wnd_proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {
		switch ( msg ) {
		case WM_CREATE: {
			g_ui.br_bg   = CreateSolidBrush( RGB( 11, 13, 20 ) );
			g_ui.br_card = CreateSolidBrush( RGB( 17, 20, 32 ) );
			g_ui.br_edit = CreateSolidBrush( RGB( 19, 22, 36 ) );
			g_ui.fnt_title = CreateFontA( 18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );
			g_ui.fnt_bold  = CreateFontA( 15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );
			g_ui.fnt_body  = CreateFontA( 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI" );

			g_ui.edit_key = CreateWindowExA( 0, "EDIT", "",
				WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
				36, 94, 368, 24, hwnd, reinterpret_cast< HMENU >( 101 ), nullptr, nullptr );
			SendMessageA( g_ui.edit_key, WM_SETFONT, reinterpret_cast< WPARAM >( g_ui.fnt_body ), TRUE );
			std::string saved = load_saved_key( );
			if ( !saved.empty( ) ) {
				SetWindowTextA( g_ui.edit_key, saved.c_str( ) );
			}
			g_ui.game_running = ( find_apex_process( ) != nullptr );
			g_ui.obs_running = is_process_running( L"obs64.exe" );
			SetTimer( hwnd, 1, 800, nullptr );
			return 0;
		}
		case WM_TIMER: {
			if ( wp == 1 ) {
				bool gr = ( find_apex_process( ) != nullptr );
				bool orun = is_process_running( L"obs64.exe" );
				if ( gr != g_ui.game_running || orun != g_ui.obs_running ) {
					g_ui.game_running = gr;
					g_ui.obs_running = orun;
					InvalidateRect( hwnd, nullptr, FALSE );
				}
			}
			return 0;
		}
		case WM_CTLCOLOREDIT: {
			HDC hdc = reinterpret_cast< HDC >( wp );
			SetTextColor( hdc, RGB( 235, 238, 248 ) );
			SetBkColor( hdc, RGB( 19, 22, 36 ) );
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
				auto* key_arg = new std::string( buf );
				HANDLE th = CreateThread( nullptr, 0, inject_worker, key_arg, 0, nullptr );
				if ( th ) CloseHandle( th );
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

			RECT hdr{ 0, 0, rc.right, 42 };
			FillRect( mem, &hdr, g_ui.br_card );

			HPEN pen_border = CreatePen( PS_SOLID, 1, RGB( 32, 36, 56 ) );
			HGDIOBJ old_pen = SelectObject( mem, pen_border );
			HGDIOBJ old_br = SelectObject( mem, GetStockObject( NULL_BRUSH ) );
			Rectangle( mem, 0, 0, rc.right, rc.bottom );
			MoveToEx( mem, 0, 42, nullptr );
			LineTo( mem, rc.right, 42 );

			SetBkMode( mem, TRANSPARENT );
			SelectObject( mem, g_ui.fnt_title );
			SetTextColor( mem, RGB( 242, 245, 255 ) );
			TextOutA( mem, 20, 12, "N E X U S", 9 );
			SelectObject( mem, g_ui.fnt_body );
			SetTextColor( mem, RGB( 110, 118, 150 ) );
			TextOutA( mem, 112, 14, "APEX LEGENDS", 12 );

			if ( g_ui.hover_min ) {
				HBRUSH b = CreateSolidBrush( RGB( 30, 34, 52 ) );
				FillRect( mem, &k_rc_min, b );
				DeleteObject( b );
			}
			SetTextColor( mem, RGB( 170, 178, 205 ) );
			DrawTextA( mem, "_", 1, const_cast< RECT* >( &k_rc_min ), DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			if ( g_ui.hover_close ) {
				HBRUSH b = CreateSolidBrush( RGB( 185, 45, 55 ) );
				FillRect( mem, &k_rc_close, b );
				DeleteObject( b );
				SetTextColor( mem, RGB( 255, 255, 255 ) );
			}
			else {
				SetTextColor( mem, RGB( 170, 178, 205 ) );
			}
			DrawTextA( mem, "X", 1, const_cast< RECT* >( &k_rc_close ), DT_CENTER | DT_VCENTER | DT_SINGLELINE );

			SelectObject( mem, g_ui.fnt_body );
			SetTextColor( mem, RGB( 150, 158, 188 ) );
			TextOutA( mem, 28, 64, "LICENSE KEY", 11 );

			SetTextColor( mem, g_ui.game_running ? RGB( 80, 230, 145 ) : RGB( 235, 110, 110 ) );
			TextOutA( mem, 240, 64, g_ui.game_running ? "GAME: OK" : "GAME: OFF", g_ui.game_running ? 8 : 9 );
			SetTextColor( mem, g_ui.obs_running ? RGB( 80, 230, 145 ) : RGB( 235, 110, 110 ) );
			TextOutA( mem, 338, 64, g_ui.obs_running ? "OBS: OK" : "OBS: OFF", g_ui.obs_running ? 7 : 8 );

			RECT edit_box{ 28, 86, 412, 124 };
			FillRect( mem, &edit_box, g_ui.br_edit );
			Rectangle( mem, edit_box.left, edit_box.top, edit_box.right, edit_box.bottom );

			RECT stat_box{ 28, 138, 412, 198 };
			FillRect( mem, &stat_box, g_ui.br_card );
			Rectangle( mem, stat_box.left, stat_box.top, stat_box.right, stat_box.bottom );

			char st_buf[ 256 ]{};
			int kind = 0, prog = 0;
			EnterCriticalSection( &g_ui.cs );
			strncpy_s( st_buf, g_ui.status_text, sizeof( st_buf ) - 1 );
			kind = g_ui.state_kind;
			prog = g_ui.progress;
			LeaveCriticalSection( &g_ui.cs );

			COLORREF st_col = RGB( 200, 206, 226 );
			if ( kind == 1 ) st_col = RGB( 140, 165, 255 );
			else if ( kind == 2 ) st_col = RGB( 80, 230, 145 );
			else if ( kind == 3 ) st_col = RGB( 255, 95, 95 );

			SetTextColor( mem, st_col );
			RECT st_txt{ 40, 146, 400, 170 };
			DrawTextA( mem, st_buf, -1, &st_txt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS );

			RECT pbar_bg{ 40, 178, 400, 185 };
			HBRUSH br_pbg = CreateSolidBrush( RGB( 28, 32, 48 ) );
			FillRect( mem, &pbar_bg, br_pbg );
			DeleteObject( br_pbg );
			if ( prog > 0 ) {
				int pw = ( ( pbar_bg.right - pbar_bg.left ) * prog ) / 100;
				RECT pbar_fg{ pbar_bg.left, pbar_bg.top, pbar_bg.left + pw, pbar_bg.bottom };
				HBRUSH br_pfg = CreateSolidBrush( kind == 2 ? RGB( 65, 215, 130 ) : RGB( 99, 102, 241 ) );
				FillRect( mem, &pbar_fg, br_pfg );
				DeleteObject( br_pfg );
			}

			COLORREF btn_rgb = g_ui.busy
				? RGB( 45, 48, 72 )
				: ( g_ui.hover_inject ? RGB( 118, 122, 255 ) : RGB( 99, 102, 241 ) );
			HBRUSH br_btn = CreateSolidBrush( btn_rgb );
			FillRect( mem, &k_rc_btn, br_btn );
			DeleteObject( br_btn );

			SelectObject( mem, g_ui.fnt_bold );
			SetTextColor( mem, g_ui.busy ? RGB( 150, 155, 180 ) : RGB( 255, 255, 255 ) );
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
			if ( g_ui.fnt_body ) DeleteObject( g_ui.fnt_body );
			if ( g_ui.fnt_bold ) DeleteObject( g_ui.fnt_bold );
			PostQuitMessage( 0 );
			return 0;
		}
		return DefWindowProcA( hwnd, msg, wp, lp );
	}
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int ) {
	SetUnhandledExceptionFilter( exception::exception_filter );
	InitializeCriticalSection( &g_ui.cs );

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof( wc );
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = hInst;
	wc.hCursor = LoadCursorA( nullptr, reinterpret_cast< LPCSTR >( 32512 ) );
	wc.lpszClassName = "NexusApexLoaderWnd";
	RegisterClassExA( &wc );

	const int w = 440, h = 282;
	const int sx = ( GetSystemMetrics( SM_CXSCREEN ) - w ) / 2;
	const int sy = ( GetSystemMetrics( SM_CYSCREEN ) - h ) / 2;
	g_ui.hwnd = CreateWindowExA( WS_EX_APPWINDOW, wc.lpszClassName, "NEXUS // APEX LEGENDS LOADER",
		WS_POPUP | WS_VISIBLE, sx, sy, w, h, nullptr, nullptr, hInst, nullptr );

	MSG msg{};
	while ( GetMessageA( &msg, nullptr, 0, 0 ) > 0 ) {
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
	return 0;
}
