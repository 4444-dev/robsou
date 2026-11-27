// orelia-internal/workspace/game/features/esp/esp.hxx
#pragma once
#include <cmath>
#include <workspace/game/unity_il2cpp.hxx>
#include <workspace/game/features/features.h>
#include <workspace/game/drawing/drawing.h>

namespace esp {
	using vec3_t = unity::vec3_t;
	struct vec2_t { float x, y; };

	enum role_kind_t : int {
		ROLE_COP    = 0,
		ROLE_ROBBER = 1,
		ROLE_CIV    = 2,
		ROLE_TEAM   = 3
	};

	// 19 HumanBodyBones IDs:
	// 0:Head(10), 1:Neck(9), 2:Chest(8), 3:Spine(7), 4:Hips(0),
	// 5:LShoulder(11), 6:LUpperArm(13), 7:LLowerArm(15), 8:LHand(17),
	// 9:RShoulder(12), 10:RUpperArm(14), 11:RLowerArm(16), 12:RHand(18),
	// 13:LUpperLeg(1), 14:LLowerLeg(3), 15:LFoot(5),
	// 16:RUpperLeg(2), 17:RLowerLeg(4), 18:RFoot(6)
	constexpr int k_bone_count = 19;
	inline constexpr int k_human_bones[ k_bone_count ] = {
		10, 9, 8, 7, 0,
		11, 13, 15, 17,
		12, 14, 16, 18,
		1, 3, 5,
		2, 4, 6
	};

	struct player_t {
		void* obj = nullptr;
		int32_t entity_id = -1;
		int32_t vehicle_eid = -1;
		vec3_t head_world{};
		vec3_t neck_world{};
		vec3_t chest_world{};
		vec3_t stomach_world{};
		vec3_t pelvis_world{};
		vec3_t root_world{};
		vec3_t velocity{};
		vec3_t look_dir{ 0.0f, 0.0f, 1.0f };

		vec2_t head_screen{};
		vec2_t neck_screen{};
		vec2_t chest_screen{};
		vec2_t pelvis_screen{};
		vec2_t root_screen{};

		vec2_t bones[ k_bone_count ]{};
		vec3_t bones_world[ k_bone_count ]{};
		bool bone_ok[ k_bone_count ]{};
		bool has_real_skeleton = false;

		// Skeleton rebuilt from the character's own transform hierarchy. This does
		// not depend on an Animator, on a humanoid rig or on any field offset, so it
		// is the path that actually attaches to the player.
		vec2_t skel_p[ 64 ]{};
		bool skel_p_ok[ 64 ]{};
		unsigned char skel_ea[ 64 ]{};
		unsigned char skel_eb[ 64 ]{};
		int skel_edge_count = 0;

		float distance = 0.0f;
		float health = 1.0f;
		float shield_value = 0.0f;
		int role_kind = ROLE_ROBBER;

		char name[ 32 ]{};
		char tag[ 16 ]{};
		char clan[ 32 ]{};
		char weapon[ 32 ]{};

		bool has_shield = false;
		bool in_vehicle = false;
		bool is_crouching = false;
		bool is_knocked = false;
		bool is_team = false;
		bool is_friend = false;
		bool is_dead = false;
		bool is_visible = true;
		bool on_screen = false;
		bool behind = false;
		bool valid = false;
	};

	struct meta_entry_t {
		int32_t entity_id = -1;
		int32_t vehicle_eid = -1;
		vec3_t pos{};
		vec3_t look_dir{ 0.0f, 0.0f, 1.0f };
		int role_kind = ROLE_ROBBER;
		char name[ 32 ]{};
		char tag[ 16 ]{};
		char clan[ 32 ]{};
		char weapon[ 32 ]{};
		bool has_armor = false;
		bool in_vehicle = false;
		bool is_crouching = false;
		bool is_knocked = false;
		bool is_dead = false;
		bool is_local = false;
		bool valid = false;
	};

	enum world_cat_t : int {
		WCAT_CAR       = 0,
		WCAT_HELI      = 1,
		WCAT_LOOSE     = 2,
		WCAT_NODE      = 3,
		WCAT_CONTAINER = 4,
		WCAT_DROPPED   = 5,
		WCAT_VEH_LOOT  = 6,
		WCAT_BUYER     = 7,
		WCAT_DOOR      = 8
	};

	struct world_item_t {
		int32_t entity_id = -1;
		vec3_t world_pos{};
		vec2_t screen_pos{};
		float distance = 0.0f;
		int category = WCAT_LOOSE;
		int rarity = 0; // 0:None,1:Common,2:Uncommon,3:Rare,4:Epic,5:Legendary,6:Exotic
		bool is_empty = false;
		bool is_police_veh = false;
		bool engine_on = false;
		float speed_mps = 0.0f;
		int cargo_count = 0;
		int trunk_count = 0;
		int laundry_eta_s = 0;
		char owner[ 32 ]{};
		char label[ 48 ]{};
		bool valid = false;
	};

	struct world_raw_t {
		int32_t entity_id = -1;
		vec3_t pos{};
		int category = WCAT_LOOSE;
		int rarity = 0;
		bool is_empty = false;
		bool is_police_veh = false;
		bool engine_on = false;
		float speed_mps = 0.0f;
		int cargo_count = 0;
		int trunk_count = 0;
		int laundry_eta_s = 0;
		char owner[ 32 ]{};
		char label[ 48 ]{};
	};

	struct death_point_t {
		vec3_t pos{};
		char name[ 32 ]{};
		ULONGLONG tick = 0;
		bool valid = false;
	};

	struct plot_summary_t {
		bool valid = false;
		char owner[ 32 ]{};
		float distance = 0.0f;
		int cash_laundry = 0;
		int laundry_eta_s = 0;
		int cargo_count = 0;
		int bushes_by_rarity[ 7 ]{};
		int defenses_count = 0;
		int elec_fuses_needed = 0;
		int closed_doors = 0;
		int storages = 0;
		int full_containers = 0;
		int parked_vehicles = 0;
	};

	// Double-buffered snapshots
	inline player_t g_players_buf[ 2 ][ 256 ]{};
	inline int g_player_count_buf[ 2 ]{ 0, 0 };
	inline volatile LONG g_active_buf = 0;

	inline world_item_t g_world_buf[ 2 ][ 256 ]{};
	inline int g_world_count_buf[ 2 ]{ 0, 0 };
	inline volatile LONG g_world_active_buf = 0;

	inline meta_entry_t g_meta[ 128 ]{};
	inline int g_meta_count = 0;
	inline world_raw_t g_world_raw[ 256 ]{};
	inline int g_world_raw_count = 0;
	inline death_point_t g_deaths[ 64 ]{};
	inline plot_summary_t g_nearest_plot{};

	inline void* g_camera = nullptr;
	inline float g_vp[ 16 ]{};
	inline bool g_vp_valid = false;
	inline vec3_t g_cam_pos{};
	inline vec3_t g_cam_prev_pos{};
	inline vec3_t g_cam_vel{};
	inline vec3_t g_cam_fwd{ 0.0f, 0.0f, 1.0f };
	inline vec3_t g_cam_right{ 1.0f, 0.0f, 0.0f };
	inline int32_t g_local_eid = -1;
	inline int32_t g_local_veh_eid = -1;
	inline int g_local_role = ROLE_ROBBER;
	inline char g_local_clan[ 32 ]{};
	inline char g_local_weapon[ 32 ]{};

	struct vel_cache_t {
		void* obj = nullptr;
		vec3_t last_pos{};
		ULONGLONG last_tick = 0;
		vec3_t vel{};
	};
	inline vel_cache_t g_vel_cache[ 128 ]{};

	struct bone_cache_t {
		void* view_obj = nullptr;
		void* animator = nullptr;
		void* nodes[ 64 ]{};
		int node_count = 0;
		unsigned char ea[ 64 ]{};
		unsigned char eb[ 64 ]{};
		int edge_count = 0;
		unsigned char expanded[ 64 ]{};
		void* bone_trs[ k_bone_count ]{};
		uintptr_t native_bone_trs[ k_bone_count ]{};
		bool has_bones = false;
		ULONGLONG resolved_tick = 0;
		ULONGLONG last_try = 0;
	};
	inline bone_cache_t g_bone_cache[ 128 ]{};

	inline void* g_locked_obj = nullptr;
	inline vec2_t g_aim_target_screen{};
	inline vec2_t g_silent_target_screen{};
	inline bool g_has_aim_target = false;
	inline bool g_has_silent_target = false;
	inline float g_aim_rem_x = 0.0f;
	inline float g_aim_rem_y = 0.0f;
	inline bool g_is_firing_or_aiming = false;
	inline volatile ULONGLONG g_last_late_update_tick = 0;

	// Free cam & FOV state
	inline bool g_freecam_active = false;
	inline vec3_t g_freecam_pos{};
	inline float g_orig_fov = 75.0f;

	inline void copy_str( char* dst, size_t cap, const char* src ) {
		if ( !dst || cap == 0 ) return;
		dst[ 0 ] = '\0';
		if ( !src ) return;
		size_t i = 0;
		while ( i + 1 < cap && src[ i ] ) {
			dst[ i ] = src[ i ];
			++i;
		}
		dst[ i ] = '\0';
	}

	inline float dist3( const vec3_t& a, const vec3_t& b ) {
		const float dx = a.x - b.x;
		const float dy = a.y - b.y;
		const float dz = a.z - b.z;
		return d3d11_ui::fast_sqrt( dx * dx + dy * dy + dz * dz );
	}

	inline void draw_esp_text( float x, float y, const char* txt, const d3d11_ui::color_t& c, float scale, bool cx = true, bool cy = true ) {
		if ( config::esp_font_8way ) d3d11_ui::add_text_outlined_8way( x, y, txt, c, scale, cx, cy );
		else d3d11_ui::add_text_outlined( x, y, txt, c, scale, cx, cy );
	}

	inline void build_vp_matrix( const unity::mat4_t& v, const unity::mat4_t& p ) {
		for ( int col = 0; col < 4; ++col ) {
			for ( int row = 0; row < 4; ++row ) {
				float sum = 0.0f;
				for ( int k = 0; k < 4; ++k ) {
					sum += p.m[ k * 4 + row ] * v.m[ col * 4 + k ];
				}
				g_vp[ col * 4 + row ] = sum;
			}
		}
		g_cam_right = { v.m[ 0 ], v.m[ 4 ], v.m[ 8 ] };
		g_cam_fwd   = { -v.m[ 2 ], -v.m[ 6 ], -v.m[ 10 ] };
		g_vp_valid = true;
	}

	inline bool w2s_vp( const vec3_t& w, vec2_t& screen, bool& behind ) {
		if ( !g_vp_valid ) return false;
		const float cx = g_vp[ 0 ] * w.x + g_vp[ 4 ] * w.y + g_vp[ 8 ]  * w.z + g_vp[ 12 ];
		const float cy = g_vp[ 1 ] * w.x + g_vp[ 5 ] * w.y + g_vp[ 9 ]  * w.z + g_vp[ 13 ];
		const float cw = g_vp[ 3 ] * w.x + g_vp[ 7 ] * w.y + g_vp[ 11 ] * w.z + g_vp[ 15 ];

		behind = ( cw <= 0.01f );
		const float inv_w = 1.0f / ( cw > 0.01f ? cw : ( cw < -0.01f ? -cw : 0.01f ) );
		const float ndc_x = cx * inv_w;
		const float ndc_y = cy * inv_w;

		screen.x = ( ndc_x + 1.0f ) * 0.5f * d3d11_ui::g_width;
		screen.y = ( 1.0f - ndc_y ) * 0.5f * d3d11_ui::g_height;
		return true;
	}

	inline bool world_to_screen( const vec3_t& world, vec2_t& screen ) {
		bool behind = false;
		if ( !w2s_vp( world, screen, behind ) ) return false;
		return !behind;
	}

	// The animator does not sit at a fixed offset inside every character view, and
	// the previous path (view+0x28 -> +0x20) was two guesses. It is tried first,
	// then the view's own reference fields are scanned for an instance whose class
	// is exactly Animator: the name check means a wrong pointer can never be handed
	// to GetBoneTransform.
	constexpr int k_skel_max_nodes = 64;

	inline int skel_index( bone_cache_t& bc, void* tr ) {
		for ( int i = 0; i < bc.node_count; ++i ) {
			if ( bc.nodes[ i ] == tr ) return i;
		}
		return -1;
	}

	inline void add_skel_edge( bone_cache_t& bc, void* a, void* b ) {
		if ( !a || !b || bc.edge_count >= k_skel_max_nodes ) return;
		int ia = skel_index( bc, a );
		int ib = skel_index( bc, b );
		if ( ia < 0 ) {
			if ( bc.node_count >= k_skel_max_nodes ) return;
			ia = bc.node_count;
			bc.nodes[ bc.node_count++ ] = a;
		}
		if ( ib < 0 ) {
			if ( bc.node_count >= k_skel_max_nodes ) return;
			ib = bc.node_count;
			bc.nodes[ bc.node_count++ ] = b;
		}
		// A bone can be reached twice (chain edge, then as a child of a shallower
		// node). Drawing it twice only doubles the cost and the brightness.
		for ( int i = 0; i < bc.edge_count; ++i ) {
			if ( ( bc.ea[ i ] == ia && bc.eb[ i ] == ib ) || ( bc.ea[ i ] == ib && bc.eb[ i ] == ia ) ) return;
		}
		bc.ea[ bc.edge_count ] = static_cast< unsigned char >( ia );
		bc.eb[ bc.edge_count ] = static_cast< unsigned char >( ib );
		++bc.edge_count;
	}

	// A real bone continues into another bone. Hair, cloth, straps and props hang
	// off a bone as leaves, and they are what put short stray segments across the
	// chest. Above the last two levels, a child with nothing hanging under it is
	// therefore not a bone. The last levels are kept as leaves because hands and
	// feet have no children.
	inline bool skel_continues( void* tr, const vec3_t& tp ) {
		const int count = unity::tr_child_count( tr );
		for ( int i = 0; i < count && i < 8; ++i ) {
			void* child = unity::tr_get_child( tr, i );
			if ( !child ) continue;
			vec3_t cp{};
			if ( !unity::get_transform_pos( child, cp ) ) continue;
			const float dx = cp.x - tp.x;
			const float dy = cp.y - tp.y;
			const float dz = cp.z - tp.z;
			const float d2 = dx * dx + dy * dy + dz * dz;
			if ( d2 >= 0.0004f && d2 <= 0.2025f ) return true;
		}
		return false;
	}

