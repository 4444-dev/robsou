#pragma once

namespace menu {
	inline bool g_open = true;
	inline float g_x = 120.0f;
	inline float g_y = 120.0f;
	inline int g_tab = 0;
	inline bool g_dragging = false;
	inline float g_drag_dx = 0.0f;
	inline float g_drag_dy = 0.0f;
	inline bool g_prev_lmb = false;
	inline bool g_prev_ins = false;
	inline int g_active_slider = -1;

	inline POINT get_mouse( ) {
		POINT pt{};
		GetCursorPos( &pt );
		if ( d3d11_ui::g_hwnd )
			ScreenToClient( d3d11_ui::g_hwnd, &pt );
		return pt;
	}

	inline bool in_rect( float mx, float my, float x, float y, float w, float h ) {
		return mx >= x && mx <= x + w && my >= y && my <= y + h;
	}

	inline void draw_toggle( float x, float y, float w, const char* label, bool& val, float mx, float my, bool clicked ) {
		const float h = 24.0f;
		bool hov = in_rect( mx, my, x, y, w, h );
		if ( hov && clicked )
			val = !val;

		d3d11_ui::add_filled_rect( x, y, w, h, hov ? d3d11_ui::color_t{ 0.11f, 0.12f, 0.17f, 0.92f } : d3d11_ui::color_t{ 0.08f, 0.09f, 0.13f, 0.86f } );
		d3d11_ui::add_text( x + 10.0f, y + h * 0.5f, label, { 0.92f, 0.94f, 0.98f, 1.0f }, 0.90f, false, true );

		const float sw = 28.0f;
		const float sh = 14.0f;
		const float sx = x + w - sw - 10.0f;
		const float sy = y + ( h - sh ) * 0.5f;
		d3d11_ui::add_filled_rect( sx, sy, sw, sh, val ? d3d11_ui::color_t{ 0.44f, 0.37f, 0.98f, 1.0f } : d3d11_ui::color_t{ 0.18f, 0.20f, 0.28f, 1.0f } );
		float knob_x = val ? ( sx + sw - 12.0f ) : ( sx + 2.0f );
		d3d11_ui::add_filled_rect( knob_x, sy + 2.0f, 10.0f, 10.0f, { 0.96f, 0.97f, 1.0f, 1.0f } );
	}

	inline void draw_slider( int id, float x, float y, float w, const char* label, float& val, float min_v, float max_v, const char* fmt, float mx, float my, bool down, bool clicked ) {
		const float h = 34.0f;
		d3d11_ui::add_filled_rect( x, y, w, h, { 0.08f, 0.09f, 0.13f, 0.86f } );

		char buf[ 64 ]{};
		wsprintfA( buf, fmt, static_cast< int >( val ) );
		d3d11_ui::add_text( x + 10.0f, y + 10.0f, label, { 0.92f, 0.94f, 0.98f, 1.0f }, 0.86f, false, true );
		float tw = d3d11_ui::measure_text_width( buf, 0.86f );
		d3d11_ui::add_text( x + w - tw - 10.0f, y + 10.0f, buf, { 0.72f, 0.75f, 0.86f, 1.0f }, 0.86f, false, true );

		const float bx = x + 10.0f;
		const float by = y + 23.0f;
		const float bw = w - 20.0f;
		const float bh = 5.0f;
		if ( clicked && in_rect( mx, my, bx - 4.0f, by - 6.0f, bw + 8.0f, bh + 12.0f ) )
			g_active_slider = id;
		if ( !down && g_active_slider == id )
			g_active_slider = -1;
		if ( g_active_slider == id && down ) {
			float t = ( mx - bx ) / bw;
			if ( t < 0.0f ) t = 0.0f;
			if ( t > 1.0f ) t = 1.0f;
			val = min_v + t * ( max_v - min_v );
		}

		float frac = ( val - min_v ) / ( max_v - min_v );
		if ( frac < 0.0f ) frac = 0.0f;
		if ( frac > 1.0f ) frac = 1.0f;
		d3d11_ui::add_filled_rect( bx, by, bw, bh, { 0.16f, 0.18f, 0.25f, 1.0f } );
		d3d11_ui::add_filled_rect( bx, by, bw * frac, bh, { 0.44f, 0.37f, 0.98f, 1.0f } );
	}

