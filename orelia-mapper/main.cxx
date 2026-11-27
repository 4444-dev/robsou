#include <impl/includes.h>

static bool is_being_analyzed( ) {
	if ( IsDebuggerPresent( ) ) return true;
	BOOL remote_debugger = FALSE;
	if ( CheckRemoteDebuggerPresent( GetCurrentProcess( ), &remote_debugger ) && remote_debugger ) return true;
#ifdef _M_X64
	const uintptr_t peb = __readgsqword( 0x60 );
	if ( peb && *reinterpret_cast< const unsigned char* >( peb + 2 ) ) return true;
#endif
	return false;
}

int __cdecl main( int argc, char** argv ) {
	if ( is_being_analyzed( ) ) return 0xDEAD;
	if ( argc < 2 ) {
		logging::print( encrypt( "Error: Missing parameters" ) );
		logging::print( encrypt( "Press any key to exit..." ) );
		return std::getchar( );
	}

	bool silent = ( argc >= 3 && std::strstr( argv[ 2 ], encrypt( "--silent" ) ) != nullptr );

	SetConsoleTitleA( encrypt( "orelia-mapper" ) );
	SetUnhandledExceptionFilter( exception::exception_filter );

	std::vector<std::uint8_t> driver_image;
	utility::open_file( argv[ 1 ], driver_image );
	if ( driver_image.empty( ) ) {
		logging::print( encrypt( "Error: Failed to get driver image" ) );
		if ( silent ) return 1;
		logging::print( encrypt( "Press any key to exit..." ) );
		return std::getchar( );
	}

	if ( !mapper::execute( driver_image ) ) {
		logging::print( encrypt( "Failed to execute payload.\n" ) );
		if ( silent ) return 1;
		return std::getchar( );
	}

	if ( silent ) return 0;
	return std::getchar( );
}