	// Real children of a transform, depth limited. Only links shorter than 45 cm
	// are kept: every human bone fits under that, while a link that jumps from one
	// side of the body to the other - the long diagonal that crossed the legs -
	// does not.
	inline void walk_skel_children( bone_cache_t& bc, void* parent, const vec3_t& pp, int depth ) {
		if ( depth <= 0 || bc.edge_count >= k_skel_max_nodes ) return;
		const int count = unity::tr_child_count( parent );
		for ( int i = 0; i < count && i < 8; ++i ) {
			if ( bc.edge_count >= k_skel_max_nodes ) return;
			void* child = unity::tr_get_child( parent, i );
			if ( !child ) continue;
			vec3_t cp{};
			if ( !unity::get_transform_pos( child, cp ) ) continue;
			const float dx = cp.x - pp.x;
			const float dy = cp.y - pp.y;
			const float dz = cp.z - pp.z;
			const float d2 = dx * dx + dy * dy + dz * dz;
			if ( d2 < 0.0004f || d2 > 0.2025f ) continue;
			if ( depth > 2 && !skel_continues( child, cp ) ) continue;
			add_skel_edge( bc, parent, child );
			const int idx = skel_index( bc, child );
			if ( idx >= 0 ) {
				// Already walked from a deeper anchor: only the link is missing.
				if ( bc.expanded[ idx ] ) continue;
				bc.expanded[ idx ] = 1;
			}
			walk_skel_children( bc, child, cp, depth - 1 );
		}
	}

	inline void* find_animator( void* view_obj ) {
		if ( !unity::is_valid_obj( view_obj ) ) return nullptr;
		const uintptr_t v = reinterpret_cast< uintptr_t >( view_obj );

		if ( unity::is_mem_readable( v + 0x28, sizeof( uintptr_t ) ) ) {
			const uintptr_t anim_view = unity::read_u64( v + 0x28 );
			if ( unity::is_managed_ptr( anim_view ) ) {
				if ( unity::obj_is( anim_view, "Animator" ) ) return reinterpret_cast< void* >( anim_view );
				if ( unity::is_mem_readable( anim_view + 0x20, sizeof( uintptr_t ) ) ) {
					const uintptr_t nested = unity::read_u64( anim_view + 0x20 );
					if ( unity::is_valid_obj( reinterpret_cast< void* >( nested ) ) && unity::obj_is( nested, "Animator" ) )
						return reinterpret_cast< void* >( nested );
				}
			}
		}

		for ( uintptr_t off = 0x10; off < 0x400; off += sizeof( uintptr_t ) ) {
			if ( !unity::is_mem_readable( v + off, sizeof( uintptr_t ) ) ) break;
			const uintptr_t field = unity::read_u64( v + off );
			if ( !unity::is_managed_ptr( field ) ) continue;
			if ( unity::obj_is( field, "Animator" ) ) return reinterpret_cast< void* >( field );
		}
		return nullptr;
	}

	inline const bone_cache_t* get_or_resolve_bones( void* view_obj, ULONGLONG now, int& budget ) {
		if ( !view_obj ) return nullptr;
		int slot = -1;
		int free_idx = -1;
		for ( int i = 0; i < 128; ++i ) {
			if ( g_bone_cache[ i ].view_obj == view_obj ) { slot = i; break; }
			if ( free_idx < 0 && g_bone_cache[ i ].view_obj == nullptr ) free_idx = i;
		}
		if ( slot < 0 ) {
			slot = ( free_idx >= 0 ) ? free_idx : 0;
			g_bone_cache[ slot ] = {};
			g_bone_cache[ slot ].view_obj = view_obj;
		}

		bone_cache_t& bc = g_bone_cache[ slot ];
		// TTL: the GC reuses addresses, so a pointer-keyed cache without expiry ends
		// up drawing the skeleton of a completely different player.
		// The hierarchy path fills nodes/edges instead of bone_trs, so the cache must
		// accept either source or it would re-resolve on every single frame.
		if ( bc.has_bones && ( now - bc.resolved_tick ) < 3000 &&
			( unity::is_valid_obj( bc.bone_trs[ 0 ] ) || ( bc.node_count > 0 && unity::is_valid_obj( bc.nodes[ 0 ] ) ) ) ) {
			return &bc;
		}
		if ( budget <= 0 || ( bc.last_try != 0 && ( now - bc.last_try ) < 1500 ) ) {
			return nullptr;
		}
		--budget;
		bc.last_try = now;
		bc.has_bones = false;

		if ( !bc.animator || !unity::is_valid_obj( bc.animator ) ) {
			bc.animator = find_animator( view_obj );
		}

		const uintptr_t anim_view = unity::read_u64( reinterpret_cast< uintptr_t >( view_obj ) + 0x28 );
		if ( unity::is_managed_ptr( anim_view ) ) {
			void* animator = bc.animator;
			if ( unity::is_valid_obj( animator ) && unity::fn_anim_get_bone ) {
				int valid_cnt = 0;
				for ( int b = 0; b < k_bone_count; ++b ) {
					void* tr = unity::get_animator_bone( animator, k_human_bones[ b ] );
					if ( unity::is_valid_obj( tr ) ) {
						bc.bone_trs[ b ] = tr;
						bc.native_bone_trs[ b ] = unity::read_u64( reinterpret_cast< uintptr_t >( tr ) + 0x10 );
						++valid_cnt;
					} else {
						bc.bone_trs[ b ] = nullptr;
						bc.native_bone_trs[ b ] = 0;
					}
				}
				if ( valid_cnt >= 6 ) {
					bc.has_bones = true;
					bc.resolved_tick = now;
					return &bc;
				}
			}
			const uintptr_t head_ctrl = unity::read_u64( anim_view + 0x28 );
			if ( unity::is_managed_ptr( head_ctrl ) ) {
				void* tr_neck = reinterpret_cast< void* >( unity::read_u64( head_ctrl + 0x20 ) );
				void* tr_head = reinterpret_cast< void* >( unity::read_u64( head_ctrl + 0x28 ) );
				void* tr_chest = reinterpret_cast< void* >( unity::read_u64( head_ctrl + 0x30 ) );
				if ( unity::is_valid_obj( tr_head ) ) bc.bone_trs[ 0 ] = tr_head;
				if ( unity::is_valid_obj( tr_neck ) ) bc.bone_trs[ 1 ] = tr_neck;
				if ( unity::is_valid_obj( tr_chest ) ) bc.bone_trs[ 2 ] = tr_chest;
			}
		}
		// Fallback source: walk the character's own transform hierarchy. The head
		// transform at view+0xE8 is proven to work (the ESP box is built from it), so
		// the spine is read by walking parents, then limbs by walking children.
		bc.node_count = 0;
		bc.edge_count = 0;
		for ( int i = 0; i < k_skel_max_nodes; ++i ) bc.expanded[ i ] = 0;
		void* head_tr = nullptr;
		if ( unity::is_mem_readable( reinterpret_cast< uintptr_t >( view_obj ) + 0xE8, sizeof( void* ) ) ) {
			head_tr = *reinterpret_cast< void** >( reinterpret_cast< uintptr_t >( view_obj ) + 0xE8 );
		}
		if ( unity::is_valid_obj( head_tr ) ) {
			void* chain[ 6 ]{};
			vec3_t chain_pos[ 6 ]{};
			int chain_n = 0;
			if ( unity::get_transform_pos( head_tr, chain_pos[ 0 ] ) ) {
				chain[ chain_n++ ] = head_tr;
				while ( chain_n < 6 ) {
					void* par = unity::tr_get_parent( chain[ chain_n - 1 ] );
					if ( !par ) break;
					vec3_t pp{};
					if ( !unity::get_transform_pos( par, pp ) ) break;
					chain[ chain_n ] = par;
					chain_pos[ chain_n ] = pp;
					++chain_n;
				}
				for ( int i = 0; i + 1 < chain_n; ++i ) add_skel_edge( bc, chain[ i ], chain[ i + 1 ] );

				// Everything below is walked deepest first, and the head is walked last
				// and shallow: a head with hair, a jaw and eyes has enough short children
				// to consume the whole edge budget, which is exactly what left the legs
				// missing while the arms were drawn.
				for ( int i = 0; i < chain_n; ++i ) {
					const int idx = skel_index( bc, chain[ i ] );
					if ( idx >= 0 ) bc.expanded[ idx ] = 1;
				}
				for ( int i = chain_n - 1; i >= 1; --i ) {
					if ( bc.edge_count >= k_skel_max_nodes ) break;
					walk_skel_children( bc, chain[ i ], chain_pos[ i ], 4 );
				}
				// The head is not expanded: its circle is drawn explicitly, and its
				// children are a jaw, eyes and hair, none of which is a bone worth a line.
			}
		}
		if ( bc.edge_count >= 4 ) {
			bc.has_bones = true;
			bc.resolved_tick = now;
			return &bc;
		}
		return nullptr;
	}

	inline vec3_t update_velocity( void* obj, const vec3_t& pos, ULONGLONG now ) {
		int free_idx = -1;
		for ( int i = 0; i < 128; ++i ) {
			if ( g_vel_cache[ i ].obj == obj ) {
				const ULONGLONG dt_ms = now - g_vel_cache[ i ].last_tick;
				if ( dt_ms >= 10 && dt_ms <= 500 ) {
					const float dt = static_cast< float >( dt_ms ) * 0.001f;
					const float vx = ( pos.x - g_vel_cache[ i ].last_pos.x ) / dt;
					const float vy = ( pos.y - g_vel_cache[ i ].last_pos.y ) / dt;
					const float vz = ( pos.z - g_vel_cache[ i ].last_pos.z ) / dt;
					g_vel_cache[ i ].vel.x = d3d11_ui::lerpf( g_vel_cache[ i ].vel.x, vx, 0.35f );
					g_vel_cache[ i ].vel.y = d3d11_ui::lerpf( g_vel_cache[ i ].vel.y, vy, 0.35f );
					g_vel_cache[ i ].vel.z = d3d11_ui::lerpf( g_vel_cache[ i ].vel.z, vz, 0.35f );
				} else if ( dt_ms > 500 ) {
					g_vel_cache[ i ].vel = {};
				}
				g_vel_cache[ i ].last_pos = pos;
				g_vel_cache[ i ].last_tick = now;
				return g_vel_cache[ i ].vel;
			}
			if ( free_idx < 0 && g_vel_cache[ i ].obj == nullptr ) free_idx = i;
		}
		if ( free_idx >= 0 ) {
			g_vel_cache[ free_idx ].obj = obj;
			g_vel_cache[ free_idx ].last_pos = pos;
			g_vel_cache[ free_idx ].last_tick = now;
			g_vel_cache[ free_idx ].vel = {};
		}
		return {};
	}

	inline bool is_aim_key_down( ) {
		static constexpr int k_vks[] = { VK_RBUTTON, VK_LBUTTON, VK_SHIFT, VK_XBUTTON2, VK_MENU };
		const int vk = config::resolve_vk( config::aim_key_mode, k_vks, 5 );
		return ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;
	}

	// xorshift: cheap enough to roll once per frame, no CRT state, no syscall.
	inline bool roll_percent( float chance ) {
		if ( chance >= 100.0f ) return true;
		if ( chance <= 0.0f ) return false;
		static uint32_t s_state = 0x9E3779B9u;
		s_state ^= s_state << 13;
		s_state ^= s_state >> 17;
		s_state ^= s_state << 5;
		return ( s_state % 10000u ) < static_cast< uint32_t >( chance * 100.0f );
	}

	inline void apply_mouse_aim_delta( float dx, float dy ) {
		const float smooth = config::aim_smooth > 1.0f ? config::aim_smooth : 1.0f;
		const float sens = config::aim_sens_calib > 0.1f ? config::aim_sens_calib : 1.0f;
		g_aim_rem_x += ( dx * sens ) / smooth;
		g_aim_rem_y += ( dy * sens ) / smooth;

		const LONG mx = static_cast< LONG >( g_aim_rem_x );
		const LONG my = static_cast< LONG >( g_aim_rem_y );
		g_aim_rem_x -= static_cast< float >( mx );
		g_aim_rem_y -= static_cast< float >( my );

		if ( mx == 0 && my == 0 ) return;
		INPUT inp{};
		inp.type = INPUT_MOUSE;
		inp.mi.dx = mx;
		inp.mi.dy = my;
		inp.mi.dwFlags = MOUSEEVENTF_MOVE;
		SendInput( 1, &inp, sizeof( INPUT ) );
	}

	inline bool apply_direct_3d_aim( uintptr_t battle_model, const vec3_t& target_w ) {
		if ( !unity::is_managed_ptr( battle_model ) ) return false;
		const uintptr_t rot_input = unity::read_u64( battle_model + 0x40 ); // PlayerRotationInputModel
		if ( !unity::is_managed_ptr( rot_input ) ) return false;

		float cur_yaw = 0.0f, cur_pitch = 0.0f;
		if ( !unity::safe_read( rot_input + 0x10, cur_yaw ) || !unity::safe_read( rot_input + 0x14, cur_pitch ) )
			return false;

		struct quat_t { float x, y, z, w; } q{};
		unity::safe_read( rot_input + 0x1C, q );

		const float dx = target_w.x - g_cam_pos.x;
		const float dy = target_w.y - g_cam_pos.y;
		const float dz = target_w.z - g_cam_pos.z;
		const float horiz = d3d11_ui::fast_sqrt( dx * dx + dz * dz );
		if ( horiz < 0.001f ) return false;

		// Desired yaw/pitch in degrees
		constexpr float k_rad2deg = 57.2957795f;
		constexpr float k_deg2rad = 0.0174532925f;
		float target_yaw = 0.0f;
		// atan2 approx via sin/cos or standard math
		{
			const float ang = static_cast< float >( std::atan2( dx, dz ) ) * k_rad2deg;
			target_yaw = ang;
		}
		const float target_pitch = -static_cast< float >( std::atan2( dy, horiz ) ) * k_rad2deg;

		// Detect radian vs degree storage from quaternion at +0x1C
		float scale = 1.0f;
		float pitch_sign = 1.0f;
		const float qlen2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
		if ( qlen2 > 0.95f && qlen2 < 1.05f ) {
			const float sinp = d3d11_ui::clampf( 2.0f * ( q.w * q.x - q.y * q.z ), -1.0f, 1.0f );
			const float qpitch = static_cast< float >( std::asin( sinp ) ) * k_rad2deg;
			const float qyaw = static_cast< float >( std::atan2( 2.0f * ( q.w * q.y + q.x * q.z ), 1.0f - 2.0f * ( q.x * q.x + q.y * q.y ) ) ) * k_rad2deg;
			if ( ( cur_yaw > -6.5f && cur_yaw < 6.5f && ( qyaw > 10.0f || qyaw < -10.0f ) ) ||
				 ( cur_pitch > -1.6f && cur_pitch < 1.6f && ( qpitch > 5.0f || qpitch < -5.0f ) ) ) {
				scale = k_deg2rad;
			}
			if ( ( cur_pitch > 0.05f || cur_pitch < -0.05f ) && ( qpitch > 3.0f || qpitch < -3.0f ) && ( cur_pitch * qpitch < 0.0f ) ) {
				pitch_sign = -1.0f;
			}
		}

		const float cur_yaw_deg = cur_yaw / scale;
		const float cur_pitch_deg = ( cur_pitch * pitch_sign ) / scale;
		float d_yaw = target_yaw - cur_yaw_deg;
		while ( d_yaw > 180.0f ) d_yaw -= 360.0f;
		while ( d_yaw < -180.0f ) d_yaw += 360.0f;
		float d_pitch = target_pitch - cur_pitch_deg;

		const float smooth = config::aim_smooth > 1.0f ? config::aim_smooth : 1.0f;
		d_yaw = d3d11_ui::clampf( d_yaw / smooth, -6.0f, 6.0f );
		d_pitch = d3d11_ui::clampf( d_pitch / smooth, -6.0f, 6.0f );

		const float new_yaw = cur_yaw + d_yaw * scale;
		const float max_p = 89.0f * scale;
		const float new_pitch = d3d11_ui::clampf( cur_pitch + d_pitch * pitch_sign * scale, -max_p, max_p );
		unity::safe_write< float >( rot_input + 0x10, new_yaw );
		unity::safe_write< float >( rot_input + 0x14, new_pitch );
		return true;
	}

