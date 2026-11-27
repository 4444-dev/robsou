namespace mm {
	namespace paging {
		constexpr auto page_4kb_size = 0x1000ull;
		constexpr auto page_2mb_size = 0x200000ull;
		constexpr auto page_1gb_size = 0x40000000ull;

		constexpr auto page_shift = 12ull;
		constexpr auto page_2mb_shift = 21ull;
		constexpr auto page_1gb_shift = 30ull;

		constexpr auto page_4kb_mask = 0xFFFull;
		constexpr auto page_2mb_mask = 0x1FFFFFull;
		constexpr auto page_1gb_mask = 0x3FFFFFFFull;

		enum class page_table_level {
			pml4,
			pdpt,
			pd,
			pt
		};

		class c_paging {
			cr3 m_dtb;
			cr3 m_shadow_dtb;
			std::uint64_t m_base_address;
			mmpfn_t* m_pfn_database;
			std::uint64_t m_lowest_physical_page = 0;
			std::uint64_t m_highest_physical_page = 0;

			c_vector< pml4e, 512 > m_pml4_table;
			c_vector< pdpte, 512 > m_pdpt_table;
			c_vector< pde, 512 > m_pd_table;
			c_vector< pte, 512 > m_pt_table;

			std::uint16_t m_cached_pml4e;
			std::uint16_t m_cached_pdpte;
			std::uint16_t m_cached_pde;

		public:
			void cleanup( ) {
				m_pml4_table.clear( );
				m_pdpt_table.clear( );
				m_pd_table.clear( );
				m_pt_table.clear( );

				m_cached_pml4e = 0;
				m_cached_pdpte = 0;
				m_cached_pde = 0;
				m_dtb.flags = 0;
				m_shadow_dtb.flags = 0;
				m_base_address = 0;
			}

			bool setup( ) {
				if ( !m_pml4_table.empty( ) )
					return false;

				this->m_pfn_database = nt::get_mm_pfn_database( );
				if ( !m_pfn_database )
					return false;

				this->m_lowest_physical_page = ~0ULL;
				this->m_highest_physical_page = 0;

				const auto ranges = nt::mm_get_physical_memory_ranges( );
				if ( ranges ) {
					for ( auto i = 0; ; ++i ) {
						const auto& range = ranges[ i ];
						if ( !range.m_base_page.m_quad_part && !range.m_page_count.m_quad_part )
							break;
						if ( !range.m_page_count.m_quad_part )
							continue;

						const std::uint64_t start_page = static_cast< std::uint64_t >( range.m_base_page.m_quad_part ) >> page_shift;
						const std::uint64_t end_page = ( static_cast< std::uint64_t >( range.m_base_page.m_quad_part ) +
							static_cast< std::uint64_t >( range.m_page_count.m_quad_part ) - 1ULL ) >> page_shift;

						if ( start_page < this->m_lowest_physical_page )
							this->m_lowest_physical_page = start_page;
						if ( end_page > this->m_highest_physical_page )
							this->m_highest_physical_page = end_page;
					}
				}

				if ( this->m_lowest_physical_page == ~0ULL )
					this->m_lowest_physical_page = 0;

				return true;
			}

			std::uint64_t translate_with_pml4( std::uint64_t pml4_pa, std::uint64_t virtual_address ) {
				if ( !pml4_pa || !virtual_address )
					return 0;

				auto va = virt_addr_t{ virtual_address };

				pml4e pml4_entry{};
				if ( phys::read_direct( ( pml4_pa & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) ),
					&pml4_entry, sizeof( pml4e ) ) != nt_status_t::success || !pml4_entry.hard.present )
					return 0;

				pdpte pdpt_entry{};
				if ( phys::read_direct( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
					&pdpt_entry, sizeof( pdpte ) ) != nt_status_t::success || !pdpt_entry.hard.present )
					return 0;

				if ( pdpt_entry.hard.page_size )
					return ( pdpt_entry.hard.pfn << page_shift ) + ( virtual_address & page_1gb_mask );

				pde pd_entry{};
				if ( phys::read_direct( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
					&pd_entry, sizeof( pde ) ) != nt_status_t::success || !pd_entry.hard.present )
					return 0;

				if ( pd_entry.hard.page_size )
					return ( pd_entry.hard.pfn << page_shift ) + ( virtual_address & page_2mb_mask );

				pte pt_entry{};
				if ( phys::read_direct( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
					&pt_entry, sizeof( pte ) ) != nt_status_t::success || !pt_entry.hard.present )
					return 0;

				return ( pt_entry.hard.pfn << page_shift ) + ( virtual_address & page_4kb_mask );
			}

			bool verify_pe_and_score( std::uint64_t pml4_pa, std::uint64_t base_address, std::uint64_t secondary_va, std::uint32_t& out_score ) {
				out_score = 0;

				auto base_pa = translate_with_pml4( pml4_pa, base_address );
				if ( !base_pa )
					return false;

				std::uint16_t mz = 0;
				if ( phys::read_direct( base_pa, &mz, sizeof( mz ) ) != nt_status_t::success || mz != 0x5A4D )
					return false;

				std::uint32_t lfanew = 0;
				if ( phys::read_direct( base_pa + 0x3C, &lfanew, sizeof( lfanew ) ) != nt_status_t::success )
					return false;
				if ( lfanew < 0x40 || lfanew > 0x1000 )
					return false;

				auto nt_pa = translate_with_pml4( pml4_pa, base_address + lfanew );
				if ( !nt_pa )
					return false;

				std::uint32_t pe_sig = 0;
				if ( phys::read_direct( nt_pa, &pe_sig, sizeof( pe_sig ) ) != nt_status_t::success || pe_sig != 0x00004550 )
					return false;

				pml4e pml4_table[ 512 ]{};
				if ( phys::read_direct( pml4_pa & ~page_4kb_mask, pml4_table, sizeof( pml4_table ) ) != nt_status_t::success )
					return false;

				bool has_kernel = false;
				std::uint32_t user_pml4_count = 0;
				for ( int i = 0; i < 256; ++i ) {
					if ( pml4_table[ i ].hard.present )
						++user_pml4_count;
				}
				for ( int i = 256; i < 512; ++i ) {
					if ( pml4_table[ i ].hard.present ) {
						has_kernel = true;
						break;
					}
				}
				if ( !has_kernel || user_pml4_count == 0 )
					return false;

				std::uint32_t score = user_pml4_count;
				if ( translate_with_pml4( pml4_pa, base_address + page_4kb_size ) )
					score += 100;
				if ( secondary_va && translate_with_pml4( pml4_pa, secondary_va ) )
					score += 1000;

				out_score = score;
				return true;
			}

			bool scan_pages( std::uint64_t base_address, std::uint64_t secondary_va = 0 ) {
				if ( !base_address )
					return false;

				this->m_dtb.flags = 0;
				this->m_shadow_dtb.flags = 0;
				this->m_base_address = base_address;

				const auto ranges = nt::mm_get_physical_memory_ranges( );

				auto run_pass = [ & ] ( bool require_self_pte ) -> bool {
					std::uint64_t best_pa = 0;
					std::uint32_t best_score = 0;
					std::uint64_t second_pa = 0;
					std::uint32_t second_score = 0;

					for ( auto i = 0; ; i++ ) {
						auto memory_range = ranges[ i ];
						if ( !memory_range.m_base_page.m_quad_part ||
							!memory_range.m_page_count.m_quad_part )
							break;

						std::uint64_t current_pa = memory_range.m_base_page.m_quad_part;
						for ( auto current_page = 0;
							current_page < memory_range.m_page_count.m_quad_part;
							current_page += 0x1000, current_pa += 0x1000 ) {
							cr3 current_dtb{ .pfn = current_pa >> page_shift };
							if ( !current_dtb.flags )
								continue;

							auto pfn_entry = &m_pfn_database[ current_dtb.pfn ];
							if ( !nt::mm_is_address_valid( pfn_entry ) ||
								!nt::mm_is_address_valid( reinterpret_cast< std::uint8_t* >( pfn_entry ) + sizeof( mmpfn_t ) - 1 ) )
								continue;
							if ( !pfn_entry->m_u3.m_reference_count ) continue;
							if ( require_self_pte && pfn_entry->m_u4.m_pte_frame != current_dtb.pfn ) continue;

							std::uint32_t score = 0;
							if ( !verify_pe_and_score( current_pa, base_address, secondary_va, score ) )
								continue;

							if ( pfn_entry->m_u4.m_pte_frame == current_dtb.pfn )
								score += 10;

							if ( score > best_score ) {
								second_pa = best_pa;
								second_score = best_score;
								best_pa = current_pa;
								best_score = score;
							}
							else if ( current_pa != best_pa && score > second_score ) {
								second_pa = current_pa;
								second_score = score;
							}
						}
					}

					if ( !best_pa )
						return false;

					if ( !this->cache_pt( best_pa, base_address, true ) )
						return false;

					this->m_dtb = cr3{ .pfn = best_pa >> page_shift };
					if ( second_pa )
						this->m_shadow_dtb = cr3{ .pfn = second_pa >> page_shift };
					return true;
				};

				if ( run_pass( true ) )
					return true;

				return run_pass( false );
			}

			bool set_dtb_direct( std::uint64_t dtb_pa, std::uint64_t base_address ) {
				std::uint64_t aligned = dtb_pa & ~page_4kb_mask;
				if ( !aligned )
					return false;

				if ( base_address ) {
					std::uint32_t score = 0;
					if ( !verify_pe_and_score( aligned, base_address, 0, score ) )
						return false;
					if ( !this->cache_pt( aligned, base_address, true ) )
						return false;
					this->m_base_address = base_address;
				}
				else {
					pml4e pml4_table[ 512 ]{};
					if ( phys::read_direct( aligned, pml4_table, sizeof( pml4_table ) ) != nt_status_t::success )
						return false;
					m_pml4_table.clear( );
					if ( !m_pml4_table.push_back_batch( pml4_table, 512 ) )
						return false;
				}

				this->m_dtb = cr3{ .pfn = aligned >> page_shift };
				return true;
			}

			bool set_dtb_from_process( std::uint64_t process_addr, std::uint64_t base_address ) {
				if ( !process_addr || !base_address )
					return false;

				auto ptr = reinterpret_cast< std::uint64_t* >( process_addr + nt::get_dtb_offset( ) );
				if ( !nt::mm_is_address_valid( ptr ) )
					return false;

				std::uint64_t val = *ptr & ~page_4kb_mask;
				if ( !val || val > 0x10000000000ULL )
					return false;

				if ( !this->cache_pt( val, base_address, true ) )
					return false;

				this->m_dtb = cr3{ .pfn = val >> page_shift };
				this->m_shadow_dtb.flags = 0;
				this->m_base_address = base_address;
				return true;
			}

			void detect_shadow_dtb( std::uint64_t process_addr ) {
				if ( this->m_shadow_dtb.flags || !process_addr || !this->m_base_address )
					return;

				for ( std::uint64_t off = 0x1A0; off <= 0x2C0; off += sizeof( std::uint64_t ) ) {
					auto ptr = reinterpret_cast< std::uint64_t* >( process_addr + off );
					if ( !nt::mm_is_address_valid( ptr ) )
						continue;

					std::uint64_t raw = *ptr;
					std::uint64_t val = raw & ~page_4kb_mask;
					if ( !val || ( raw & 0xFFE ) > 0x0FF || val == ( this->m_dtb.flags & ~page_4kb_mask ) )
						continue;
					if ( val > 0x10000000000ULL )
						continue;

					std::uint32_t score = 0;
					if ( this->verify_pe_and_score( val, this->m_base_address, 0, score ) ) {
						this->m_shadow_dtb = cr3{ .pfn = val >> page_shift };
						break;
					}
				}
			}

			bool cache_pt( std::uint64_t physical_address, std::uint64_t base_address, bool cache_tables = true ) {
				auto va = virt_addr_t{ base_address };

				pml4e pml4_table[ 512 ]{};
				if ( phys::read_direct( physical_address, pml4_table, 4096Ui64 ) != nt_status_t::success )
					return false;

				auto pml4_entry = pml4_table[ va.pml4e_index ];
				if ( !pml4_entry.hard.present )
					return false;

				pdpte pdpt_entry{};
				if ( phys::read_direct( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
					&pdpt_entry, sizeof( pdpte ) ) != nt_status_t::success )
					return false;

				if ( !pdpt_entry.hard.present )
					return false;

				pde pd_entry{};
				if ( phys::read_direct( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
					&pd_entry, sizeof( pde ) ) != nt_status_t::success )
					return false;

				if ( !pd_entry.hard.present )
					return false;

				pte pt_entry{};
				if ( phys::read_direct( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
					&pt_entry, sizeof( pte ) ) != nt_status_t::success )
					return false;

				if ( !pt_entry.hard.present )
					return false;

				if ( !cache_tables )
					return true;

				m_pml4_table.clear( );
				if ( !m_pml4_table.push_back_batch( pml4_table, 512 ) )
					return false;

				m_cached_pml4e = va.pml4e_index;
				m_cached_pdpte = va.pdpte_index;
				m_cached_pde = va.pde_index;
				return true;
			}

			std::uint64_t translate( std::uint64_t virtual_address, std::uint32_t* out_page_size = nullptr ) {
				if ( !m_dtb.flags ) {
					return 0;
				}

				auto va = virt_addr_t{ virtual_address };
				if ( va.pml4e_index >= m_pml4_table.size( ) )
					return 0;

				auto pml4_entry = this->m_pml4_table[ va.pml4e_index ];
				if ( !pml4_entry.hard.present ) {
					auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) );
					if ( phys::read_direct( pml4e_phys, &pml4_entry, sizeof( pml4e ) ) != nt_status_t::success )
						return 0;
					if ( !pml4_entry.hard.present )
						return 0;
					this->m_pml4_table.data( )[ va.pml4e_index ] = pml4_entry;
				}

				pdpte pdpt_entry{};
				if ( phys::read_direct( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
					&pdpt_entry, sizeof( pdpte ) ) != nt_status_t::success )
					return 0;

				if ( !pdpt_entry.hard.present )
					return 0;

				if ( pdpt_entry.hard.page_size ) {
					auto final_pa = ( pdpt_entry.hard.pfn << page_shift ) + ( virtual_address & page_1gb_mask );
					if ( out_page_size )
						*out_page_size = page_1gb_size;
					return final_pa;
				}

				pde pd_entry{};
				if ( phys::read_direct( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
					&pd_entry, sizeof( pde ) ) != nt_status_t::success )
					return 0;

				if ( !pd_entry.hard.present )
					return 0;

				if ( pd_entry.hard.page_size ) {
					auto final_pa = ( pd_entry.hard.pfn << page_shift ) + ( virtual_address & page_2mb_mask );
					if ( out_page_size )
						*out_page_size = page_2mb_size;
					return final_pa;
				}

				pte pt_entry{};
				if ( phys::read_direct( ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) ),
					&pt_entry, sizeof( pte ) ) != nt_status_t::success )
					return 0;

				if ( !pt_entry.hard.present )
					return 0;

				auto final_pa = ( pt_entry.hard.pfn << page_shift ) + ( virtual_address & page_4kb_mask );
				if ( out_page_size )
					*out_page_size = page_4kb_size;
				return final_pa;
			}

			std::uint64_t map_kernel_pte( std::uint64_t physical_address ) {
				if ( !physical_address || !is_valid_pa( physical_address, page_4kb_size ) ) {
					return 0;
				}

				int free_pte_index = -1;
				for ( int i = 0; i < m_pt_table.size( ); ++i ) {
					if ( !m_pt_table[ i ].hard.present ) {
						free_pte_index = i;
						break;
					}
				}
				if ( free_pte_index == -1 ) {
					return 0;
				}

				auto pd_entry = m_pd_table[ m_cached_pde ];
				if ( pd_entry.hard.page_size || !pd_entry.hard.present ) {
					return 0;
				}

				std::uint64_t user_va =
					( static_cast< std::uint64_t >( m_cached_pml4e ) << 39 ) |
					( static_cast< std::uint64_t >( m_cached_pdpte ) << 30 ) |
					( static_cast< std::uint64_t >( m_cached_pde ) << 21 ) |
					( static_cast< std::uint64_t >( free_pte_index ) << 12 );

				pte existing_pte;
				std::uint64_t pt_phys = pd_entry.hard.pfn << page_shift;
				std::uint64_t pte_phys = pt_phys + ( free_pte_index * sizeof( pte ) );
				if ( phys::read_direct( pte_phys, &existing_pte, sizeof( pte ) ) != nt_status_t::success ) {
					return 0;
				}

				if ( existing_pte.hard.present ) {
					return 0;
				}

				pte new_pte{};
				new_pte.hard.pfn = physical_address >> page_shift;
				new_pte.hard.present = 1;
				new_pte.hard.read_write = 1;
				new_pte.hard.user_supervisor = 1;
				new_pte.hard.no_execute = 0;

				auto result = phys::write_direct( pte_phys, &new_pte, sizeof( pte ) );
				if ( result != nt_status_t::success ) {
					return 0;
				}

				m_pt_table[ free_pte_index ] = new_pte;
				__invlpg( reinterpret_cast< void* >( user_va ) );

				return user_va;
			}

			bool map_process_pte( std::uint64_t virtual_address ) {
				if ( !virtual_address ) {
					return 0;
				}

				auto va = virt_addr_t{ virtual_address };
				if ( va.pml4e_index >= m_pml4_table.size( ) )
					return false;

				auto pml4_entry = this->m_pml4_table[ va.pml4e_index ];
				if ( !pml4_entry.hard.present ) {
					auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) );
					if ( phys::read_direct( pml4e_phys, &pml4_entry, sizeof( pml4e ) ) != nt_status_t::success )
						return false;
					if ( !pml4_entry.hard.present )
						return false;
					this->m_pml4_table.data( )[ va.pml4e_index ] = pml4_entry;
				}

				pdpte pdpt_entry{};
				if ( phys::read_direct( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
					&pdpt_entry, sizeof( pdpte ) ) != nt_status_t::success )
					return false;

				if ( !pdpt_entry.hard.present )
					return false;

				pde pd_entry{};
				if ( phys::read_direct( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
					&pd_entry, sizeof( pde ) ) != nt_status_t::success )
					return false;

				if ( !pd_entry.hard.present )
					return false;

				if ( pd_entry.hard.page_size )
					return false;

				pte existing_pte;
				auto pt_phys = pd_entry.hard.pfn << page_shift;
				auto pte_phys = pt_phys + ( va.pte_index * sizeof( pte ) );
				if ( phys::read_direct( pte_phys, &existing_pte, sizeof( pte ) ) != nt_status_t::success )
					return false;

				if ( !existing_pte.hard.present )
					return false;

				pte new_pte{};
				new_pte.hard.pfn = existing_pte.hard.pfn;
				new_pte.hard.present = 1;
				new_pte.hard.read_write = 1;
				new_pte.hard.user_supervisor = 1;
				new_pte.hard.no_execute = 0;

				auto result = phys::write_direct( pte_phys, &new_pte, sizeof( pte ) );
				if ( result != nt_status_t::success )
					return false;

				__invlpg( reinterpret_cast< void* >( virtual_address ) );
				return true;
			}

			bool make_range_executable( std::uint64_t start_va, std::size_t size ) {
				if ( !start_va || !size )
					return false;

				auto clear_nx_in_dtb = [ ] ( cr3 dtb, std::uint64_t vaddr ) -> bool {
					if ( !dtb.flags )
						return true;
					auto va = virt_addr_t{ vaddr };
					pml4e pml4_entry{};
					auto pml4e_phys = ( dtb.flags & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) );
					if ( phys::read_direct( pml4e_phys, &pml4_entry, sizeof( pml4e ) ) != nt_status_t::success || !pml4_entry.hard.present )
						return false;

					pdpte pdpt_entry{};
					if ( phys::read_direct( ( pml4_entry.hard.pfn << page_shift ) + ( va.pdpte_index * sizeof( pdpte ) ),
						&pdpt_entry, sizeof( pdpte ) ) != nt_status_t::success || !pdpt_entry.hard.present )
						return false;
					if ( pdpt_entry.hard.page_size )
						return true;

					pde pd_entry{};
					if ( phys::read_direct( ( pdpt_entry.hard.pfn << page_shift ) + ( va.pde_index * sizeof( pde ) ),
						&pd_entry, sizeof( pde ) ) != nt_status_t::success || !pd_entry.hard.present )
						return false;
					if ( pd_entry.hard.page_size )
						return true;

					pte existing_pte{};
					auto pte_phys = ( pd_entry.hard.pfn << page_shift ) + ( va.pte_index * sizeof( pte ) );
					if ( phys::read_direct( pte_phys, &existing_pte, sizeof( pte ) ) != nt_status_t::success || !existing_pte.hard.present )
						return false;

					existing_pte.hard.read_write = 1;
					existing_pte.hard.user_supervisor = 1;
					existing_pte.hard.no_execute = 0;
					return phys::write_region( pte_phys, &existing_pte, sizeof( pte ) );
				};

				const std::uint64_t aligned_start = start_va & ~page_4kb_mask;
				const std::uint64_t aligned_end = ( start_va + size + page_4kb_mask ) & ~page_4kb_mask;
				if ( aligned_end < aligned_start )
					return false;

				for ( std::uint64_t cur = aligned_start; cur < aligned_end; cur += page_4kb_size ) {
					if ( !clear_nx_in_dtb( this->m_dtb, cur ) )
						return false;
					if ( this->m_shadow_dtb.flags )
						clear_nx_in_dtb( this->m_shadow_dtb, cur );
					__invlpg( reinterpret_cast< void* >( cur ) );
				}

				return true;
			}

			std::uint64_t allocate_user_pages( std::size_t size, nt_status_t* out_err = nullptr ) {
				auto fail = [ & ] ( std::uint32_t code ) -> std::uint64_t {
					if ( out_err ) *out_err = static_cast< nt_status_t >( code );
					return 0;
				};

				if ( !size || !( this->m_dtb.flags & ~page_4kb_mask ) )
					return fail( 0xE0000001 );

				std::size_t page_count = ( size + page_4kb_mask ) >> page_shift;
				if ( !page_count || page_count > ( 512ULL * 512ULL ) )
					return fail( 0xE0000002 );

				int free_pml4 = -1;
				for ( int idx = 1; idx < 256; ++idx ) {
					if ( this->m_pml4_table[ idx ].hard.present )
						continue;
					pml4e check{};
					auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( idx * sizeof( pml4e ) );
					if ( phys::read_direct( pml4e_phys, &check, sizeof( pml4e ) ) != nt_status_t::success || check.hard.present )
						continue;
					if ( this->m_shadow_dtb.flags ) {
						pml4e shadow_check{};
						auto shadow_phys = ( this->m_shadow_dtb.flags & ~page_4kb_mask ) + ( idx * sizeof( pml4e ) );
						if ( phys::read_direct( shadow_phys, &shadow_check, sizeof( pml4e ) ) != nt_status_t::success || shadow_check.hard.present )
							continue;
					}
					free_pml4 = idx;
					break;
				}
				if ( free_pml4 == -1 )
					return fail( 0xE0000003 );

				auto alloc_aligned_pages = [ ] ( std::size_t pages ) -> std::uint8_t* {
					auto raw = reinterpret_cast< std::uintptr_t >(
						nt::ex_allocate_pool_with_tag( ( pages + 1 ) * page_4kb_size, 'xela', pool_type::nonpaged_pool )
					);
					if ( !raw )
						return nullptr;
					auto aligned = ( raw + page_4kb_mask ) & ~page_4kb_mask;
					nt::zero_memory( reinterpret_cast< void* >( aligned ), pages * page_4kb_size );
					return reinterpret_cast< std::uint8_t* >( aligned );
				};

				auto kernel_buf = alloc_aligned_pages( page_count );
				if ( !kernel_buf )
					return fail( 0xE0000004 );

				auto pdpt_va = reinterpret_cast< pdpte* >( alloc_aligned_pages( 1 ) );
				auto pd_va = reinterpret_cast< pde* >( alloc_aligned_pages( 1 ) );
				if ( !pdpt_va || !pd_va )
					return fail( 0xE0000004 );

				std::size_t global_page = 0;
				std::size_t pt_count = ( page_count + 511 ) / 512;
				for ( std::size_t pd_idx = 0; pd_idx < pt_count; ++pd_idx ) {
					auto pt_va = reinterpret_cast< pte* >( alloc_aligned_pages( 1 ) );
					if ( !pt_va )
						return fail( 0xE0000004 );

					for ( std::size_t pt_idx = 0; pt_idx < 512 && global_page < page_count; ++pt_idx, ++global_page ) {
						auto page_pa = nt::get_physical_address(
							reinterpret_cast< std::uintptr_t >( kernel_buf ) + ( global_page * page_4kb_size )
						);
						if ( !page_pa )
							return fail( 0xE0000005 );

						pt_va[ pt_idx ].hard.pfn = page_pa >> page_shift;
						pt_va[ pt_idx ].hard.present = 1;
						pt_va[ pt_idx ].hard.read_write = 1;
						pt_va[ pt_idx ].hard.user_supervisor = 1;
						pt_va[ pt_idx ].hard.no_execute = 0;
					}

					auto pt_pa = nt::get_physical_address( reinterpret_cast< std::uintptr_t >( pt_va ) );
					if ( !pt_pa )
						return fail( 0xE0000005 );

					pd_va[ pd_idx ].hard.pfn = pt_pa >> page_shift;
					pd_va[ pd_idx ].hard.present = 1;
					pd_va[ pd_idx ].hard.read_write = 1;
					pd_va[ pd_idx ].hard.user_supervisor = 1;
					pd_va[ pd_idx ].hard.no_execute = 0;
				}

				auto pd_pa = nt::get_physical_address( reinterpret_cast< std::uintptr_t >( pd_va ) );
				auto pdpt_pa = nt::get_physical_address( reinterpret_cast< std::uintptr_t >( pdpt_va ) );
				if ( !pd_pa || !pdpt_pa )
					return fail( 0xE0000005 );

				pdpt_va[ 0 ].hard.pfn = pd_pa >> page_shift;
				pdpt_va[ 0 ].hard.present = 1;
				pdpt_va[ 0 ].hard.read_write = 1;
				pdpt_va[ 0 ].hard.user_supervisor = 1;
				pdpt_va[ 0 ].hard.no_execute = 0;

				pml4e new_pml4{};
				new_pml4.hard.pfn = pdpt_pa >> page_shift;
				new_pml4.hard.present = 1;
				new_pml4.hard.read_write = 1;
				new_pml4.hard.user_supervisor = 1;
				new_pml4.hard.no_execute = 0;

				auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( free_pml4 * sizeof( pml4e ) );
				if ( !phys::write_region( pml4e_phys, &new_pml4, sizeof( pml4e ) ) )
					return fail( 0xE0000006 );

				if ( this->m_shadow_dtb.flags ) {
					auto shadow_phys = ( this->m_shadow_dtb.flags & ~page_4kb_mask ) + ( free_pml4 * sizeof( pml4e ) );
					if ( !phys::write_region( shadow_phys, &new_pml4, sizeof( pml4e ) ) )
						return fail( 0xE0000006 );
				}

				this->m_pml4_table.data( )[ free_pml4 ] = new_pml4;
				std::uint64_t user_va = static_cast< std::uint64_t >( free_pml4 ) << 39;
				for ( std::size_t p = 0; p < page_count; ++p )
					__invlpg( reinterpret_cast< void* >( user_va + p * page_4kb_size ) );

				return user_va;
			}

			bool update_pml4e( std::uint64_t virt, std::uint64_t new_pdpt_pfn ) {
				auto va = virt_addr_t{ virt };
				auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) );

				auto new_entry = this->m_pml4_table[ va.pml4e_index ];
				new_entry.hard.pfn = new_pdpt_pfn;

				auto result = mm::phys::write_direct( pml4e_phys, &new_entry, sizeof( pml4e ) );
				if ( result != nt_status_t::success )
					return false;

				this->m_pml4_table[ va.pml4e_index ] = new_entry;

				__invlpg( reinterpret_cast< void* >( virt ) );
				return true;
			}

			std::uint64_t create_page_table( ) {
				auto kernel_va = reinterpret_cast< std::uintptr_t >(
					nt::mm_allocate_non_cached_memory( page_4kb_size )
					);
				if ( !kernel_va )
					return 0;

				auto kernel_pa = nt::get_physical_address( kernel_va );
				if ( !kernel_pa ) {
					nt::mm_free_non_cached_memory( reinterpret_cast< void* >( kernel_va ), page_4kb_size );
					return 0;
				}

				auto user_va = map_kernel_pte( kernel_pa );
				if ( !user_va ) {
					nt::mm_free_non_cached_memory( reinterpret_cast< void* >( kernel_va ), page_4kb_size );
					return 0;
				}

				return user_va;
			}

			std::uint64_t copy_physical_page( std::uint64_t phys_addr ) {
				auto kernel_va = reinterpret_cast< std::uintptr_t >(
					nt::mm_allocate_non_cached_memory( page_4kb_size )
					);
				if ( !kernel_va )
					return 0;

				auto new_phys = nt::get_physical_address( kernel_va );
				if ( !new_phys ) {
					nt::mm_free_non_cached_memory( reinterpret_cast< void* >( kernel_va ), page_4kb_size );
					return 0;
				}

				auto status = mm::phys::read_direct( phys_addr, reinterpret_cast< void* >( kernel_va ), page_4kb_size );
				if ( status != nt_status_t::success ) {
					nt::mm_free_non_cached_memory( reinterpret_cast< void* >( kernel_va ), page_4kb_size );
					return 0;
				}

				auto user_va = map_kernel_pte( new_phys );
				if ( !user_va ) {
					nt::mm_free_non_cached_memory( reinterpret_cast< void* >( kernel_va ), page_4kb_size );
					return 0;
				}

				return user_va;
			}

			bool is_pt_entry_valid( std::uint64_t pfn, int index, page_table_level level ) {
				switch ( level ) {
				case page_table_level::pml4: {
					pml4e entry{};
					if ( phys::read_direct( ( pfn << page_shift ) + ( index * sizeof( pml4e ) ),
						&entry, sizeof( pml4e ) ) != nt_status_t::success )
						return false;
					return entry.hard.present;
				}
				case page_table_level::pdpt: {
					pdpte entry{};
					if ( phys::read_direct( ( pfn << page_shift ) + ( index * sizeof( pdpte ) ),
						&entry, sizeof( pdpte ) ) != nt_status_t::success )
						return false;
					return entry.hard.present;
				}
				case page_table_level::pd: {
					pde entry{};
					if ( phys::read_direct( ( pfn << page_shift ) + ( index * sizeof( pde ) ),
						&entry, sizeof( pde ) ) != nt_status_t::success )
						return false;
					return entry.hard.present;
				}
				case page_table_level::pt: {
					pte entry{};
					if ( phys::read_direct( ( pfn << page_shift ) + ( index * sizeof( pte ) ),
						&entry, sizeof( pte ) ) != nt_status_t::success )
						return false;
					return entry.hard.present;
				}
				default:
					return false;
				}
			}

			bool is_valid_pml4( std::uint64_t pfn ) {
				pml4e pml4_table[ 512 ]{};
				if ( phys::read_direct( pfn << page_shift, pml4_table, 4096Ui64 ) != nt_status_t::success )
					return false;

				int present_count = 0;
				bool has_kernel_mappings = false;

				for ( int i = 0; i < 512; i++ ) {
					if ( pml4_table[ i ].hard.present ) {
						present_count++;
						if ( i >= 256 )
							has_kernel_mappings = true;
					}
				}

				return present_count > 0 && has_kernel_mappings;
			}

			bool can_map_user_and_kernel_addresses( std::uint64_t pfn ) {
				pml4e pml4_table[ 512 ]{};
				if ( phys::read_direct( pfn << page_shift, pml4_table, 4096Ui64 ) != nt_status_t::success )
					return false;

				bool has_user_mappings = false;
				for ( int i = 0; i < 256; i++ ) {
					if ( pml4_table[ i ].hard.present ) {
						has_user_mappings = true;
						break;
					}
				}

				bool has_kernel_mappings = false;
				for ( int i = 256; i < 512; i++ ) {
					if ( pml4_table[ i ].hard.present ) {
						has_kernel_mappings = true;
						break;
					}
				}

				return has_user_mappings && has_kernel_mappings;
			}

			bool is_valid_pa( std::uint64_t physical_address, std::size_t size ) const {
				if ( !size || !this->m_highest_physical_page || physical_address > ( ~0ULL - size ) )
					return false;

				const auto lowest_pa = this->m_lowest_physical_page << page_shift;
				const auto highest_pa = ( this->m_highest_physical_page << page_shift ) + page_4kb_size;

				return ( physical_address >= lowest_pa &&
					( physical_address + size ) <= highest_pa );
			}

			bool copy_page_table( std::uint64_t dst_va, std::uint64_t src_va ) {
				std::size_t bytes_transferred = 0;
				auto result = nt::mm_copy_memory(
					reinterpret_cast< void* >( dst_va ),
					{ .m_virtual_address = src_va },
					page_4kb_size,
					copy_virtual,
					&bytes_transferred
				);
				return result == nt_status_t::success && bytes_transferred == page_4kb_size;
			}

			bool split_large_page( std::uint64_t pde_va, std::uint64_t pt_va ) {
				auto va = virt_addr_t{ pde_va };
				auto pd_entry = this->m_pd_table[ va.pde_index ];

				if ( !pd_entry.hard.present || !pd_entry.hard.page_size )
					return false;

				auto start_pfn = pd_entry.hard.pfn;
				pte* pt_entries = reinterpret_cast< pte* >( pt_va );

				for ( int idx = 0; idx < 512; idx++ ) {
					pt_entries[ idx ].value = pd_entry.value;
					pt_entries[ idx ].hard.global = 0;
					pt_entries[ idx ].hard.pfn = start_pfn + idx;
				}

				return true;
			}

			std::uint64_t get_pml4e_pfn( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pml4_entry = this->m_pml4_table[ va.pml4e_index ];
				if ( !pml4_entry.hard.present ) {
					auto pml4e_phys = ( this->m_dtb.flags & ~page_4kb_mask ) + ( va.pml4e_index * sizeof( pml4e ) );
					if ( phys::read_direct( pml4e_phys, &pml4_entry, sizeof( pml4e ) ) != nt_status_t::success )
						return 0;
					if ( !pml4_entry.hard.present )
						return 0;
					this->m_pml4_table.data( )[ va.pml4e_index ] = pml4_entry;
				}
				return pml4_entry.hard.pfn;
			}

			std::uint64_t get_pdpte_pfn( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pdpt_entry = this->m_pdpt_table[ va.pdpte_index ];
				if ( !pdpt_entry.hard.present )
					return 0;
				return pdpt_entry.hard.pfn;
			}

			std::uint64_t get_pde_pfn( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pd_entry = this->m_pd_table[ va.pde_index ];
				if ( !pd_entry.hard.present )
					return 0;
				return pd_entry.hard.pfn;
			}

			std::uint64_t get_pte_pfn( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pt_entry = this->m_pt_table[ va.pte_index ];
				if ( !pt_entry.hard.present )
					return 0;
				return pt_entry.hard.pfn;
			}

			bool is_large_page( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pd_entry = this->m_pd_table[ va.pde_index ];
				return pd_entry.hard.present && pd_entry.hard.page_size;
			}

			bool is_huge_page( std::uint64_t virt ) {
				auto va = virt_addr_t{ virt };
				auto pdpt_entry = this->m_pdpt_table[ va.pdpte_index ];
				return pdpt_entry.hard.present && pdpt_entry.hard.page_size;
			}

			std::uint64_t page_align( std::uint64_t virt ) {
				return virt & ~page_4kb_mask;
			}

			std::uint64_t page_offset( std::uint64_t virt ) {
				return virt & page_4kb_mask;
			}

			std::uintptr_t get_dtb( ) const {
				return this->m_dtb.flags;
			}
		};
	}
}