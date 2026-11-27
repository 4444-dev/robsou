#pragma once

namespace config {
	inline bool esp_boxes = true;
	inline bool esp_corner_box = true;
	inline bool esp_skeleton = true;
	inline bool esp_names = true;
	inline bool esp_weapon = true;
	inline bool esp_distance = true;
	inline bool esp_health = true;
	inline bool esp_shield = true;
	inline bool esp_hp_text = true;
	inline bool esp_head_dot = true;
	inline bool esp_status = true;
	inline bool esp_snaplines = false;
	inline bool esp_spectators = true;
	inline bool esp_radar = true;
	inline bool esp_oof_arrows = true;
	inline bool esp_aim_warning = true;
	inline bool esp_world = true;
	inline bool esp_grenades = true;
	inline bool esp_vis_check = true;
	inline float esp_world_dist = 80.0f;
	inline bool loot_weapons = true;
	inline bool loot_armor = true;
	inline bool loot_heals = true;
	inline bool loot_ammo = false;
	inline bool loot_deathbox = true;
	inline bool esp_team_check = true;
	inline float esp_max_dist = 300.0f;

	inline bool aim_enabled = false;
	inline bool aim_silent = false;
	inline bool aim_vis_only = false;
	inline bool aim_predict = true;
	inline int aim_bone = 0;
	inline int aim_priority = 0;
	inline bool aim_fov_circle = false;
	inline bool aim_target_line = true;
	inline float aim_fov_radius = 110.0f;
	inline float aim_smooth = 4.0f;
	inline bool rcs_enabled = false;
	inline bool no_spread = false;
	inline float rcs_strength = 80.0f;
	inline bool trigger_enabled = false;
	inline float trigger_delay_ms = 35.0f;

	inline bool panic_active = false;
	inline char cfg_status[ 32 ] = "";

	inline bool watermark = true;
	inline bool crosshair = false;
	inline bool hud_info = true;
	inline bool fov_changer = false;
	inline float fov_value = 110.0f;
	inline bool third_person = false;
	inline bool bhop = false;
	inline bool superglide = false;
	inline bool neo_strafe = false;
	inline bool safe_mode = true;
	inline bool skin_changer = false;
	inline float skin_id = 1.0f;
	inline int weapon_skins[ 256 ]{};
	inline bool weapon_skin_enabled[ 256 ]{};
	inline int selected_weapon_id = 0;
	inline int current_held_weapon_id = -1;

	inline bool heirloom_enabled = false;
	inline float heirloom_model_idx = 0.0f;
	inline float heirloom_skin = 0.0f;
	inline int current_vm_model_idx = 0;

	struct disk_cfg_t {
		std::uint32_t magic;
		bool esp_boxes, esp_corner_box, esp_skeleton, esp_names, esp_weapon, esp_distance;
		bool esp_health, esp_shield, esp_hp_text, esp_head_dot, esp_status, esp_snaplines;
		bool esp_spectators, esp_radar, esp_oof_arrows, esp_aim_warning, esp_world, esp_grenades, esp_vis_check;
		bool loot_weapons, loot_armor, loot_heals, loot_ammo, loot_deathbox, esp_team_check;
		float esp_world_dist, esp_max_dist;
		bool aim_enabled, aim_silent, aim_vis_only, aim_predict, aim_fov_circle, aim_target_line;
		int aim_bone, aim_priority;
		float aim_fov_radius, aim_smooth, rcs_strength, trigger_delay_ms;
		bool rcs_enabled, no_spread, trigger_enabled;
		bool watermark, crosshair, hud_info, fov_changer, third_person, bhop, skin_changer, heirloom_enabled;
		bool superglide, neo_strafe, safe_mode;
		int selected_weapon_id;
		float fov_value, heirloom_skin, skin_id;
		int weapon_skins[ 256 ];
		bool weapon_skin_enabled[ 256 ];
	};

