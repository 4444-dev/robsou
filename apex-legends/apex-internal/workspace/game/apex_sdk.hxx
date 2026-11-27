#pragma once
#include <workspace/game/netvars.hxx>

namespace apex_sdk {
	struct vec3_t {
		float x, y, z;
	};

	struct matrix4x4_t {
		float m[ 4 ][ 4 ];
	};

	namespace offsets {
		inline constexpr uintptr_t entity_list = 0x6b515d8;
		inline constexpr uintptr_t local_player = 0x3eac668;
		inline constexpr uintptr_t view_render = 0x5267cc0;
		inline constexpr uintptr_t view_matrix = 0x11a390;
		inline constexpr uintptr_t name_list = 0x95bc1a0;
		inline constexpr uintptr_t observer_list = 0x6b535f8;
		inline constexpr uintptr_t cl_fov_scale = 0x370c3d0;
		inline constexpr uintptr_t cinput = 0x7083088;
		inline constexpr uintptr_t command_number = 0x368113c;

		inline constexpr std::uint32_t origin = 0x170;
		inline constexpr std::uint32_t view_angles = 0x25c8;
		inline constexpr std::uint32_t camera_origin = 0x1f78;
		inline constexpr std::uint32_t last_visible_time = 0x1a6c;
		inline constexpr std::uint32_t studio_hdr = 0xfd0;
		inline constexpr std::uint32_t observer_index = 0x94;

		inline constexpr std::uint32_t team_num = apex_map_metadata::DT_BaseEntity::m_iTeamNum;
		inline constexpr std::uint32_t signifier_name = apex_map_metadata::DT_BaseEntity::m_iSignifierName;
		inline constexpr std::uint32_t health = apex_map_metadata::DT_Player::m_iHealth;
		inline constexpr std::uint32_t max_health = apex_map_metadata::DT_Player::m_iMaxHealth;
		inline constexpr std::uint32_t life_state = apex_map_metadata::DT_Player::m_lifeState;
		inline constexpr std::uint32_t bleedout_state = apex_map_metadata::DT_Player::m_bleedoutState;
		inline constexpr std::uint32_t shield = apex_map_metadata::DT_BaseCombatCharacter::m_shieldHealth;
		inline constexpr std::uint32_t max_shield = apex_map_metadata::DT_BaseCombatCharacter::m_shieldHealthMax;
		inline constexpr std::uint32_t armor_type = apex_map_metadata::DT_Player::m_armorType;
		inline constexpr std::uint32_t duck_state = apex_map_metadata::DT_Player::m_duckState;
		inline constexpr std::uint32_t skydive_state = apex_map_metadata::DT_Player::m_skydiveState;
		inline constexpr std::uint32_t zooming = apex_map_metadata::DT_Player::m_bZooming;
		inline constexpr std::uint32_t squad_id = apex_map_metadata::DT_BaseCombatCharacter::m_squadID;
		inline constexpr std::uint32_t observer_mode = apex_map_metadata::DT_LocalPlayerExclusive::m_iObserverMode;
		inline constexpr std::uint32_t observer_target = apex_map_metadata::DT_LocalPlayerExclusive::m_hObserverTarget;
		inline constexpr std::uint32_t flags = apex_map_metadata::DT_Player::m_fFlags;
		inline constexpr std::uint32_t abs_velocity = apex_map_metadata::DT_LocalPlayerExclusive::m_vecAbsVelocity;
		inline constexpr std::uint32_t latest_primary_weapons = apex_map_metadata::DT_BaseCombatCharacter::m_latestPrimaryWeapons;
		inline constexpr std::uint32_t weapon_player_data = apex_map_metadata::DT_WeaponX::m_playerData;
		inline constexpr std::uint32_t cur_zoom_fov = apex_map_metadata::DT_WeaponPlayerData::m_curZoomFOV;
		inline constexpr std::uint32_t target_zoom_fov = apex_map_metadata::DT_WeaponPlayerData::m_targetZoomFOV;
		inline constexpr std::uint32_t weapon_skin = apex_map_metadata::DT_BaseAnimating::m_nSkin;
		inline constexpr std::uint32_t weapon_skin_mod = apex_map_metadata::DT_BaseAnimating::m_skinMod;
		inline constexpr std::uint32_t weapon_skin_override = apex_map_metadata::DT_WeaponX::m_skinOverride;
		inline constexpr std::uint32_t weapon_skin_valid = apex_map_metadata::DT_WeaponX::m_skinOverrideIsValid;
		inline constexpr std::uint32_t weapon_ammo = apex_map_metadata::DT_WeaponX_LocalWeaponData::m_ammoInClip;
		inline constexpr std::uint32_t punch_angle = apex_map_metadata::DT_LocalPlayerExclusive::m_currentFrameLocalPlayer + apex_map_metadata::DT_CurrentData_LocalPlayer::m_vecPunchWeapon_Angle;
		inline constexpr std::uint32_t third_person_shoulder = apex_map_metadata::DT_LocalPlayerExclusive::m_thirdPersonShoulderView;
		inline constexpr std::uint32_t force_bone = apex_map_metadata::DT_BaseAnimating::m_nForceBone;
		inline constexpr std::uint32_t bone_array = force_bone + 0x48;
		inline constexpr std::uint32_t view_models = apex_map_metadata::DT_Player::m_hViewModels;
		inline constexpr std::uint32_t vm_model_index = apex_map_metadata::DT_BaseViewModel::m_nModelIndex;
		inline constexpr std::uint32_t vm_anim_model_index = apex_map_metadata::DT_BaseViewModel::m_animModelIndex;
		inline constexpr std::uint32_t weapon_name_index = apex_map_metadata::DT_WeaponX::m_weaponNameIndex;
		inline constexpr std::uint32_t prop_custom_script_int = apex_map_metadata::DT_PropSurvival::m_customScriptInt;
		inline constexpr std::uint32_t prop_weapon_index = apex_map_metadata::DT_PropSurvival::m_weaponNameIndex;
		inline constexpr std::uint32_t prop_local_origin = apex_map_metadata::DT_PropSurvival::m_localOrigin;
		inline constexpr std::uint32_t move_spread = apex_map_metadata::DT_WeaponPlayerData::m_moveSpread;
		inline constexpr std::uint32_t spread_hip = apex_map_metadata::DT_WeaponPlayerData::m_spreadStartFracHip;
		inline constexpr std::uint32_t spread_ads = apex_map_metadata::DT_WeaponPlayerData::m_spreadStartFracADS;
		inline constexpr std::uint32_t kick_spread_hip = apex_map_metadata::DT_WeaponPlayerData::m_kickSpreadHipfire;
		inline constexpr std::uint32_t kick_spread_ads = apex_map_metadata::DT_WeaponPlayerData::m_kickSpreadADS;
		inline constexpr std::uint32_t laser_spread = apex_map_metadata::DT_WeaponX::m_sustainedLaserCurrentSpread;
		inline constexpr std::uint32_t grade = apex_map_metadata::DT_BaseEntity::m_grade;
		inline constexpr std::uint32_t ent_name = apex_map_metadata::DT_BaseEntity::m_iName;
		inline constexpr std::uint32_t collision_maxs = apex_map_metadata::DT_BaseEntity::m_Collision + apex_map_metadata::DT_CollisionProperty::m_vecMaxs;
		inline constexpr std::uint32_t traversal_progress = apex_map_metadata::DT_LocalPlayerExclusive::m_traversalProgress;
	}