	inline void render( ) {
		static bool s_prev_end = false;
		bool end_down = ( GetAsyncKeyState( VK_END ) & 0x8000 ) != 0;
		if ( end_down && !s_prev_end )
			config::panic_active = !config::panic_active;
		s_prev_end = end_down;

		if ( config::panic_active )
			return;

		bool ins = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0 || ( GetAsyncKeyState( VK_F8 ) & 0x8000 ) != 0;
		if ( ins && !g_prev_ins )
			g_open = !g_open;
		g_prev_ins = ins;

		if ( !g_open )
			return;

		POINT pt = get_mouse( );
		float mx = static_cast< float >( pt.x );
		float my = static_cast< float >( pt.y );
		bool lmb = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
		bool clicked = lmb && !g_prev_lmb;
		g_prev_lmb = lmb;

		const float W = 500.0f;
		const float H = 406.0f;
		const float sidebar_w = 126.0f;

		if ( clicked && in_rect( mx, my, g_x, g_y, W, 34.0f ) ) {
			g_dragging = true;
			g_drag_dx = mx - g_x;
			g_drag_dy = my - g_y;
		}
		if ( !lmb )
			g_dragging = false;
		if ( g_dragging ) {
			g_x = mx - g_drag_dx;
			g_y = my - g_drag_dy;
		}

		d3d11_ui::add_filled_rect( g_x, g_y, W, H, { 0.045f, 0.048f, 0.075f, 0.96f } );
		d3d11_ui::add_rect( g_x, g_y, W, H, 1.0f, { 0.16f, 0.18f, 0.28f, 1.0f } );

		d3d11_ui::add_filled_rect( g_x, g_y, sidebar_w, H, { 0.035f, 0.038f, 0.060f, 0.98f } );
		d3d11_ui::add_text( g_x + 16.0f, g_y + 20.0f, "NEXUS APEX", { 0.95f, 0.96f, 1.0f, 1.0f }, 1.02f, false, true );

		const char* tabs[ 5 ] = { "Visuals", "Loot", "Targeting", "Skins", "Misc" };
		for ( int i = 0; i < 5; ++i ) {
			float ty = g_y + 52.0f + i * 32.0f;
			bool active = ( g_tab == i );
			if ( clicked && in_rect( mx, my, g_x + 8.0f, ty, sidebar_w - 16.0f, 26.0f ) )
				g_tab = i;
			if ( active )
				d3d11_ui::add_filled_rect( g_x + 8.0f, ty, sidebar_w - 16.0f, 26.0f, { 0.44f, 0.37f, 0.98f, 0.24f } );
			d3d11_ui::add_text( g_x + 18.0f, ty + 13.0f, tabs[ i ], active ? d3d11_ui::color_t{ 0.96f, 0.97f, 1.0f, 1.0f } : d3d11_ui::color_t{ 0.62f, 0.65f, 0.76f, 1.0f }, 0.92f, false, true );
		}
		d3d11_ui::add_text( g_x + 14.0f, g_y + H - 18.0f, "Panic Key: END", { 0.55f, 0.58f, 0.68f, 0.90f }, 0.78f, false, true );

		const float cx = g_x + sidebar_w + 14.0f;
		const float cw = W - sidebar_w - 28.0f;
		float cy = g_y + 18.0f;

		auto draw_btn = [&]( float bx, float by, float bw, float bh, const char* lbl ) -> bool {
			bool hov = in_rect( mx, my, bx, by, bw, bh );
			d3d11_ui::add_filled_rect( bx, by, bw, bh, hov ? d3d11_ui::color_t{ 0.16f, 0.17f, 0.25f, 0.96f } : d3d11_ui::color_t{ 0.10f, 0.11f, 0.16f, 0.92f } );
			d3d11_ui::add_rect( bx, by, bw, bh, 1.0f, { 0.24f, 0.26f, 0.38f, 0.9f } );
			d3d11_ui::add_text( bx + bw * 0.5f, by + bh * 0.5f, lbl, { 0.94f, 0.95f, 1.0f, 1.0f }, 0.84f, true, true );
			return hov && clicked;
		};

		if ( g_tab == 0 ) {
			const float half_w = ( cw - 8.0f ) * 0.5f;
			const float rx = cx + half_w + 8.0f;
			draw_toggle( cx, cy, half_w, "Boxes", config::esp_boxes, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Corner Box", config::esp_corner_box, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Skeleton", config::esp_skeleton, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Visible Check", config::esp_vis_check, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Names", config::esp_names, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Weapon + Icon", config::esp_weapon, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Distance", config::esp_distance, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Health Bar", config::esp_health, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Shield Bar", config::esp_shield, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "HP / SH Text", config::esp_hp_text, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Head Dot", config::esp_head_dot, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Status Tags", config::esp_status, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Snaplines", config::esp_snaplines, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "2D Radar", config::esp_radar, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "OOF Arrows", config::esp_oof_arrows, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Aim Warning", config::esp_aim_warning, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Spectators", config::esp_spectators, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Team Check", config::esp_team_check, mx, my, clicked ); cy += 30.0f;
			draw_slider( 1, cx, cy, cw, "Player Max Dist", config::esp_max_dist, 50.0f, 600.0f, "%dm", mx, my, lmb, clicked );
		}
		else if ( g_tab == 1 ) {
			const float half_w = ( cw - 8.0f ) * 0.5f;
			const float rx = cx + half_w + 8.0f;
			draw_toggle( cx, cy, half_w, "World Loot", config::esp_world, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Grenade Alert", config::esp_grenades, mx, my, clicked ); cy += 28.0f;
			draw_toggle( cx, cy, half_w, "Weapons", config::loot_weapons, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Armor / Gear", config::loot_armor, mx, my, clicked ); cy += 28.0f;
			draw_toggle( cx, cy, half_w, "Heals / Shields", config::loot_heals, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Ammo", config::loot_ammo, mx, my, clicked ); cy += 28.0f;
			draw_toggle( cx, cy, cw, "Deathboxes", config::loot_deathbox, mx, my, clicked ); cy += 32.0f;
			draw_slider( 7, cx, cy, cw, "World Max Dist", config::esp_world_dist, 20.0f, 250.0f, "%dm", mx, my, lmb, clicked );
		}
		else if ( g_tab == 2 ) {
			const float half_w = ( cw - 8.0f ) * 0.5f;
			const float rx = cx + half_w + 8.0f;
			draw_toggle( cx, cy, half_w, "Enable Targeting", config::aim_enabled, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Silent Aim (Cmd)", config::aim_silent, mx, my, clicked ); cy += 25.0f;
			draw_toggle( cx, cy, half_w, "FOV Circle", config::aim_fov_circle, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Target Snap Line", config::aim_target_line, mx, my, clicked ); cy += 25.0f;
			draw_toggle( cx, cy, half_w, "Visible Only", config::aim_vis_only, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Prediction", config::aim_predict, mx, my, clicked ); cy += 26.0f;

			const char* bone_names[ 3 ] = { "Target Bone: Head", "Target Bone: Neck", "Target Bone: Chest" };
			if ( draw_btn( cx, cy, 28.0f, 22.0f, "<" ) )
				config::aim_bone = ( config::aim_bone + 2 ) % 3;
			d3d11_ui::add_filled_rect( cx + 32.0f, cy, cw - 64.0f, 22.0f, { 0.08f, 0.09f, 0.13f, 0.86f } );
			d3d11_ui::add_text( cx + cw * 0.5f, cy + 11.0f, bone_names[ config::aim_bone % 3 ], { 0.94f, 0.95f, 1.0f, 1.0f }, 0.84f, true, true );
			if ( draw_btn( cx + cw - 28.0f, cy, 28.0f, 22.0f, ">" ) )
				config::aim_bone = ( config::aim_bone + 1 ) % 3;
			cy += 25.0f;

			const char* prio_names[ 4 ] = { "Priority: Crosshair", "Priority: Closest Dist", "Priority: Lowest HP", "Priority: Smart" };
			if ( draw_btn( cx, cy, 28.0f, 22.0f, "<" ) )
				config::aim_priority = ( config::aim_priority + 3 ) & 3;
			d3d11_ui::add_filled_rect( cx + 32.0f, cy, cw - 64.0f, 22.0f, { 0.08f, 0.09f, 0.13f, 0.86f } );
			d3d11_ui::add_text( cx + cw * 0.5f, cy + 11.0f, prio_names[ config::aim_priority & 3 ], { 0.94f, 0.95f, 1.0f, 1.0f }, 0.84f, true, true );
			if ( draw_btn( cx + cw - 28.0f, cy, 28.0f, 22.0f, ">" ) )
				config::aim_priority = ( config::aim_priority + 1 ) & 3;
			cy += 26.0f;

			draw_slider( 2, cx, cy, cw, "FOV Radius", config::aim_fov_radius, 30.0f, 350.0f, "%dpx", mx, my, lmb, clicked ); cy += 33.0f;
			draw_slider( 3, cx, cy, cw, "Smoothness", config::aim_smooth, 1.0f, 20.0f, "%d", mx, my, lmb, clicked ); cy += 33.0f;
			draw_toggle( cx, cy, half_w, "No Recoil (RCS)", config::rcs_enabled, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "No Spread", config::no_spread, mx, my, clicked ); cy += 25.0f;
			draw_slider( 4, cx, cy, cw, "RCS Strength", config::rcs_strength, 10.0f, 100.0f, "%d%%", mx, my, lmb, clicked ); cy += 33.0f;
			draw_toggle( cx, cy, cw, "Triggerbot (Input)", config::trigger_enabled, mx, my, clicked ); cy += 24.0f;
			draw_slider( 10, cx, cy, cw, "Trigger Delay", config::trigger_delay_ms, 0.0f, 150.0f, "%dms", mx, my, lmb, clicked );
		}
		else if ( g_tab == 3 ) {
			draw_toggle( cx, cy, cw, "Per-Weapon Skin Changer", config::skin_changer, mx, my, clicked ); cy += 28.0f;

			char held_buf[ 48 ]{};
			if ( config::current_held_weapon_id >= 0 )
				wsprintfA( held_buf, "Held Weapon ID: %d", config::current_held_weapon_id );
			else
				wsprintfA( held_buf, "Held Weapon: None / Melee" );
			d3d11_ui::add_filled_rect( cx, cy, cw - 96.0f, 24.0f, { 0.08f, 0.09f, 0.13f, 0.86f } );
			d3d11_ui::add_text( cx + 10.0f, cy + 12.0f, held_buf, { 0.78f, 0.84f, 0.96f, 1.0f }, 0.84f, false, true );
			if ( draw_btn( cx + cw - 90.0f, cy, 90.0f, 24.0f, "Select Held" ) && config::current_held_weapon_id >= 0 && config::current_held_weapon_id < 256 ) {
				config::selected_weapon_id = config::current_held_weapon_id;
			}
			cy += 28.0f;

			if ( draw_btn( cx, cy, 28.0f, 24.0f, "<" ) ) {
				config::selected_weapon_id = ( config::selected_weapon_id + 255 ) & 255;
			}
			char sel_buf[ 48 ]{};
			wsprintfA( sel_buf, "Edit Weapon ID: %d", config::selected_weapon_id );
			d3d11_ui::add_filled_rect( cx + 32.0f, cy, cw - 64.0f, 24.0f, { 0.08f, 0.09f, 0.13f, 0.86f } );
			d3d11_ui::add_text( cx + cw * 0.5f, cy + 12.0f, sel_buf, { 0.94f, 0.95f, 1.0f, 1.0f }, 0.86f, true, true );
			if ( draw_btn( cx + cw - 28.0f, cy, 28.0f, 24.0f, ">" ) ) {
				config::selected_weapon_id = ( config::selected_weapon_id + 1 ) & 255;
			}
			cy += 28.0f;

			int wid = config::selected_weapon_id & 255;
			draw_toggle( cx, cy, cw, "Enable Skin For This Weapon", config::weapon_skin_enabled[ wid ], mx, my, clicked ); cy += 28.0f;
			float wskin = static_cast< float >( config::weapon_skins[ wid ] );
			draw_slider( 6, cx, cy, cw, "Weapon Skin ID", wskin, 0.0f, 25.0f, "%d", mx, my, lmb, clicked );
			config::weapon_skins[ wid ] = static_cast< int >( wskin + 0.5f );
			cy += 40.0f;

			draw_toggle( cx, cy, cw, "Viewmodel / Melee Skin", config::heirloom_enabled, mx, my, clicked ); cy += 28.0f;
			draw_slider( 9, cx, cy, cw, "Melee / VM Skin ID", config::heirloom_skin, 0.0f, 20.0f, "%d", mx, my, lmb, clicked );
		}
		else {
			const float half_w = ( cw - 8.0f ) * 0.5f;
			const float rx = cx + half_w + 8.0f;
			draw_toggle( cx, cy, half_w, "Watermark", config::watermark, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Crosshair", config::crosshair, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Speed/Ammo HUD", config::hud_info, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Third Person", config::third_person, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Auto Bhop", config::bhop, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "Superglide", config::superglide, mx, my, clicked ); cy += 26.0f;
			draw_toggle( cx, cy, half_w, "Neo-Strafe", config::neo_strafe, mx, my, clicked );
			draw_toggle( rx, cy, half_w, "FOV Changer", config::fov_changer, mx, my, clicked ); cy += 28.0f;
			draw_slider( 5, cx, cy, cw, "FOV Value", config::fov_value, 70.0f, 140.0f, "%d", mx, my, lmb, clicked ); cy += 38.0f;
			draw_toggle( cx, cy, cw, "Safe Mode (Block Mem Writes)", config::safe_mode, mx, my, clicked ); cy += 30.0f;
			if ( draw_btn( cx, cy, half_w, 26.0f, "Save Config" ) )
				config::save_config( );
			if ( draw_btn( rx, cy, half_w, 26.0f, "Load Config" ) )
				config::load_config( );
			cy += 32.0f;
			if ( config::cfg_status[ 0 ] )
				d3d11_ui::add_text( cx + 4.0f, cy + 6.0f, config::cfg_status, { 0.45f, 0.92f, 0.65f, 1.0f }, 0.84f, false, true );
		}
	}
}
