#define NT_SUCCESS(Status) (((nt_status_t)(Status)) >= 0)
namespace handler {
    nt_status_t read_memory( user::s_command_data* input_data ) {
        if ( !input_data || !input_data->m_address || !input_data->m_buffer || !input_data->m_size )
            return nt_status_t::invalid_parameter;

        auto find_min = [ input_data ] ( std::size_t size ) {
            return ( size < ( input_data->m_size ) ? ( size ) : ( input_data->m_size ) );
            };

        auto physical_address = mm::g_paging.translate( input_data->m_address );
        if ( !physical_address ) {
            return nt_status_t::unsuccessful;
        }

        auto final_size = find_min( mm::paging::page_4kb_size - ( physical_address & mm::paging::page_4kb_mask ) );
        if ( !final_size )
            return nt_status_t::invalid_parameter;

        if ( final_size <= 256 ) {
            std::uint8_t stack_buf[ 256 ]{};
            auto result = mm::phys::read_direct( physical_address, stack_buf, final_size );
            if ( result != nt_status_t::success )
                return nt_status_t::unsuccessful;

            __try {
                nt::probe_for_write( input_data->m_buffer, final_size, 1 );
                crt::memcpy( input_data->m_buffer, stack_buf, final_size );
            }
            __except ( 1 ) {
                return nt_status_t::access_violation;
            }
            return nt_status_t::success;
        }

        auto result = mm::phys::copy_verified(
            physical_address ,
            input_data->m_buffer ,
            final_size
        );

        return result == nt_status_t::success ? nt_status_t::success : nt_status_t::access_violation;
    }

    nt_status_t write_memory( user::s_command_data* input_data ) {
        if ( !input_data || !input_data->m_address || !input_data->m_buffer || !input_data->m_size )
            return nt_status_t::invalid_parameter;

        auto find_min = [ input_data ] ( std::size_t size ) {
            return ( size < ( input_data->m_size ) ? ( size ) : ( input_data->m_size ) );
            };

        auto physical_address = mm::g_paging.translate( input_data->m_address );
        if ( !physical_address ) {
            return nt_status_t::unsuccessful;
        }

        auto final_size = find_min( mm::paging::page_4kb_size - ( physical_address & mm::paging::page_4kb_mask ) );
        if ( !final_size )
            return nt_status_t::invalid_parameter;

        if ( final_size <= 256 ) {
            std::uint8_t stack_buf[ 256 ]{};
            __try {
                nt::probe_for_read( input_data->m_buffer, final_size, 1 );
                crt::memcpy( stack_buf, input_data->m_buffer, final_size );
            }
            __except ( 1 ) {
                return nt_status_t::access_violation;
            }

            auto result = mm::phys::write_region(
                physical_address ,
                stack_buf ,
                final_size
            );
            return result ? nt_status_t::success : nt_status_t::access_violation;
        }

        auto staging = nt::ex_allocate_pool( final_size );
        if ( !staging )
            return nt_status_t::insufficient_resources;

        __try {
            nt::probe_for_read( input_data->m_buffer, final_size, 1 );
            crt::memcpy( staging, input_data->m_buffer, final_size );
        }
        __except ( 1 ) {
            nt::ex_free_pool( staging );
            return nt_status_t::access_violation;
        }

        auto result = mm::phys::write_region(
            physical_address ,
            staging ,
            final_size
        );

        nt::ex_free_pool( staging );
        return result ? nt_status_t::success : nt_status_t::access_violation;
    }

    nt_status_t translate_linear( user::s_command_data* input_data ) {
        std::uint32_t page_size = 0;
        input_data->m_address2 = mm::g_paging.translate( input_data->m_address , &page_size );
        input_data->m_size = static_cast< std::size_t >( page_size );
        return input_data->m_address2 ? nt_status_t::success : nt_status_t::unsuccessful;
    }

    inline bool has_valid_eprocess_dtb( eprocess_t* process ) {
        if ( !process )
            return false;
        auto ptr = reinterpret_cast< std::uint64_t* >( reinterpret_cast< std::uint64_t >( process ) + nt::get_dtb_offset( ) );
        if ( !nt::mm_is_address_valid( ptr ) )
            return false;
        return ( *ptr & ~mm::paging::page_4kb_mask ) != 0;
    }

    nt_status_t map_process_pte( user::s_command_data* input_data ) {
        if ( !has_valid_eprocess_dtb( input_data->m_process ) ) {
            if ( !mm::g_paging.map_process_pte( input_data->m_address ) )
                return nt_status_t::access_violation;
            return input_data->m_address ? nt_status_t::success : nt_status_t::unsuccessful;
        }

        kapc_state_t state{};

        nt::m_attach_process( input_data->m_process , &state );

        if ( !mm::g_paging.map_process_pte( input_data->m_address ) ) {
            nt::m_detach_process( &state );
            return nt_status_t::access_violation;
        }

        nt::m_detach_process( &state );
        return input_data->m_address ? nt_status_t::success : nt_status_t::unsuccessful;
    }

