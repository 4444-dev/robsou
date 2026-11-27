// orelia-internal/workspace/game/features/features.h
#pragma once
#include <Windows.h>
#include <cstdint>

namespace config {
	struct rgba_t {
		float r, g, b, a;
	};

	// ---- System / Menu ----
	inline bool menu_open = true;
	inline int menu_tab = 0; // 0: Combat, 1: Visuals, 2: Radar/World, 3: Misc/Plots
	inline int menu_key_mode = 0; // 0: INS/F8, 1: DEL, 2: HOME, 3: END, 4: F4
	inline int theme_accent = 0;

	// ---- Diagnostics ----
	inline bool il2cpp_hook_ready = false;
	inline bool il2cpp_hook_failed = false;
	inline bool model_chain_ok = false;
	inline int diag_players_tracked = 0;
	inline bool show_diag = false;
	inline int diag_skel_nodes = 0;
	inline int diag_skel_edges = 0;
	inline int diag_silent_eid = -1;
	inline char diag_aim_class[ 48 ]{};
	inline int diag_world_tracked = 0;
	inline int diag_models_read = 0;
	inline volatile LONG diag_seh_faults = 0;

	// ---- Combat / Aim ----
	inline bool aim_master = true;
	inline int  aim_mode = 0; // 0: Aim Assist (Mouse), 1: Direct 3D Aim (6B), 2: Silent Aim
	inline int  aim_key_mode = 0; // 0: RMB, 1: LMB, 2: Shift, 3: XButton2, 4: Alt
	inline float aim_fov = 140.0f;
	inline float aim_smooth = 4.0f;
	inline float aim_sens_calib = 1.0f;
	inline float aim_max_dist = 350.0f;

	inline float aim_silent_fov = 95.0f;
	inline float aim_hit_chance = 100.0f;
	inline bool aim_silent_indicator = true;
	inline bool aim_draw_fov = true;
	inline bool aim_draw_line = true;
	inline bool aim_sticky = true;

	inline bool trig_enabled = false;
	inline float trig_delay_ms = 25.0f;
	inline bool trig_semi_auto = false;
	inline float trig_fov = 16.0f;

	inline bool rcs_enabled = false;
	inline float rcs_strength = 0.85f;
	inline bool no_recoil = false;
	inline bool no_spread = false;
	inline bool no_cam_shake = false;

	inline bool pred_enabled = true;
	inline bool pred_vehicle = true;
	inline bool pred_camera = true;
	inline float pred_net_buffer_ms = 35.0f;
	inline float pred_lead_scale = 1.0f;

	// 0:Head, 1:Neck, 2:Chest, 3:Pelvis, 4:Stomach, 5:Arms, 6:Legs, 7:Face, 8:Hands, 9:Feet, 10:Closest
	inline int  aim_bone_mode = 0;
	inline bool aim_multipoint = false;
	inline float aim_multipoint_scale = 0.65f;
	inline bool aim_vis_check = false;
	inline bool aim_weapon_aware = false;

	inline bool aim_ignore_clan = true;
	inline bool aim_ignore_police = false;
	inline bool aim_ignore_knocked = true;
	inline bool aim_ignore_friends = true;
	inline bool aim_closest_first = false;
	inline bool aim_prefer_head = true;

	// ---- Player ESP Profiles (0: Police, 1: Robbers, 2: Civilians) ----
	struct esp_profile_t {
		bool enabled = true;
		float max_dist = 500.0f;
		rgba_t color{ 0.97f, 0.34f, 0.38f, 1.0f };
	};
	inline esp_profile_t esp_prof[ 3 ] = {
		{ true, 500.0f, { 0.25f, 0.72f, 1.00f, 1.0f } }, // Police
		{ true, 500.0f, { 0.97f, 0.34f, 0.38f, 1.0f } }, // Robbers
		{ true, 350.0f, { 0.86f, 0.88f, 0.92f, 1.0f } }  // Civilians
	};

	inline bool esp_master = true;
	inline bool esp_police_coworkers = true;
	inline bool esp_box = true;
	inline bool esp_box_outline = true;
	inline bool esp_box_fill = false;
	inline bool esp_skeleton = true;
	inline float esp_skel_width = 1.3f;
	inline bool esp_hitboxes = false;
	inline bool esp_oof_arrows = true;
	inline float esp_oof_size = 9.0f;
	inline float esp_oof_inset = 0.36f;
	inline bool esp_snaplines = false;
	inline float esp_snap_radius = 0.0f;
	inline bool esp_corpse = true;
	inline bool esp_death_point = true;