	inline void apply_silent_target( uintptr_t battle_model, int32_t eid ) {
		if ( !unity::is_managed_ptr( battle_model ) || eid <= 0 ) return;
		const uintptr_t aim_m = unity::read_u64( battle_model + 0x198 ); // AimTargetModel
		if ( !unity::is_managed_ptr( aim_m ) ) return;
		unity::safe_write< int32_t >( aim_m + 0x10, eid );
		unity::safe_write< int32_t >( aim_m + 0x14, 3 ); // 3 = character
	}

	inline int classify_role( const char* role_id, char* out_tag, size_t tag_cap ) {
		out_tag[ 0 ] = '\0';
		if ( !role_id || !role_id[ 0 ] ) return ROLE_ROBBER;
		if ( unity::str_icontains( role_id, "Cop" ) || unity::str_icontains( role_id, "Police" ) ) {
			copy_str( out_tag, tag_cap, "COP" );
			return ROLE_COP;
		}
		if ( unity::str_icontains( role_id, "Robber" ) || unity::str_icontains( role_id, "Criminal" ) || unity::str_icontains( role_id, "Prisoner" ) ) {
			copy_str( out_tag, tag_cap, "ROBBER" );
			return ROLE_ROBBER;
		}
		if ( unity::str_icontains( role_id, "Citizen" ) || unity::str_icontains( role_id, "Civ" ) || unity::str_icontains( role_id, "Peace" ) ||
			 unity::str_icontains( role_id, "Settler" ) || unity::str_icontains( role_id, "Hubber" ) ) {
			copy_str( out_tag, tag_cap, "CIVIL" );
			return ROLE_CIV;
		}
		copy_str( out_tag, tag_cap, role_id );
		return ROLE_ROBBER;
	}

	inline void apply_combat_and_misc_mods( uintptr_t battle_model, uintptr_t entities_model ) {
		if ( !unity::is_managed_ptr( entities_model ) ) return;
		const uintptr_t local_ent = unity::read_u64( entities_model + 0xD0 );
		if ( !unity::is_managed_ptr( local_ent ) ) return;

		const uintptr_t client_m = unity::read_u64( local_ent + 0x18 );
		const uintptr_t char_m = unity::read_u64( local_ent + 0x20 );

		// Instant Loot Reveal / Search
		if ( config::instant_loot_reveal && unity::is_managed_ptr( char_m ) ) {
			const uintptr_t pim = unity::read_u64( char_m + 0x340 );
			if ( unity::is_managed_ptr( pim ) ) {
				const uintptr_t end_ts_data = unity::read_u64( pim + 0x18 );
				if ( unity::is_managed_ptr( end_ts_data ) ) {
					int64_t end_ts = 0;
					if ( unity::safe_read< int64_t >( end_ts_data + 0x18, end_ts ) && end_ts > 1 ) {
						unity::safe_write< int64_t >( end_ts_data + 0x18, 1 );
					}
				}
			}
		}

		// Recoil / RCS / Spread
		if ( unity::is_managed_ptr( client_m ) ) {
			if ( config::no_recoil || config::rcs_enabled ) {
				const uintptr_t recoil_m = unity::read_u64( client_m + 0x2B8 );
				if ( unity::is_managed_ptr( recoil_m ) ) {
					const float scale = config::no_recoil ? 0.0f : ( 1.0f - d3d11_ui::clampf( config::rcs_strength, 0.0f, 1.0f ) );
					float rx = 0.0f, ry = 0.0f;
					if ( unity::safe_read( recoil_m + 0x10, rx ) ) unity::safe_write< float >( recoil_m + 0x10, rx * scale );
					if ( unity::safe_read( recoil_m + 0x14, ry ) ) unity::safe_write< float >( recoil_m + 0x14, ry * scale );
				}
			}
			if ( config::no_spread ) {
				const uintptr_t fake_spread_m = unity::read_u64( client_m + 0x2C0 );
				if ( unity::is_managed_ptr( fake_spread_m ) ) {
					unity::safe_write< float >( fake_spread_m + 0x10, 0.0f );
					unity::safe_write< float >( fake_spread_m + 0x14, 0.0f );
					unity::safe_write< float >( fake_spread_m + 0x18, 0.0f );
				}
			}
		}

		// WeaponDescription Spread & ShotPattern (Actual Recoil & Spread)
		if ( ( config::no_spread || config::no_recoil ) && unity::is_managed_ptr( char_m ) ) {
			const uintptr_t weapon_m = unity::read_u64( char_m + 0x150 );
			const uintptr_t item_in_hands = unity::is_managed_ptr( weapon_m ) ? unity::read_u64( weapon_m + 0x10 ) : 0;
			const uintptr_t item_desc = unity::is_managed_ptr( item_in_hands ) ? unity::read_u64( item_in_hands + 0x10 ) : 0;
			const uintptr_t wpn_func = unity::is_managed_ptr( item_desc ) ? unity::read_u64( item_desc + 0x50 ) : 0;
			const uintptr_t wpn_target = unity::is_managed_ptr( wpn_func ) ? unity::read_u64( wpn_func + 0x20 ) : 0;
			const uintptr_t wpn_desc = unity::is_managed_ptr( wpn_target ) ? unity::read_u64( wpn_target + 0x10 ) : 0;
			if ( unity::is_managed_ptr( wpn_desc ) && unity::obj_is( wpn_desc, "WeaponDescription" ) ) {
				if ( config::no_spread ) {
					const uintptr_t sp_fn = unity::read_u64( wpn_desc + 0x70 );
					const uintptr_t sp_tgt = unity::is_managed_ptr( sp_fn ) ? unity::read_u64( sp_fn + 0x20 ) : 0;
					const uintptr_t sp_desc = unity::is_managed_ptr( sp_tgt ) ? unity::read_u64( sp_tgt + 0x10 ) : 0;
					const uintptr_t wpn_spread = unity::is_managed_ptr( sp_desc ) ? unity::read_u64( sp_desc + 0x20 ) : 0;
					if ( unity::is_managed_ptr( wpn_spread ) ) {
						for ( uintptr_t off = 0x10; off <= 0x38; off += 0x08 ) {
							const uintptr_t st = unity::read_u64( wpn_spread + off );
							if ( unity::is_managed_ptr( st ) ) {
								unity::safe_write< float >( st + 0x10, 0.0f );
								unity::safe_write< float >( st + 0x14, 0.0f );
							}
						}
						unity::safe_write< float >( wpn_spread + 0x40, 0.0f );
					}
				}
				if ( config::no_recoil ) {
					static constexpr uintptr_t pat_offs[ 2 ] = { 0x78, 0x80 };
					for ( int pi = 0; pi < 2; ++pi ) {
						const uintptr_t pt_fn = unity::read_u64( wpn_desc + pat_offs[ pi ] );
						const uintptr_t pt_tgt = unity::is_managed_ptr( pt_fn ) ? unity::read_u64( pt_fn + 0x20 ) : 0;
						const uintptr_t pt_desc = unity::is_managed_ptr( pt_tgt ) ? unity::read_u64( pt_tgt + 0x10 ) : 0;
						const uintptr_t pat_arr = unity::is_managed_ptr( pt_desc ) ? unity::read_u64( pt_desc + 0x20 ) : 0;
						if ( unity::is_managed_ptr( pat_arr ) ) {
							const int32_t len = unity::read_i32( pat_arr + 0x18 );
							for ( int32_t k = 0; k < len && k < 64; ++k ) {
								const uintptr_t pat = unity::read_u64( pat_arr + 0x20 + static_cast< size_t >( k ) * 8 );
								if ( unity::is_managed_ptr( pat ) ) {
									unity::safe_write< float >( pat + 0x10, 0.0f );
									unity::safe_write< float >( pat + 0x14, 0.0f );
									unity::safe_write< float >( pat + 0x18, 0.0f );
									unity::safe_write< float >( pat + 0x1C, 0.0f );
								}
							}
						}
					}
				}
			}
		}

		// No Camera / Optics Shake
		if ( config::no_cam_shake && unity::is_managed_ptr( battle_model ) ) {
			const uintptr_t cam_m = unity::read_u64( battle_model + 0x358 ); // BattleCameraModel
			if ( unity::is_managed_ptr( cam_m ) ) {
				unity::safe_write< float >( cam_m + 0x2C, 0.0f );
			}
		}
	}

	inline void toggle_nearest_door( uintptr_t battle_model ) {
		if ( !unity::is_managed_ptr( battle_model ) ) return;
		const uintptr_t bem = unity::read_u64( battle_model + 0x90 );
		if ( !unity::is_managed_ptr( bem ) ) return;
		const uintptr_t phys_scene = unity::read_u64( bem + 0x10 );
		const uintptr_t pdata = unity::is_managed_ptr( phys_scene ) ? unity::read_u64( phys_scene + 0x10 ) : 0;
		const uintptr_t world_m = unity::read_u64( battle_model + 0x518 );
		if ( !unity::is_managed_ptr( pdata ) || !unity::is_managed_ptr( world_m ) ) return;

		const int32_t st_type = unity::read_i32( pdata + 0x30 );
		const uintptr_t st_bodies = unity::read_u64( pdata + 0x48 );
		const int32_t dy_type = unity::read_i32( pdata + 0x50 );
		const uintptr_t dy_bodies = unity::read_u64( pdata + 0x68 );

		const uintptr_t doors_dict = unity::read_u64( world_m + 0x48 );
		uintptr_t entries = 0;
		const int n = unity::read_dict( doors_dict, entries );
		for ( int i = 0; i < n; ++i ) {
			const uintptr_t ent = entries + static_cast< size_t >( i ) * 0x18;
			if ( unity::read_i32( ent ) < 0 ) continue;
			const uintptr_t door_m = unity::read_u64( ent + 0x10 );
			if ( !unity::is_managed_ptr( door_m ) ) continue;
			const uintptr_t bodies_m = unity::read_u64( door_m + 0x28 );
			const uintptr_t body_list = unity::is_managed_ptr( bodies_m ) ? unity::read_u64( bodies_m + 0x10 ) : 0;
			if ( !unity::is_managed_ptr( body_list ) ) continue;
			const uintptr_t items = unity::read_u64( body_list + 0x10 );
			const int32_t sz = unity::read_i32( body_list + 0x18 );
			if ( !unity::is_managed_ptr( items ) || sz <= 0 || sz > 64 ) continue;
			for ( int j = 0; j < sz; ++j ) {
				const uintptr_t elem = items + 0x20 + static_cast< size_t >( j ) * 8;
				const int32_t bval = unity::read_i32( elem );
				const uint8_t btype = unity::read_u8( elem + 4 );
				uintptr_t arr = ( btype == st_type ) ? st_bodies : ( btype == dy_type ? dy_bodies : 0 );
				if ( !unity::is_managed_ptr( arr ) ) continue;
				if ( bval < 0 ) continue;
				const int32_t arr_len = unity::read_i32( arr + 0x18 );
				if ( arr_len <= 0 || bval >= arr_len ) continue;
				const uintptr_t rb = arr + 0x20 + static_cast< size_t >( bval ) * 0xCC;
				int32_t flags = 0;
				if ( unity::safe_read( rb + 0x00, flags ) ) {
					unity::safe_write< int32_t >( rb + 0x00, flags ^ 256 );
				}
			}
		}
	}

	inline int rarity_from_value( int base_price, int item_rarity ) {
		if ( base_price > 0 ) {
			if ( base_price >= 5000 ) return 5;
			if ( base_price >= 2000 ) return 4;
			if ( base_price >= 750 )  return 3;
			if ( base_price >= 250 )  return 2;
			return 1;
		}
		if ( item_rarity >= 0 && item_rarity <= 6 ) return item_rarity;
		return 1;
	}

	inline d3d11_ui::color_t get_rarity_color( int rarity ) {
		switch ( rarity ) {
		case 1: return { 0.88f, 0.90f, 0.94f, 1.0f }; // Common
		case 2: return { 0.25f, 0.90f, 0.48f, 1.0f }; // Uncommon
		case 3: return { 0.22f, 0.78f, 1.00f, 1.0f }; // Rare
		case 4: return { 0.74f, 0.36f, 0.98f, 1.0f }; // Epic
		case 5: return { 1.00f, 0.72f, 0.16f, 1.0f }; // Legendary
		case 6: return { 1.00f, 0.26f, 0.36f, 1.0f }; // Exotic
		default: return { 0.70f, 0.74f, 0.82f, 1.0f }; // No rarity
		}
	}

	// get_class_name walks the klass name string for every entity on every metadata
	// refresh. Every entity of a given type shares the same klass pointer, so the
	// scan only has to happen once per class instead of once per entity.
	struct klass_name_t { uintptr_t klass; char name[ 48 ]; };
	inline klass_name_t g_klass_names[ 512 ]{};

	inline bool cached_class_name( uintptr_t obj, char* out, size_t cap ) {
		if ( !out || cap == 0 ) return false;
		out[ 0 ] = '\0';
		const uintptr_t klass = unity::read_u64( obj );
		if ( !klass ) return false;

		klass_name_t& e = g_klass_names[ ( klass >> 4 ) & 511 ];
		if ( e.klass == klass ) {
			copy_str( out, cap, e.name );
			return e.name[ 0 ] != '\0';
		}

		if ( !unity::get_class_name( obj, e.name, sizeof( e.name ) ) ) {
			e.klass = 0;
			return false;
		}
		e.klass = klass;
		copy_str( out, cap, e.name );
		return true;
	}

