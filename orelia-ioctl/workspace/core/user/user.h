namespace user {
	enum class e_command_type : std::uint32_t {
		open_process,
		attach_process,
		detach_process,
		get_process_peb,
		read_memory,
		write_memory,
		get_eprocess,
		get_base_address,
		translate_linear,
		protect_virtual,
		allocate_virtual,
		free_virtual,
		map_process_pte,
		map_physical,
		get_dtb,
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
			e_command_type type,
			s_command_data data )
			: m_type( type ), m_status( nt_status_t::unsuccessful ), m_data( data ) {
		}

		nt_status_t get_status( ) const { return m_status; }
		void set_status( nt_status_t status_code ) {
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
		nt_status_t m_status{};
		s_command_data m_data{};
	};
#pragma pack(pop)
}