	inline bool esp_name = true;
	inline bool esp_role = true;
	inline bool esp_clan = true;
	inline bool esp_distance = true;
	inline bool esp_weapon = true;
	inline bool esp_armor = true;
	inline bool esp_crouch = true;
	inline bool esp_knocked = true;
	inline bool esp_font_8way = true;

	// ---- Radar ----
	inline bool esp_radar = true;
	inline float radar_range = 150.0f;
	inline float radar_size = 156.0f;
	inline bool radar_world_markers = true;

	// ---- World / Loot / Vehicles / Plots ----
	inline bool world_master = true;
	inline bool world_loose = true;
	inline bool world_nodes = true;
	inline bool world_containers = true;
	inline bool world_hide_empty = true;
	inline bool world_dropped = true;
	// bitmask: bit 0=None, 1=Common, 2=Uncommon, 3=Rare, 4=Epic, 5=Legendary, 6=Exotic
	inline uint32_t world_rarity_mask = 0x7F;
	inline float world_loot_dist = 200.0f;
	inline float world_obj_dist = 300.0f;

	inline bool world_cars = true;
	inline bool world_helis = true;
	inline float world_veh_dist = 400.0f;
	inline bool world_hide_local_veh = true;

	inline bool plot_panel_enabled = false;
	inline int  plot_panel_key = 0; // 0: F6, 1: F7, 2: F9, 3: P

	// ---- Misc / Camera / Priorities / Windows ----
	inline bool freecam_enabled = false;
	inline int  freecam_key = 0; // 0: F5, 1: V, 2: C, 3: X
	inline float freecam_speed = 18.0f;
	inline float freecam_boost = 2.5f;

	inline bool fov_changer_enabled = false;
	inline float fov_value = 95.0f;
	inline bool fov_preserve_ads = true;

	inline bool instant_loot_reveal = false;
	inline int  door_toggle_key = 0; // 0: G, 1: E, 2: F, 3: T

	inline bool player_list_open = false;
	inline int  player_list_key = 0; // 0: TAB, 1: F2, 2: F3, 3: L

	// 0: Z-Order, 1: Overlap Suppress, 2: Minimal Noise on Fire/Aim
	inline int esp_priority_mode = 0;

	// ---- Colors ----
	struct esp_colors_t {
		rgba_t team{ 0.28f, 0.92f, 0.58f, 1.0f };
		rgba_t friend_col{ 0.30f, 0.85f, 1.00f, 1.0f };
		rgba_t shield{ 0.22f, 0.83f, 0.93f, 1.0f };
		rgba_t skeleton{ 0.96f, 0.97f, 1.00f, 0.90f };
		rgba_t fov{ 0.48f, 0.44f, 0.98f, 0.80f };
		rgba_t vehicle{ 0.30f, 0.78f, 1.00f, 0.95f };
		rgba_t heli{ 1.00f, 0.62f, 0.22f, 0.95f };
		rgba_t buyer{ 0.28f, 0.92f, 0.58f, 0.95f };
	};
	inline esp_colors_t esp_col;

	// ---- Friend Whitelist ----
	inline int32_t friend_eids[ 64 ]{};
	inline int friend_count = 0;

	inline bool is_friend( int32_t eid ) {
		if ( eid <= 0 ) return false;
		for ( int i = 0; i < friend_count; ++i ) {
			if ( friend_eids[ i ] == eid ) return true;
		}
		return false;
	}

	inline void toggle_friend( int32_t eid ) {
		if ( eid <= 0 ) return;
		for ( int i = 0; i < friend_count; ++i ) {
			if ( friend_eids[ i ] == eid ) {
				friend_eids[ i ] = friend_eids[ --friend_count ];
				return;
			}
		}
		if ( friend_count < 64 ) friend_eids[ friend_count++ ] = eid;
	}

	inline int resolve_vk( int idx, const int* vks, int n ) {
		if ( idx < 0 || idx >= n ) return vks[ 0 ];
		return vks[ idx ];
	}
}