	inline void refresh_metadata_and_world_cache( ULONGLONG now, uintptr_t battle_model, uintptr_t entities_model, uintptr_t slots_model ) {
		static ULONGLONG s_last_refresh = 0;
		if ( s_last_refresh != 0 && ( now - s_last_refresh ) < 800 ) return;
		s_last_refresh = now;

		g_meta_count = 0;
		g_world_raw_count = 0;

		if ( !unity::is_managed_ptr( entities_model ) ) {
			config::model_chain_ok = false;
			config::diag_models_read = 0;
			return;
		}
		config::model_chain_ok = true;

		int64_t server_now = 0;
		if ( unity::is_managed_ptr( battle_model ) ) {
			const uintptr_t time_m = unity::read_u64( battle_model + 0x38 );
			const uintptr_t set_now = unity::is_managed_ptr( time_m ) ? unity::read_u64( time_m + 0x58 ) : 0;
			if ( unity::is_managed_ptr( set_now ) ) unity::safe_read( set_now + 0x10, server_now );
		}

		const uintptr_t local_player_ent = unity::read_u64( entities_model + 0xD0 );
		int32_t local_id = -1;
		if ( unity::is_managed_ptr( local_player_ent ) ) {
			local_id = unity::read_i32( local_player_ent + 0x10 );
			g_local_eid = local_id;
		}

		struct slot_cache_t { int32_t eid; char name[ 32 ]; char clan[ 32 ]; };
		slot_cache_t slot_arr[ 128 ]{};
		int slot_cnt = 0;
		if ( slots_model ) {
			const uintptr_t slot_dict = unity::read_u64( slots_model + 0x10 );
			uintptr_t entries = 0;
			int n = unity::read_dict( slot_dict, entries );
			if ( n == 0 ) {
				const uintptr_t slot_dict2 = unity::read_u64( slots_model + 0x18 );
				n = unity::read_dict( slot_dict2, entries );
			}
			for ( int i = 0; i < n && slot_cnt < 128; ++i ) {
				const uintptr_t ent = entries + static_cast< size_t >( i ) * 0x18;
				if ( unity::read_i32( ent ) < 0 ) continue;
				const uintptr_t slot_model = unity::read_u64( ent + 0x10 );
				if ( !unity::is_managed_ptr( slot_model ) ) continue;
				const uintptr_t slot_data = unity::read_u64( slot_model + 0x10 );
				if ( !unity::is_managed_ptr( slot_data ) ) continue;

				slot_cache_t& sc = slot_arr[ slot_cnt ];
				int32_t eid_v = -1;
				if ( !unity::read_prim_data( unity::read_u64( slot_data + 0x20 ), eid_v ) ) {
					eid_v = unity::read_i32( ent + 0x08 );
				}
				sc.eid = eid_v;
				uintptr_t nick_str = 0, clan_str = 0;
				if ( unity::read_prim_data( unity::read_u64( slot_data + 0x30 ), nick_str ) ) {
					unity::read_il2cpp_string( nick_str, sc.name, sizeof( sc.name ) );
				}
				if ( unity::read_prim_data( unity::read_u64( slot_data + 0x38 ), clan_str ) ) {
					unity::read_il2cpp_string( clan_str, sc.clan, sizeof( sc.clan ) );
				}
				++slot_cnt;
			}
		}

		const uintptr_t model_dict = unity::read_u64( entities_model + 0x50 );
		uintptr_t entries = 0;
		const int n = unity::read_dict( model_dict, entries );
		config::diag_models_read = n;

		auto push_world_raw = [&]( int32_t eid, const vec3_t& wpos, int cat, int rarity, bool is_empty, const char* lbl ) {
			if ( g_world_raw_count >= 256 ) return;
			world_raw_t& wi = g_world_raw[ g_world_raw_count++ ];
			wi = {};
			wi.entity_id = eid;
			wi.pos = wpos;
			wi.category = cat;
			wi.rarity = rarity;
			wi.is_empty = is_empty;
			copy_str( wi.label, sizeof( wi.label ), lbl );
		};

		plot_summary_t next_plot{};
		float best_plot_dist = 1e9f;

		for ( int i = 0; i < n; ++i ) {
			const uintptr_t ent = entries + static_cast< size_t >( i ) * 0x18;
			if ( unity::read_i32( ent ) < 0 ) continue;
			const uintptr_t emodel = unity::read_u64( ent + 0x10 );
			if ( !unity::is_managed_ptr( emodel ) ) continue;

			char cls[ 48 ]{};
			if ( !cached_class_name( emodel, cls, sizeof( cls ) ) ) continue;

			if ( unity::str_eq( cls, "BattleCharacterEntityModel" ) || unity::str_eq( cls, "PlayerEntityModel" ) ) {
				if ( g_meta_count >= 128 ) continue;
				uintptr_t char_model = 0;
				if ( unity::str_eq( cls, "BattleCharacterEntityModel" ) ) {
					char_model = unity::read_u64( emodel + 0x30 );
					if ( !unity::is_managed_ptr( char_model ) ) char_model = unity::read_u64( emodel + 0x28 );
				} else {
					char_model = unity::read_u64( emodel + 0x20 );
				}
				if ( !unity::is_managed_ptr( char_model ) ) continue;

				const uintptr_t cdata = unity::read_u64( char_model + 0x40 );
				if ( !unity::is_managed_ptr( cdata ) ) continue;
				const uintptr_t common = unity::read_u64( cdata + 0x20 );
				if ( !unity::is_managed_ptr( common ) ) continue;
				const uintptr_t pub = unity::read_u64( cdata + 0x30 );

				vec3_t pos{};
				if ( !unity::read_prim_data( unity::read_u64( common + 0x28 ), pos ) ) continue;

				meta_entry_t& m = g_meta[ g_meta_count ];
				m = {};
				m.entity_id = unity::read_i32( emodel + 0x10 );
				m.pos = pos;
				unity::read_prim_data( unity::read_u64( common + 0x30 ), m.look_dir );
				m.is_local = ( local_id >= 0 && m.entity_id == local_id );

				uint8_t dead_b = 0;
				if ( unity::read_prim_data( unity::read_u64( common + 0xE0 ), dead_b ) ) m.is_dead = ( dead_b != 0 );

				int32_t veh_eid = -1;
				if ( unity::read_prim_data( unity::read_u64( common + 0xF0 ), veh_eid ) && veh_eid > 0 ) {
					m.in_vehicle = true;
					m.vehicle_eid = veh_eid;
				}
				if ( m.is_local ) g_local_veh_eid = m.vehicle_eid;

				// Knockout / Knockdown check
				if ( server_now > 0 ) {
					auto check_ts = [&]( uintptr_t km ) -> bool {
						if ( !unity::is_managed_ptr( km ) ) return false;
						int64_t end_ts = 0;
						if ( unity::read_prim_data( unity::read_u64( km + 0x10 ), end_ts ) ) {
							return end_ts > server_now && end_ts < ( server_now + 600000 );
						}
						return false;
					};
					m.is_knocked = check_ts( unity::read_u64( char_model + 0x298 ) ) || check_ts( unity::read_u64( char_model + 0x2D0 ) );
				}

				if ( unity::is_managed_ptr( pub ) ) {
					uint8_t armor_b = 0;
					if ( unity::read_prim_data( unity::read_u64( pub + 0x38 ), armor_b ) ) m.has_armor = ( armor_b != 0 );
				}

				uintptr_t role_obj = 0;
				if ( unity::read_prim_data( unity::read_u64( common + 0x20 ), role_obj ) ) {
					char role_id[ 32 ]{};
					unity::read_enum_id( role_obj, role_id, sizeof( role_id ) );
					m.role_kind = classify_role( role_id, m.tag, sizeof( m.tag ) );
				}

				uintptr_t item_obj = 0;
				if ( unity::read_prim_data( unity::read_u64( common + 0xB8 ), item_obj ) ) {
					unity::read_enum_id( item_obj, m.weapon, sizeof( m.weapon ) );
				}

				for ( int s = 0; s < slot_cnt; ++s ) {
					if ( slot_arr[ s ].eid == m.entity_id ) {
						copy_str( m.name, sizeof( m.name ), slot_arr[ s ].name );
						copy_str( m.clan, sizeof( m.clan ), slot_arr[ s ].clan );
						break;
					}
				}
				if ( m.is_local ) {
					g_local_role = m.role_kind;
					copy_str( g_local_clan, sizeof( g_local_clan ), m.clan );
					copy_str( g_local_weapon, sizeof( g_local_weapon ), m.weapon );
				}
				if ( m.is_dead ) {
					// Record death point if not already recorded nearby
					bool found = false;
					for ( int d = 0; d < 64; ++d ) {
						if ( g_deaths[ d ].valid && dist3( g_deaths[ d ].pos, pos ) < 2.0f ) { found = true; break; }
					}
					if ( !found ) {
						for ( int d = 0; d < 64; ++d ) {
							if ( !g_deaths[ d ].valid ) {
								g_deaths[ d ].pos = pos;
								copy_str( g_deaths[ d ].name, 32, m.name[ 0 ] ? m.name : m.tag );
								g_deaths[ d ].tick = now;
								g_deaths[ d ].valid = true;
								break;
							}
						}
					}
				}
				m.valid = true;
				++g_meta_count;
			}
			else if ( config::world_master ) {
				const int32_t eid = unity::read_i32( emodel + 0x10 );
				if ( config::world_cars && unity::str_eq( cls, "CarEntityModel" ) ) {
					const uintptr_t client_m = unity::read_u64( emodel + 0x20 );
					if ( unity::is_managed_ptr( client_m ) ) {
						vec3_t pos{};
						if ( unity::safe_read( client_m + 0x7C, pos ) ) {
							push_world_raw( eid, pos, WCAT_CAR, 1, false, "CAR" );
							if ( dist3( pos, g_cam_pos ) < 55.0f ) ++next_plot.parked_vehicles;
						}
					}
				} else if ( config::world_helis && unity::str_eq( cls, "HelicopterEntityModel" ) ) {
					const uintptr_t client_m = unity::read_u64( emodel + 0x20 );
					if ( unity::is_managed_ptr( client_m ) ) {
						vec3_t pos{};
						if ( unity::safe_read( client_m + 0x7C, pos ) ) {
							push_world_raw( eid, pos, WCAT_HELI, 3, false, "HELICOPTER" );
						}
					}
				} else if ( config::world_dropped && unity::str_eq( cls, "LootEntityModel" ) ) {
					const uintptr_t client_m = unity::read_u64( emodel + 0x20 );
					if ( unity::is_managed_ptr( client_m ) ) {
						const uintptr_t set_pos = unity::read_u64( client_m + 0x20 );
						if ( unity::is_managed_ptr( set_pos ) ) {
							vec3_t pos{};
							if ( unity::safe_read( set_pos + 0x10, pos ) ) {
								push_world_raw( eid, pos, WCAT_DROPPED, 2, false, "DROPPED LOOT" );
							}
						}
					}
				} else if ( config::world_containers && unity::str_eq( cls, "ContainerEntityModel" ) ) {
					const uintptr_t srv = unity::read_u64( emodel + 0x18 );
					if ( unity::is_managed_ptr( srv ) ) {
						uint8_t is_empty = 0;
						unity::read_prim_model( unity::read_u64( srv + 0x38 ), is_empty );
						if ( config::world_hide_empty && is_empty ) continue;
						const uintptr_t tr_m = unity::read_u64( srv + 0x28 );
						if ( unity::is_managed_ptr( tr_m ) ) {
							vec3_t pos{};
							if ( unity::read_prim_data( unity::read_u64( tr_m + 0x10 ), pos.x ) &&
								 unity::read_prim_data( unity::read_u64( tr_m + 0x18 ), pos.y ) &&
								 unity::read_prim_data( unity::read_u64( tr_m + 0x20 ), pos.z ) ) {
								push_world_raw( eid, pos, WCAT_CONTAINER, is_empty ? 0 : 3, is_empty != 0, is_empty ? "CONTAINER [EMPTY]" : "CONTAINER [FULL]" );
								if ( dist3( pos, g_cam_pos ) < 55.0f ) {
									++next_plot.storages;
									if ( !is_empty ) ++next_plot.full_containers;
								}
							}
						}
					}
				}
			}
		}

		// WorldEntitiesModel direct dictionaries for loose loot, nodes, vehicle loot, buyers
		if ( config::world_master && unity::is_managed_ptr( battle_model ) ) {
			const uintptr_t world_m = unity::read_u64( battle_model + 0x518 );
			if ( unity::is_managed_ptr( world_m ) ) {
				// Loose items (_loots @ 0x38)
				if ( config::world_loose ) {
					uintptr_t l_entries = 0;
					const int ln = unity::read_dict( unity::read_u64( world_m + 0x38 ), l_entries );
					for ( int i = 0; i < ln; ++i ) {
						const uintptr_t ent = l_entries + static_cast< size_t >( i ) * 0x18;
						if ( unity::read_i32( ent ) < 0 ) continue;
						const uintptr_t srv = unity::read_u64( ent + 0x10 );
						if ( !unity::is_managed_ptr( srv ) ) continue;
						const uintptr_t desc = unity::read_u64( srv + 0x18 );
						if ( !unity::is_managed_ptr( desc ) ) continue;
						vec3_t pos{};
						if ( unity::safe_read( desc + 0x30, pos ) ) {
							char raw_id[ 32 ]{};
							unity::read_enum_id( desc, raw_id, sizeof( raw_id ) );
							push_world_raw( -1, pos, WCAT_LOOSE, 1, false, raw_id[ 0 ] ? raw_id : "LOOT" );
						}
					}
				}
				// Loot nodes (_lootNodes @ 0xB0)
				if ( config::world_nodes ) {
					uintptr_t n_entries = 0;
					const int nn = unity::read_dict( unity::read_u64( world_m + 0xB0 ), n_entries );
					for ( int i = 0; i < nn; ++i ) {
						const uintptr_t ent = n_entries + static_cast< size_t >( i ) * 0x18;
						if ( unity::read_i32( ent ) < 0 ) continue;
						const uintptr_t node_m = unity::read_u64( ent + 0x10 );
						if ( !unity::is_managed_ptr( node_m ) ) continue;
						uint8_t is_empty = 0;
						unity::read_prim_model( unity::read_u64( node_m + 0x30 ), is_empty );
						if ( config::world_hide_empty && is_empty ) continue;
						const uintptr_t desc = unity::read_u64( node_m + 0x10 );
						if ( !unity::is_managed_ptr( desc ) ) continue;
						vec3_t pos{};
						if ( unity::safe_read( desc + 0x38, pos ) ) {
							char raw_id[ 32 ]{};
							unity::read_enum_id( desc, raw_id, sizeof( raw_id ) );
							push_world_raw( -1, pos, WCAT_NODE, 2, is_empty != 0, raw_id[ 0 ] ? raw_id : "NODE" );
							const float pd = dist3( pos, g_cam_pos );
							if ( pd < 55.0f ) {
								if ( pd < best_plot_dist ) { best_plot_dist = pd; next_plot.distance = pd; next_plot.valid = true; }
								++next_plot.bushes_by_rarity[ 2 ];
							}
						}
					}
				}
			}
		}
		g_nearest_plot = next_plot;
	}

