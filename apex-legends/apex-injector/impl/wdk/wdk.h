#pragma once
#include <ntstatus.h>

extern "C" inline std::uint32_t g_nt_device_io_control_ssn = 0;

extern "C" __int64 direct_device_control(
	HANDLE ,
	HANDLE ,
	PIO_APC_ROUTINE ,
	PVOID ,
	PIO_STATUS_BLOCK ,
	std::uint32_t ,
	PVOID ,
	std::uint32_t ,
	PVOID ,
	std::uint32_t
);

namespace driver {
	inline bool resolve_device_io_control_ssn( ) {
		if ( g_nt_device_io_control_ssn )
			return true;

		HMODULE ntdll = GetModuleHandleW( L"ntdll.dll" );
		if ( !ntdll )
			return false;

		auto scan_stub = [ ] ( const std::uint8_t* code, std::uint32_t& out_ssn ) -> bool {
			if ( !code )
				return false;
			if ( code[ 0 ] == 0x4C && code[ 1 ] == 0x8B && code[ 2 ] == 0xD1 && code[ 3 ] == 0xB8 ) {
				for ( int j = 8; j < 28; ++j ) {
					if ( code[ j ] == 0x0F && code[ j + 1 ] == 0x05 && code[ j + 2 ] == 0xC3 ) {
						out_ssn = *reinterpret_cast< const std::uint32_t* >( code + 4 );
						return out_ssn != 0 && out_ssn < 0x1000;
					}
				}
			}
			return false;
		};

		const auto* fn = reinterpret_cast< const std::uint8_t* >(
			GetProcAddress( ntdll, "NtDeviceIoControlFile" )
		);

		std::uint32_t ssn = 0;
		if ( scan_stub( fn, ssn ) ) {
			g_nt_device_io_control_ssn = ssn;
			return true;
		}

		if ( fn ) {
			for ( int delta = -32; delta <= 32; ++delta ) {
				if ( delta == 0 )
					continue;
				std::uint32_t neighbor_ssn = 0;
				if ( scan_stub( fn + static_cast< std::ptrdiff_t >( delta ) * 32, neighbor_ssn ) ) {
					const auto derived = static_cast< std::int32_t >( neighbor_ssn ) - delta;
					if ( derived > 0 && derived < 0x1000 ) {
						g_nt_device_io_control_ssn = static_cast< std::uint32_t >( derived );
						return true;
					}
				}
			}
		}

		using rtl_get_version_t = NTSTATUS( NTAPI* )( PRTL_OSVERSIONINFOW );
		auto rtl_get_version = reinterpret_cast< rtl_get_version_t >(
			GetProcAddress( ntdll, "RtlGetVersion" )
		);
		if ( rtl_get_version ) {
			RTL_OSVERSIONINFOW os_ver{};
			os_ver.dwOSVersionInfoSize = sizeof( os_ver );
			if ( rtl_get_version( &os_ver ) == STATUS_SUCCESS ) {
				if ( os_ver.dwBuildNumber >= 10240 )
					g_nt_device_io_control_ssn = 0x0007;
				else if ( os_ver.dwBuildNumber >= 9600 )
					g_nt_device_io_control_ssn = 0x0006;
				else if ( os_ver.dwBuildNumber >= 9200 )
					g_nt_device_io_control_ssn = 0x0005;
				else
					g_nt_device_io_control_ssn = 0x0004;
				return true;
			}
		}

		return false;
	}

	enum class e_command_type : std::uint32_t {
		open_process ,
		attach_process ,
		detach_process ,
		get_process_peb ,
		read_memory ,
		write_memory ,
		get_eprocess ,
		get_base_address ,
		translate_linear ,
		protect_virtual ,
		allocate_virtual ,
		free_virtual ,
		map_process_pte ,
		map_physical ,
		get_dtb ,
		set_dtb
	};

#pragma pack(push, 8)
	struct s_command_data {
		std::uint32_t m_pid;
		union {
			int m_new_protection;
			int m_old_protection;
		};
		eprocess_t* m_process;
		union {
			struct {
				std::uintptr_t m_address;
				union {
					std::uintptr_t m_address2;
					void* m_buffer;
					peb_t* m_peb;
				};
				std::size_t m_size;
			};
			kapc_state_t m_apc_state;
		};
	};

	class c_command {
	public:
		c_command( ) : m_type{}, m_status{}, m_data{} {}
		c_command(
			e_command_type type ,
			s_command_data data )
			: m_type( type ) , m_status( STATUS_SUCCESS ) , m_data( data ) {
		}

		NTSTATUS get_status( ) const { return m_status; }
		void set_status( NTSTATUS status_code ) {
			m_status = status_code;
		}

		e_command_type get_type( ) const { return m_type; }
		const s_command_data& get_data( ) const { return m_data; }

		void crypt( std::uint32_t pid ) {
			std::uint64_t state = ( static_cast< std::uint64_t >( pid ) * 0x9E3779B97F4A7C15ULL ) ^ 0xA5A55A5AF0F00F0FULL;
			auto* raw = reinterpret_cast< std::uint8_t* >( this );
			for ( std::size_t i = 0; i < sizeof( c_command ); ++i ) {
				state ^= state >> 12;
				state ^= state << 25;
				state ^= state >> 27;
				raw[ i ] ^= static_cast< std::uint8_t >( ( state * 0x2545F4914F6CDD1DULL ) >> 56 );
			}
		}

	private:
		e_command_type m_type{};
		NTSTATUS m_status{};
		s_command_data m_data{};
	};
#pragma pack(pop)
}