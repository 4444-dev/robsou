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

	// HumanBodyBones ids resolved through Animator.GetBoneTransform. The order of
	// this table is what the skeleton segments below are written against.
	constexpr int k_bone_count = 19;
	inline constexpr int k_human_bones[ k_bone_count ] = {
		10, 9, 8, 7, 0,          // head, neck, chest, spine, hips
		11, 13, 15, 17,          // left shoulder, upper arm, lower arm, hand
		12, 14, 16, 18,          // right shoulder, upper arm, lower arm, hand
		1, 3, 5,                 // left upper leg, lower leg, foot
		2, 4, 6                  // right upper leg, lower leg, foot
	};
	constexpr int BONE_HEAD = 0, BONE_NECK = 1, BONE_CHEST = 2, BONE_SPINE = 3, BONE_HIPS = 4;
	constexpr int BONE_LUPPERLEG = 13, BONE_LLOWERLEG = 14, BONE_LFOOT = 15;
	constexpr int BONE_RUPPERLEG = 16, BONE_RLOWERLEG = 17, BONE_RFOOT = 18;

	constexpr int k_max_players = 256;
	constexpr int k_max_world = 256;
	constexpr int k_max_world_raw = 384;
	constexpr int k_max_meta = 128;
	constexpr int k_max_deaths = 64;
	constexpr int k_max_skeleton_cache = 128;

	struct player_t {
		void* obj = nullptr;
		int32_t entity_id = -1;
		int32_t vehicle_eid = -1;
		vec3_t root_world{};
		vec3_t head_world{};
		vec3_t chest_world{};
		vec3_t pelvis_world{};
		vec3_t velocity{};
		vec3_t look_dir{ 0.0f, 0.0f, 1.0f };

		vec2_t head_screen{};
		vec2_t root_screen{};

		vec2_t bones[ k_bone_count ]{};
		vec3_t bones_world[ k_bone_count ]{};
		bool bone_ok[ k_bone_count ]{};
		int bone_count = 0;

		float distance = 0.0f;
		float health = 1.0f;

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
		bool on_screen = false;
		bool behind = false;
		bool is_visible = true;
		bool has_skeleton = false;
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
		WCAT_BUYER     = 6,
		WCAT_DOOR      = 7,
		WCAT_CAMERA    = 8
	};

	struct world_item_t {
		int32_t entity_id = -1;
		vec3_t world_pos{};
		vec2_t screen_pos{};
		float distance = 0.0f;
		int category = WCAT_LOOSE;
		int rarity = 0; // 0:None 1:Common 2:Uncommon 3:Rare 4:Epic 5:Legendary 6:Exotic
		bool is_empty = false;
		bool locked = false;
		int value = 0;
		char label[ 48 ]{};
		bool valid = false;
	};

	struct world_raw_t {
		int32_t entity_id = -1;
		vec3_t pos{};
		int category = WCAT_LOOSE;
		int rarity = 0;
		bool is_empty = false;
		bool locked = false;
		int value = 0;
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
		float distance = 0.0f;
		int cargo_count = 0;
		int bushes_by_rarity[ 7 ]{};
		int storages = 0;
		int full_containers = 0;
		int parked_vehicles = 0;
	};

	// Double-buffered snapshots: collect() fills the back buffer, render() reads
	// the published one, so a frame never walks a half-written list.
	inline player_t g_players_buf[ 2 ][ k_max_players ]{};
	inline int g_player_count_buf[ 2 ]{ 0, 0 };
	inline volatile LONG g_active_buf = 0;

	inline world_item_t g_world_buf[ 2 ][ k_max_world ]{};
	inline int g_world_count_buf[ 2 ]{ 0, 0 };
	inline volatile LONG g_world_active_buf = 0;

	inline meta_entry_t g_meta[ k_max_meta ]{};
	inline int g_meta_count = 0;
	inline world_raw_t g_world_raw[ k_max_world_raw ]{};
	inline int g_world_raw_count = 0;
	inline death_point_t g_deaths[ k_max_deaths ]{};
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

	// Every pointer below belongs to one build of the scene. Death and respawn
	// destroy and rebuild the battle models, so these identities are compared
	// every frame and all caches are dropped when they change.
	inline uintptr_t g_scene_battle = 0;
	inline uintptr_t g_scene_entities = 0;
	inline uintptr_t g_scene_local_ent = 0;
	inline uintptr_t g_local_char_model = 0;
	inline bool g_local_dead = false;
	inline volatile LONG g_force_reset = 0;

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

	inline d3d11_ui::color_t rgba( const config::rgba_t& c, float a = 1.0f ) {
		return { c.r, c.g, c.b, a };
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

	// Heading of a world point in the camera's own frame. Used by the off-screen
	// arrows: a projection of a point behind the camera mirrors it, so the old
	// head_screen direction pointed to the wrong side of the screen.
	inline void cam_space_dir( const vec3_t& world, float& right_dot, float& fwd_dot ) {
		const float dx = world.x - g_cam_pos.x;
		const float dy = world.y - g_cam_pos.y;
		const float dz = world.z - g_cam_pos.z;
		right_dot = dx * g_cam_right.x + dy * g_cam_right.y + dz * g_cam_right.z;
		fwd_dot = dx * g_cam_fwd.x + dy * g_cam_fwd.y + dz * g_cam_fwd.z;
	}

	inline const char* safe_label( const char* s, const char* fallback ) {
		return ( s && s[ 0 ] ) ? s : fallback;
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

	// get_class_name walks the klass name string for every entity on every scan.
	// Entities of a type share the klass pointer, so the string is resolved once
	// per class instead of once per entity.
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

	// ---- Model lookup by exact class name --------------------------------
	// Writes into live managed models are only made after the object is checked
	// against the exact class name the dump gives, and the klass pointer cached
	// with it is re-checked before every write: a reused address fails the check
	// and is never written into.
	struct model_slot_t {
		uintptr_t root = 0;
		uintptr_t obj = 0;
		uintptr_t klass = 0;
		ULONGLONG tick = 0;
	};

	inline bool model_alive( const model_slot_t& s, const char* cls ) {
		if ( !s.obj || !unity::is_managed_ptr( s.obj ) ) return false;
		if ( unity::read_u64( s.obj ) != s.klass ) return false;
		char nm[ 64 ]{};
		if ( !unity::get_class_name( s.obj, nm, sizeof( nm ) ) ) return false;
		return unity::str_eq( nm, cls );
	}

	inline uintptr_t find_class_field( uintptr_t root, const char* cls ) {
		if ( !unity::is_managed_ptr( root ) ) return 0;
		for ( uintptr_t off = 0x10; off <= 0x900; off += sizeof( uintptr_t ) ) {
			if ( !unity::is_mem_readable( root + off, sizeof( uintptr_t ) ) ) break;
			const uintptr_t v = unity::read_u64( root + off );
			if ( !unity::is_managed_ptr( v ) ) continue;
			char nm[ 64 ]{};
			if ( !unity::get_class_name( v, nm, sizeof( nm ) ) ) continue;
			if ( unity::str_eq( nm, cls ) ) return v;
		}
		return 0;
	}

	// A failed scan is retried at most every two seconds: walking ~280 fields per
	// root on every frame is exactly the kind of cost that shows up as a drop.
	inline uintptr_t ensure_model( model_slot_t& s, uintptr_t root, const char* cls, ULONGLONG now ) {
		if ( s.obj && s.root == root && model_alive( s, cls ) ) return s.obj;
		if ( s.obj || s.root != root ) {
			s.obj = 0;
			s.klass = 0;
			s.root = root;
		}
		if ( s.tick != 0 && ( now - s.tick ) < 2000 ) return 0;
		s.tick = now;
		const uintptr_t obj = find_class_field( root, cls );
		if ( !obj ) return 0;
		s.obj = obj;
		s.klass = unity::read_u64( obj );
		return obj;
	}

	inline model_slot_t g_aim_target_slot{};
	inline model_slot_t g_rot_input_slot{};
	inline model_slot_t g_look_dir_slot{};

	struct silent_state_t {
		uintptr_t model = 0;
		uintptr_t klass = 0;
		int32_t orig_eid = 0;
		int32_t orig_type = 0;
		bool saved = false;
		bool written = false;
	};
	inline silent_state_t g_silent_aim{};

	inline uintptr_t g_silent_look = 0;
	inline uintptr_t g_silent_look_klass = 0;
	inline vec3_t g_silent_look_value{};
	inline float g_silent_look_yaw = 0.0f;
	inline float g_silent_look_pitch = 0.0f;
	inline bool g_silent_look_saved = false;

	inline uintptr_t g_silent_rot = 0;
	inline uintptr_t g_silent_rot_klass = 0;
	inline float g_silent_rot_yaw = 0.0f;
	inline float g_silent_rot_pitch = 0.0f;
	inline bool g_silent_rot_saved = false;

	inline int g_silent_pulse_frames = 0;
	inline ULONGLONG g_silent_last_pulse = 0;
	inline bool g_silent_prev_fire = false;

	inline void forget_models( ) {
		g_aim_target_slot = {};
		g_rot_input_slot = {};
		g_look_dir_slot = {};
	}

	// Restoring is the dangerous half: it runs on paths where the scene may have
	// been torn down between the save and the restore, so each model is checked
	// for its klass and its class name again before anything is written back.
	inline void silent_release( ) {
		if ( g_silent_aim.written && g_silent_aim.model &&
			 unity::is_managed_ptr( g_silent_aim.model ) &&
			 unity::read_u64( g_silent_aim.model ) == g_silent_aim.klass &&
			 unity::obj_is( g_silent_aim.model, "AimTargetModel" ) ) {
			unity::safe_write< int32_t >( g_silent_aim.model + 0x10, g_silent_aim.orig_eid );
			unity::safe_write< int32_t >( g_silent_aim.model + 0x14, g_silent_aim.orig_type );
		}
		g_silent_aim = {};

		if ( g_silent_look_saved && g_silent_look && unity::is_managed_ptr( g_silent_look ) &&
			 unity::read_u64( g_silent_look ) == g_silent_look_klass &&
			 unity::obj_is( g_silent_look, "LookDirectionModel" ) ) {
			unity::safe_write< vec3_t >( g_silent_look + 0x10, g_silent_look_value );
			unity::safe_write< float >( g_silent_look + 0x1C, g_silent_look_yaw );
			unity::safe_write< float >( g_silent_look + 0x20, g_silent_look_pitch );
		}
		g_silent_look_saved = false;
		g_silent_look = 0;

		if ( g_silent_rot_saved && g_silent_rot && unity::is_managed_ptr( g_silent_rot ) &&
			 unity::read_u64( g_silent_rot ) == g_silent_rot_klass &&
			 unity::obj_is( g_silent_rot, "PlayerRotationInputModel" ) ) {
			unity::safe_write< float >( g_silent_rot + 0x10, g_silent_rot_yaw );
			unity::safe_write< float >( g_silent_rot + 0x14, g_silent_rot_pitch );
		}
		g_silent_rot_saved = false;
		g_silent_rot = 0;

		g_silent_pulse_frames = 0;
		config::diag_silent_eid = -1;
	}

	// One pulse. The shot direction is set for this frame and restored on the
	// next one, so the camera - which follows the look direction through its own
	// smoothing - barely moves while the bullet leaves along the target vector.
	// Offsets from the dump:
	//   LookDirectionModel        0x10 Vector3 _value, 0x1C _yaw, 0x20 _pitch
	//   PlayerRotationInputModel  0x10 yaw, 0x14 pitch
	//   AimTargetModel            0x10 entity id, 0x14 type (BattleCharacter = 3)
	// The aim-target write is read back, so the diagnostic only reports a target
	// that was really set instead of one that was merely attempted.
	inline void silent_begin( uintptr_t battle_model, uintptr_t char_model, int32_t eid, const vec3_t& target, ULONGLONG now ) {
		const float dx = target.x - g_cam_pos.x;
		const float dy = target.y - g_cam_pos.y;
		const float dz = target.z - g_cam_pos.z;
		const float horiz = d3d11_ui::fast_sqrt( dx * dx + dz * dz );
		if ( !( horiz == horiz ) || horiz < 0.05f ) return;

		constexpr float k_rad2deg = 57.2957795f;
		constexpr float k_deg2rad = 0.0174532925f;
		const float yaw_deg = static_cast< float >( std::atan2( dx, dz ) ) * k_rad2deg;
		const float pitch_deg = -static_cast< float >( std::atan2( dy, horiz ) ) * k_rad2deg;

		const uintptr_t look = ensure_model( g_look_dir_slot, char_model ? char_model : battle_model, "LookDirectionModel", now );
		if ( look ) {
			if ( !g_silent_look_saved ) {
				g_silent_look = look;
				g_silent_look_klass = g_look_dir_slot.klass;
				unity::safe_read( look + 0x10, g_silent_look_value );
				unity::safe_read( look + 0x1C, g_silent_look_yaw );
				unity::safe_read( look + 0x20, g_silent_look_pitch );
				g_silent_look_saved = true;
			}
			const float cp = static_cast< float >( std::cos( pitch_deg * k_deg2rad ) );
			const vec3_t dir{
				static_cast< float >( std::sin( yaw_deg * k_deg2rad ) ) * cp,
				-static_cast< float >( std::sin( pitch_deg * k_deg2rad ) ),
				static_cast< float >( std::cos( yaw_deg * k_deg2rad ) ) * cp
			};
			unity::safe_write< vec3_t >( look + 0x10, dir );
			unity::safe_write< float >( look + 0x1C, yaw_deg );
			unity::safe_write< float >( look + 0x20, pitch_deg );
		}

		const uintptr_t rot = ensure_model( g_rot_input_slot, battle_model, "PlayerRotationInputModel", now );
		if ( rot ) {
			if ( !g_silent_rot_saved ) {
				g_silent_rot = rot;
				g_silent_rot_klass = g_rot_input_slot.klass;
				unity::safe_read( rot + 0x10, g_silent_rot_yaw );
				unity::safe_read( rot + 0x14, g_silent_rot_pitch );
				g_silent_rot_saved = true;
			}
			unity::safe_write< float >( rot + 0x10, yaw_deg );
			unity::safe_write< float >( rot + 0x14, pitch_deg );
		}

		const uintptr_t atm = ensure_model( g_aim_target_slot, battle_model, "AimTargetModel", now );
		if ( atm ) {
			if ( !g_silent_aim.saved ) {
				int32_t oid = 0, otype = 0;
				if ( unity::safe_read< int32_t >( atm + 0x10, oid ) && unity::safe_read< int32_t >( atm + 0x14, otype ) ) {
					g_silent_aim.model = atm;
					g_silent_aim.klass = g_aim_target_slot.klass;
					g_silent_aim.orig_eid = oid;
					g_silent_aim.orig_type = otype;
					g_silent_aim.saved = true;
				}
			}
			if ( g_silent_aim.saved && unity::safe_write< int32_t >( atm + 0x10, eid ) ) {
				unity::safe_write< int32_t >( atm + 0x14, 3 );
				int32_t back = 0;
				if ( unity::safe_read< int32_t >( atm + 0x10, back ) && back == eid ) {
					g_silent_aim.written = true;
					config::diag_silent_eid = eid;
				}
			}
		}

		// Which levers resolved, in the diagnostic line: AT = aim target model,
		// LOOK = look direction model, ROT = rotation input model.
		{
			char ds[ 48 ]{};
			char* dp = ds;
			char* de = ds + sizeof( ds ) - 1;
			if ( atm ) dp = d3d11_ui::put_str( dp, de, "AT " );
			if ( look ) dp = d3d11_ui::put_str( dp, de, "LOOK " );
			if ( rot ) dp = d3d11_ui::put_str( dp, de, "ROT" );
			if ( dp == ds ) dp = d3d11_ui::put_str( dp, de, "no model" );
			*dp = '\0';
			copy_str( config::diag_aim_class, sizeof( config::diag_aim_class ), ds );
		}
	}

	inline void silent_aim_update( uintptr_t battle_model, uintptr_t char_model, bool firing, int32_t eid,
								   const vec3_t& target, ULONGLONG now ) {
		const bool fire = firing && eid > 0 && !g_local_dead;
		const bool edge = fire && !g_silent_prev_fire;
		g_silent_prev_fire = fire;
		if ( !fire ) {
			silent_release( );
			return;
		}
		if ( g_silent_pulse_frames > 0 ) {
			if ( --g_silent_pulse_frames == 0 ) silent_release( );
			return;
		}
		// A held trigger fires again at the weapon cadence, so the direction is
		// re-aimed every ~70 ms while the button stays down.
		if ( edge || ( now - g_silent_last_pulse ) >= 70 ) {
			silent_begin( battle_model, char_model, eid, target, now );
			g_silent_pulse_frames = 2;
			g_silent_last_pulse = now;
		}
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
		// The field is checked against the class name the dump gives, so a build
		// where it holds something else cannot be written into by mistake.
		if ( !unity::is_managed_ptr( rot_input ) || !unity::obj_is( rot_input, "PlayerRotationInputModel" ) )
			return false;

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

		constexpr float k_rad2deg = 57.2957795f;
		constexpr float k_deg2rad = 0.0174532925f;
		const float target_yaw = static_cast< float >( std::atan2( dx, dz ) ) * k_rad2deg;
		const float target_pitch = -static_cast< float >( std::atan2( dy, horiz ) ) * k_rad2deg;

		// Detect radian vs degree storage from the quaternion at +0x1C.
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
				// The class name is checked before the write: a build where this field
				// holds something else must not have two floats overwritten in it.
				if ( unity::is_managed_ptr( recoil_m ) && unity::obj_contains( recoil_m, "Recoil" ) ) {
					const float scale = config::no_recoil ? 0.0f : ( 1.0f - d3d11_ui::clampf( config::rcs_strength, 0.0f, 1.0f ) );
					float rx = 0.0f, ry = 0.0f;
					if ( unity::safe_read( recoil_m + 0x10, rx ) ) unity::safe_write< float >( recoil_m + 0x10, rx * scale );
					if ( unity::safe_read( recoil_m + 0x14, ry ) ) unity::safe_write< float >( recoil_m + 0x14, ry * scale );
				}
			}
			if ( config::no_spread ) {
				const uintptr_t fake_spread_m = unity::read_u64( client_m + 0x2C0 );
				if ( unity::is_managed_ptr( fake_spread_m ) && unity::obj_contains( fake_spread_m, "Spread" ) ) {
					unity::safe_write< float >( fake_spread_m + 0x10, 0.0f );
					unity::safe_write< float >( fake_spread_m + 0x14, 0.0f );
					unity::safe_write< float >( fake_spread_m + 0x18, 0.0f );
				}
			}
		}

		// WeaponDescription Spread & ShotPattern (actual recoil and spread)
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
					if ( unity::is_managed_ptr( wpn_spread ) && unity::obj_contains( wpn_spread, "Spread" ) ) {
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

		// No camera / optics shake
		if ( config::no_cam_shake && unity::is_managed_ptr( battle_model ) ) {
			const uintptr_t cam_m = unity::read_u64( battle_model + 0x358 ); // BattleCameraModel
			if ( unity::is_managed_ptr( cam_m ) ) {
				unity::safe_write< float >( cam_m + 0x2C, 0.0f );
			}
		}
	}

	inline uintptr_t battle_phys_data( uintptr_t battle_model ) {
		if ( !unity::is_managed_ptr( battle_model ) ) return 0;
		const uintptr_t bem = unity::read_u64( battle_model + 0x90 );
		if ( !unity::is_managed_ptr( bem ) ) return 0;
		const uintptr_t phys_scene = unity::read_u64( bem + 0x10 );
		return unity::is_managed_ptr( phys_scene ) ? unity::read_u64( phys_scene + 0x10 ) : 0;
	}

	// The flags word of a door body: the same one the door toggle flips, so its
	// bit 8 is what keeps a door shut. Read only, to label the door.
	inline bool door_first_body_flags( uintptr_t door_m, uintptr_t pdata, int32_t& out_flags ) {
		if ( !unity::is_managed_ptr( door_m ) || !unity::is_managed_ptr( pdata ) ) return false;
		const int32_t st_type = unity::read_i32( pdata + 0x30 );
		const uintptr_t st_bodies = unity::read_u64( pdata + 0x48 );
		const int32_t dy_type = unity::read_i32( pdata + 0x50 );
		const uintptr_t dy_bodies = unity::read_u64( pdata + 0x68 );
		const uintptr_t bodies_m = unity::read_u64( door_m + 0x28 );
		const uintptr_t body_list = unity::is_managed_ptr( bodies_m ) ? unity::read_u64( bodies_m + 0x10 ) : 0;
		if ( !unity::is_managed_ptr( body_list ) ) return false;
		const uintptr_t items = unity::read_u64( body_list + 0x10 );
		const int32_t sz = unity::read_i32( body_list + 0x18 );
		if ( !unity::is_managed_ptr( items ) || sz <= 0 || sz > 64 ) return false;
		const int32_t bval = unity::read_i32( items + 0x20 );
		const uint8_t btype = unity::read_u8( items + 0x24 );
		const uintptr_t arr = ( btype == st_type ) ? st_bodies : ( btype == dy_type ? dy_bodies : 0 );
		if ( !unity::is_managed_ptr( arr ) || bval < 0 ) return false;
		const int32_t arr_len = unity::read_i32( arr + 0x18 );
		if ( arr_len <= 0 || bval >= arr_len ) return false;
		return unity::safe_read( arr + 0x20 + static_cast< size_t >( bval ) * 0xCC, out_flags );
	}

	inline bool door_flags_for_eid( uintptr_t world_m, uintptr_t pdata, int32_t eid, int32_t& out_flags ) {
		if ( !unity::is_managed_ptr( world_m ) || !unity::is_managed_ptr( pdata ) || eid < 0 ) return false;
		uintptr_t entries = 0;
		const int n = unity::read_dict( unity::read_u64( world_m + 0x48 ), entries );
		for ( int i = 0; i < n; ++i ) {
			const uintptr_t ent = entries + static_cast< size_t >( i ) * 0x18;
			if ( unity::read_i32( ent ) != eid ) continue;
			return door_first_body_flags( unity::read_u64( ent + 0x10 ), pdata, out_flags );
		}
		return false;
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

	inline bool world_pos_sane( const vec3_t& p ) {
		if ( !( p.x == p.x ) || !( p.y == p.y ) || !( p.z == p.z ) ) return false;
		if ( p.y < -300.0f || p.y > 900.0f ) return false;
		const float d = dist3( p, g_cam_pos );
		return d > 0.8f && d < 900.0f;
	}

	// Entity models that carry their transform on a client object (cars,
	// helicopters) keep the position at client + 0x7C; containers hang theirs on
	// a transform model reached through the server object. Both known layouts are
	// tried, and a position that fails a sanity check is rejected instead of being
	// drawn at the origin.
	inline bool entity_world_pos( uintptr_t emodel, vec3_t& out ) {
		vec3_t p{};
		const uintptr_t client_m = unity::read_u64( emodel + 0x20 );
		if ( unity::is_managed_ptr( client_m ) && unity::safe_read( client_m + 0x7C, p ) && world_pos_sane( p ) ) {
			out = p;
			return true;
		}
		const uintptr_t srv = unity::read_u64( emodel + 0x18 );
		if ( unity::is_managed_ptr( srv ) ) {
			const uintptr_t tr_m = unity::read_u64( srv + 0x28 );
			if ( unity::is_managed_ptr( tr_m ) &&
				 unity::read_prim_data( unity::read_u64( tr_m + 0x10 ), p.x ) &&
				 unity::read_prim_data( unity::read_u64( tr_m + 0x18 ), p.y ) &&
				 unity::read_prim_data( unity::read_u64( tr_m + 0x20 ), p.z ) &&
				 world_pos_sane( p ) ) {
				out = p;
				return true;
			}
		}
		return false;
	}

	// World entity tables, read straight from the IL2CPP dump of this build
	// (Models.Models.WorldEntities.WorldEntitiesModel):
	//   0x20 _containers          0x38 _loots             0x48 _doors
	//   0x90 _metalDetectors      0x98 _staticLootObjects 0xA0 _randomLootObjects
	//   0xB0 _lootNodes           0x110 _safeDoors        0x128 _storages
	//   0x148 _shelvings          0x210 _locationDoors    0x218 _builderDoors
	// Every table is a Dictionary<int, Model> keyed by entity id.
	inline uintptr_t world_model_for_eid( uintptr_t world_m, size_t table_off, int32_t eid ) {
		if ( !unity::is_managed_ptr( world_m ) || eid < 0 ) return 0;
		uintptr_t entries = 0;
		const int n = unity::read_dict( unity::read_u64( world_m + table_off ), entries );
		for ( int i = 0; i < n; ++i ) {
			const uintptr_t ent = entries + static_cast< size_t >( i ) * 0x18;
			if ( unity::read_i32( ent ) != eid ) continue;
			const uintptr_t model = unity::read_u64( ent + 0x10 );
			return unity::is_managed_ptr( model ) ? model : 0;
		}
		return 0;
	}

	// LootObjectModel._isTaken @ 0x10 (dump: Models.Models.LootObjectModel). An
	// emptied loot object leaves the world ESP instead of being drawn forever.
	inline bool loot_object_taken( uintptr_t world_m, int32_t eid ) {
		if ( !config::world_loot_objects ) return false;
		uintptr_t model = world_model_for_eid( world_m, 0x98, eid );
		if ( !model ) model = world_model_for_eid( world_m, 0xA0, eid );
		if ( !model ) return false;
		bool taken = false;
		return unity::read_prim_data( unity::read_u64( model + 0x10 ), taken ) && taken;
	}

	// LocationSafeDoorModel guards the real safe state:
	//   0x10 _isSafeDoorUnlockInProcessData  0x18 _isSafeDoorUnlockedData
	// The body-flag guess used for swing doors says nothing about a safe, so when
	// this lookup resolves it wins and the label reflects drilling / open / locked.
	inline bool safe_door_state( uintptr_t world_m, int32_t eid, bool& unlocked, bool& in_process ) {
		unlocked = false;
		in_process = false;
		if ( !config::world_safes ) return false;
		const uintptr_t model = world_model_for_eid( world_m, 0x110, eid );
		if ( !model ) return false;
		bool u = false, p = false;
		const bool ok_u = unity::read_prim_data( unity::read_u64( model + 0x18 ), u );
		const bool ok_p = unity::read_prim_data( unity::read_u64( model + 0x10 ), p );
		if ( !ok_u && !ok_p ) return false;
		unlocked = ok_u && u;
		in_process = ok_p && p;
		return true;
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
		default: return { 0.70f, 0.74f, 0.82f, 1.0f };
		}
	}

	// The price of an item is not read yet: which field of the loot descriptor
	// carries it is unknown for this build, and inventing an offset would print a
	// wrong amount on every label. The probe reports the positive per-type
	// constants it finds on the nearest loot, so one look at the panel gives the
	// offset to wire into config::world_value_off.
	inline char g_value_probe_id[ 32 ]{};
	inline int g_value_probe_vals[ 10 ]{};
	inline int g_value_probe_offs[ 10 ]{};
	inline int g_value_probe_n = 0;

	inline void fill_value_probe( const char* id, uintptr_t desc ) {
		g_value_probe_n = 0;
		copy_str( g_value_probe_id, sizeof( g_value_probe_id ), id );
		if ( !unity::is_managed_ptr( desc ) ) return;
		static const int k_offs[ 10 ] = { 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x34, 0x38, 0x40, 0x44 };
		for ( int i = 0; i < 10; ++i ) {
			int v = 0;
			if ( !unity::safe_read( desc + k_offs[ i ], v ) ) continue;
			if ( v <= 0 || v > 2000000 ) continue;
			g_value_probe_offs[ g_value_probe_n ] = k_offs[ i ];
			g_value_probe_vals[ g_value_probe_n ] = v;
			++g_value_probe_n;
		}
	}

	inline int world_label_len( const char* s ) {
		int n = 0;
		while ( s && s[ n ] ) ++n;
		return n;
	}

	// Item labels arrive as engine ids: LootItemBeanCan, item_bean_can,
	// CannedBeans01. Rendering those raw is most of why the world ESP reads as
	// noise, so the id is made readable once, at projection.
	inline void pretty_world_label( const char* in, char* out, size_t cap ) {
		if ( !out || cap < 2 ) return;
		out[ 0 ] = '\0';
		if ( !in || !in[ 0 ] ) return;

		static const char* const k_prefix[] = { "LootItem", "LootNode", "Lootable", "Loot", "WorldEntity", "Entity", "Item" };
		const char* s = in;
		for ( int p = 0; p < 7; ++p ) {
			const int pl = world_label_len( k_prefix[ p ] );
			bool match = true;
			for ( int i = 0; i < pl; ++i ) {
				if ( s[ i ] != k_prefix[ p ][ i ] ) {
					match = false;
					break;
				}
			}
			if ( !match ) continue;
			// Only strip when the prefix ends a word, so "Carcass" keeps its C.
			const char nxt = s[ pl ];
			if ( ( nxt >= 'A' && nxt <= 'Z' ) || nxt == '_' ) {
				s += pl;
				break;
			}
		}
		if ( s[ 0 ] == '_' ) ++s;

		// Some loot descriptions only carry a numeric id. "46" on the map says
		// nothing at all, so those fall back to a plain name.
		if ( s[ 0 ] ) {
			bool numeric = true;
			for ( int i = 0; s[ i ]; ++i ) {
				if ( !( ( s[ i ] >= '0' && s[ i ] <= '9' ) || s[ i ] == '-' || s[ i ] == '.' ) ) {
					numeric = false;
					break;
				}
			}
			if ( numeric ) {
				copy_str( out, cap, "LOOT" );
				return;
			}
		}

		size_t o = 0;
		char prev = 0;
		for ( int i = 0; s[ i ] && o + 2 < cap; ++i ) {
			const char ch = s[ i ];
			if ( ch == '_' || ch == '-' ) {
				if ( o > 0 && out[ o - 1 ] != ' ' ) out[ o++ ] = ' ';
				prev = ' ';
				continue;
			}
			const bool upper = ( ch >= 'A' && ch <= 'Z' );
			const bool prev_word = ( prev >= 'a' && prev <= 'z' ) || ( prev >= '0' && prev <= '9' );
			if ( upper && prev_word && o > 0 && out[ o - 1 ] != ' ' ) out[ o++ ] = ' ';
			out[ o++ ] = ch;
			prev = ch;
		}
		while ( o > 0 && out[ o - 1 ] == ' ' ) --o;
		out[ o ] = '\0';
	}

	inline void refresh_metadata_and_world_cache( ULONGLONG now, uintptr_t battle_model, uintptr_t entities_model, uintptr_t slots_model ) {
		static ULONGLONG s_last_refresh = 0;
		if ( s_last_refresh != 0 && ( now - s_last_refresh ) < 800 ) return;
		s_last_refresh = now;

		g_meta_count = 0;
		g_world_raw_count = 0;
		g_value_probe_n = 0;

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
		g_local_veh_eid = -1;
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

		auto push_world_raw = [&]( int32_t eid, const vec3_t& wpos, int cat, int rarity, bool is_empty, const char* lbl, int value = 0, bool locked = false ) {
			if ( g_world_raw_count >= k_max_world_raw ) return;
			world_raw_t& wi = g_world_raw[ g_world_raw_count++ ];
			wi = {};
			wi.entity_id = eid;
			wi.pos = wpos;
			wi.category = cat;
			wi.rarity = rarity;
			wi.is_empty = is_empty;
			wi.value = value;
			wi.locked = locked;
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
				if ( g_meta_count >= k_max_meta ) continue;
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

				// Knockout / knock-down windows
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
				if ( m.is_dead && !m.is_local ) {
					// The local player is skipped: a marker on your own corpse is
					// noise, and on death that entry is the one being rebuilt.
					bool found = false;
					for ( int d = 0; d < k_max_deaths; ++d ) {
						if ( g_deaths[ d ].valid && dist3( g_deaths[ d ].pos, pos ) < 2.0f ) { found = true; break; }
					}
					if ( !found ) {
						for ( int d = 0; d < k_max_deaths; ++d ) {
							if ( !g_deaths[ d ].valid ) {
								g_deaths[ d ].pos = pos;
								copy_str( g_deaths[ d ].name, sizeof( g_deaths[ d ].name ), m.name[ 0 ] ? m.name : m.tag );
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
				const uintptr_t wtable = unity::is_managed_ptr( battle_model ) ? unity::read_u64( battle_model + 0x518 ) : 0;
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
				} else if ( config::world_containers && ( unity::str_icontains( cls, "Shelving" ) || unity::str_icontains( cls, "Trunk" ) ) ) {
					vec3_t pos{};
					if ( entity_world_pos( emodel, pos ) ) {
						push_world_raw( eid, pos, WCAT_CONTAINER, 3, false, unity::str_icontains( cls, "Trunk" ) ? "TRUNK" : "SHELF" );
					}
				} else if ( config::world_containers && ( unity::str_icontains( cls, "Container" ) || unity::str_icontains( cls, "Locker" ) ||
						 unity::str_icontains( cls, "Cabinet" ) || unity::str_icontains( cls, "Storage" ) ) ) {
					const uintptr_t srv = unity::read_u64( emodel + 0x18 );
					if ( unity::is_managed_ptr( srv ) ) {
						uint8_t is_empty = 0;
						unity::read_prim_model( unity::read_u64( srv + 0x38 ), is_empty );
						// An empty cupboard has been opened and emptied: there is nothing
						// left to walk to, so it leaves the ESP instead of sitting there.
						if ( is_empty ) continue;
						const uintptr_t tr_m = unity::read_u64( srv + 0x28 );
						if ( unity::is_managed_ptr( tr_m ) ) {
							vec3_t pos{};
							if ( unity::read_prim_data( unity::read_u64( tr_m + 0x10 ), pos.x ) &&
								 unity::read_prim_data( unity::read_u64( tr_m + 0x18 ), pos.y ) &&
								 unity::read_prim_data( unity::read_u64( tr_m + 0x20 ), pos.z ) ) {
								const char* clbl = unity::str_icontains( cls, "Locker" ) ? "LOCKER" :
												   unity::str_icontains( cls, "Cabinet" ) ? "CABINET" :
												   unity::str_icontains( cls, "Storage" ) ? "STORAGE" : "CUPBOARD";
								push_world_raw( eid, pos, WCAT_CONTAINER, 3, false, clbl );
								if ( dist3( pos, g_cam_pos ) < 55.0f ) {
									++next_plot.storages;
									++next_plot.full_containers;
								}
							}
						}
					}
				} else if ( config::world_doors && unity::str_icontains( cls, "Door" ) ) {
					vec3_t pos{};
					if ( entity_world_pos( emodel, pos ) ) {
						int32_t flags = 0;
						const bool locked = door_flags_for_eid( wtable, battle_phys_data( battle_model ), eid, flags ) &&
							( flags & 256 ) != 0;
						bool safe_open = false, safe_drilling = false;
						if ( safe_door_state( wtable, eid, safe_open, safe_drilling ) ) {
							if ( safe_open ) continue; // already open: nothing to walk to
							push_world_raw( eid, pos, WCAT_DOOR, 4, false,
											safe_drilling ? "SAFE DRILLING" : "SAFE LOCKED", 0, true );
						} else {
							if ( !locked ) continue; // an unlocked door is not a target
							push_world_raw( eid, pos, WCAT_DOOR, 3, false, "DOOR LOCKED", 0, true );
						}
					}
				} else if ( config::world_loot_objects && unity::str_icontains( cls, "LootObject" ) ) {
					vec3_t pos{};
					if ( entity_world_pos( emodel, pos ) ) {
						if ( loot_object_taken( wtable, eid ) ) continue;
						push_world_raw( eid, pos, WCAT_CONTAINER, 2, false, "LOOT OBJECT" );
					}
				} else if ( config::world_cameras && unity::str_icontains( cls, "Camera" ) ) {
					vec3_t pos{};
					if ( entity_world_pos( emodel, pos ) ) {
						push_world_raw( eid, pos, WCAT_CAMERA, 2, false, "CAMERA" );
					}
				}
			}
		}

		// WorldEntitiesModel direct dictionaries for loose loot, nodes and buyers.
		if ( config::world_master && unity::is_managed_ptr( battle_model ) ) {
			const uintptr_t world_m = unity::read_u64( battle_model + 0x518 );
			if ( unity::is_managed_ptr( world_m ) ) {
				// Loose items (_loots @ 0x38)
				float probe_best = 1e9f;
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
							const float pd = dist3( pos, g_cam_pos );
							if ( pd < probe_best ) {
								probe_best = pd;
								fill_value_probe( raw_id, desc );
							}
							int value = 0;
							if ( config::world_value_off > 0 &&
								 unity::safe_read( desc + config::world_value_off, value ) &&
								 ( value <= 0 || value > 1000000 ) ) {
								value = 0;
							}
							push_world_raw( -1, pos, WCAT_LOOSE, rarity_from_value( value, 1 ), false,
											raw_id[ 0 ] ? raw_id : "LOOT", value );
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
						// LootNodeState: 0 Empty, 1 Small, 2 Half, 3 Full. Read as an int
						// so a value outside the enum falls back to the plain bool above.
						int32_t node_state = -1;
						if ( !unity::read_prim_model( unity::read_u64( node_m + 0x30 ), node_state ) ||
							 node_state < 0 || node_state > 3 ) {
							node_state = -1;
						}
						const uintptr_t desc = unity::read_u64( node_m + 0x10 );
						if ( !unity::is_managed_ptr( desc ) ) continue;
						vec3_t pos{};
						if ( unity::safe_read( desc + 0x38, pos ) ) {
							char raw_id[ 32 ]{};
							unity::read_enum_id( desc, raw_id, sizeof( raw_id ) );
							char node_lbl[ 40 ]{};
							copy_str( node_lbl, sizeof( node_lbl ), raw_id[ 0 ] ? raw_id : "NODE" );
							if ( node_state >= 1 && node_state <= 3 ) {
								const char* sfx = node_state == 3 ? " FULL" : node_state == 2 ? " HALF" : " SMALL";
								size_t L = 0;
								while ( node_lbl[ L ] ) ++L;
								for ( int c = 0; sfx[ c ] && L + 1 < sizeof( node_lbl ); ++c ) node_lbl[ L++ ] = sfx[ c ];
								node_lbl[ L ] = '\0';
							}
							push_world_raw( -1, pos, WCAT_NODE, 2, is_empty != 0, node_lbl );
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
	// ---- Skeleton -------------------------------------------------------
	// The skeleton is built only from Animator.GetBoneTransform on the nineteen
	// HumanBodyBones. That is the character's real rig, so the figure follows the
	// pose, and every joint is validated against the body's own volume before it
	// is drawn: the hierarchy walk used before also matched straps, hair and
	// holstered props, which is where the lines cutting across the screen came
	// from.
	struct bone_cache_t {
		void* view = nullptr;
		void* animator = nullptr;
		void* trs[ k_bone_count ]{};
		uintptr_t native[ k_bone_count ]{};
		bool ok[ k_bone_count ]{};
		int ok_count = 0;
		ULONGLONG resolved = 0;
		ULONGLONG last_try = 0;
	};
	inline bone_cache_t g_bone_cache[ k_max_skeleton_cache ]{};

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

	// One resolve per player per three seconds. The transform of every joint is
	// kept natively, so the per-frame cost is nineteen position reads and no
	// managed call.
	inline bone_cache_t* get_or_resolve_bones( void* view_obj, ULONGLONG now, int& budget ) {
		if ( !view_obj ) return nullptr;
		int slot = -1;
		int free_idx = -1;
		for ( int i = 0; i < k_max_skeleton_cache; ++i ) {
			if ( g_bone_cache[ i ].view == view_obj ) { slot = i; break; }
			if ( free_idx < 0 && g_bone_cache[ i ].view == nullptr ) free_idx = i;
		}
		if ( slot < 0 ) {
			slot = ( free_idx >= 0 ) ? free_idx : 0;
			g_bone_cache[ slot ] = {};
			g_bone_cache[ slot ].view = view_obj;
		}

		bone_cache_t& bc = g_bone_cache[ slot ];
		// The GC reuses addresses, so a pointer-keyed cache without expiry ends up
		// drawing the skeleton of a completely different player.
		if ( bc.ok_count >= 6 && bc.resolved != 0 && ( now - bc.resolved ) < 3000 &&
			 ( unity::is_valid_obj( bc.trs[ BONE_CHEST ] ) || unity::is_valid_obj( bc.trs[ BONE_HIPS ] ) ) ) {
			return &bc;
		}
		if ( budget <= 0 || ( bc.last_try != 0 && ( now - bc.last_try ) < 1500 ) ) return nullptr;
		--budget;
		bc.last_try = now;
		bc.ok_count = 0;

		if ( !bc.animator || !unity::is_valid_obj( bc.animator ) ) {
			bc.animator = find_animator( view_obj );
		}
		if ( !bc.animator || !unity::fn_anim_get_bone ) {
			// No animator yet: still allow the head fallback without a skeleton,
			// and retry later instead of caching a failure forever.
			bc.resolved = 0;
			return nullptr;
		}

		int valid_cnt = 0;
		for ( int b = 0; b < k_bone_count; ++b ) {
			void* tr = unity::get_animator_bone( bc.animator, k_human_bones[ b ] );
			bc.trs[ b ] = tr;
			bc.native[ b ] = 0;
			bc.ok[ b ] = false;
			if ( !unity::is_valid_obj( tr ) ) continue;
			const uintptr_t native = unity::read_u64( reinterpret_cast< uintptr_t >( tr ) + 0x10 );
			if ( native < 0x10000 ) continue;
			bc.native[ b ] = native;
			bc.ok[ b ] = true;
			++valid_cnt;
		}
		bc.ok_count = valid_cnt;
		bc.resolved = ( valid_cnt >= 6 ) ? now : 0;
		return ( valid_cnt >= 6 ) ? &bc : nullptr;
	}

	// Fills the player's bones from the cache. Every joint has to sit inside the
	// body volume: a stale native pointer or a mismatched rig is dropped instead
	// of being turned into a line.
	inline void fill_player_bones( player_t& p, void* view, ULONGLONG now, int& budget ) {
		p.bone_count = 0;
		if ( !view || p.distance > 110.0f ) return;
		bone_cache_t* bc = get_or_resolve_bones( view, now, budget );
		if ( !bc ) return;

		int ok_cnt = 0;
		vec3_t bpos{};
		for ( int b = 0; b < k_bone_count; ++b ) {
			if ( !bc->ok[ b ] || !bc->native[ b ] ) continue;
			if ( !unity::get_transform_pos_native( bc->native[ b ], bpos ) ) continue;
			const float dy = bpos.y - p.root_world.y;
			const float dx = bpos.x - p.root_world.x;
			const float dz = bpos.z - p.root_world.z;
			if ( dy < -0.35f || dy > 2.35f || ( dx * dx + dz * dz ) > 1.69f ) continue;
			p.bones_world[ b ] = bpos;
			if ( !world_to_screen( bpos, p.bones[ b ] ) ) continue;
			p.bone_ok[ b ] = true;
			++ok_cnt;
		}
		p.bone_count = ok_cnt;
		p.has_skeleton = ( ok_cnt >= 6 );
		if ( p.has_skeleton && p.bone_ok[ BONE_HEAD ] ) {
			p.head_world = p.bones_world[ BONE_HEAD ];
		}
		if ( p.has_skeleton ) {
			float feet_y = p.root_world.y;
			if ( p.bone_ok[ BONE_LFOOT ] ) feet_y = p.bones_world[ BONE_LFOOT ].y;
			if ( p.bone_ok[ BONE_RFOOT ] && p.bones_world[ BONE_RFOOT ].y < feet_y ) feet_y = p.bones_world[ BONE_RFOOT ].y;
			p.root_world.y = feet_y;
			if ( p.bone_ok[ BONE_CHEST ] ) p.chest_world = p.bones_world[ BONE_CHEST ];
			if ( p.bone_ok[ BONE_HIPS ] ) p.pelvis_world = p.bones_world[ BONE_HIPS ];
		}
		config::diag_skel_nodes = p.bone_count;
	}

	inline bool project_player_box( player_t& p, bool with_head_margin = true ) {
		const float top_y = p.head_world.y + ( with_head_margin ? 0.16f : 0.0f );
		const vec3_t top_w{ p.head_world.x, top_y, p.head_world.z };
		vec2_t hs{}, rs{};
		bool behind_h = false, behind_r = false;
		if ( !w2s_vp( top_w, hs, behind_h ) ) return false;
		if ( !w2s_vp( p.root_world, rs, behind_r ) ) return false;
		p.head_screen = hs;
		p.root_screen = rs;
		p.behind = behind_h || behind_r;
		p.on_screen = !p.behind &&
			hs.x >= -120.0f && hs.x <= d3d11_ui::g_width + 120.0f &&
			hs.y >= -120.0f && hs.y <= d3d11_ui::g_height + 120.0f;
		return true;
	}

	// ---- World snapshot -------------------------------------------------
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
		for ( int i = 0; i < g_world_raw_count && w_cnt < k_max_world; ++i ) {
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
			wi.value = raw.value;
			wi.locked = raw.locked;
			pretty_world_label( raw.label, wi.label, sizeof( wi.label ) );
			wi.valid = true;
		}
		g_world_count_buf[ w_back ] = w_cnt;
		InterlockedExchange( &g_world_active_buf, w_back );
		config::diag_world_tracked = w_cnt;
	}

	inline void reset_shared_caches( bool keep_view ) {
		// Release the aim writes before the pointers are dropped: the restore path
		// checks the klass of every model, so a model that has already been freed
		// is skipped instead of written back into whatever reused the address.
		silent_release( );
		forget_models( );
		g_locked_obj = nullptr;
		g_has_aim_target = false;
		g_has_silent_target = false;
		g_aim_rem_x = 0.0f;
		g_aim_rem_y = 0.0f;
		g_meta_count = 0;
		g_world_raw_count = 0;
		g_value_probe_n = 0;
		for ( int i = 0; i < k_max_skeleton_cache; ++i ) g_bone_cache[ i ] = {};
		for ( int i = 0; i < 128; ++i ) g_vel_cache[ i ] = {};
		for ( int i = 0; i < 512; ++i ) g_klass_names[ i ] = {};
		g_player_count_buf[ 0 ] = 0;
		g_player_count_buf[ 1 ] = 0;
		g_world_count_buf[ 0 ] = 0;
		g_world_count_buf[ 1 ] = 0;
		InterlockedExchange( &g_active_buf, 0 );
		InterlockedExchange( &g_world_active_buf, 0 );
		config::diag_players_tracked = 0;
		config::diag_world_tracked = 0;
		if ( !keep_view ) {
			config::diag_models_read = 0;
			config::model_chain_ok = false;
			g_camera = nullptr;
			g_vp_valid = false;
			g_freecam_active = false;
			unity::reset_page_cache( );
			unity::g_unity_entry_obj = 0;
			for ( int i = 0; i < k_max_deaths; ++i ) g_deaths[ i ].valid = false;
		}
	}

	inline void reset_scene_caches( ) { reset_shared_caches( false ); }
	inline void reset_scene_caches_keep_view( ) { reset_shared_caches( true ); }

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

		// FOV changer, preserving the ADS scope zoom.
		if ( config::fov_changer_enabled && unity::fn_cam_get_fov && unity::fn_cam_set_fov && unity::is_valid_obj( cam ) ) {
			const float cur_fov = unity::fn_cam_get_fov( cam, unity::m_cam_get_fov );
			const bool is_ads = config::fov_preserve_ads && ( ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) != 0 || cur_fov < 52.0f );
			if ( !is_ads ) {
				unity::fn_cam_set_fov( cam, config::fov_value, unity::m_cam_set_fov );
			}
		}

		// Free cam
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
		case 1: return p.bone_ok[ BONE_NECK ] ? p.bones_world[ BONE_NECK ] : p.head_world;
		case 2: return p.chest_world;
		case 3: return p.pelvis_world;
		case 4: return stomach_w;
		case 5: return p.bone_ok[ 6 ] ? p.bones_world[ 6 ] : p.chest_world;
		case 6: return p.bone_ok[ BONE_LUPPERLEG ] ? p.bones_world[ BONE_LUPPERLEG ] : p.pelvis_world;
		case 7: return { p.head_world.x, p.head_world.y - 0.04f, p.head_world.z };
		case 8: return p.bone_ok[ 8 ] ? p.bones_world[ 8 ] : p.chest_world;
		case 9: return p.bone_ok[ BONE_LFOOT ] ? p.bones_world[ BONE_LFOOT ] : p.root_world;
		case 10: {
			const vec3_t cands[ 5 ] = { p.head_world, p.chest_world, stomach_w, p.pelvis_world, p.root_world };
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
		if ( InterlockedExchange( &g_force_reset, 0 ) != 0 ) {
			reset_scene_caches( );
			return;
		}
		const ULONGLONG now = GetTickCount64( );
		static ULONGLONG s_last_collect = 0;
		if ( s_last_collect != 0 && ( now - s_last_collect ) < 16 ) return;
		s_last_collect = now;

		static void* s_views[ 128 ]{};
		static vec3_t s_view_pos[ 128 ]{};
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

		// Death and respawn destroy and rebuild the battle models. Every pointer
		// cached against the previous scene then refers to freed memory, so the
		// caches are dropped at the first frame of the new one.
		if ( entities_model != g_scene_entities || battle_model != g_scene_battle ||
			 local_ent_check != g_scene_local_ent ) {
			reset_scene_caches_keep_view( );
			g_scene_entities = entities_model;
			g_scene_battle = battle_model;
			g_scene_local_ent = local_ent_check;
			g_local_char_model = unity::read_u64( local_ent_check + 0x20 );
			g_local_dead = false;
		}

		// Local death flag, read from the same chain the ESP already uses for every
		// player (common + 0xE0 is the dead bit). While it is set nothing may be
		// written into the scene: the character is torn down a few frames later
		// and the aim writes in particular must be released first.
		bool local_dead_now = false;
		{
			const uintptr_t cm = g_local_char_model;
			const uintptr_t cd = unity::is_managed_ptr( cm ) ? unity::read_u64( cm + 0x40 ) : 0;
			const uintptr_t common = unity::is_managed_ptr( cd ) ? unity::read_u64( cd + 0x20 ) : 0;
			if ( unity::is_managed_ptr( common ) ) {
				uint8_t dead_b = 0;
				if ( unity::read_prim_data( unity::read_u64( common + 0xE0 ), dead_b ) ) local_dead_now = ( dead_b != 0 );
			}
		}
		if ( local_dead_now && !g_local_dead ) {
			silent_release( );
			g_locked_obj = nullptr;
			g_has_aim_target = false;
			g_has_silent_target = false;
		}
		g_local_dead = local_dead_now;

		handle_hotkeys_and_camera( cam, battle_model );
		if ( !g_local_dead ) {
			apply_combat_and_misc_mods( battle_model, entities_model );
		} else {
			silent_release( );
		}
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

		// Position of every view once, reused by the fusion below.
		for ( int i = 0; i < s_view_count; ++i ) {
			s_view_pos[ i ] = {};
			if ( !unity::is_valid_obj( s_views[ i ] ) ) continue;
			void* tr = unity::get_component_transform( s_views[ i ] );
			if ( !unity::is_valid_obj( tr ) ) continue;
			unity::get_transform_pos( tr, s_view_pos[ i ] );
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
		vec3_t best_silent_world{};

		// Budget per frame for the expensive resolve; position reads afterwards are
		// native and cheap, so every on-screen player keeps their skeleton.
		int bone_budget = 4;

		const bool aim_key = !config::menu_open && is_aim_key_down( );
		g_is_firing_or_aiming = aim_key || ( ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 );

		auto consider_target = [&]( player_t& out ) {
			if ( !config::aim_master || g_local_dead || !out.on_screen || out.distance > eff_aim_dist ) return;
			if ( out.is_dead ) return;
			if ( config::aim_ignore_clan && g_local_clan[ 0 ] && out.clan[ 0 ] && unity::str_eq( g_local_clan, out.clan ) ) return;
			if ( config::aim_ignore_police && out.role_kind == ROLE_COP ) return;
			if ( config::aim_ignore_knocked && out.is_knocked ) return;
			if ( config::aim_ignore_friends && out.is_friend ) return;
			if ( config::aim_vis_check && !out.is_visible ) return;

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
				const float lead_t = ( ( out.distance / 400.0f ) + ( config::pred_net_buffer_ms * 0.001f ) ) * config::pred_lead_scale;
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
			if ( !world_to_screen( aim_w, aim_s ) ) return;
			const float adx = aim_s.x - cx;
			const float ady = aim_s.y - cy;
			const float fov_d = d3d11_ui::fast_sqrt( adx * adx + ady * ady );
			const float score = config::aim_closest_first ? out.distance : fov_d;
			const bool is_locked = ( config::aim_sticky && g_locked_obj == out.obj && fov_d <= config::aim_fov * 1.35f );
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
				best_silent_world = aim_w;
			}
		};

		// Pass 1: every entity the battle table knows. This is the authoritative
		// list - it keeps a player while they drive, sit or stream out of view -
		// and the character view is attached to it, not the other way round.
		int view_used[ 128 ]{};
		for ( int i = 0; i < 128; ++i ) view_used[ i ] = 0;

		for ( int i = 0; i < g_meta_count && next_count < k_max_players; ++i ) {
			const meta_entry_t& m = g_meta[ i ];
			if ( !m.valid || m.is_local || m.entity_id < 0 ) continue;

			player_t& out = dst[ next_count];
			out = {};
			out.entity_id = m.entity_id;
			out.vehicle_eid = m.vehicle_eid;
			out.role_kind = m.role_kind;
			out.is_team = ( m.role_kind == ROLE_TEAM );
			out.is_friend = config::is_friend( m.entity_id );
			out.has_shield = m.has_armor;
			out.health = 1.0f;
			out.in_vehicle = m.in_vehicle;
			out.is_knocked = m.is_knocked;
			out.is_dead = m.is_dead;
			out.look_dir = m.look_dir;
			copy_str( out.name, sizeof( out.name ), m.name );
			copy_str( out.tag, sizeof( out.tag ), m.tag );
			copy_str( out.clan, sizeof( out.clan ), m.clan );
			copy_str( out.weapon, sizeof( out.weapon ), m.weapon );

			out.root_world = m.pos;

			// Attach the closest unused character view, so the skeleton and the true
			// head height come from the body that is standing at this entity.
			int best_view = -1;
			float best_view_d = 3.5f;
			for ( int v = 0; v < s_view_count; ++v ) {
				if ( view_used[ v ] || !unity::is_valid_obj( s_views[ v ] ) ) continue;
				const float d = dist3( s_view_pos[ v ], m.pos );
				if ( d < best_view_d ) { best_view_d = d; best_view = v; }
			}

			const float stand_h = m.in_vehicle ? 1.42f : 1.58f;
			out.head_world = { m.pos.x, m.pos.y + stand_h, m.pos.z };
			out.chest_world = { m.pos.x, m.pos.y + stand_h * 0.70f, m.pos.z };
			out.pelvis_world = { m.pos.x, m.pos.y + stand_h * 0.50f, m.pos.z };
			out.obj = nullptr;
			out.distance = dist3( m.pos, g_cam_pos );
			out.is_visible = true;
			out.valid = true;

			if ( best_view >= 0 ) {
				view_used[ best_view ] = 1;
				out.obj = s_views[ best_view ];
				out.root_world = s_view_pos[ best_view ];
				out.distance = dist3( out.root_world, g_cam_pos );
				out.velocity = update_velocity( out.obj, out.root_world, now );

				void* head_tr = nullptr;
				if ( unity::is_mem_readable( reinterpret_cast< uintptr_t >( out.obj ) + 0xE8, sizeof( void* ) ) ) {
					head_tr = *reinterpret_cast< void** >( reinterpret_cast< uintptr_t >( out.obj ) + 0xE8 );
				}
				if ( unity::is_valid_obj( head_tr ) ) {
					vec3_t hp{};
					if ( unity::get_transform_pos( head_tr, hp ) && ( hp.y - out.root_world.y ) > 0.35f &&
						 ( hp.y - out.root_world.y ) < 2.35f ) {
						out.head_world = hp;
						// A crouching or seated body is shorter than its origin suggests;
						// keeping the box proportional to the head keeps it on the body.
						const float span = hp.y - out.root_world.y;
						out.is_crouching = ( !m.in_vehicle && span < 1.25f );
						out.chest_world = { hp.x, out.root_world.y + span * 0.70f, hp.z };
						out.pelvis_world = { hp.x, out.root_world.y + span * 0.50f, hp.z };
					}
				}

				if ( ( config::esp_skeleton || config::esp_hitboxes ) && out.distance <= 110.0f ) {
					fill_player_bones( out, out.obj, now, bone_budget );
					if ( out.has_skeleton ) {
						out.chest_world = out.bone_ok[ BONE_CHEST ] ? out.bones_world[ BONE_CHEST ] : out.chest_world;
						out.pelvis_world = out.bone_ok[ BONE_HIPS ] ? out.bones_world[ BONE_HIPS ] : out.pelvis_world;
					}
				}
			}

			if ( !project_player_box( out ) ) continue;

			// Line of sight for the aim path only: a physics linecast per player is
			// not free, and the ESP itself does not need it.
			if ( config::aim_vis_check && out.on_screen && out.distance <= eff_aim_dist && unity::fn_phys_linecast ) {
				out.is_visible = !unity::unity_linecast( g_cam_pos, out.head_world );
			}

			consider_target( out );
			++next_count;
		}

		// Pass 2: character views with no entity entry (streamed in ahead of the
		// table). They are drawn without identity rather than dropped.
		for ( int v = 0; v < s_view_count && next_count < k_max_players; ++v ) {
			if ( view_used[ v ] || !unity::is_valid_obj( s_views[ v ] ) ) continue;
			const vec3_t pos = s_view_pos[ v ];
			if ( pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f ) continue;

			player_t& out = dst[ next_count ];
			out = {};
			out.entity_id = -1;
			out.obj = s_views[ v ];
			out.root_world = pos;
			out.distance = dist3( pos, g_cam_pos );
			// Never box the local player's own body.
			if ( out.distance < 1.2f ) continue;

			out.head_world = { pos.x, pos.y + 1.58f, pos.z };
			out.chest_world = { pos.x, pos.y + 1.11f, pos.z };
			out.pelvis_world = { pos.x, pos.y + 0.79f, pos.z };
			out.health = 1.0f;
			out.role_kind = ROLE_ROBBER;
			out.velocity = update_velocity( out.obj, pos, now );
			out.is_visible = true;
			out.valid = true;

			void* head_tr = nullptr;
			if ( unity::is_mem_readable( reinterpret_cast< uintptr_t >( out.obj ) + 0xE8, sizeof( void* ) ) ) {
				head_tr = *reinterpret_cast< void** >( reinterpret_cast< uintptr_t >( out.obj ) + 0xE8 );
			}
			if ( unity::is_valid_obj( head_tr ) ) {
				vec3_t hp{};
				if ( unity::get_transform_pos( head_tr, hp ) && ( hp.y - pos.y ) > 0.35f && ( hp.y - pos.y ) < 2.35f ) {
					out.head_world = hp;
					const float span = hp.y - pos.y;
					out.chest_world = { hp.x, pos.y + span * 0.70f, hp.z };
					out.pelvis_world = { hp.x, pos.y + span * 0.50f, hp.z };
					out.is_crouching = ( span < 1.25f );
				}
			}

			if ( ( config::esp_skeleton || config::esp_hitboxes ) && out.distance <= 110.0f ) {
				fill_player_bones( out, out.obj, now, bone_budget );
				if ( out.has_skeleton ) {
					out.chest_world = out.bone_ok[ BONE_CHEST ] ? out.bones_world[ BONE_CHEST ] : out.chest_world;
					out.pelvis_world = out.bone_ok[ BONE_HIPS ] ? out.bones_world[ BONE_HIPS ] : out.pelvis_world;
				}
			}

			if ( !project_player_box( out ) ) continue;
			if ( config::aim_vis_check && out.on_screen && out.distance <= eff_aim_dist && unity::fn_phys_linecast ) {
				out.is_visible = !unity::unity_linecast( g_cam_pos, out.head_world );
			}
			consider_target( out );
			++next_count;
		}

		g_player_count_buf[ back ] = next_count;
		InterlockedExchange( &g_active_buf, back );
		config::diag_players_tracked = next_count;

		if ( best_aim_idx >= 0 && best_aim_idx < next_count ) {
			g_locked_obj = dst[ best_aim_idx ].obj;
			g_aim_target_screen = best_aim_screen;
			g_has_aim_target = true;
			if ( aim_key && roll_percent( config::aim_hit_chance ) ) {
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

		// Silent aim: a class-verified direction pulse instead of a permanent write
		// into a model that was only picked because its name looked right.
		if ( config::aim_master && config::aim_mode == 2 && !g_local_dead &&
			 best_silent_idx >= 0 && best_silent_idx < next_count && dst[ best_silent_idx ].entity_id > 0 ) {
			g_silent_target_screen = best_silent_screen;
			g_has_silent_target = true;
			silent_aim_update( battle_model, g_local_char_model, g_is_firing_or_aiming,
							   dst[ best_silent_idx ].entity_id, best_silent_world, now );
		} else {
			g_has_silent_target = false;
			silent_aim_update( battle_model, g_local_char_model, false, -1, vec3_t{}, now );
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
	// ---- Rendering ------------------------------------------------------
	inline d3d11_ui::color_t get_role_color( const player_t& p ) {
		if ( p.is_friend ) return rgba( config::esp_col.friend_col );
		if ( config::esp_police_coworkers && g_local_role == ROLE_COP && p.role_kind == ROLE_COP ) {
			return rgba( config::esp_col.team );
		}
		int idx = 1;
		if ( p.role_kind == ROLE_COP ) idx = 0;
		else if ( p.role_kind == ROLE_CIV ) idx = 2;
		return rgba( config::esp_prof[ idx ].color );
	}

	// Bone index pairs plus the length each segment may have, in metres. The
	// distance between two joints does not depend on how far the player is from
	// the camera, so a wrong transform is rejected at any range.
	struct skel_seg_t { int a, b; float min_m; float max_m; };
	inline constexpr skel_seg_t k_skel_segs[] = {
		{ 0, 1, 0.01f, 0.30f }, { 1, 2, 0.01f, 0.32f }, { 2, 3, 0.01f, 0.36f }, { 3, 4, 0.01f, 0.40f }, // spine
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

		// Second gate in screen space: a correct rig never projects a limb longer
		// than the body box, so anything that does is a stale transform.
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

		if ( !p.bone_ok[ BONE_HEAD ] ) return;
		float hr = d3d11_ui::clampf( body_h * 0.072f, 2.5f, 20.0f );
		if ( p.bone_ok[ BONE_NECK ] ) {
			const float dx = p.bones[ BONE_HEAD ].x - p.bones[ BONE_NECK ].x;
			const float dy = p.bones[ BONE_HEAD ].y - p.bones[ BONE_NECK ].y;
			const float d = d3d11_ui::fast_sqrt( dx * dx + dy * dy );
			if ( d > 1.0f ) hr = d3d11_ui::clampf( d * 0.62f, 2.5f, 20.0f );
		}
		d3d11_ui::add_circle( p.bones[ BONE_HEAD ].x, p.bones[ BONE_HEAD ].y, hr, 18, lw + 1.1f, shadow );
		d3d11_ui::add_circle( p.bones[ BONE_HEAD ].x, p.bones[ BONE_HEAD ].y, hr, 18, lw, c );
	}

	// Fallback figure, built entirely in screen space. It is only used when the
	// rig did not resolve; building it from the 3D look direction put the arms in
	// front of the torso whenever the player faced the camera.
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

		seg( cx, neck_y, cx, pelvis_y );
		seg( cx, shoulder_y, cx - sh_w, shoulder_y );
		seg( cx, shoulder_y, cx + sh_w, shoulder_y );
		seg( cx - sh_w, shoulder_y, cx - hand_w, hand_y );
		seg( cx + sh_w, shoulder_y, cx + hand_w, hand_y );
		seg( cx, pelvis_y, cx - foot_w, bottom );
		seg( cx, pelvis_y, cx + foot_w, bottom );

		const float hr = d3d11_ui::clampf( h * 0.072f, 2.5f, 20.0f );
		const float hy = top + h * 0.085f;
		d3d11_ui::add_circle( cx, hy, hr, 18, lw + 1.1f, shadow );
		d3d11_ui::add_circle( cx, hy, hr, 18, lw, c );
	}

	inline void draw_skeleton( const player_t& p, float body_h, const d3d11_ui::color_t& c ) {
		const float lw = config::esp_skel_width > 0.5f ? config::esp_skel_width : 1.3f;
		const d3d11_ui::color_t shadow{ 0.0f, 0.0f, 0.0f, c.a * 0.85f };

		if ( p.has_skeleton ) {
			int valid = 0;
			for ( int i = 0; i < k_skel_seg_count; ++i ) {
				if ( skeleton_segment_ok( p, k_skel_segs[ i ], body_h ) ) ++valid;
			}
			// Four good segments already read as a body; requiring more threw away
			// perfectly good rigs with a partially resolved skeleton.
			if ( valid >= 4 ) {
				draw_skeleton_real( p, body_h, c, shadow, lw );
				return;
			}
		}
		draw_skeleton_fallback( p, body_h, c, shadow, lw );
	}

	inline void draw_3d_hitboxes( const player_t& p, const d3d11_ui::color_t& c ) {
		vec2_t hs{}, htop{};
		if ( world_to_screen( p.head_world, hs ) && world_to_screen( { p.head_world.x, p.head_world.y + 0.13f, p.head_world.z }, htop ) ) {
			const float rad = d3d11_ui::clampf( hs.y - htop.y, 3.0f, 30.0f );
			d3d11_ui::add_circle( hs.x, hs.y, rad, 16, 1.3f, c );
		}
		vec2_t ch{}, pv{};
		if ( world_to_screen( p.chest_world, ch ) && world_to_screen( p.pelvis_world, pv ) ) {
			const float cap_w = d3d11_ui::clampf( ( pv.y - ch.y ) * 0.48f, 4.0f, 34.0f );
			d3d11_ui::add_rounded_rect( ch.x - cap_w * 0.5f, ch.y, cap_w, pv.y - ch.y, cap_w * 0.35f, 1.2f, c );
		}
	}

	// ---- Label layout ---------------------------------------------------
	// Every label is placed through one registry. A label that would land on one
	// already placed moves up in whole lines, then below its anchor, and is only
	// dropped when the screen has no free room left. That replaces the old
	// per-label guesswork, where two players standing together printed their
	// names on top of each other.
	struct label_rect_t { float x0, y0, x1, y1; };
	inline label_rect_t g_label_rects[ 512 ]{};
	inline int g_label_rect_count = 0;

	inline void reset_label_layout( ) { g_label_rect_count = 0; }

	inline bool label_rect_free( float x0, float y0, float x1, float y1 ) {
		for ( int i = 0; i < g_label_rect_count; ++i ) {
			const label_rect_t& r = g_label_rects[ i ];
			if ( x1 < r.x0 || x0 > r.x1 ) continue;
			if ( y1 < r.y0 || y0 > r.y1 ) continue;
			return false;
		}
		return true;
	}

	inline bool place_label( float cx, float preferred_top, float w, float h, float& out_top ) {
		if ( w < 1.0f ) w = 1.0f;
		if ( h < 1.0f ) h = 1.0f;
		const float half = w * 0.5f;
		for ( int attempt = 0; attempt < 6; ++attempt ) {
			float top = preferred_top - static_cast< float >( attempt ) * ( h + 2.0f );
			if ( top < 2.0f ) top = preferred_top + static_cast< float >( attempt + 1 ) * ( h + 2.0f );
			if ( top < 2.0f || top + h > d3d11_ui::g_height - 2.0f ) continue;
			const float x0 = cx - half - 1.0f;
			const float x1 = cx + half + 1.0f;
			if ( !label_rect_free( x0, top, x1, top + h ) ) continue;
			if ( g_label_rect_count < 512 ) g_label_rects[ g_label_rect_count++ ] = { x0, top, x1, top + h };
			out_top = top;
			return true;
		}
		return false;
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
					? rgba( it.category == WCAT_HELI ? config::esp_col.heli : config::esp_col.vehicle, 0.85f )
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
				d3d11_ui::add_circle( px, py, 3.2f, 10, 1.3f, rc );
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
		if ( count <= 0 ) return;

		// A loot pile is a dozen identical items on one square metre, and the same
		// cupboard is published by several tables at once. Everything loot-like
		// inside 2.5 m becomes one label; the most specific name wins.
		struct world_group_t {
			vec3_t world_pos{};
			vec2_t screen_pos{};
			float distance = 0.0f;
			int category = WCAT_LOOSE;
			int rarity = 0;
			int num = 1;
			int value = 0;
			bool locked = false;
			char label[ 48 ]{};
		};
		auto is_lootish = []( int cat ) -> bool {
			return cat == WCAT_LOOSE || cat == WCAT_NODE || cat == WCAT_CONTAINER || cat == WCAT_DROPPED;
		};
		auto cat_rank = []( int cat ) -> int {
			if ( cat == WCAT_CONTAINER ) return 4;
			if ( cat == WCAT_DOOR ) return 3;
			if ( cat == WCAT_NODE ) return 2;
			if ( cat == WCAT_DROPPED ) return 2;
			return 1;
		};

		static world_group_t grp[ 96 ];
		int n = 0;
		for ( int i = 0; i < count; ++i ) {
			const world_item_t& it = items[ i ];
			if ( !it.valid ) continue;
			const char* lbl = it.label[ 0 ] ? it.label : "ITEM";
			const bool lootish = is_lootish( it.category );
			int hit = -1;
			for ( int g = 0; g < n; ++g ) {
				if ( is_lootish( grp[ g ].category ) != lootish ) continue;
				if ( dist3( grp[ g ].world_pos, it.world_pos ) > 2.5f ) continue;
				if ( !lootish &&
					 ( grp[ g ].category != it.category || !unity::str_eq( grp[ g ].label, lbl ) ) ) continue;
				hit = g;
				break;
			}
			if ( hit >= 0 ) {
				world_group_t& g = grp[ hit ];
				if ( unity::str_eq( g.label, lbl ) ) ++g.num;
				if ( lootish && cat_rank( it.category ) > cat_rank( g.category ) ) {
					g.category = it.category;
					g.rarity = it.rarity;
					g.locked = it.locked;
					g.value = it.value;
					copy_str( g.label, sizeof( g.label ), lbl );
				}
				// Keep the nearest member as the anchor: it is the one the player
				// walks to, and the one whose screen position is most accurate.
				if ( it.distance < g.distance ) {
					g.distance = it.distance;
					g.world_pos = it.world_pos;
					g.screen_pos = it.screen_pos;
					if ( g.value <= 0 ) g.value = it.value;
					g.locked = g.locked || it.locked;
				}
				continue;
			}
			if ( n >= 96 ) continue;
			world_group_t& g = grp[ n++ ];
			g.world_pos = it.world_pos;
			g.screen_pos = it.screen_pos;
			g.distance = it.distance;
			g.category = it.category;
			g.rarity = it.rarity;
			g.num = 1;
			g.value = it.value;
			g.locked = it.locked;
			copy_str( g.label, sizeof( g.label ), lbl );
		}

		// Nearest first: when two labels collide, the near one is worth reading.
		for ( int i = 1; i < n; ++i ) {
			const world_group_t key = grp[ i ];
			int j = i - 1;
			while ( j >= 0 && grp[ j ].distance > key.distance ) {
				grp[ j + 1 ] = grp[ j ];
				--j;
			}
			grp[ j + 1 ] = key;
		}

		for ( int i = 0; i < n; ++i ) {
			const world_group_t& g = grp[ i ];
			const float sx = g.screen_pos.x;
			const float sy = g.screen_pos.y;
			if ( sx < -80.0f || sx > d3d11_ui::g_width + 80.0f || sy < -80.0f || sy > d3d11_ui::g_height + 80.0f ) continue;

			d3d11_ui::color_t c = get_rarity_color( g.rarity );
			if ( g.category == WCAT_CAR ) c = rgba( config::esp_col.vehicle );
			else if ( g.category == WCAT_HELI ) c = rgba( config::esp_col.heli );
			else if ( g.category == WCAT_BUYER ) c = rgba( config::esp_col.buyer );
			else if ( g.category == WCAT_CAMERA ) c = { 0.35f, 0.85f, 0.95f, 1.0f };
			if ( g.locked ) c = { 1.00f, 0.30f, 0.32f, 1.0f };

			char buf[ 80 ]{};
			char* bend = buf + sizeof( buf ) - 1;
			char* bp = d3d11_ui::put_str( buf, bend, g.label );
			if ( g.num > 1 ) {
				bp = d3d11_ui::put_str( bp, bend, " x" );
				bp = d3d11_ui::put_int( bp, bend, g.num );
			}
			bp = d3d11_ui::put_char( bp, bend, ' ' );
			bp = d3d11_ui::put_char( bp, bend, '[' );
			bp = d3d11_ui::put_int( bp, bend, static_cast< int >( g.distance + 0.5f ) );
			bp = d3d11_ui::put_char( bp, bend, 'm' );
			bp = d3d11_ui::put_char( bp, bend, ']' );
			if ( g.value > 0 ) {
				bp = d3d11_ui::put_char( bp, bend, ' ' );
				bp = d3d11_ui::put_int( bp, bend, g.value );
			}
			*bp = '\0';

			const float w = d3d11_ui::measure_text_width( buf, 0.76f );
			float top = 0.0f;
			const float anchor_top = sy + 3.0f;
			if ( !place_label( sx, anchor_top, w, 11.0f, top ) ) continue;

			d3d11_ui::add_filled_circle( sx, sy - 8.0f, 3.0f, 8, { 0.0f, 0.0f, 0.0f, 0.85f } );
			d3d11_ui::add_filled_circle( sx, sy - 8.0f, 2.0f, 8, c );
			draw_esp_text( sx, top + 5.5f, buf, c, 0.76f );
		}
	}
	inline void append_str( char* dst, size_t cap, const char* s ) {
		if ( !dst || cap == 0 || !s ) return;
		size_t o = 0;
		while ( o + 1 < cap && dst[ o ] ) ++o;
		for ( size_t i = 0; s[ i ] && o + 1 < cap; ++i ) dst[ o++ ] = s[ i ];
		dst[ o ] = '\0';
	}

	inline void render_players( ) {
		const LONG active = InterlockedCompareExchange( &g_active_buf, 0, 0 );
		const int count = g_player_count_buf[ active ];
		const player_t* players = g_players_buf[ active ];
		if ( count <= 0 ) return;

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;
		const bool minimal_noise = ( config::esp_priority_mode == 2 && g_is_firing_or_aiming );

		// Eligible players, sorted far to near. Far boxes are drawn first so the
		// near ones stay on top; labels are placed in the opposite order further
		// down, so the nearest label always wins a collision.
		int order[ k_max_players ];
		int n = 0;
		for ( int i = 0; i < count; ++i ) {
			const player_t& p = players[ i ];
			if ( !p.valid ) continue;
			if ( p.is_dead && !config::esp_corpse ) continue;
			const int prof_idx = ( p.role_kind == ROLE_COP ) ? 0 : ( p.role_kind == ROLE_CIV ? 2 : 1 );
			if ( !config::esp_prof[ prof_idx ].enabled ) continue;
			if ( p.distance > config::esp_prof[ prof_idx ].max_dist ) continue;
			order[ n++ ] = i;
		}
		for ( int i = 1; i < n; ++i ) {
			const int key = order[ i ];
			const float kd = players[ key ].distance;
			int j = i - 1;
			while ( j >= 0 && players[ order[ j ] ].distance < kd ) {
				order[ j + 1 ] = order[ j ];
				--j;
			}
			order[ j + 1 ] = key;
		}

		// Off-screen arrows first: they are outside the boxes and never collide.
		if ( config::esp_oof_arrows && !minimal_noise ) {
			const float inset = d3d11_ui::clampf( config::esp_oof_inset, 0.15f, 0.48f );
			const float rad_x = d3d11_ui::g_width * inset;
			const float rad_y = d3d11_ui::g_height * inset;
			const float sz = d3d11_ui::clampf( config::esp_oof_size, 5.0f, 18.0f );
			for ( int k = 0; k < n; ++k ) {
				const player_t& p = players[ order[ k ] ];
				if ( p.on_screen ) continue;
				// Direction in the camera's frame: a point behind the camera does not
				// project, so its screen position cannot be used to aim the arrow.
				float rd = 0.0f, fd = 0.0f;
				cam_space_dir( p.root_world, rd, fd );
				float dx = rd, dy = -fd;
				const float len = d3d11_ui::fast_sqrt( dx * dx + dy * dy );
				if ( len < 0.001f ) continue;
				dx /= len;
				dy /= len;
				float px = cx + dx * rad_x;
				float py = cy + dy * rad_y;
				px = d3d11_ui::clampf( px, 14.0f, d3d11_ui::g_width - 14.0f );
				py = d3d11_ui::clampf( py, 14.0f, d3d11_ui::g_height - 14.0f );
				const float nx = -dy, ny = dx;
				const d3d11_ui::color_t col = get_role_color( p );
				d3d11_ui::add_filled_triangle(
					px + dx * sz, py + dy * sz,
					px - dx * ( sz * 0.5f ) + nx * ( sz * 0.68f ), py - dy * ( sz * 0.5f ) + ny * ( sz * 0.68f ),
					px - dx * ( sz * 0.5f ) - nx * ( sz * 0.68f ), py - dy * ( sz * 0.5f ) - ny * ( sz * 0.68f ),
					col
				);
			}
		}

		for ( int k = 0; k < n; ++k ) {
			const player_t& p = players[ order[ k ] ];
			if ( !p.on_screen ) continue;

			const float box_h = p.root_screen.y - p.head_screen.y;
			if ( !( box_h > 4.0f ) || box_h > 1400.0f ) continue;
			const bool tiny_target = box_h < 9.0f;
			const float box_w = d3d11_ui::clampf( box_h * 0.5f, 8.0f, 220.0f );
			const float box_x = p.head_screen.x - box_w * 0.5f;
			const float box_y = p.head_screen.y;
			const d3d11_ui::color_t col = get_role_color( p );

			if ( config::esp_box_fill ) {
				d3d11_ui::add_filled_rect( box_x, box_y, box_w, box_h, { col.r, col.g, col.b, 0.14f } );
			}
			if ( config::esp_box ) {
				if ( config::esp_box_outline ) d3d11_ui::add_rect_outlined( box_x, box_y, box_w, box_h, 1.0f, col, { 0.0f, 0.0f, 0.0f, 0.72f } );
				else d3d11_ui::add_rect( box_x, box_y, box_w, box_h, 1.0f, col );
			}
			if ( config::esp_skeleton && !minimal_noise && !tiny_target ) {
				draw_skeleton( p, box_h, rgba( config::esp_col.skeleton, config::esp_col.skeleton.a ) );
			}
			if ( config::esp_hitboxes && !minimal_noise && !tiny_target ) {
				draw_3d_hitboxes( p, { col.r, col.g, col.b, 0.75f } );
			}
			if ( config::esp_snaplines && !minimal_noise ) {
				const float dx = p.root_screen.x - cx;
				const float dy = p.root_screen.y - cy;
				if ( d3d11_ui::fast_sqrt( dx * dx + dy * dy ) >= config::esp_snap_radius ) {
					d3d11_ui::add_line( cx, d3d11_ui::g_height, p.root_screen.x, p.root_screen.y, 1.0f, col );
				}
			}
		}

		// Labels, nearest first, through the shared layout so nothing overlaps.
		const float line_h = 12.5f;
		for ( int k = n - 1; k >= 0; --k ) {
			const player_t& p = players[ order[ k ] ];
			if ( !p.on_screen ) continue;

			const float box_h = p.root_screen.y - p.head_screen.y;
			if ( !( box_h > 8.5f ) || minimal_noise ) continue;
			const float box_y = p.head_screen.y;
			const d3d11_ui::color_t col = get_role_color( p );

			char above[ 2 ][ 80 ]{};
			int n_above = 0;
			if ( config::esp_name && ( p.name[ 0 ] || p.tag[ 0 ] ) ) {
				char* ap = above[ n_above ];
				char* aend = ap + sizeof( above[ 0 ] ) - 1;
				char* q = ap;
				if ( config::esp_role && p.tag[ 0 ] && p.name[ 0 ] ) {
					q = d3d11_ui::put_char( q, aend, '[' );
					q = d3d11_ui::put_str( q, aend, p.tag );
					q = d3d11_ui::put_char( q, aend, ']' );
					q = d3d11_ui::put_char( q, aend, ' ' );
				}
				q = d3d11_ui::put_str( q, aend, p.name[ 0 ] ? p.name : p.tag );
				*q = '\0';
				++n_above;
			}
			if ( config::esp_clan && p.clan[ 0 ] ) {
				// A clan tag goes on the name line: a separate line per clan was
				// three lines of text per player in a busy fight.
				if ( n_above > 0 ) {
					append_str( above[ 0 ], sizeof( above[ 0 ] ), " <" );
					append_str( above[ 0 ], sizeof( above[ 0 ] ), p.clan );
					append_str( above[ 0 ], sizeof( above[ 0 ] ), ">" );
				} else {
					char* ap = above[ n_above ];
					char* aend = ap + sizeof( above[ 0 ] ) - 1;
					char* q = d3d11_ui::put_char( ap, aend, '<' );
					q = d3d11_ui::put_str( q, aend, p.clan );
					q = d3d11_ui::put_char( q, aend, '>' );
					*q = '\0';
					++n_above;
				}
			}
			if ( config::esp_weapon && p.weapon[ 0 ] && n_above < 2 ) {
				copy_str( above[ n_above ], sizeof( above[ 0 ] ), p.weapon );
				++n_above;
			}

			char below[ 2 ][ 80 ]{};
			int n_below = 0;
			{
				char* bp = below[ n_below ];
				char* bend = bp + sizeof( below[ 0 ] ) - 1;
				char* q = d3d11_ui::put_int( bp, bend, static_cast< int >( p.distance + 0.5f ) );
				q = d3d11_ui::put_char( q, bend, 'm' );
				*q = '\0';
				if ( config::esp_armor && p.has_shield ) append_str( below[ n_below ], sizeof( below[ 0 ] ), " ARMOR" );
				if ( config::esp_crouch && p.is_crouching ) append_str( below[ n_below ], sizeof( below[ 0 ] ), " CROUCH" );
				if ( config::esp_knocked && p.is_knocked ) append_str( below[ n_below ], sizeof( below[ 0 ] ), " KNOCKED" );
				++n_below;
			}

			float widest = 0.0f;
			for ( int i = 0; i < n_above; ++i ) {
				const float w = d3d11_ui::measure_text_width( above[ i ], 0.80f );
				if ( w > widest ) widest = w;
			}
			for ( int i = 0; i < n_below; ++i ) {
				const float w = d3d11_ui::measure_text_width( below[ i ], 0.74f );
				if ( w > widest ) widest = w;
			}
			if ( widest <= 0.0f ) continue;

			if ( n_above > 0 ) {
				const float h = static_cast< float >( n_above ) * line_h;
				// Above the box by default; a player standing at the top edge has no room
				// there, and the block goes under the box instead of onto it.
				const float preferred = ( box_y - 4.0f - h >= 2.0f ) ? ( box_y - 4.0f - h ) : ( box_y + box_h + 4.0f );
				float top = 0.0f;
				if ( place_label( p.head_screen.x, preferred, widest, h, top ) ) {
					for ( int i = 0; i < n_above; ++i ) {
						const d3d11_ui::color_t c = ( i == 0 ) ? col : d3d11_ui::color_t{ 0.93f, 0.95f, 1.0f, 1.0f };
						draw_esp_text( p.head_screen.x, top + static_cast< float >( i ) * line_h + line_h * 0.5f,
									   above[ i ], c, i == 0 ? 0.80f : 0.74f );
					}
				}
			}
			if ( n_below > 0 ) {
				const float h = static_cast< float >( n_below ) * line_h;
				const float below_top = box_y + box_h + 4.0f;
				const float fallback = ( below_top + h <= d3d11_ui::g_height - 2.0f ) ? below_top : ( box_y - 4.0f - h );
				float top = 0.0f;
				if ( place_label( p.head_screen.x, fallback, widest, h, top ) ) {
					for ( int i = 0; i < n_below; ++i ) {
						draw_esp_text( p.head_screen.x, top + static_cast< float >( i ) * line_h + line_h * 0.5f,
									   below[ i ], { 0.78f, 0.82f, 0.90f, 1.0f }, 0.74f );
					}
				}
			}
		}
	}

	inline void render_diag( ) {
		if ( !config::show_diag ) return;
		char line[ 128 ]{};
		char* end = line + sizeof( line ) - 1;
		char* p = d3d11_ui::put_str( line, end, "BONES " );
		p = d3d11_ui::put_int( p, end, config::diag_skel_nodes );
		p = d3d11_ui::put_str( p, end, "   PLAYERS " );
		p = d3d11_ui::put_int( p, end, config::diag_players_tracked );
		p = d3d11_ui::put_str( p, end, "   WORLD " );
		p = d3d11_ui::put_int( p, end, config::diag_world_tracked );
		p = d3d11_ui::put_str( p, end, "   AIM " );
		p = d3d11_ui::put_str( p, end, config::diag_aim_class[ 0 ] ? config::diag_aim_class : "none" );
		p = d3d11_ui::put_str( p, end, "   SILENT " );
		if ( config::diag_silent_eid > 0 ) p = d3d11_ui::put_int( p, end, config::diag_silent_eid );
		else p = d3d11_ui::put_str( p, end, "idle" );
		*p = '\0';
		d3d11_ui::add_text( 18.0f, 120.0f, line, { 0.55f, 1.0f, 0.70f, 0.95f } );

		if ( g_value_probe_n > 0 ) {
			char vline[ 128 ]{};
			char* vend = vline + sizeof( vline ) - 1;
			char* vp = d3d11_ui::put_str( vline, vend, "VAL " );
			vp = d3d11_ui::put_str( vp, vend, g_value_probe_id );
			for ( int i = 0; i < g_value_probe_n && vp < vend - 24; ++i ) {
				vp = d3d11_ui::put_char( vp, vend, ' ' );
				vp = d3d11_ui::put_int( vp, vend, g_value_probe_offs[ i ] );
				vp = d3d11_ui::put_char( vp, vend, '=' );
				vp = d3d11_ui::put_int( vp, vend, g_value_probe_vals[ i ] );
			}
			*vp = '\0';
			d3d11_ui::add_text( 18.0f, 132.0f, vline, { 0.55f, 1.0f, 0.70f, 0.95f } );
		}
	}

	inline void render( ) {
		// A stale LateUpdate means the scene changed under the drawing thread (a
		// loading screen, a rebuild): skip instead of drawing the previous frame's
		// pointers.
		if ( !g_camera || !config::model_chain_ok ) return;
		const ULONGLONG lu = g_last_late_update_tick;
		if ( lu == 0 || ( GetTickCount64( ) - lu ) > 350 ) return;

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;

		render_diag( );

		if ( config::aim_master && config::aim_draw_fov ) {
			d3d11_ui::add_circle( cx, cy, config::aim_fov, 64, 1.2f, rgba( config::esp_col.fov, config::esp_col.fov.a ) );
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

		reset_label_layout( );
		render_world( );
		if ( !config::esp_master ) return;

		const LONG active = InterlockedCompareExchange( &g_active_buf, 0, 0 );
		const int count = g_player_count_buf[ active ];
		const player_t* players = g_players_buf[ active ];

		if ( config::esp_radar ) render_radar( players, count );

		if ( config::esp_death_point ) {
			const ULONGLONG now = GetTickCount64( );
			for ( int d = 0; d < k_max_deaths; ++d ) {
				if ( !g_deaths[ d ].valid ) continue;
				// Markers used to live for the whole session: after 64 deaths the
				// array was full and nothing new could be recorded for the match.
				if ( now - g_deaths[ d ].tick > 300000ull ) {
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

		render_players( );
	}
}