	inline void project_world_cache( ) {
		const LONG w_back = 1 - InterlockedCompareExchange( &g_world_active_buf, 0, 0 );
		if ( !config::world_master ) {
			g_world_count_buf[ w_back ] = 0;
			InterlockedExchange( &g_world_active_buf, w_back );
			config::diag_world_tracked = 0;
			return;
		}
		world_item_t* w_dst = g_world_buf[ w_back ];
		int w_cnt = 0;
		for ( int i = 0; i < g_world_raw_count && w_cnt < 256; ++i ) {
			const world_raw_t& raw = g_world_raw[ i ];
			if ( config::world_hide_local_veh && ( raw.category == WCAT_CAR || raw.category == WCAT_HELI ) &&
				 g_local_veh_eid > 0 && raw.entity_id == g_local_veh_eid ) {
				continue;
			}
			if ( ( raw.category == WCAT_LOOSE || raw.category == WCAT_NODE || raw.category == WCAT_DROPPED ) &&
				 ( ( config::world_rarity_mask & ( 1u << ( raw.rarity & 7 ) ) ) == 0 ) ) {
				continue;
			}
			const float dist = dist3( raw.pos, g_cam_pos );
			const float max_d = ( raw.category == WCAT_CAR || raw.category == WCAT_HELI ) ? config::world_veh_dist :
								( raw.category == WCAT_LOOSE || raw.category == WCAT_NODE || raw.category == WCAT_DROPPED ) ? config::world_loot_dist :
								config::world_obj_dist;
			if ( dist > max_d || dist < 1.5f ) continue;
			vec2_t spos{};
			if ( !world_to_screen( raw.pos, spos ) ) continue;
			world_item_t& wi = w_dst[ w_cnt++ ];
			wi = {};
			wi.entity_id = raw.entity_id;
			wi.world_pos = raw.pos;
			wi.screen_pos = spos;
			wi.distance = dist;
			wi.category = raw.category;
			wi.rarity = raw.rarity;
			wi.is_empty = raw.is_empty;
			copy_str( wi.label, sizeof( wi.label ), raw.label );
			wi.valid = true;
		}
		g_world_count_buf[ w_back ] = w_cnt;
		InterlockedExchange( &g_world_active_buf, w_back );
		config::diag_world_tracked = w_cnt;
	}

	inline const meta_entry_t* find_nearest_meta( const vec3_t& pos ) {
		const meta_entry_t* best = nullptr;
		float best_d2 = 16.0f;
		for ( int i = 0; i < g_meta_count; ++i ) {
			if ( !g_meta[ i ].valid ) continue;
			const float dx = g_meta[ i ].pos.x - pos.x;
			const float dy = g_meta[ i ].pos.y - pos.y;
			const float dz = g_meta[ i ].pos.z - pos.z;
			const float d2 = dx * dx + dy * dy + dz * dz;
			if ( d2 < best_d2 ) {
				best_d2 = d2;
				best = &g_meta[ i ];
			}
		}
		return best;
	}

	inline void reset_scene_caches( ) {
		unity::reset_page_cache( );
		g_camera = nullptr;
		g_vp_valid = false;
		g_locked_obj = nullptr;
		g_has_aim_target = false;
		g_has_silent_target = false;
		g_aim_rem_x = 0.0f;
		g_aim_rem_y = 0.0f;
		g_meta_count = 0;
		g_world_raw_count = 0;
		g_freecam_active = false;
		for ( int i = 0; i < 128; ++i ) {
			g_bone_cache[ i ] = {};
			g_vel_cache[ i ] = {};
		}
		for ( int i = 0; i < 512; ++i ) {
			g_klass_names[ i ] = {};
		}
		g_player_count_buf[ 0 ] = 0;
		g_player_count_buf[ 1 ] = 0;
		g_world_count_buf[ 0 ] = 0;
		g_world_count_buf[ 1 ] = 0;
		config::diag_players_tracked = 0;
		config::diag_world_tracked = 0;
		config::diag_models_read = 0;
		config::model_chain_ok = false;
		unity::g_unity_entry_obj = 0;
	}

	inline void handle_hotkeys_and_camera( void* cam, uintptr_t battle_model ) {
		static bool s_prev_fc = false, s_prev_door = false, s_prev_plot = false, s_prev_plist = false;
		static constexpr int k_fc_vks[] = { VK_F5, 'V', 'C', 'X' };
		static constexpr int k_dr_vks[] = { 'G', 'E', 'F', 'T' };
		static constexpr int k_pl_vks[] = { VK_F6, VK_F7, VK_F9, 'P' };
		static constexpr int k_ls_vks[] = { VK_TAB, VK_F2, VK_F3, 'L' };

		const bool fc_down = ( GetAsyncKeyState( config::resolve_vk( config::freecam_key, k_fc_vks, 4 ) ) & 0x8000 ) != 0;
		if ( fc_down && !s_prev_fc ) {
			config::freecam_enabled = !config::freecam_enabled;
			if ( config::freecam_enabled ) g_freecam_pos = g_cam_pos;
		}
		s_prev_fc = fc_down;

		const bool dr_down = ( GetAsyncKeyState( config::resolve_vk( config::door_toggle_key, k_dr_vks, 4 ) ) & 0x8000 ) != 0;
		if ( dr_down && !s_prev_door ) toggle_nearest_door( battle_model );
		s_prev_door = dr_down;

		const bool pl_down = ( GetAsyncKeyState( config::resolve_vk( config::plot_panel_key, k_pl_vks, 4 ) ) & 0x8000 ) != 0;
		if ( pl_down && !s_prev_plot ) config::plot_panel_enabled = !config::plot_panel_enabled;
		s_prev_plot = pl_down;

		const bool ls_down = ( GetAsyncKeyState( config::resolve_vk( config::player_list_key, k_ls_vks, 4 ) ) & 0x8000 ) != 0;
		if ( ls_down && !s_prev_plist ) config::player_list_open = !config::player_list_open;
		s_prev_plist = ls_down;

		// FOV Changer (preserving ADS scope zoom)
		if ( config::fov_changer_enabled && unity::fn_cam_get_fov && unity::fn_cam_set_fov && unity::is_valid_obj( cam ) ) {
			const float cur_fov = unity::fn_cam_get_fov( cam, unity::m_cam_get_fov );
			const bool is_ads = config::fov_preserve_ads && ( ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) != 0 || cur_fov < 52.0f );
			if ( !is_ads ) {
				unity::fn_cam_set_fov( cam, config::fov_value, unity::m_cam_set_fov );
			}
		}