	inline std::uint32_t g_studio_hdr_off = offsets::studio_hdr;
	inline uintptr_t g_observer_list_ptr = 0;
	inline uintptr_t g_cl_fov_scale_ptr = 0;
	inline uintptr_t g_thirdperson_ptr = 0;
	inline uintptr_t g_game_base = 0;
	inline bool g_initialized = false;

	inline bool init( ) {
		if ( g_initialized && g_game_base )
			return true;

		g_game_base = reinterpret_cast< uintptr_t >( GetModuleHandleA( nullptr ) );
		if ( g_game_base ) {
			uintptr_t str_fov = 0;
			uintptr_t str_tp = 0;
			auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( g_game_base );
			auto* nt = ( dos && dos->e_magic == IMAGE_DOS_SIGNATURE )
				? reinterpret_cast< const IMAGE_NT_HEADERS64* >( g_game_base + dos->e_lfanew )
				: nullptr;
			if ( nt && nt->Signature == IMAGE_NT_SIGNATURE ) {
				uintptr_t img_end = g_game_base + nt->OptionalHeader.SizeOfImage;
				auto* sec = IMAGE_FIRST_SECTION( nt );
				bool studio_found = false;

				for ( WORD s = 0; s < nt->FileHeader.NumberOfSections; ++s ) {
					DWORD vsize = sec[ s ].Misc.VirtualSize ? sec[ s ].Misc.VirtualSize : sec[ s ].SizeOfRawData;
					if ( !sec[ s ].VirtualAddress || !vsize )
						continue;
					uintptr_t sec_start = g_game_base + sec[ s ].VirtualAddress;
					uintptr_t sec_end = sec_start + vsize;
					if ( sec_end > img_end ) sec_end = img_end;
					if ( sec_end <= sec_start ) continue;
					size_t sec_len = static_cast< size_t >( sec_end - sec_start );

					bool is_exec = ( sec[ s ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
					bool is_write = ( sec[ s ].Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
					bool is_read = ( sec[ s ].Characteristics & IMAGE_SCN_MEM_READ ) != 0;
					if ( !is_read ) continue;

					MEMORY_BASIC_INFORMATION mbi{};
					if ( !VirtualQuery( reinterpret_cast< LPCVOID >( sec_start ), &mbi, sizeof( mbi ) ) ||
						 mbi.State != MEM_COMMIT || ( mbi.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) ) )
						continue;

					if ( is_exec && ( !studio_found || !g_observer_list_ptr ) ) {
						const std::uint8_t* p = reinterpret_cast< const std::uint8_t* >( sec_start );
						size_t len = sec_len > 0x40 ? sec_len - 0x20 : 0;
						if ( len > 0x3000000 ) len = 0x3000000;
						for ( size_t i = 0; i < len; ++i ) {
							if ( !studio_found &&
								 p[ i ] == 0x48 && p[ i + 1 ] == 0x8B && p[ i + 2 ] == 0x9F &&
								 p[ i + 7 ] == 0x48 && p[ i + 8 ] == 0x85 && p[ i + 9 ] == 0xDB &&
								 p[ i + 10 ] == 0x75 && p[ i + 12 ] == 0x48 && p[ i + 13 ] == 0x8D && p[ i + 14 ] == 0x0D ) {
								std::uint32_t off = *reinterpret_cast< const std::uint32_t* >( p + i + 3 );
								if ( off >= 0x800 && off <= 0x2000 ) {
									g_studio_hdr_off = off;
									studio_found = true;
								}
							}
							if ( !g_observer_list_ptr &&
								 p[ i ] == 0x48 && p[ i + 1 ] == 0x8B && p[ i + 2 ] == 0x0D &&
								 p[ i + 7 ] == 0x48 && p[ i + 8 ] == 0x85 && p[ i + 9 ] == 0xC9 &&
								 p[ i + 10 ] == 0x74 && p[ i + 12 ] == 0x48 && p[ i + 13 ] == 0x8B && p[ i + 14 ] == 0x01 &&
								 p[ i + 15 ] == 0xFF && p[ i + 16 ] == 0x50 ) {
								std::int32_t rel = *reinterpret_cast< const std::int32_t* >( p + i + 3 );
								uintptr_t target = sec_start + i + 7 + rel;
								if ( target > g_game_base && target < img_end )
									g_observer_list_ptr = target;
							}
							if ( studio_found && g_observer_list_ptr )
								break;
						}
					}
					else if ( !is_write && ( !str_fov || !str_tp ) ) {
						const char* p = reinterpret_cast< const char* >( sec_start );
						size_t len = sec_len > 24 ? sec_len - 24 : 0;
						for ( size_t i = 0; i < len; ++i ) {
							if ( !str_fov && p[ i ] == 'c' && p[ i + 1 ] == 'l' && p[ i + 2 ] == '_' &&
								 p[ i + 3 ] == 'f' && p[ i + 4 ] == 'o' && p[ i + 5 ] == 'v' &&
								 p[ i + 6 ] == 'S' && p[ i + 7 ] == 'c' && p[ i + 8 ] == 'a' &&
								 p[ i + 9 ] == 'l' && p[ i + 10 ] == 'e' && p[ i + 11 ] == '\0' ) {
								str_fov = sec_start + i;
							}
							else if ( !str_tp && p[ i ] == 't' && p[ i + 1 ] == 'h' && p[ i + 2 ] == 'i' &&
									  p[ i + 3 ] == 'r' && p[ i + 4 ] == 'd' && p[ i + 5 ] == 'p' &&
									  p[ i + 6 ] == 'e' && p[ i + 7 ] == 'r' && p[ i + 8 ] == 's' &&
									  p[ i + 9 ] == 'o' && p[ i + 10 ] == 'n' && p[ i + 11 ] == '_' &&
									  p[ i + 12 ] == 'o' && p[ i + 13 ] == 'v' && p[ i + 14 ] == 'e' &&
									  p[ i + 15 ] == 'r' && p[ i + 16 ] == 'r' && p[ i + 17 ] == 'i' &&
									  p[ i + 18 ] == 'd' && p[ i + 19 ] == 'e' && p[ i + 20 ] == '\0' ) {
								str_tp = sec_start + i;
							}
							if ( str_fov && str_tp ) break;
						}
					}
				}

				if ( str_fov || str_tp ) {
					for ( WORD s = 0; s < nt->FileHeader.NumberOfSections && ( ( str_fov && !g_cl_fov_scale_ptr ) || ( str_tp && !g_thirdperson_ptr ) ); ++s ) {
						bool is_write = ( sec[ s ].Characteristics & IMAGE_SCN_MEM_WRITE ) != 0;
						bool is_exec = ( sec[ s ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) != 0;
						if ( !is_write || is_exec ) continue;
						DWORD vsize = sec[ s ].Misc.VirtualSize ? sec[ s ].Misc.VirtualSize : sec[ s ].SizeOfRawData;
						if ( !sec[ s ].VirtualAddress || vsize < 0x80 ) continue;
						uintptr_t sec_start = g_game_base + sec[ s ].VirtualAddress;
						uintptr_t sec_end = sec_start + vsize;
						if ( sec_end > img_end ) sec_end = img_end;

						uintptr_t cur = sec_start;
						MEMORY_BASIC_INFORMATION mbi{};
						while ( cur < sec_end && ( ( str_fov && !g_cl_fov_scale_ptr ) || ( str_tp && !g_thirdperson_ptr ) ) &&
								VirtualQuery( reinterpret_cast< LPCVOID >( cur ), &mbi, sizeof( mbi ) ) ) {
							uintptr_t reg_start = reinterpret_cast< uintptr_t >( mbi.BaseAddress );
							uintptr_t reg_end = reg_start + mbi.RegionSize;
							if ( reg_start < cur ) reg_start = cur;
							if ( reg_end > sec_end ) reg_end = sec_end;
							size_t rsize = ( reg_end > reg_start ) ? static_cast< size_t >( reg_end - reg_start ) : 0;
							if ( mbi.State == MEM_COMMIT && ( mbi.Protect & PAGE_READWRITE ) && !( mbi.Protect & PAGE_GUARD ) && rsize > 0x80 ) {
								size_t count = ( rsize - 0x80 ) / 8;
								const uintptr_t* q = reinterpret_cast< const uintptr_t* >( reg_start );
								for ( size_t i = 0; i < count; ++i ) {
									if ( str_fov && !g_cl_fov_scale_ptr && q[ i ] == str_fov ) {
										uintptr_t base_q = reg_start + i * 8;
										for ( uintptr_t off = 0x48; off <= 0x60; off += 4 ) {
											float v = *reinterpret_cast< const float* >( base_q + off );
											if ( v >= 0.99f && v <= 1.75f ) {
												g_cl_fov_scale_ptr = base_q + off;
												break;
											}
										}
									}
									if ( str_tp && !g_thirdperson_ptr && q[ i ] == str_tp ) {
										uintptr_t base_q = reg_start + i * 8;
										for ( uintptr_t off = 0x48; off <= 0x64; off += 4 ) {
											float fv = *reinterpret_cast< const float* >( base_q + off );
											int iv = *reinterpret_cast< const int* >( base_q + off + 4 );
											if ( fv == -1.0f && iv == -1 ) {
												g_thirdperson_ptr = base_q + off;
												break;
											}
										}
									}
								}
							}
							if ( mbi.RegionSize == 0 || reg_end <= cur ) break;
							cur = reg_end;
						}
					}
				}
			}
		}
		g_initialized = ( g_game_base != 0 );
		return g_initialized;
	}

	inline bool is_addr_valid( uintptr_t p ) {
		return p >= 0x10000 && p <= 0x00007FFFFFFFFFFF;
	}

	inline bool is_ptr_valid( uintptr_t p ) {
		return is_addr_valid( p ) && ( p & 0x7 ) == 0;
	}

	inline constexpr size_t PAGE_CACHE_SIZE = 4096;
	struct page_cache_entry_t {
		uintptr_t page;
		std::uint8_t state; // 0 = empty, 1 = readable, 2 = unreadable
	};
	inline page_cache_entry_t g_page_cache[ PAGE_CACHE_SIZE ]{};
	inline ULONGLONG g_page_cache_time = 0;

	inline void reset_page_cache( ) {
		for ( size_t i = 0; i < PAGE_CACHE_SIZE; ++i )
			g_page_cache[ i ] = {};
		g_page_cache_time = GetTickCount64( );
	}

	inline void clear_page_cache( ) {
		reset_page_cache( );
	}

	inline bool is_page_readable( uintptr_t page ) {
		size_t idx = ( page ^ ( page >> 12 ) ) & ( PAGE_CACHE_SIZE - 1 );
		if ( g_page_cache[ idx ].page == page && g_page_cache[ idx ].state != 0 )
			return g_page_cache[ idx ].state == 1;

		MEMORY_BASIC_INFORMATION mbi{};
		bool ok = false;
		if ( VirtualQuery( reinterpret_cast< LPCVOID >( page << 12 ), &mbi, sizeof( mbi ) ) ) {
			constexpr DWORD k_read_mask = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
				PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
			ok = ( mbi.State == MEM_COMMIT ) &&
				 !( mbi.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) ) &&
				 ( ( mbi.Protect & k_read_mask ) != 0 );

			uintptr_t base_page = reinterpret_cast< uintptr_t >( mbi.BaseAddress ) >> 12;
			uintptr_t end_page = ( reinterpret_cast< uintptr_t >( mbi.BaseAddress ) + mbi.RegionSize ) >> 12;
			if ( end_page > page + 16 ) end_page = page + 16;
			std::uint8_t st = ok ? 1 : 2;
			for ( uintptr_t p = ( base_page > page ? base_page : page ); p < end_page; ++p ) {
				size_t p_idx = ( p ^ ( p >> 12 ) ) & ( PAGE_CACHE_SIZE - 1 );
				g_page_cache[ p_idx ].page = p;
				g_page_cache[ p_idx ].state = st;
			}
		}
		else {
			g_page_cache[ idx ].page = page;
			g_page_cache[ idx ].state = 2;
		}
		return ok;
	}

	inline bool is_mem_readable( uintptr_t addr, size_t size ) {
		if ( !is_addr_valid( addr ) || size == 0 || addr + size < addr || !is_addr_valid( addr + size - 1 ) )
			return false;
		uintptr_t start_page = addr >> 12;
		uintptr_t end_page = ( addr + size - 1 ) >> 12;
		for ( uintptr_t p = start_page; p <= end_page; ++p ) {
			if ( !is_page_readable( p ) )
				return false;
		}
		return true;
	}

	inline bool is_mem_writable( uintptr_t addr, size_t size ) {
		if ( !is_addr_valid( addr ) || size == 0 || addr + size < addr )
			return false;
		MEMORY_BASIC_INFORMATION mbi{};
		if ( !VirtualQuery( reinterpret_cast< LPCVOID >( addr ), &mbi, sizeof( mbi ) ) )
			return false;
		if ( mbi.State != MEM_COMMIT )
			return false;
		if ( mbi.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) )
			return false;
		constexpr DWORD k_write_mask = PAGE_READWRITE | PAGE_WRITECOPY |
			PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
		if ( !( mbi.Protect & k_write_mask ) )
			return false;
		uintptr_t region_end = reinterpret_cast< uintptr_t >( mbi.BaseAddress ) + mbi.RegionSize;
		return addr + size <= region_end;
	}

	inline bool is_sig_ptr_readable( uintptr_t sig_ptr ) {
		return is_mem_readable( sig_ptr, 8 );
	}

	template<typename T>
	inline bool safe_read( uintptr_t addr, T& out ) {
		if ( sizeof( T ) <= 16 ) {
			if ( !is_sig_ptr_readable( addr ) )
				return false;
		}
		else if ( !is_mem_readable( addr, sizeof( T ) ) ) {
			return false;
		}
		out = *reinterpret_cast< const T* >( addr );
		return true;
	}

	template<typename T>
	inline bool checked_read( uintptr_t addr, T& out ) {
		if ( !is_mem_readable( addr, sizeof( T ) ) )
			return false;
		out = *reinterpret_cast< const T* >( addr );
		return true;
	}

	template<typename T>
	inline bool safe_write( uintptr_t addr, const T& val ) {
		if ( !is_mem_writable( addr, sizeof( T ) ) )
			return false;
		*reinterpret_cast< T* >( addr ) = val;
		return true;
	}

	inline uintptr_t get_local_player( ) {
		uintptr_t lp = 0;
		if ( !g_game_base || !safe_read( g_game_base + offsets::local_player, lp ) )
			return 0;
		return is_ptr_valid( lp ) ? lp : 0;
	}

	inline uintptr_t get_entity( int index ) {
		uintptr_t ent = 0;
		if ( !g_game_base || index < 0 || index > 65535 )
			return 0;
		uintptr_t slot = g_game_base + offsets::entity_list + ( static_cast< uintptr_t >( index ) << 5 );
		if ( !safe_read( slot, ent ) )
			return 0;
		return is_ptr_valid( ent ) ? ent : 0;
	}

	inline bool get_view_matrix( matrix4x4_t& out ) {
		uintptr_t vr = 0;
		if ( !g_game_base || !checked_read( g_game_base + offsets::view_render, vr ) || !is_ptr_valid( vr ) )
			return false;
		uintptr_t vm_ptr = 0;
		if ( !checked_read( vr + offsets::view_matrix, vm_ptr ) || !is_ptr_valid( vm_ptr ) )
			return false;
		return checked_read( vm_ptr, out );
	}

	enum class ent_kind : std::uint8_t {
		none = 0,
		player = 1,
		npc = 2,
		loot = 3,
		deathbox = 4,
		projectile = 5
	};

	inline bool str_contains( const char* s, const char* sub ) {
		for ( int i = 0; i < 24 && s[ i ]; ++i ) {
			int j = 0;
			while ( sub[ j ] && s[ i + j ] == sub[ j ] ) ++j;
			if ( !sub[ j ] ) return true;
		}
		return false;
	}

	inline ent_kind classify_entity( uintptr_t ent ) {
		uintptr_t sig_ptr = 0;
		if ( !safe_read( ent + offsets::signifier_name, sig_ptr ) )
			return ent_kind::none;
		if ( !is_sig_ptr_readable( sig_ptr ) )
			return ent_kind::none;
		const char* s = reinterpret_cast< const char* >( sig_ptr );
		if ( s[ 0 ] == 'p' && s[ 1 ] == 'l' && s[ 2 ] == 'a' && s[ 3 ] == 'y' && s[ 4 ] == 'e' && s[ 5 ] == 'r' && s[ 6 ] == '\0' )
			return ent_kind::player;
		if ( str_contains( s, "grenade" ) || str_contains( s, "projectile" ) || str_contains( s, "arc_star" ) )
			return ent_kind::projectile;
		if ( s[ 0 ] == 'n' && s[ 1 ] == 'p' && s[ 2 ] == 'c' && s[ 3 ] == '_' )
			return ent_kind::npc;
		if ( s[ 0 ] == 'p' && s[ 1 ] == 'r' && s[ 2 ] == 'o' && s[ 3 ] == 'p' && s[ 4 ] == '_' ) {
			if ( s[ 5 ] == 's' && s[ 6 ] == 'u' && s[ 7 ] == 'r' && s[ 8 ] == 'v' )
				return ent_kind::loot;
			if ( s[ 5 ] == 'd' && s[ 6 ] == 'e' && s[ 7 ] == 'a' && s[ 8 ] == 't' && s[ 9 ] == 'h' )
				return ent_kind::deathbox;
		}
		return ent_kind::none;
	}

	inline bool is_entity_visible( int idx, uintptr_t ent ) {
		if ( idx <= 0 || idx > 512 )
			return true;
		float vt = 0.0f;
		if ( !safe_read( ent + offsets::last_visible_time, vt ) || vt <= 0.0f )
			return false;
		static float s_prev_vt[ 513 ]{};
		static ULONGLONG s_last_seen[ 513 ]{};
		ULONGLONG now = GetTickCount64( );
		if ( vt > s_prev_vt[ idx ] ) {
			s_prev_vt[ idx ] = vt;
			s_last_seen[ idx ] = now;
			return true;
		}
		if ( vt < s_prev_vt[ idx ] - 5.0f )
			s_prev_vt[ idx ] = vt;
		return ( now - s_last_seen[ idx ] ) < 250;
	}

	inline bool get_entity_origin( uintptr_t ent, vec3_t& out ) {
		if ( !ent )
			return false;
		if ( safe_read( ent + offsets::origin, out ) ) {
			bool finite = ( out.x == out.x && out.y == out.y && out.z == out.z );
			bool in_bounds = fabsf( out.x ) < 200000.0f && fabsf( out.y ) < 200000.0f && fabsf( out.z ) < 200000.0f;
			if ( finite && in_bounds && ( out.x != 0.0f || out.y != 0.0f || out.z != 0.0f ) )
				return true;
		}
		if ( safe_read( ent + offsets::prop_local_origin, out ) ) {
			bool finite = ( out.x == out.x && out.y == out.y && out.z == out.z );
			bool in_bounds = fabsf( out.x ) < 200000.0f && fabsf( out.y ) < 200000.0f && fabsf( out.z ) < 200000.0f;
			if ( finite && in_bounds && ( out.x != 0.0f || out.y != 0.0f || out.z != 0.0f ) )
				return true;
		}
		return false;
	}

	inline bool is_spectating_local( int idx, uintptr_t ent, uintptr_t local ) {
		uintptr_t obs_ptr = g_observer_list_ptr ? g_observer_list_ptr : ( g_game_base ? g_game_base + offsets::observer_list : 0 );
		if ( obs_ptr ) {
			uintptr_t obs_list = 0;
			if ( safe_read( obs_ptr, obs_list ) && is_ptr_valid( obs_list ) && is_sig_ptr_readable( obs_list ) ) {
				int obs_idx = idx;
				int ent_idx = 0;
				if ( safe_read( ent + 0x38, ent_idx ) && ent_idx > 0 && ent_idx < 256 )
					obs_idx = ent_idx;
				uintptr_t slot_addr = obs_list + static_cast< uintptr_t >( obs_idx ) * 0x10 + 0x940;
				uintptr_t val = 0;
				if ( is_sig_ptr_readable( slot_addr ) && safe_read( slot_addr, val ) && val ) {
					std::uint32_t handle = 0xFFFFFFFF;
					if ( is_ptr_valid( val ) && is_sig_ptr_readable( val ) )
						safe_read( val, handle );
					else
						handle = static_cast< std::uint32_t >( val );
					if ( handle != 0xFFFFFFFF && get_entity( handle & 0xFFFF ) == local )
						return true;
				}
			}
		}
		int obs_mode = 0;
		std::uint32_t obs_handle = 0;
		safe_read( ent + offsets::observer_mode, obs_mode );
		safe_read( ent + offsets::observer_target, obs_handle );
		return obs_mode > 0 && obs_handle != 0xFFFFFFFF && get_entity( obs_handle & 0xFFFF ) == local;
	}

	inline bool is_player_entity( uintptr_t ent ) {
		ent_kind k = classify_entity( ent );
		return k == ent_kind::player || k == ent_kind::npc;
	}

	struct bone_matrix3x4_t {
		float m[ 3 ][ 4 ];
	};

	inline bool read_bone_pos( uintptr_t bone_arr, int bone_id, const vec3_t& origin, bool add_origin, vec3_t& out ) {
		if ( !bone_arr || bone_id < 0 || bone_id >= 256 )
			return false;
		uintptr_t mat = bone_arr + static_cast< uintptr_t >( bone_id ) * 0x30;
		if ( !is_sig_ptr_readable( mat ) || !is_sig_ptr_readable( mat + 0x20 ) )
			return false;
		float bx = *reinterpret_cast< const float* >( mat + 0x0C );
		float by = *reinterpret_cast< const float* >( mat + 0x1C );
		float bz = *reinterpret_cast< const float* >( mat + 0x2C );
		if ( add_origin ) {
			out.x = origin.x + bx;
			out.y = origin.y + by;
			out.z = origin.z + bz;
		}
		else {
			out.x = bx;
			out.y = by;
			out.z = bz;
		}
		return true;
	}

	inline std::uint32_t g_found_bone_off = 0;

	inline bool is_bone_array_valid( uintptr_t bone_arr, const vec3_t& origin ) {
		if ( !is_ptr_valid( bone_arr ) || !is_mem_readable( bone_arr, 16 * sizeof( bone_matrix3x4_t ) ) )
			return false;
		int rot_ok = 0;
		int local_cnt = 0, world_cnt = 0;
		float l_min_z = 1e9f, l_max_z = -1e9f;
		float w_min_z = 1e9f, w_max_z = -1e9f;
		for ( int b = 0; b < 16; ++b ) {
			uintptr_t mat = bone_arr + static_cast< uintptr_t >( b ) * 0x30;
			const auto& bm = *reinterpret_cast< const bone_matrix3x4_t* >( mat );
			float r0 = bm.m[ 0 ][ 0 ] * bm.m[ 0 ][ 0 ] + bm.m[ 0 ][ 1 ] * bm.m[ 0 ][ 1 ] + bm.m[ 0 ][ 2 ] * bm.m[ 0 ][ 2 ];
			float r1 = bm.m[ 1 ][ 0 ] * bm.m[ 1 ][ 0 ] + bm.m[ 1 ][ 1 ] * bm.m[ 1 ][ 1 ] + bm.m[ 1 ][ 2 ] * bm.m[ 1 ][ 2 ];
			float r2 = bm.m[ 2 ][ 0 ] * bm.m[ 2 ][ 0 ] + bm.m[ 2 ][ 1 ] * bm.m[ 2 ][ 1 ] + bm.m[ 2 ][ 2 ] * bm.m[ 2 ][ 2 ];
			float d01 = bm.m[ 0 ][ 0 ] * bm.m[ 1 ][ 0 ] + bm.m[ 0 ][ 1 ] * bm.m[ 1 ][ 1 ] + bm.m[ 0 ][ 2 ] * bm.m[ 1 ][ 2 ];
			if ( r0 < 0.85f || r0 > 1.15f || r1 < 0.85f || r1 > 1.15f || r2 < 0.85f || r2 > 1.15f || fabsf( d01 ) >= 0.15f )
				continue;
			++rot_ok;
			vec3_t p{ bm.m[ 0 ][ 3 ], bm.m[ 1 ][ 3 ], bm.m[ 2 ][ 3 ] };
			if ( !( p.x == p.x && p.y == p.y && p.z == p.z ) )
				continue;
			if ( fabsf( p.x ) <= 95.0f && fabsf( p.y ) <= 95.0f && p.z >= -20.0f && p.z <= 95.0f &&
				 ( p.x != 0.0f || p.y != 0.0f || p.z != 0.0f ) ) {
				++local_cnt;
				if ( p.z < l_min_z ) l_min_z = p.z;
				if ( p.z > l_max_z ) l_max_z = p.z;
			}
			if ( fabsf( p.x - origin.x ) <= 95.0f && fabsf( p.y - origin.y ) <= 95.0f && fabsf( p.z - origin.z ) <= 105.0f ) {
				++world_cnt;
				if ( p.z < w_min_z ) w_min_z = p.z;
				if ( p.z > w_max_z ) w_max_z = p.z;
			}
		}
		if ( rot_ok < 6 )
			return false;
		if ( local_cnt >= 6 && ( l_max_z - l_min_z ) >= 20.0f && ( l_max_z - l_min_z ) <= 95.0f )
			return true;
		if ( world_cnt >= 6 && ( w_max_z - w_min_z ) >= 20.0f && ( w_max_z - w_min_z ) <= 95.0f )
			return true;
		return false;
	}

	inline uintptr_t get_bone_array( uintptr_t ent, const vec3_t& origin ) {
		static int s_invalid_streak = 0;
		static ULONGLONG s_last_valid_tick = 0;

		if ( g_found_bone_off ) {
			uintptr_t bone_arr = 0;
			if ( safe_read( ent + g_found_bone_off, bone_arr ) && is_bone_array_valid( bone_arr, origin ) ) {
				s_invalid_streak = 0;
				s_last_valid_tick = GetTickCount64( );
				return bone_arr;
			}
			++s_invalid_streak;
			if ( s_invalid_streak >= 32 && ( GetTickCount64( ) - s_last_valid_tick ) > 2000 ) {
				g_found_bone_off = 0;
				s_invalid_streak = 0;
			}
			return 0;
		}
		uintptr_t def_arr = 0;
		if ( safe_read( ent + offsets::bone_array, def_arr ) ) {
			if ( is_bone_array_valid( def_arr, origin ) ) {
				g_found_bone_off = offsets::bone_array;
				s_invalid_streak = 0;
				s_last_valid_tick = GetTickCount64( );
				return def_arr;
			}
		}
		static ULONGLONG s_last_probe = 0;
		ULONGLONG now = GetTickCount64( );
		if ( now - s_last_probe < 250 )
			return 0;
		s_last_probe = now;
		for ( std::uint32_t off = offsets::force_bone; off <= offsets::studio_hdr + 0x100; off += 8 ) {
			if ( off == offsets::studio_hdr || off == g_studio_hdr_off )
				continue;
			uintptr_t bone_arr = 0;
			if ( safe_read( ent + off, bone_arr ) && is_bone_array_valid( bone_arr, origin ) ) {
				g_found_bone_off = off;
				s_invalid_streak = 0;
				s_last_valid_tick = now;
				return bone_arr;
			}
		}
		return 0;
	}

	inline bool find_head_bone( uintptr_t bone_arr, const vec3_t& origin, bool& out_add_origin, vec3_t& out_head ) {
		if ( !bone_arr )
			return false;

		vec3_t ref_p{};
		bool have_ref = false;
		for ( int rb = 0; rb <= 5; ++rb ) {
			if ( read_bone_pos( bone_arr, rb, origin, false, ref_p ) ) {
				have_ref = true;
				break;
			}
		}
		if ( !have_ref )
			return false;

		float d_local = fabsf( ref_p.x ) + fabsf( ref_p.y ) + fabsf( ref_p.z );
		float d_world = fabsf( ref_p.x - origin.x ) + fabsf( ref_p.y - origin.y ) + fabsf( ref_p.z - origin.z );
		out_add_origin = ( d_local + 50.0f < d_world );
		if ( !out_add_origin && fabsf( ref_p.x ) < 95.0f && fabsf( ref_p.y ) < 95.0f && fabsf( ref_p.z ) < 95.0f &&
			 ( fabsf( origin.x ) > 120.0f || fabsf( origin.y ) > 120.0f || fabsf( origin.z ) > 120.0f ) ) {
			out_add_origin = true;
		}

		static constexpr int k_head_cands[ 6 ] = { 8, 7, 10, 6, 5, 12 };
		for ( int ci = 0; ci < 6; ++ci ) {
			vec3_t p{};
			if ( !read_bone_pos( bone_arr, k_head_cands[ ci ], origin, out_add_origin, p ) )
				continue;
			float dx = p.x - origin.x;
			float dy = p.y - origin.y;
			float dz = p.z - origin.z;
			if ( dz >= 16.0f && dz <= 84.0f && ( dx * dx + dy * dy ) <= ( 45.0f * 45.0f ) ) {
				if ( k_head_cands[ ci ] == 8 || dz >= 40.0f ) {
					out_head = p;
					return true;
				}
			}
		}

		float best_z = -1e9f;
		vec3_t best_world{};
		bool found = false;
		for ( int b = 0; b <= 12; ++b ) {
			vec3_t p{};
			if ( !read_bone_pos( bone_arr, b, origin, out_add_origin, p ) )
				continue;
			float dx = p.x - origin.x;
			float dy = p.y - origin.y;
			float dz = p.z - origin.z;
			if ( dz >= 16.0f && dz <= 84.0f && ( dx * dx + dy * dy ) <= ( 45.0f * 45.0f ) && p.z > best_z ) {
				best_z = p.z;
				best_world = p;
				found = true;
			}
		}
		if ( !found )
			return false;
		out_head = best_world;
		return true;
	}

	inline bool get_player_name( int index, char* out, int max_len ) {
		if ( !g_game_base || index <= 0 || !out || max_len <= 1 )
			return false;

		const uintptr_t addrs[ 4 ] = {
			g_game_base + offsets::name_list + static_cast< uintptr_t >( index - 1 ) * 0x18,
			g_game_base + offsets::name_list + static_cast< uintptr_t >( index ) * 0x18,
			g_game_base + offsets::name_list + static_cast< uintptr_t >( index - 1 ) * 0x10,
			g_game_base + offsets::name_list + static_cast< uintptr_t >( index ) * 0x10
		};

		for ( int ai = 0; ai < 4; ++ai ) {
			uintptr_t name_ptr = 0;
			if ( !safe_read( addrs[ ai ], name_ptr ) || !is_addr_valid( name_ptr ) )
				continue;
			if ( !is_mem_readable( name_ptr, 1 ) )
				continue;

			int i = 0;
			bool valid = true;
			for ( ; i < max_len - 1 && i < 31; ++i ) {
				if ( i > 0 && ( ( name_ptr + i ) & 0xFFF ) == 0 && !is_mem_readable( name_ptr + i, 1 ) ) {
					valid = false;
					break;
				}
				char ch = *reinterpret_cast< const char* >( name_ptr + i );
				if ( ch == '\0' )
					break;
				if ( static_cast< unsigned char >( ch ) < 0x20 || static_cast< unsigned char >( ch ) > 0x7E ) {
					valid = false;
					break;
				}
				out[ i ] = ch;
			}
			if ( valid && i > 0 ) {
				out[ i ] = '\0';
				return true;
			}
		}
		return false;
	}

	inline vec3_t calc_angle( const vec3_t& src, const vec3_t& dst ) {
		float dx = dst.x - src.x;
		float dy = dst.y - src.y;
		float dz = dst.z - src.z;
		float hyp = sqrtf( dx * dx + dy * dy );
		vec3_t ang{};
		ang.x = -atan2f( dz, hyp ) * ( 180.0f / 3.14159265f );
		ang.y = atan2f( dy, dx ) * ( 180.0f / 3.14159265f );
		ang.z = 0.0f;
		if ( ang.x > 89.0f ) ang.x = 89.0f;
		if ( ang.x < -89.0f ) ang.x = -89.0f;
		while ( ang.y > 180.0f ) ang.y -= 360.0f;
		while ( ang.y < -180.0f ) ang.y += 360.0f;
		return ang;
	}

	inline bool apply_silent_usercmd( const vec3_t& aim_ang ) {
		if ( !g_game_base )
			return false;
		int cmd_num = 0;
		if ( !safe_read( g_game_base + offsets::command_number, cmd_num ) || cmd_num <= 0 )
			return false;

		static uintptr_t s_cmds_base = 0;
		static std::uint32_t s_cmd_size = 0;
		if ( !s_cmds_base || !s_cmd_size ) {
			uintptr_t bases[ 2 ] = { g_game_base + offsets::cinput, 0 };
			safe_read( g_game_base + offsets::cinput, bases[ 1 ] );
			static constexpr std::uint32_t k_offs[ 4 ] = { 0xF8, 0xF0, 0x100, 0x108 };
			static constexpr std::uint32_t k_sizes[ 4 ] = { 0x228, 0x220, 0x218, 0x230 };
			for ( int bi = 0; bi < 2 && !s_cmds_base; ++bi ) {
				if ( !is_ptr_valid( bases[ bi ] ) )
					continue;
				for ( int oi = 0; oi < 4 && !s_cmds_base; ++oi ) {
					uintptr_t ptr = 0;
					if ( !safe_read( bases[ bi ] + k_offs[ oi ], ptr ) || !is_ptr_valid( ptr ) )
						continue;
					for ( int si = 0; si < 4; ++si ) {
						std::uint32_t sz = k_sizes[ si ];
						for ( int d = 0; d <= 1; ++d ) {
							int target_num = cmd_num - d;
							uintptr_t cmd = ptr + static_cast< uintptr_t >( target_num % 750 ) * sz;
							int num_read = 0;
							if ( safe_read( cmd, num_read ) && num_read == target_num ) {
								s_cmds_base = ptr;
								s_cmd_size = sz;
								break;
							}
						}
						if ( s_cmds_base )
							break;
					}
				}
			}
		}

		if ( !s_cmds_base || !s_cmd_size )
			return false;

		bool wrote = false;
		for ( int d = -1; d <= 1; ++d ) {
			int n = cmd_num + d;
			if ( n <= 0 )
				continue;
			uintptr_t cmd = s_cmds_base + static_cast< uintptr_t >( n % 750 ) * s_cmd_size;
			int num_read = 0;
			if ( safe_read( cmd, num_read ) && ( num_read == n || d >= 0 ) ) {
				if ( safe_write( cmd + 0x0C, aim_ang ) )
					wrote = true;
			}
		}
		return wrote;
	}

	inline bool world_to_screen( const vec3_t& world, const matrix4x4_t& vm, float screen_w, float screen_h, float& sx, float& sy ) {
		float w = vm.m[ 3 ][ 0 ] * world.x + vm.m[ 3 ][ 1 ] * world.y + vm.m[ 3 ][ 2 ] * world.z + vm.m[ 3 ][ 3 ];
		if ( w < 0.01f )
			return false;

		float x = vm.m[ 0 ][ 0 ] * world.x + vm.m[ 0 ][ 1 ] * world.y + vm.m[ 0 ][ 2 ] * world.z + vm.m[ 0 ][ 3 ];
		float y = vm.m[ 1 ][ 0 ] * world.x + vm.m[ 1 ][ 1 ] * world.y + vm.m[ 1 ][ 2 ] * world.z + vm.m[ 1 ][ 3 ];

		float inv_w = 1.0f / w;
		sx = ( screen_w * 0.5f ) * ( 1.0f + x * inv_w );
		sy = ( screen_h * 0.5f ) * ( 1.0f - y * inv_w );
		return true;
	}
}