    nt_status_t protect_virtual( user::s_command_data* input_data ) {
        if ( !input_data || !input_data->m_address || !input_data->m_size )
            return nt_status_t::invalid_parameter;

        constexpr std::uint32_t exec_mask = 0x10 | 0x20 | 0x40 | 0x80;
        const bool wants_exec = ( static_cast< std::uint32_t >( input_data->m_new_protection ) & exec_mask ) != 0;

        if ( wants_exec || !has_valid_eprocess_dtb( input_data->m_process ) ) {
            if ( has_valid_eprocess_dtb( input_data->m_process ) ) {
                if ( !mm::g_paging.get_dtb( ) ) {
                    auto pe_base = reinterpret_cast< std::uint64_t >( nt::ps_get_process_section_base_address( input_data->m_process ) );
                    mm::g_paging.set_dtb_from_process( reinterpret_cast< std::uint64_t >( input_data->m_process ), pe_base );
                }
                mm::g_paging.detect_shadow_dtb( reinterpret_cast< std::uint64_t >( input_data->m_process ) );
            }
            if ( !mm::g_paging.make_range_executable( input_data->m_address, input_data->m_size ) )
                return nt_status_t::unsuccessful;
            input_data->m_old_protection = 0x04;
            return nt_status_t::success;
        }

        kapc_state_t state{};
        nt::m_attach_process( input_data->m_process, &state );

        size_t size = input_data->m_size;
        void* base_address = reinterpret_cast< void* >( input_data->m_address );
        std::uint32_t old_protection = 0;

        nt_status_t status = nt::zw_protect_virtual_memory(
            reinterpret_cast< void* >( -1ULL ),
            &base_address,
            &size,
            input_data->m_new_protection,
            &old_protection
        );

        nt::m_detach_process( &state );

        if ( !nt::nt_success( status ) ) {
            return nt_status_t::unsuccessful;
        }

        input_data->m_old_protection = static_cast< int >( old_protection );
        return nt_status_t::success;
    }

    nt_status_t allocate_virtual( user::s_command_data* input_data ) {
        if ( !input_data || !input_data->m_pid || !input_data->m_size ) {
            return nt_status_t::invalid_parameter;
        }

        eprocess_t* process = nt::ps_lookup_process_by_process_id( input_data->m_pid );
        if ( !process ) {
            return nt_status_t::unsuccessful;
        }

        if ( !has_valid_eprocess_dtb( process ) ) {
            nt_status_t alloc_err = nt_status_t::unsuccessful;
            auto allocated_va = mm::g_paging.allocate_user_pages( input_data->m_size, &alloc_err );
            if ( !allocated_va ) {
                nt::ob_dereference_object( process );
                return alloc_err;
            }
            input_data->m_address = allocated_va;
            input_data->m_size = ( input_data->m_size + mm::paging::page_4kb_mask ) & ~mm::paging::page_4kb_mask;
            nt::ob_dereference_object( process );
            return nt_status_t::success;
        }

        kapc_state_t state{};
        nt::m_attach_process( process, &state );

        void* current_process = reinterpret_cast< void* >( -1ULL );
        void* base_address = nullptr;
        std::size_t region_size = input_data->m_size;

        nt_status_t status = nt::zw_allocate_virtual_memory(
            current_process,
            &base_address,
            0,
            &region_size,
            0x00001000 | 0x00002000,
            0x04
        );

        if ( !nt::nt_success( status ) || !base_address ) {
            nt::m_detach_process( &state );
            nt::ob_dereference_object( process );
            return nt::nt_success( status ) ? nt_status_t::unsuccessful : status;
        }

        nt::zero_memory( base_address, region_size );

        if ( !mm::g_paging.get_dtb( ) ) {
            auto pe_base = reinterpret_cast< std::uint64_t >( nt::ps_get_process_section_base_address( process ) );
            mm::g_paging.set_dtb_from_process( reinterpret_cast< std::uint64_t >( process ), pe_base );
        }
        mm::g_paging.detect_shadow_dtb( reinterpret_cast< std::uint64_t >( process ) );

        if ( !mm::g_paging.make_range_executable( reinterpret_cast< std::uintptr_t >( base_address ), region_size ) ) {
            std::size_t rel_size = 0;
            void* rel_base = base_address;
            nt::zw_free_virtual_memory( current_process, &rel_base, &rel_size, 0x8000 );
            nt::m_detach_process( &state );
            nt::ob_dereference_object( process );
            return nt_status_t::unsuccessful;
        }

        mm::g_paging.map_process_pte( reinterpret_cast< std::uintptr_t >( base_address ) );

        input_data->m_address = reinterpret_cast< std::uintptr_t >( base_address );
        input_data->m_size = region_size;

        nt::m_detach_process( &state );
        nt::ob_dereference_object( process );
        return nt_status_t::success;
    }

    nt_status_t free_virtual( user::s_command_data* input_data ) {
        if ( !input_data || !input_data->m_process || !input_data->m_address ) {
            return nt_status_t::invalid_parameter;
        }

        kapc_state_t state{};
        nt::m_attach_process( input_data->m_process, &state );

        void* base = reinterpret_cast< void* >( input_data->m_address );
        std::size_t size = input_data->m_size;

        nt_status_t status = nt::zw_free_virtual_memory(
            reinterpret_cast< void* >( -1ULL ),
            &base,
            &size,
            0x8000
        );

        nt::m_detach_process( &state );

        if ( !nt::nt_success( status ) ) {
            return nt_status_t::unsuccessful;
        }

        return nt_status_t::success;
    }

    nt_status_t map_physical( user::s_command_data* input_data ) {
        if ( !input_data->m_address || !input_data->m_size ) {
            return nt_status_t::invalid_parameter;
        }

        auto aligned_phys = mm::g_paging.page_align( input_data->m_address );
        auto page_offset = input_data->m_address - aligned_phys;

        auto total_size = mm::g_paging.page_align( input_data->m_size + page_offset );
        if ( total_size == 0 ) {
            total_size = mm::paging::page_4kb_size;
        }

        auto mapped_va = reinterpret_cast< std::uintptr_t >(
            nt::mm_map_io_space( aligned_phys , total_size )
            );

        if ( !mapped_va ) {
            return nt_status_t::unsuccessful;
        }

        input_data->m_address2 = mapped_va + page_offset;
        input_data->m_size = total_size;

        return nt_status_t::success;
    }
}