		// Free Cam
		if ( config::freecam_enabled && unity::is_valid_obj( cam ) ) {
			void* cam_tr = unity::get_component_transform( cam );
			if ( unity::is_valid_obj( cam_tr ) ) {
				float spd = config::freecam_speed * 0.016f;
				if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) spd *= config::freecam_boost;
				if ( GetAsyncKeyState( 'W' ) & 0x8000 ) { g_freecam_pos.x += g_cam_fwd.x * spd; g_freecam_pos.y += g_cam_fwd.y * spd; g_freecam_pos.z += g_cam_fwd.z * spd; }
				if ( GetAsyncKeyState( 'S' ) & 0x8000 ) { g_freecam_pos.x -= g_cam_fwd.x * spd; g_freecam_pos.y -= g_cam_fwd.y * spd; g_freecam_pos.z -= g_cam_fwd.z * spd; }
				if ( GetAsyncKeyState( 'D' ) & 0x8000 ) { g_freecam_pos.x += g_cam_right.x * spd; g_freecam_pos.z += g_cam_right.z * spd; }
				if ( GetAsyncKeyState( 'A' ) & 0x8000 ) { g_freecam_pos.x -= g_cam_right.x * spd; g_freecam_pos.z -= g_cam_right.z * spd; }
				if ( GetAsyncKeyState( VK_SPACE ) & 0x8000 ) g_freecam_pos.y += spd;
				if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) g_freecam_pos.y -= spd;
				unity::set_transform_pos( cam_tr, g_freecam_pos );
			}
		}
	}

	inline vec3_t select_aim_point( const player_t& p, float cx, float cy, bool use_prefer_head ) {
		const vec3_t stomach_w{ p.chest_world.x, d3d11_ui::lerpf( p.pelvis_world.y, p.chest_world.y, 0.45f ), p.chest_world.z };
		if ( use_prefer_head && config::aim_prefer_head && p.is_visible ) return p.head_world;
		switch ( config::aim_bone_mode ) {
		case 0: return p.head_world;
		case 1: return p.neck_world;
		case 2: return p.chest_world;
		case 3: return p.pelvis_world;
		case 4: return stomach_w;
		case 5: return p.bone_ok[ 6 ] ? p.bones_world[ 6 ] : p.chest_world;
		case 6: return p.bone_ok[ 13 ] ? p.bones_world[ 13 ] : p.pelvis_world;
		case 7: return { p.head_world.x, p.head_world.y - 0.04f, p.head_world.z };
		case 8: return p.bone_ok[ 8 ] ? p.bones_world[ 8 ] : p.chest_world;
		case 9: return p.bone_ok[ 15 ] ? p.bones_world[ 15 ] : p.root_world;
		case 10: {
			const vec3_t cands[ 5 ] = { p.head_world, p.neck_world, p.chest_world, stomach_w, p.pelvis_world };
			float best_d = 1e9f;
			vec3_t best_w = p.head_world;
			for ( int b = 0; b < 5; ++b ) {
				vec2_t bs{};
				if ( world_to_screen( cands[ b ], bs ) ) {
					const float d = ( bs.x - cx ) * ( bs.x - cx ) + ( bs.y - cy ) * ( bs.y - cy );
					if ( d < best_d ) { best_d = d; best_w = cands[ b ]; }
				}
			}
			return best_w;
		}
		default: return p.head_world;
		}
	}

	inline void collect( ) {
		const ULONGLONG now = GetTickCount64( );
		static ULONGLONG s_last_collect = 0;
		if ( s_last_collect != 0 && ( now - s_last_collect ) < 16 ) return;
		s_last_collect = now;

		static void* s_views[ 128 ]{};
		static int s_view_count = 0;
		static ULONGLONG s_last_scan = 0;
		static void* s_last_cam = nullptr;

		if ( !unity::g_ready || !unity::g_scene_ready_cached ) {
			if ( s_last_cam != nullptr || s_view_count > 0 ) {
				s_last_cam = nullptr;
				s_view_count = 0;
				s_last_scan = 0;
				reset_scene_caches( );
			}
			return;
		}

		void* cam = unity::get_main_camera( );
		if ( !unity::is_valid_obj( cam ) ) {
			if ( s_last_cam != nullptr || s_view_count > 0 ) {
				s_last_cam = nullptr;
				s_view_count = 0;
				s_last_scan = 0;
				reset_scene_caches( );
			}
			return;
		}
		if ( cam != s_last_cam ) {
			s_last_cam = cam;
			s_view_count = 0;
			s_last_scan = 0;
			reset_scene_caches( );
		}
		g_camera = cam;

		unity::mat4_t v_mat{}, p_mat{};
		if ( !unity::get_camera_matrices( cam, v_mat, p_mat ) ) {
			g_vp_valid = false;
			g_has_aim_target = false;
			g_has_silent_target = false;
			return;
		}
		build_vp_matrix( v_mat, p_mat );

		void* cam_tr = unity::get_component_transform( cam );
		if ( unity::is_valid_obj( cam_tr ) ) {
			g_cam_prev_pos = g_cam_pos;
			unity::get_transform_pos( cam_tr, g_cam_pos );
			g_cam_vel = { ( g_cam_pos.x - g_cam_prev_pos.x ) * 60.0f, ( g_cam_pos.y - g_cam_prev_pos.y ) * 60.0f, ( g_cam_pos.z - g_cam_prev_pos.z ) * 60.0f };
		}

		static ULONGLONG s_last_page_reset = 0;
		if ( s_last_page_reset == 0 || ( now - s_last_page_reset ) >= 2000 ) {
			s_last_page_reset = now;
			unity::reset_page_cache( );
		}

		uintptr_t entities_model = 0, slots_model = 0, battle_model = 0;
		if ( !unity::resolve_battle_models( entities_model, slots_model, &battle_model ) ) {
			reset_scene_caches( );
			return;
		}
		const uintptr_t local_ent_check = unity::read_u64( entities_model + 0xD0 );
		if ( !unity::is_managed_ptr( local_ent_check ) ) {
			reset_scene_caches( );
			return;
		}

		handle_hotkeys_and_camera( cam, battle_model );
		apply_combat_and_misc_mods( battle_model, entities_model );
		refresh_metadata_and_world_cache( now, battle_model, entities_model, slots_model );
		project_world_cache( );

		if ( !unity::g_char_type_obj || !unity::m_obj_find_all ) return;
		// FindObjectsOfType is a managed call that walks the whole scene: it is the
		// main source of micro-freezes. Scan every 2 s and prune dead views between.
		int alive_views = 0;
		for ( int v = 0; v < s_view_count; ++v ) {
			if ( unity::is_valid_obj( s_views[ v ] ) ) s_views[ alive_views++ ] = s_views[ v ];
		}
		s_view_count = alive_views;

		if ( s_last_scan == 0 || now - s_last_scan >= 2000 ) {
			s_last_scan = now;
			s_view_count = 0;
			void* args[ 1 ] = { unity::g_char_type_obj };
			void* exc = nullptr;
			void* arr = unity::f_invoke( unity::m_obj_find_all, nullptr, args, &exc );
			if ( !exc && unity::is_managed_ptr( arr ) ) {
				struct il2cpp_array_t { void* k; void* m; uintptr_t b; uintptr_t max_length; void* data[ 1 ]; };
				auto* a = reinterpret_cast< il2cpp_array_t* >( arr );
				for ( uintptr_t i = 0; i < a->max_length && s_view_count < 128; ++i ) {
					if ( unity::is_valid_obj( a->data[ i ] ) ) {
						s_views[ s_view_count++ ] = a->data[ i ];
					}
				}
			}
		}

		const float flen = d3d11_ui::fast_sqrt( g_cam_fwd.x * g_cam_fwd.x + g_cam_fwd.z * g_cam_fwd.z );
		const float ux = flen > 0.001f ? ( g_cam_fwd.x / flen ) : 0.0f;
		const float uz = flen > 0.001f ? ( g_cam_fwd.z / flen ) : 1.0f;

		vec3_t raw_pos[ 128 ]{};
		bool raw_ok[ 128 ]{};
		int local_idx = -1;
		float best_local_dist = 4.8f;

		for ( int i = 0; i < s_view_count; ++i ) {
			void* p = s_views[ i ];
			if ( !unity::is_valid_obj( p ) ) continue;
			void* tr = unity::get_component_transform( p );
			if ( !unity::is_valid_obj( tr ) ) continue;
			if ( !unity::get_transform_pos( tr, raw_pos[ i ] ) ) continue;
			raw_ok[ i ] = true;

			const float dx = raw_pos[ i ].x - g_cam_pos.x;
			const float dy = raw_pos[ i ].y - g_cam_pos.y;
			const float dz = raw_pos[ i ].z - g_cam_pos.z;
			const float dist = d3d11_ui::fast_sqrt( dx * dx + dy * dy + dz * dz );
			const float horiz = d3d11_ui::fast_sqrt( dx * dx + dz * dz );
			const float dot_fwd = dx * ux + dz * uz;
			if ( ( horiz < 1.2f || dot_fwd > 0.1f ) && dist < best_local_dist ) {
				best_local_dist = dist;
				local_idx = i;
			}
		}

		const LONG back = 1 - InterlockedCompareExchange( &g_active_buf, 0, 0 );
		player_t* dst = g_players_buf[ back ];
		int next_count = 0;

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;

		float eff_aim_dist = config::aim_max_dist;
		if ( config::aim_weapon_aware && g_local_weapon[ 0 ] ) {
			if ( unity::str_icontains( g_local_weapon, "Shotgun" ) ) eff_aim_dist = 45.0f;
			else if ( unity::str_icontains( g_local_weapon, "Pistol" ) ) eff_aim_dist = 95.0f;
			else if ( unity::str_icontains( g_local_weapon, "SMG" ) ) eff_aim_dist = 135.0f;
			else if ( unity::str_icontains( g_local_weapon, "Sniper" ) ) eff_aim_dist = 500.0f;
		}

		float best_score = 1e9f;
		int best_aim_idx = -1;
		vec3_t best_aim_world{};
		vec2_t best_aim_screen{};

		float best_silent_fov = config::aim_silent_fov;
		int best_silent_idx = -1;
		vec2_t best_silent_screen{};
		int bone_budget = 2;
		// Each player with a resolved skeleton costs 19 native transform reads per
		// frame: 128 players in range is ~2400 managed calls every frame. Past ~80 m
		// the real skeleton is a few dozen pixels tall and the screen-space figure
		// reads better, so the bone work is capped per frame.
		int skeleton_slots = 32;

		for ( int i = 0; i < s_view_count && next_count < 256; ++i ) {
			if ( !raw_ok[ i ] || i == local_idx ) continue;
			void* p = s_views[ i ];
			const vec3_t& pos = raw_pos[ i ];
			const meta_entry_t* meta = find_nearest_meta( pos );
			if ( meta && meta->is_local ) continue;

			const float dist = dist3( pos, g_cam_pos );
			vec3_t head_bone_w{ pos.x, pos.y + 1.58f, pos.z };
			vec3_t box_top_w{ pos.x, pos.y + 1.75f, pos.z };
			void* head_tr = nullptr;
			if ( unity::is_mem_readable( reinterpret_cast< uintptr_t >( p ) + 0xE8, sizeof( void* ) ) ) {
				head_tr = *reinterpret_cast< void** >( reinterpret_cast< uintptr_t >( p ) + 0xE8 );
			}
			if ( unity::is_valid_obj( head_tr ) ) {
				vec3_t hp{};
				if ( unity::get_transform_pos( head_tr, hp ) && ( hp.y - pos.y ) > 0.35f ) {
					head_bone_w = hp;
					box_top_w = { hp.x, hp.y + 0.16f, hp.z };
				}
			}

			const float h_span = head_bone_w.y - pos.y;
			vec3_t neck_w{ head_bone_w.x, pos.y + h_span * 0.88f, head_bone_w.z };
			vec3_t chest_w{ pos.x, pos.y + h_span * 0.70f, pos.z };
			vec3_t pelvis_w{ pos.x, pos.y + h_span * 0.50f, pos.z };

			vec2_t hs{}, ns{}, cs{}, ps{}, rs{};
			bool behind_h = false, behind_r = false;
			if ( !w2s_vp( box_top_w, hs, behind_h ) ) continue;
			if ( !w2s_vp( pos, rs, behind_r ) ) continue;

			const bool on_scr = !behind_h && !behind_r &&
				hs.x >= -100.0f && hs.x <= d3d11_ui::g_width + 100.0f &&
				hs.y >= -100.0f && hs.y <= d3d11_ui::g_height + 100.0f;

			player_t& out = dst[ next_count ];
			out = {};
			out.obj = p;
			out.root_world = pos;
			out.head_world = head_bone_w;
			out.neck_world = neck_w;
			out.chest_world = chest_w;
			out.pelvis_world = pelvis_w;
			out.velocity = update_velocity( p, pos, now );
			out.head_screen = hs;
			out.root_screen = rs;
			out.distance = dist;
			out.health = 1.0f;
			out.on_screen = on_scr;
			out.behind = behind_h || behind_r;
			out.valid = true;

			if ( on_scr ) {
				world_to_screen( neck_w, ns );
				world_to_screen( chest_w, cs );
				world_to_screen( pelvis_w, ps );
				out.neck_screen = ns;
				out.chest_screen = cs;
				out.pelvis_screen = ps;

				if ( skeleton_slots > 0 && dist <= 80.0f && ( config::esp_skeleton || config::esp_hitboxes ) ) {
					--skeleton_slots;
					const bone_cache_t* bc = get_or_resolve_bones( p, now, bone_budget );
					if ( bc ) {
						int ok_cnt = 0;
						vec3_t bpos{};
						for ( int b = 0; b < k_bone_count; ++b ) {
							if ( bc->native_bone_trs[ b ] && unity::get_transform_pos_native( bc->native_bone_trs[ b ], bpos ) ) {
								// Reject bones that are nowhere near the body. A stale native pointer or
								// a mismatched rig otherwise draws lines straight across the screen.
								const float bone_dy = bpos.y - pos.y;
								const float bone_dx = bpos.x - pos.x;
								const float bone_dz = bpos.z - pos.z;
								if ( bone_dy < -0.35f || bone_dy > 2.30f || ( bone_dx * bone_dx + bone_dz * bone_dz ) > 1.69f ) continue;
								out.bones_world[ b ] = bpos;
								if ( world_to_screen( bpos, out.bones[ b ] ) ) {
									out.bone_ok[ b ] = true;
									++ok_cnt;
								}
								if ( b == 0 ) {
									out.head_world = bpos;
									vec2_t top_s{};
									if ( world_to_screen( { bpos.x, bpos.y + 0.16f, bpos.z }, top_s ) ) out.head_screen = top_s;
								}
								else if ( b == 1 ) out.neck_world = bpos;
								else if ( b == 2 ) out.chest_world = bpos;
								else if ( b == 4 ) out.pelvis_world = bpos;
							}
						}
						out.has_real_skeleton = ( ok_cnt >= 6 );

						out.skel_edge_count = 0;
						float lo_y = 1e9f;
						float hi_y = -1e9f;
						if ( bc->edge_count > 0 ) {
							for ( int n2 = 0; n2 < bc->node_count; ++n2 ) {
								vec3_t np{};
								const bool got = unity::get_transform_pos( bc->nodes[ n2 ], np );
								out.skel_p_ok[ n2 ] = got && world_to_screen( np, out.skel_p[ n2 ] );
								if ( !got ) continue;
								// Only nodes that belong to this body feed the vertical extent.
								const float ndy = np.y - pos.y;
								const float ndx = np.x - pos.x;
								const float ndz = np.z - pos.z;
								if ( ndy < -0.60f || ndy > 2.40f || ( ndx * ndx + ndz * ndz ) > 1.69f ) continue;
								if ( np.y < lo_y ) lo_y = np.y;
								if ( np.y > hi_y ) hi_y = np.y;
							}
							for ( int e = 0; e < bc->edge_count; ++e ) {
								const int ia = bc->ea[ e ];
								const int ib = bc->eb[ e ];
								if ( !out.skel_p_ok[ ia ] || !out.skel_p_ok[ ib ] ) continue;
								out.skel_ea[ out.skel_edge_count ] = static_cast< unsigned char >( ia );
								out.skel_eb[ out.skel_edge_count ] = static_cast< unsigned char >( ib );
								++out.skel_edge_count;
							}

							// The box and the body points follow the bones, not the head alone.
							// Crouching, sitting or lying down lowers the head without moving the
							// entity origin, which is what made the box read as "head glued to the
							// character, rest of the box left standing". The span gate keeps a
							// partially resolved skeleton from shrinking the box onto the torso.
							if ( hi_y - lo_y >= 0.85f && hi_y - lo_y <= 2.60f ) {
								const float span = hi_y - lo_y;
								out.head_world = { out.head_world.x, hi_y, out.head_world.z };
								out.root_world = { out.root_world.x, lo_y, out.root_world.z };
								vec2_t top_s{}, bot_s{};
								if ( world_to_screen( { out.head_world.x, hi_y + 0.16f, out.head_world.z }, top_s ) ) out.head_screen = top_s;
								if ( world_to_screen( { out.root_world.x, lo_y, out.root_world.z }, bot_s ) ) out.root_screen = bot_s;
								out.neck_world = { out.head_world.x, lo_y + span * 0.88f, out.head_world.z };
								out.chest_world = { out.root_world.x, lo_y + span * 0.70f, out.root_world.z };
								out.pelvis_world = { out.root_world.x, lo_y + span * 0.50f, out.root_world.z };
								world_to_screen( out.neck_world, ns );
								world_to_screen( out.chest_world, cs );
								world_to_screen( out.pelvis_world, ps );
								out.neck_screen = ns;
								out.chest_screen = cs;
								out.pelvis_screen = ps;
							}
						}
					}
				}
			}

			if ( meta ) {
				out.entity_id = meta->entity_id;
				out.vehicle_eid = meta->vehicle_eid;
				out.role_kind = meta->role_kind;
				out.is_team = ( meta->role_kind == ROLE_TEAM );
				out.is_friend = config::is_friend( meta->entity_id );
				out.has_shield = meta->has_armor;
				out.shield_value = meta->has_armor ? 1.0f : 0.0f;
				out.in_vehicle = meta->in_vehicle;
				out.is_knocked = meta->is_knocked;
				out.is_dead = meta->is_dead;
				out.is_crouching = ( h_span < 1.25f && !meta->in_vehicle );
				out.look_dir = meta->look_dir;
				copy_str( out.name, sizeof( out.name ), meta->name );
				copy_str( out.tag, sizeof( out.tag ), meta->tag );
				copy_str( out.clan, sizeof( out.clan ), meta->clan );
				copy_str( out.weapon, sizeof( out.weapon ), meta->weapon );
			}

			// Visibility LoS check
			if ( config::aim_vis_check && on_scr && dist <= eff_aim_dist && unity::fn_phys_linecast ) {
				out.is_visible = !unity::unity_linecast( g_cam_pos, out.head_world );
			}

			const bool skip_clan = ( config::aim_ignore_clan && g_local_clan[ 0 ] && out.clan[ 0 ] && unity::str_eq( g_local_clan, out.clan ) );
			const bool skip_police = ( config::aim_ignore_police && out.role_kind == ROLE_COP );
			const bool skip_knocked = ( config::aim_ignore_knocked && out.is_knocked );
			const bool skip_friend = ( config::aim_ignore_friends && out.is_friend );
			const bool skip_dead = out.is_dead;

			if ( config::aim_master && on_scr && dist <= eff_aim_dist &&
				 !skip_clan && !skip_police && !skip_knocked && !skip_friend && !skip_dead &&
				 ( !config::aim_vis_check || out.is_visible ) ) {
				vec3_t aim_w = select_aim_point( out, cx, cy, true );
				if ( config::aim_multipoint ) {
					const float r = 0.07f * config::aim_multipoint_scale;
					const vec3_t pts[ 5 ] = {
						aim_w,
						{ aim_w.x + r, aim_w.y, aim_w.z },
						{ aim_w.x - r, aim_w.y, aim_w.z },
						{ aim_w.x, aim_w.y + r, aim_w.z },
						{ aim_w.x, aim_w.y - r, aim_w.z }
					};
					float best_mp = 1e9f;
					for ( int m = 0; m < 5; ++m ) {
						vec2_t ms{};
						if ( world_to_screen( pts[ m ], ms ) ) {
							const float d = ( ms.x - cx ) * ( ms.x - cx ) + ( ms.y - cy ) * ( ms.y - cy );
							if ( d < best_mp ) { best_mp = d; aim_w = pts[ m ]; }
						}
					}
				}
				if ( config::pred_enabled ) {
					const float lead_t = ( ( dist / 400.0f ) + ( config::pred_net_buffer_ms * 0.001f ) ) * config::pred_lead_scale;
					if ( !out.in_vehicle || config::pred_vehicle ) {
						aim_w.x += out.velocity.x * lead_t;
						aim_w.y += out.velocity.y * lead_t;
						aim_w.z += out.velocity.z * lead_t;
					}
					if ( config::pred_camera ) {
						aim_w.x -= g_cam_vel.x * ( lead_t * 0.25f );
						aim_w.z -= g_cam_vel.z * ( lead_t * 0.25f );
					}
				}

				vec2_t aim_s{};
				if ( world_to_screen( aim_w, aim_s ) ) {
					const float adx = aim_s.x - cx;
					const float ady = aim_s.y - cy;
					const float fov_d = d3d11_ui::fast_sqrt( adx * adx + ady * ady );
					const float score = config::aim_closest_first ? dist : fov_d;
					const bool is_locked = ( config::aim_sticky && g_locked_obj == p && fov_d <= config::aim_fov * 1.35f );
					if ( ( is_locked || fov_d <= config::aim_fov ) && ( is_locked || score < best_score ) ) {
						best_score = is_locked ? -1.0f : score;
						best_aim_idx = next_count;
						best_aim_world = aim_w;
						best_aim_screen = aim_s;
					}
					if ( config::aim_mode == 2 && fov_d <= best_silent_fov ) {
						best_silent_fov = fov_d;
						best_silent_idx = next_count;
						best_silent_screen = aim_s;
					}
				}
			}

			++next_count;
		}

		g_player_count_buf[ back ] = next_count;
		InterlockedExchange( &g_active_buf, back );
		config::diag_players_tracked = next_count;

		const bool key_down = !config::menu_open && is_aim_key_down( );
		g_is_firing_or_aiming = key_down || ( ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 );

		if ( best_aim_idx >= 0 ) {
			g_locked_obj = dst[ best_aim_idx ].obj;
			g_aim_target_screen = best_aim_screen;
			g_has_aim_target = true;
			if ( key_down && roll_percent( config::aim_hit_chance ) ) {
				if ( config::aim_mode == 1 ) apply_direct_3d_aim( battle_model, best_aim_world );
				else if ( config::aim_mode == 0 ) apply_mouse_aim_delta( best_aim_screen.x - cx, best_aim_screen.y - cy );
			} else {
				g_aim_rem_x = 0.0f;
				g_aim_rem_y = 0.0f;
			}
		} else {
			g_locked_obj = nullptr;
			g_has_aim_target = false;
			g_aim_rem_x = 0.0f;
			g_aim_rem_y = 0.0f;
		}

		// Silent Aim
		if ( config::aim_master && config::aim_mode == 2 && best_silent_idx >= 0 ) {
			g_silent_target_screen = best_silent_screen;
			g_has_silent_target = true;
			if ( dst[ best_silent_idx ].entity_id > 0 ) {
				apply_silent_target( battle_model, dst[ best_silent_idx ].entity_id );
			}
		} else {
			g_has_silent_target = false;
		}

		// Triggerbot
		static ULONGLONG s_trig_last = 0;
		static bool s_trig_armed = true;
		if ( !config::trig_enabled || config::menu_open || !g_has_aim_target ) s_trig_armed = true;
		if ( config::trig_enabled && !config::menu_open && g_has_aim_target ) {
			const float tdx = g_aim_target_screen.x - cx;
			const float tdy = g_aim_target_screen.y - cy;
			if ( s_trig_armed && d3d11_ui::fast_sqrt( tdx * tdx + tdy * tdy ) <= config::trig_fov &&
				 ( now - s_trig_last ) >= static_cast< ULONGLONG >( config::trig_delay_ms ) ) {
				s_trig_last = now;
				if ( config::trig_semi_auto ) s_trig_armed = false;
				INPUT inp[ 2 ]{};
				inp[ 0 ].type = INPUT_MOUSE;
				inp[ 0 ].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				inp[ 1 ].type = INPUT_MOUSE;
				inp[ 1 ].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SendInput( 2, inp, sizeof( INPUT ) );
			}
		}
	}

	inline d3d11_ui::color_t get_role_color( const player_t& p ) {
		if ( p.is_friend ) {
			return { config::esp_col.friend_col.r, config::esp_col.friend_col.g, config::esp_col.friend_col.b, 1.0f };
		}
		if ( config::esp_police_coworkers && g_local_role == ROLE_COP && p.role_kind == ROLE_COP ) {
			return { config::esp_col.team.r, config::esp_col.team.g, config::esp_col.team.b, 1.0f };
		}
		int idx = 1;
		if ( p.role_kind == ROLE_COP ) idx = 0;
		else if ( p.role_kind == ROLE_CIV ) idx = 2;
		const config::rgba_t& r = config::esp_prof[ idx ].color;
		return { r.r, r.g, r.b, r.a };
	}

	// Bone index pairs (see k_human_bones) plus the maximum screen length of each
	// segment, as a fraction of the ESP box height. A longer segment means the bone
	// resolved to a bad transform, and drawing it is what produced the lines cutting
	// straight across the body.
	// Bounds are in metres, because the distance between two bones does not depend
	// on how far the player is from the camera. Judging by screen pixels meant the
	// same rig was accepted at 5 m and rejected at 40 m.
	struct skel_seg_t { int a, b; float min_m; float max_m; };
	inline constexpr skel_seg_t k_skel_segs[] = {
		{ 0, 1, 0.01f, 0.30f }, { 1, 2, 0.01f, 0.30f }, { 2, 3, 0.01f, 0.35f }, { 3, 4, 0.01f, 0.40f }, // spine
		{ 2, 5, 0.02f, 0.32f }, { 5, 6, 0.02f, 0.28f }, { 6, 7, 0.05f, 0.50f }, { 7, 8, 0.03f, 0.45f }, // left arm
		{ 2, 9, 0.02f, 0.32f }, { 9, 10, 0.02f, 0.28f }, { 10, 11, 0.05f, 0.50f }, { 11, 12, 0.03f, 0.45f }, // right arm
		{ 4, 13, 0.02f, 0.32f }, { 13, 14, 0.10f, 0.68f }, { 14, 15, 0.05f, 0.68f },                    // left leg
		{ 4, 16, 0.02f, 0.32f }, { 16, 17, 0.10f, 0.68f }, { 17, 18, 0.05f, 0.68f }                     // right leg
	};
	inline constexpr int k_skel_seg_count = static_cast< int >( sizeof( k_skel_segs ) / sizeof( k_skel_segs[ 0 ] ) );

	inline bool skeleton_segment_ok( const player_t& p, const skel_seg_t& s, float body_h ) {
		if ( !p.bone_ok[ s.a ] || !p.bone_ok[ s.b ] ) return false;

		const vec3_t& wa = p.bones_world[ s.a ];
		const vec3_t& wb = p.bones_world[ s.b ];
		const float wx = wa.x - wb.x;
		const float wy = wa.y - wb.y;
		const float wz = wa.z - wb.z;
		const float world_len = d3d11_ui::fast_sqrt( wx * wx + wy * wy + wz * wz );
		if ( world_len < s.min_m || world_len > s.max_m ) return false;

		// Second gate in screen space: a correct rig never projects a limb longer than
		// the body box, so anything that does is a stale transform.
		const float dx = p.bones[ s.a ].x - p.bones[ s.b ].x;
		const float dy = p.bones[ s.a ].y - p.bones[ s.b ].y;
		const float screen_len = d3d11_ui::fast_sqrt( dx * dx + dy * dy );
		return screen_len > 0.5f && screen_len <= body_h * 1.05f;
	}

	inline void draw_skeleton_real( const player_t& p, float body_h, const d3d11_ui::color_t& c, const d3d11_ui::color_t& shadow, float lw ) {
		for ( int i = 0; i < k_skel_seg_count; ++i ) {
			const skel_seg_t& s = k_skel_segs[ i ];
			if ( !skeleton_segment_ok( p, s, body_h ) ) continue;
			d3d11_ui::add_line( p.bones[ s.a ].x, p.bones[ s.a ].y, p.bones[ s.b ].x, p.bones[ s.b ].y, lw + 1.2f, shadow );
			d3d11_ui::add_line( p.bones[ s.a ].x, p.bones[ s.a ].y, p.bones[ s.b ].x, p.bones[ s.b ].y, lw, c );
		}

		if ( !p.bone_ok[ 0 ] ) return;
		float hr = d3d11_ui::clampf( body_h * 0.075f, 2.5f, 22.0f );
		if ( p.bone_ok[ 1 ] ) {
			const float dx = p.bones[ 0 ].x - p.bones[ 1 ].x;
			const float dy = p.bones[ 0 ].y - p.bones[ 1 ].y;
			const float d = d3d11_ui::fast_sqrt( dx * dx + dy * dy );
			if ( d > 1.0f ) hr = d3d11_ui::clampf( d * 0.62f, 2.5f, 22.0f );
		}
		d3d11_ui::add_circle( p.bones[ 0 ].x, p.bones[ 0 ].y, hr, 18, lw + 1.1f, shadow );
		d3d11_ui::add_circle( p.bones[ 0 ].x, p.bones[ 0 ].y, hr, 18, lw, c );
	}

	// Fallback figure drawn entirely in screen space. Building it from the 3D
	// look_dir put the elbows and hands in front of the torso when the character
	// faced the camera, which is exactly what made it unreadable.
	inline void draw_skeleton_fallback( const player_t& p, float body_h, const d3d11_ui::color_t& c, const d3d11_ui::color_t& shadow, float lw ) {
		const float bottom = p.root_screen.y;
		const float h = body_h > 8.0f ? body_h : 8.0f;
		const float top = bottom - h;
		const float cx = ( p.head_screen.x + p.root_screen.x ) * 0.5f;

		const float neck_y = top + h * 0.155f;
		const float shoulder_y = top + h * 0.195f;
		const float hand_y = top + h * 0.47f;
		const float pelvis_y = top + h * 0.52f;

		const float sh_w = h * 0.115f;
		const float hand_w = h * 0.205f;
		const float foot_w = h * 0.115f;

		auto seg = [ & ]( float x1, float y1, float x2, float y2 ) {
			d3d11_ui::add_line( x1, y1, x2, y2, lw + 1.1f, shadow );
			d3d11_ui::add_line( x1, y1, x2, y2, lw, c );
		};

		seg( cx, neck_y, cx, pelvis_y );                          // spine
		seg( cx, shoulder_y, cx - sh_w, shoulder_y );             // left clavicle
		seg( cx, shoulder_y, cx + sh_w, shoulder_y );             // right clavicle
		seg( cx - sh_w, shoulder_y, cx - hand_w, hand_y );        // left arm
		seg( cx + sh_w, shoulder_y, cx + hand_w, hand_y );        // right arm
		seg( cx, pelvis_y, cx - foot_w, bottom );                 // left leg
		seg( cx, pelvis_y, cx + foot_w, bottom );                 // right leg

		const float hr = d3d11_ui::clampf( h * 0.075f, 2.5f, 22.0f );
		const float hy = top + h * 0.085f;
		d3d11_ui::add_circle( cx, hy, hr, 18, lw + 1.1f, shadow );
		d3d11_ui::add_circle( cx, hy, hr, 18, lw, c );
	}

	inline void draw_skeleton( const player_t& p, float body_h, const d3d11_ui::color_t& c ) {
		const float lw = config::esp_skel_width > 0.5f ? config::esp_skel_width : 1.3f;
		const d3d11_ui::color_t shadow{ 0.0f, 0.0f, 0.0f, c.a * 0.85f };

		// Hierarchy skeleton first: those are the player's own transforms, so the
		// shape follows the pose whatever the rig looks like.
		if ( p.skel_edge_count >= 4 ) {
			for ( int e = 0; e < p.skel_edge_count; ++e ) {
				const vec2_t& sa = p.skel_p[ p.skel_ea[ e ] ];
				const vec2_t& sb = p.skel_p[ p.skel_eb[ e ] ];
				d3d11_ui::add_line( sa.x, sa.y, sb.x, sb.y, lw + 1.2f, shadow );
				d3d11_ui::add_line( sa.x, sa.y, sb.x, sb.y, lw, c );
			}
			const float hr = d3d11_ui::clampf( body_h * 0.075f, 2.5f, 22.0f );
			if ( p.skel_p_ok[ 0 ] ) {
				d3d11_ui::add_circle( p.skel_p[ 0 ].x, p.skel_p[ 0 ].y, hr, 18, lw + 1.1f, shadow );
				d3d11_ui::add_circle( p.skel_p[ 0 ].x, p.skel_p[ 0 ].y, hr, 18, lw, c );
			}
			return;
		}

		{
			// Four valid segments already read as a body. The previous gate demanded
			// nine of eighteen, which a rig with six to ten resolved bones can never
			// reach: the real skeleton was discarded in favour of the synthetic one
			// even when the bones were perfectly good.
			int valid = 0;
			for ( int i = 0; i < k_skel_seg_count; ++i ) {
				if ( skeleton_segment_ok( p, k_skel_segs[ i ], body_h ) ) ++valid;
			}
			if ( valid >= 4 ) {
				draw_skeleton_real( p, body_h, c, shadow, lw );
				return;
			}
		}

		draw_skeleton_fallback( p, body_h, c, shadow, lw );
	}

	inline void draw_3d_hitboxes( const player_t& p, const d3d11_ui::color_t& c ) {
		// Head sphere projection + torso capsule
		vec2_t hs{}, htop{};
		if ( world_to_screen( p.head_world, hs ) && world_to_screen( { p.head_world.x, p.head_world.y + 0.13f, p.head_world.z }, htop ) ) {
			const float rad = d3d11_ui::clampf( hs.y - htop.y, 3.0f, 32.0f );
			d3d11_ui::add_circle( hs.x, hs.y, rad, 16, 1.3f, c );
		}
		vec2_t ch{}, pv{};
		if ( world_to_screen( p.chest_world, ch ) && world_to_screen( p.pelvis_world, pv ) ) {
			const float cap_w = d3d11_ui::clampf( ( pv.y - ch.y ) * 0.48f, 4.0f, 36.0f );
			d3d11_ui::add_rounded_rect( ch.x - cap_w * 0.5f, ch.y, cap_w, pv.y - ch.y, cap_w * 0.35f, 1.2f, c );
		}
	}

	inline void render_radar( const player_t* players, int count ) {
		const float r_size = d3d11_ui::clampf( config::radar_size, 110.0f, 260.0f );
		const float rx = 18.0f, ry = 18.0f;
		const float rcx = rx + r_size * 0.5f, rcy = ry + r_size * 0.5f;

		d3d11_ui::add_shadow_rect( rx, ry, r_size, r_size, 7.0f, 0.25f );
		d3d11_ui::add_filled_rounded_rect( rx, ry, r_size, r_size, 7.0f, { 0.047f, 0.055f, 0.082f, 0.88f } );
		d3d11_ui::add_rounded_rect( rx, ry, r_size, r_size, 7.0f, 1.0f, { 0.155f, 0.184f, 0.275f, 0.92f } );
		d3d11_ui::add_line( rcx, ry + 5.0f, rcx, ry + r_size - 5.0f, 1.0f, { 0.18f, 0.21f, 0.32f, 0.45f } );
		d3d11_ui::add_line( rx + 5.0f, rcy, rx + r_size - 5.0f, rcy, 1.0f, { 0.18f, 0.21f, 0.32f, 0.45f } );
		d3d11_ui::add_circle( rcx, rcy, r_size * 0.24f, 24, 1.0f, { 0.18f, 0.21f, 0.32f, 0.38f } );
		d3d11_ui::add_circle( rcx, rcy, r_size * 0.44f, 32, 1.0f, { 0.18f, 0.21f, 0.32f, 0.38f } );
		d3d11_ui::add_filled_circle( rcx, rcy, 3.0f, 8, { 1.0f, 1.0f, 1.0f, 1.0f } );

		const float flen = d3d11_ui::fast_sqrt( g_cam_fwd.x * g_cam_fwd.x + g_cam_fwd.z * g_cam_fwd.z );
		const float fx = flen > 0.001f ? ( g_cam_fwd.x / flen ) : 0.0f;
		const float fz = flen > 0.001f ? ( g_cam_fwd.z / flen ) : 1.0f;
		const float r_x = fz, r_z = -fx;
		const float range = config::radar_range > 10.0f ? config::radar_range : 150.0f;
		const float scale = ( r_size * 0.45f ) / range;

		if ( config::radar_world_markers && config::world_master ) {
			const LONG wact = InterlockedCompareExchange( &g_world_active_buf, 0, 0 );
			const int wcnt = g_world_count_buf[ wact ];
			const world_item_t* witems = g_world_buf[ wact ];
			for ( int i = 0; i < wcnt; ++i ) {
				const world_item_t& it = witems[ i ];
				if ( !it.valid ) continue;
				const float dx = it.world_pos.x - g_cam_pos.x;
				const float dz = it.world_pos.z - g_cam_pos.z;
				const float px = d3d11_ui::clampf( rcx + ( dx * r_x + dz * r_z ) * scale, rx + 5.0f, rx + r_size - 5.0f );
				const float py = d3d11_ui::clampf( rcy - ( dx * fx + dz * fz ) * scale, ry + 5.0f, ry + r_size - 5.0f );
				const d3d11_ui::color_t wc = ( it.category == WCAT_CAR || it.category == WCAT_HELI )
					? d3d11_ui::color_t{ config::esp_col.vehicle.r, config::esp_col.vehicle.g, config::esp_col.vehicle.b, 0.85f }
					: get_rarity_color( it.rarity );
				d3d11_ui::add_filled_rect( px - 1.5f, py - 1.5f, 3.0f, 3.0f, wc );
			}
		}

		for ( int i = 0; i < count; ++i ) {
			const player_t& p = players[ i ];
			if ( !p.valid || p.is_dead ) continue;
			const float dx = p.root_world.x - g_cam_pos.x;
			const float dz = p.root_world.z - g_cam_pos.z;
			const float px = d3d11_ui::clampf( rcx + ( dx * r_x + dz * r_z ) * scale, rx + 5.0f, rx + r_size - 5.0f );
			const float py = d3d11_ui::clampf( rcy - ( dx * fx + dz * fz ) * scale, ry + 5.0f, ry + r_size - 5.0f );
			const d3d11_ui::color_t rc = get_role_color( p );
			if ( p.is_knocked ) {
				d3d11_ui::add_circle( px, py, 3.2f, 10, 1.3f, rc ); // hollow dot for knocked
			} else {
				d3d11_ui::add_filled_circle( px, py, 3.6f, 8, { 0.0f, 0.0f, 0.0f, 0.85f } );
				d3d11_ui::add_filled_circle( px, py, 2.6f, 8, rc );
			}
		}
	}

	inline void render_world( ) {
		if ( !config::world_master ) return;
		const LONG active = InterlockedCompareExchange( &g_world_active_buf, 0, 0 );
		const int count = g_world_count_buf[ active ];
		const world_item_t* items = g_world_buf[ active ];
		for ( int i = 0; i < count; ++i ) {
			const world_item_t& it = items[ i ];
			if ( !it.valid ) continue;
			d3d11_ui::color_t c = get_rarity_color( it.rarity );
			if ( it.category == WCAT_CAR ) c = { config::esp_col.vehicle.r, config::esp_col.vehicle.g, config::esp_col.vehicle.b, 1.0f };
			else if ( it.category == WCAT_HELI ) c = { config::esp_col.heli.r, config::esp_col.heli.g, config::esp_col.heli.b, 1.0f };
			else if ( it.category == WCAT_BUYER ) c = { config::esp_col.buyer.r, config::esp_col.buyer.g, config::esp_col.buyer.b, 1.0f };

			d3d11_ui::add_filled_circle( it.screen_pos.x, it.screen_pos.y - 8.0f, 3.0f, 8, { 0.0f, 0.0f, 0.0f, 0.85f } );
			d3d11_ui::add_filled_circle( it.screen_pos.x, it.screen_pos.y - 8.0f, 2.0f, 8, c );
			char buf[ 80 ]{};
			char* bend = buf + sizeof( buf ) - 1;
			char* bp = d3d11_ui::put_str( buf, bend, it.label[ 0 ] ? it.label : "ITEM" );
			bp = d3d11_ui::put_char( bp, bend, ' ' );
			bp = d3d11_ui::put_char( bp, bend, '[' );
			bp = d3d11_ui::put_int( bp, bend, static_cast< int >( it.distance ) );
			bp = d3d11_ui::put_char( bp, bend, 'm' );
			bp = d3d11_ui::put_char( bp, bend, ']' );
			*bp = '\0';
			draw_esp_text( it.screen_pos.x, it.screen_pos.y + 4.0f, buf, c, 0.76f );
		}
	}

	inline void render( ) {
		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;
		const ULONGLONG lu_tick = g_last_late_update_tick;
		if ( !g_camera || !config::model_chain_ok || lu_tick == 0 || ( GetTickCount64( ) - lu_tick ) > 350 ) return;

		if ( config::aim_master && config::aim_draw_fov ) {
			d3d11_ui::add_circle( cx, cy, config::aim_fov, 64, 1.2f,
				{ config::esp_col.fov.r, config::esp_col.fov.g, config::esp_col.fov.b, config::esp_col.fov.a } );
			if ( config::aim_mode == 2 ) {
				d3d11_ui::add_circle( cx, cy, config::aim_silent_fov, 48, 1.0f, { 1.0f, 0.35f, 0.45f, 0.65f } );
			}
		}
		if ( config::aim_master && config::aim_draw_line && g_has_aim_target ) {
			const d3d11_ui::color_t lcol{ config::esp_col.fov.r, config::esp_col.fov.g, config::esp_col.fov.b, 0.90f };
			d3d11_ui::add_line( cx, cy, g_aim_target_screen.x, g_aim_target_screen.y, 1.2f, lcol );
			d3d11_ui::add_filled_circle( g_aim_target_screen.x, g_aim_target_screen.y, 3.0f, 8, lcol );
		}
		if ( config::aim_master && config::aim_silent_indicator && g_has_silent_target ) {
			d3d11_ui::add_circle( g_silent_target_screen.x, g_silent_target_screen.y, 6.0f, 16, 1.4f, { 1.0f, 0.25f, 0.35f, 0.95f } );
		}

		render_world( );
		if ( !config::esp_master ) return;

		const LONG active = InterlockedCompareExchange( &g_active_buf, 0, 0 );
		const int count = g_player_count_buf[ active ];
		const player_t* players = g_players_buf[ active ];

		if ( config::esp_radar ) render_radar( players, count );

		if ( config::esp_death_point ) {
			const ULONGLONG death_now = GetTickCount64( );
			for ( int d = 0; d < 64; ++d ) {
				if ( !g_deaths[ d ].valid ) continue;
				// Markers used to live for the whole session: after 64 deaths the array
				// was full and nothing new could be recorded for the rest of the match.
				if ( death_now - g_deaths[ d ].tick > 300000ull ) {
					g_deaths[ d ].valid = false;
					continue;
				}
				vec2_t ds{};
				if ( world_to_screen( g_deaths[ d ].pos, ds ) ) {
					char dbuf[ 48 ]{};
					char* dend = dbuf + sizeof( dbuf ) - 1;
					char* dp = d3d11_ui::put_str( dbuf, dend, "DEATH: " );
					dp = d3d11_ui::put_str( dp, dend, g_deaths[ d ].name );
					*dp = '\0';
					draw_esp_text( ds.x, ds.y, dbuf, { 0.95f, 0.35f, 0.35f, 0.85f }, 0.74f );
				}
			}
		}

		// Minimal noise on fire/aim (priority mode 6)
		const bool minimal_noise = ( config::esp_priority_mode == 2 && g_is_firing_or_aiming );
		vec2_t drawn_labels[ 64 ]{};
		int drawn_label_cnt = 0;

		for ( int i = 0; i < count; ++i ) {
			const player_t& p = players[ i ];
			if ( !p.valid ) continue;
			if ( p.is_dead && !config::esp_corpse ) continue;

			const int prof_idx = ( p.role_kind == ROLE_COP ) ? 0 : ( p.role_kind == ROLE_CIV ? 2 : 1 );
			if ( !config::esp_prof[ prof_idx ].enabled ) continue;
			if ( p.distance > config::esp_prof[ prof_idx ].max_dist ) continue;

			const d3d11_ui::color_t col = get_role_color( p );

			if ( !p.on_screen ) {
				if ( config::esp_oof_arrows && !minimal_noise ) {
					float ax = p.head_screen.x - cx;
					float ay = p.head_screen.y - cy;
					if ( p.behind ) { ax = -ax; ay = -ay; }
					const float alen = d3d11_ui::fast_sqrt( ax * ax + ay * ay );
					if ( alen > 0.001f ) {
						const float dx = ax / alen, dy = ay / alen;
						const float radius = d3d11_ui::g_height * config::esp_oof_inset;
						const float px = cx + dx * radius, py = cy + dy * radius;
						const float nx = -dy, ny = dx;
						const float sz = config::esp_oof_size;
						d3d11_ui::add_filled_triangle(
							px + dx * sz, py + dy * sz,
							px - dx * ( sz * 0.5f ) + nx * ( sz * 0.68f ), py - dy * ( sz * 0.5f ) + ny * ( sz * 0.68f ),
							px - dx * ( sz * 0.5f ) - nx * ( sz * 0.68f ), py - dy * ( sz * 0.5f ) - ny * ( sz * 0.68f ),
							col
						);
					}
				}
				continue;
			}

			const vec2_t& head_s = p.head_screen;
			const vec2_t& root_s = p.root_screen;
			const float box_h = root_s.y - head_s.y;
			if ( box_h < 4.0f || box_h > 1500.0f ) continue;
			// Below ~9 px the labels are unreachable anyway; generating 6 outlined
			// strings for them is pure vertex churn in big lobbies.
			const bool tiny_target = box_h < 9.0f;
			const float box_w = box_h * 0.52f;
			const float box_x = head_s.x - box_w * 0.5f;
			const float box_y = head_s.y;

			if ( config::esp_box_fill ) {
				d3d11_ui::add_filled_rect( box_x, box_y, box_w, box_h, { col.r, col.g, col.b, 0.14f } );
			}
			if ( config::esp_box ) {
				if ( config::esp_box_outline ) d3d11_ui::add_rect_outlined( box_x, box_y, box_w, box_h, 1.0f, col, { 0.0f, 0.0f, 0.0f, 0.72f } );
				else d3d11_ui::add_rect( box_x, box_y, box_w, box_h, 1.0f, col );
			}
			if ( config::esp_skeleton && !minimal_noise && !tiny_target ) {
				draw_skeleton( p, box_h, { config::esp_col.skeleton.r, config::esp_col.skeleton.g, config::esp_col.skeleton.b, config::esp_col.skeleton.a } );
			}
			if ( config::esp_hitboxes && !minimal_noise && !tiny_target ) {
				draw_3d_hitboxes( p, { col.r, col.g, col.b, 0.75f } );
			}
			if ( config::esp_snaplines && !minimal_noise ) {
				const float sx = cx, sy = d3d11_ui::g_height;
				const float dx = root_s.x - cx, dy = root_s.y - cy;
				const float d = d3d11_ui::fast_sqrt( dx * dx + dy * dy );
				if ( d >= config::esp_snap_radius ) d3d11_ui::add_line( sx, sy, root_s.x, root_s.y, 1.0f, col );
			}

			// Overlap suppression (mode 1)
			bool suppress_text = minimal_noise || tiny_target;
			if ( config::esp_priority_mode == 1 ) {
				for ( int k = 0; k < drawn_label_cnt; ++k ) {
					if ( d3d11_ui::fast_sqrt( ( head_s.x - drawn_labels[ k ].x ) * ( head_s.x - drawn_labels[ k ].x ) +
											  ( box_y - drawn_labels[ k ].y ) * ( box_y - drawn_labels[ k ].y ) ) < 26.0f ) {
						suppress_text = true;
						break;
					}
				}
			}
			if ( !suppress_text && drawn_label_cnt < 64 ) {
				drawn_labels[ drawn_label_cnt++ ] = { head_s.x, box_y };
				float ty = box_y - 12.0f;
				if ( config::esp_name && ( p.name[ 0 ] || p.tag[ 0 ] ) ) {
					char line[ 80 ]{};
					char* lend = line + sizeof( line ) - 1;
					char* lp = line;
					if ( config::esp_role && p.tag[ 0 ] && p.name[ 0 ] ) {
						lp = d3d11_ui::put_char( lp, lend, '[' );
						lp = d3d11_ui::put_str( lp, lend, p.tag );
						lp = d3d11_ui::put_char( lp, lend, ']' );
						lp = d3d11_ui::put_char( lp, lend, ' ' );
					}
					lp = d3d11_ui::put_str( lp, lend, p.name[ 0 ] ? p.name : p.tag );
					*lp = '\0';
					draw_esp_text( head_s.x, ty, line, col, 0.84f );
					ty -= 13.0f;
				}
				if ( config::esp_clan && p.clan[ 0 ] ) {
					char cbuf[ 40 ]{};
					char* cend = cbuf + sizeof( cbuf ) - 1;
					char* cp = d3d11_ui::put_char( cbuf, cend, '<' );
					cp = d3d11_ui::put_str( cp, cend, p.clan );
					cp = d3d11_ui::put_char( cp, cend, '>' );
					*cp = '\0';
					draw_esp_text( head_s.x, ty, cbuf, { 0.68f, 0.74f, 0.90f, 1.0f }, 0.76f );
				}

				float by = box_y + box_h + 7.0f;
				if ( config::esp_weapon && p.weapon[ 0 ] ) {
					draw_esp_text( head_s.x, by, p.weapon, { 0.95f, 0.96f, 1.0f, 1.0f }, 0.78f );
					by += 12.0f;
				}
				if ( config::esp_distance ) {
					char dbuf[ 24 ]{};
					char* dend = dbuf + sizeof( dbuf ) - 1;
					char* dp = d3d11_ui::put_int( dbuf, dend, static_cast< int >( p.distance ) );
					dp = d3d11_ui::put_char( dp, dend, 'm' );
					*dp = '\0';
					draw_esp_text( head_s.x, by, dbuf, { 0.78f, 0.82f, 0.90f, 1.0f }, 0.76f );
					by += 12.0f;
				}
				if ( config::esp_armor && p.has_shield ) {
					draw_esp_text( head_s.x, by, "ARMOR", { config::esp_col.shield.r, config::esp_col.shield.g, config::esp_col.shield.b, 1.0f }, 0.74f );
					by += 11.0f;
				}
				if ( config::esp_crouch && p.is_crouching ) {
					draw_esp_text( head_s.x, by, "CROUCH", { 0.95f, 0.82f, 0.35f, 1.0f }, 0.74f );
					by += 11.0f;
				}
				if ( config::esp_knocked && p.is_knocked ) {
					draw_esp_text( head_s.x, by, "KNOCKED", { 1.0f, 0.25f, 0.35f, 1.0f }, 0.76f );
				}
			}
		}
	}
}