	inline bool save_config( ) {
		disk_cfg_t c{};
		c.magic = 0x4E584151;
		c.esp_boxes = esp_boxes; c.esp_corner_box = esp_corner_box; c.esp_skeleton = esp_skeleton;
		c.esp_names = esp_names; c.esp_weapon = esp_weapon; c.esp_distance = esp_distance;
		c.esp_health = esp_health; c.esp_shield = esp_shield; c.esp_hp_text = esp_hp_text;
		c.esp_head_dot = esp_head_dot; c.esp_status = esp_status; c.esp_snaplines = esp_snaplines;
		c.esp_spectators = esp_spectators; c.esp_radar = esp_radar; c.esp_oof_arrows = esp_oof_arrows;
		c.esp_aim_warning = esp_aim_warning; c.esp_world = esp_world; c.esp_grenades = esp_grenades;
		c.esp_vis_check = esp_vis_check;
		c.loot_weapons = loot_weapons; c.loot_armor = loot_armor; c.loot_heals = loot_heals;
		c.loot_ammo = loot_ammo; c.loot_deathbox = loot_deathbox; c.esp_team_check = esp_team_check;
		c.esp_world_dist = esp_world_dist; c.esp_max_dist = esp_max_dist;
		c.aim_enabled = aim_enabled; c.aim_silent = aim_silent; c.aim_vis_only = aim_vis_only;
		c.aim_predict = aim_predict; c.aim_fov_circle = aim_fov_circle; c.aim_target_line = aim_target_line;
		c.aim_bone = aim_bone; c.aim_priority = aim_priority; c.aim_fov_radius = aim_fov_radius; c.aim_smooth = aim_smooth;
		c.rcs_strength = rcs_strength; c.trigger_delay_ms = trigger_delay_ms;
		c.rcs_enabled = rcs_enabled; c.no_spread = no_spread; c.trigger_enabled = trigger_enabled;
		c.watermark = watermark; c.crosshair = crosshair; c.hud_info = hud_info;
		c.fov_changer = fov_changer; c.third_person = third_person; c.bhop = bhop;
		c.skin_changer = skin_changer; c.heirloom_enabled = heirloom_enabled;
		c.superglide = superglide; c.neo_strafe = neo_strafe; c.safe_mode = safe_mode;
		c.selected_weapon_id = selected_weapon_id;
		c.fov_value = fov_value; c.heirloom_skin = heirloom_skin; c.skin_id = skin_id;
		for ( int i = 0; i < 256; ++i ) {
			c.weapon_skins[ i ] = weapon_skins[ i ];
			c.weapon_skin_enabled[ i ] = weapon_skin_enabled[ i ];
		}
		HANDLE h = CreateFileA( "C:\\Users\\Public\\nexus_apex.cfg", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE ) {
			wsprintfA( cfg_status, "Save failed" );
			return false;
		}
		DWORD written = 0;
		WriteFile( h, &c, sizeof( c ), &written, nullptr );
		CloseHandle( h );
		wsprintfA( cfg_status, "Saved!" );
		return written == sizeof( c );
	}

	inline bool load_config( ) {
		HANDLE h = CreateFileA( "C:\\Users\\Public\\nexus_apex.cfg", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
		if ( h == INVALID_HANDLE_VALUE ) {
			wsprintfA( cfg_status, "No config" );
			return false;
		}
		disk_cfg_t c{};
		DWORD read_bytes = 0;
		BOOL ok = ReadFile( h, &c, sizeof( c ), &read_bytes, nullptr );
		CloseHandle( h );
		if ( !ok || read_bytes != sizeof( c ) || c.magic != 0x4E584151 ) {
			wsprintfA( cfg_status, "Invalid cfg" );
			return false;
		}
		esp_boxes = c.esp_boxes; esp_corner_box = c.esp_corner_box; esp_skeleton = c.esp_skeleton;
		esp_names = c.esp_names; esp_weapon = c.esp_weapon; esp_distance = c.esp_distance;
		esp_health = c.esp_health; esp_shield = c.esp_shield; esp_hp_text = c.esp_hp_text;
		esp_head_dot = c.esp_head_dot; esp_status = c.esp_status; esp_snaplines = c.esp_snaplines;
		esp_spectators = c.esp_spectators; esp_radar = c.esp_radar; esp_oof_arrows = c.esp_oof_arrows;
		esp_aim_warning = c.esp_aim_warning; esp_world = c.esp_world; esp_grenades = c.esp_grenades;
		esp_vis_check = c.esp_vis_check;
		loot_weapons = c.loot_weapons; loot_armor = c.loot_armor; loot_heals = c.loot_heals;
		loot_ammo = c.loot_ammo; loot_deathbox = c.loot_deathbox; esp_team_check = c.esp_team_check;
		esp_world_dist = c.esp_world_dist; esp_max_dist = c.esp_max_dist;
		aim_enabled = c.aim_enabled; aim_silent = c.aim_silent; aim_vis_only = c.aim_vis_only;
		aim_predict = c.aim_predict; aim_fov_circle = c.aim_fov_circle; aim_target_line = c.aim_target_line;
		aim_bone = c.aim_bone; aim_priority = c.aim_priority; aim_fov_radius = c.aim_fov_radius; aim_smooth = c.aim_smooth;
		rcs_strength = c.rcs_strength; trigger_delay_ms = c.trigger_delay_ms;
		rcs_enabled = c.rcs_enabled; no_spread = c.no_spread; trigger_enabled = c.trigger_enabled;
		watermark = c.watermark; crosshair = c.crosshair; hud_info = c.hud_info;
		fov_changer = c.fov_changer; third_person = c.third_person; bhop = c.bhop;
		skin_changer = c.skin_changer; heirloom_enabled = c.heirloom_enabled;
		superglide = c.superglide; neo_strafe = c.neo_strafe; safe_mode = c.safe_mode;
		selected_weapon_id = c.selected_weapon_id;
		fov_value = c.fov_value; heirloom_skin = c.heirloom_skin; skin_id = c.skin_id;
		for ( int i = 0; i < 256; ++i ) {
			weapon_skins[ i ] = c.weapon_skins[ i ];
			weapon_skin_enabled[ i ] = c.weapon_skin_enabled[ i ];
		}
		wsprintfA( cfg_status, "Loaded!" );
		return true;
	}
}
