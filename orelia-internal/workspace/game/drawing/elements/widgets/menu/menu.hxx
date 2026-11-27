// orelia-internal/workspace/game/drawing/elements/widgets/menu/menu.hxx
#pragma once
#include <workspace/game/drawing/elements/widgets/widgets.h>
#include <workspace/game/features/features.h>
#include <workspace/game/features/esp/esp.hxx>
#include <workspace/game/unity_il2cpp.hxx>

namespace menu {
	inline float g_win_x = 120.0f;
	inline float g_win_y = 85.0f;
	inline float g_win_w = 580.0f;
	inline float g_win_h = 480.0f;
	inline bool g_dragging = false;
	inline float g_drag_ox = 0.0f;
	inline float g_drag_oy = 0.0f;
	inline float g_scroll_y[ 4 ]{};
	inline float g_content_h[ 4 ]{};
	inline widgets::ui_input_t g_in{};

	inline bool is_menu_toggle_key_pressed( ) {
		static constexpr int k_vks[] = { VK_INSERT, VK_DELETE, VK_HOME, VK_END, VK_F4 };
		const int vk = config::resolve_vk( config::menu_key_mode, k_vks, 5 );
		return ( GetAsyncKeyState( vk ) & 1 ) != 0 || ( config::menu_key_mode == 0 && ( GetAsyncKeyState( VK_F8 ) & 1 ) != 0 );
	}

	inline void update_input( ) {
		// The game captures and clips the cursor while playing: without releasing
		// the clip the OS cursor stays pinned to the centre and no widget can ever
		// be hovered, which makes the menu unusable in game.
		static bool s_cursor_released = false;
		if ( config::menu_open ) {
			ClipCursor( nullptr );
			if ( !s_cursor_released ) {
				ShowCursor( TRUE );
				s_cursor_released = true;
			}
		} else if ( s_cursor_released ) {
			ShowCursor( FALSE );
			s_cursor_released = false;
		}

		POINT pt{};
		GetCursorPos( &pt );
		if ( d3d11_ui::g_hwnd ) ScreenToClient( d3d11_ui::g_hwnd, &pt );
		g_in.mx = static_cast< float >( pt.x );
		g_in.my = static_cast< float >( pt.y );
		const bool down = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
		g_in.lmb_click = down && !g_in.lmb_down;
		g_in.lmb_down = down;
	}

	inline void render_plot_panel( ) {
		if ( !config::plot_panel_enabled || !config::model_chain_ok || !esp::g_camera ) return;
		if ( !esp::g_nearest_plot.valid ) return;
		const float px = d3d11_ui::g_width - 270.0f;
		const float py = 18.0f;
		const float pw = 252.0f;
		const float ph = 128.0f;
		d3d11_ui::add_shadow_rect( px, py, pw, ph, 6.0f, 0.25f );
		d3d11_ui::add_filled_rounded_rect( px, py, pw, ph, 6.0f, { 0.055f, 0.063f, 0.094f, 0.90f } );
		d3d11_ui::add_rounded_rect( px, py, pw, ph, 6.0f, 1.0f, { 0.20f, 0.24f, 0.36f, 0.95f } );
		d3d11_ui::add_text_outlined( px + 10.0f, py + 10.0f, "NEAREST PLOT", { 0.48f, 0.44f, 0.98f, 1.0f }, 0.85f );

		const auto& pl = esp::g_nearest_plot;
		char buf[ 64 ]{};
		wsprintfA( buf, "Dist: %dm", static_cast< int >( pl.distance ) );
		d3d11_ui::add_text_outlined( px + 10.0f, py + 32.0f, buf, { 0.90f, 0.92f, 0.98f, 1.0f }, 0.76f );
		wsprintfA( buf, "Storages: %d | Full Cont: %d", pl.storages, pl.full_containers );
		d3d11_ui::add_text_outlined( px + 10.0f, py + 50.0f, buf, { 0.90f, 0.92f, 0.98f, 1.0f }, 0.76f );
		wsprintfA( buf, "Loot Nodes Nearby: %d", pl.bushes_by_rarity[ 2 ] );
		d3d11_ui::add_text_outlined( px + 10.0f, py + 68.0f, buf, { 0.98f, 0.78f, 0.25f, 1.0f }, 0.76f );
		wsprintfA( buf, "Parked Vehicles: %d", pl.parked_vehicles );
		d3d11_ui::add_text_outlined( px + 10.0f, py + 86.0f, buf, { 0.30f, 0.78f, 1.00f, 1.0f }, 0.76f );
	}

	inline void render_player_list( ) {
		if ( !config::player_list_open || !config::model_chain_ok || !esp::g_camera ) return;
		const float wx = d3d11_ui::g_width - 310.0f;
		const float wy = 212.0f;
		const float ww = 292.0f;
		const float wh = 310.0f;
		d3d11_ui::add_shadow_rect( wx, wy, ww, wh, 6.0f, 0.25f );
		d3d11_ui::add_filled_rounded_rect( wx, wy, ww, wh, 6.0f, { 0.055f, 0.063f, 0.094f, 0.92f } );
		d3d11_ui::add_rounded_rect( wx, wy, ww, wh, 6.0f, 1.0f, { 0.20f, 0.24f, 0.36f, 0.95f } );
		d3d11_ui::add_text_outlined( wx + 10.0f, wy + 10.0f, "PLAYER ROSTER (Click = Friend)", { 0.48f, 0.44f, 0.98f, 1.0f }, 0.82f );

		const LONG active = InterlockedCompareExchange( &esp::g_active_buf, 0, 0 );
		const int count = esp::g_player_count_buf[ active ];
		const esp::player_t* players = esp::g_players_buf[ active ];

		float ry = wy + 34.0f;
		for ( int i = 0; i < count && ry + 18.0f < wy + wh - 8.0f; ++i ) {
			const auto& p = players[ i ];
			if ( !p.valid ) continue;
			const bool hov = g_in.hovered( wx + 8.0f, ry, ww - 16.0f, 18.0f );
			if ( hov && g_in.lmb_click ) {
				config::toggle_friend( p.entity_id );
			}
			const bool fr = config::is_friend( p.entity_id );
			if ( hov || fr ) {
				d3d11_ui::add_filled_rounded_rect( wx + 8.0f, ry, ww - 16.0f, 17.0f, 3.0f,
					fr ? d3d11_ui::color_t{ 0.16f, 0.45f, 0.62f, 0.45f } : d3d11_ui::color_t{ 0.20f, 0.22f, 0.32f, 0.45f } );
			}
			char row[ 64 ]{};
			wsprintfA( row, "%s[%s] %s (%dm)", fr ? "* " : "", p.tag[ 0 ] ? p.tag : "PLY",
				p.name[ 0 ] ? p.name : "Player", static_cast< int >( p.distance ) );
			d3d11_ui::add_text_outlined( wx + 12.0f, ry + 2.0f, row,
				fr ? d3d11_ui::color_t{ 0.30f, 0.88f, 1.0f, 1.0f } : d3d11_ui::color_t{ 0.90f, 0.92f, 0.98f, 1.0f }, 0.74f );
			ry += 19.0f;
		}
	}

	inline void tick( ) {
		update_input( );
		if ( is_menu_toggle_key_pressed( ) ) {
			config::menu_open = !config::menu_open;
		}

		render_plot_panel( );
		render_player_list( );

		if ( !config::menu_open ) return;

		const auto pal = widgets::get_palette( );
		const float hdr_h = 36.0f;
		if ( g_in.lmb_click && g_in.hovered( g_win_x, g_win_y, g_win_w, hdr_h ) ) {
			g_dragging = true;
			g_drag_ox = g_in.mx - g_win_x;
			g_drag_oy = g_in.my - g_win_y;
		}
		if ( !g_in.lmb_down ) g_dragging = false;
		if ( g_dragging ) {
			g_win_x = d3d11_ui::clampf( g_in.mx - g_drag_ox, 0.0f, d3d11_ui::g_width - g_win_w );
			g_win_y = d3d11_ui::clampf( g_in.my - g_drag_oy, 0.0f, d3d11_ui::g_height - g_win_h );
		}

		d3d11_ui::add_shadow_rect( g_win_x, g_win_y, g_win_w, g_win_h, 8.0f, 0.30f );
		d3d11_ui::add_filled_rounded_rect( g_win_x, g_win_y, g_win_w, g_win_h, 8.0f, pal.bg );
		d3d11_ui::add_rounded_rect( g_win_x, g_win_y, g_win_w, g_win_h, 8.0f, 1.0f, pal.border );
		d3d11_ui::add_text_outlined( g_win_x + 14.0f, g_win_y + 10.0f, "ORELIA INTERNAL // ROB RUN", pal.text_hi, 0.92f );

		const char* tabs[ 4 ] = { "Combat", "Visuals", "Radar & World", "Misc & Plots" };
		const float tab_w = ( g_win_w - 28.0f ) / 4.0f;
		for ( int t = 0; t < 4; ++t ) {
			const float tx = g_win_x + 14.0f + t * tab_w;
			const float ty = g_win_y + 38.0f;
			const bool active = ( config::menu_tab == t );
			if ( g_in.hovered( tx, ty, tab_w - 4.0f, 24.0f ) && g_in.lmb_click ) {
				config::menu_tab = t;
			}
			d3d11_ui::add_filled_rounded_rect( tx, ty, tab_w - 4.0f, 24.0f, 4.0f, active ? pal.accent : pal.ctrl_bg );
			d3d11_ui::add_text( tx + ( tab_w - 4.0f ) * 0.5f, ty + 12.0f, tabs[ t ], pal.text_hi, 0.82f, true, true );
		}

		const float body_x = g_win_x + 14.0f;
		const float body_y = g_win_y + 70.0f;
		const float body_w = g_win_w - 28.0f;
		const float body_h = g_win_h - 82.0f;

		int tab = config::menu_tab;
		if ( g_in.hovered( body_x, body_y, body_w, body_h ) ) {
			if ( GetAsyncKeyState( VK_NEXT ) & 1 ) g_scroll_y[ tab ] += 32.0f;
			if ( GetAsyncKeyState( VK_PRIOR ) & 1 ) g_scroll_y[ tab ] -= 32.0f;
		}
		const float max_scr = g_content_h[ tab ] > body_h ? ( g_content_h[ tab ] - body_h ) : 0.0f;
		g_scroll_y[ tab ] = d3d11_ui::clampf( g_scroll_y[ tab ], 0.0f, max_scr );

		float y = body_y - g_scroll_y[ tab ];
		const float cw = ( body_w - 12.0f ) * 0.5f;
		const float cx1 = body_x;
		const float cx2 = body_x + cw + 12.0f;
		float y1 = y, y2 = y;
		int sid = 100 + tab * 100;

		auto draw_chk = [&]( float x, float& cy, float w, const char* lbl, bool& val ) {
			if ( cy >= body_y - 4.0f && cy + 24.0f <= body_y + body_h + 4.0f ) {
				widgets::draw_toggle( x, cy, w, lbl, val, g_in, pal );
			}
			cy += 25.0f;
		};
		auto draw_sld = [&]( float x, float& cy, float w, const char* lbl, float& val, float mn, float mx, const char* fmt ) {
			++sid;
			if ( cy >= body_y - 4.0f && cy + 34.0f <= body_y + body_h + 4.0f ) {
				widgets::draw_slider( sid, x, cy, w, lbl, val, mn, mx, fmt, g_in, pal );
			}
			cy += 35.0f;
		};
		auto draw_cyc = [&]( float x, float& cy, float w, const char* lbl, int& idx, const char* const* opts, int n ) {
			if ( cy >= body_y - 4.0f && cy + 26.0f <= body_y + body_h + 4.0f ) {
				widgets::draw_cycle( x, cy, w, lbl, idx, opts, n, g_in, pal );
			}
			cy += 28.0f;
		};

		if ( tab == 0 ) {
			static constexpr const char* k_modes[] = { "Mouse Assist", "Direct 3D (6B)", "Silent Aim" };
			static constexpr const char* k_keys[] = { "RMB", "LMB", "Shift", "X2", "Alt" };
			static constexpr const char* k_bones[] = { "Head", "Neck", "Chest", "Pelvis", "Stomach", "Arms", "Legs", "Face", "Hands", "Feet", "Closest" };

			draw_chk( cx1, y1, cw, "Aim Master", config::aim_master );
			draw_cyc( cx1, y1, cw, "Aim Mode", config::aim_mode, k_modes, 3 );
			draw_cyc( cx1, y1, cw, "Aim Key", config::aim_key_mode, k_keys, 5 );
			draw_cyc( cx1, y1, cw, "Bone / Region", config::aim_bone_mode, k_bones, 11 );
			draw_sld( cx1, y1, cw, "FOV", config::aim_fov, 15.0f, 400.0f, "%d px" );
			draw_sld( cx1, y1, cw, "Smoothing", config::aim_smooth, 1.0f, 20.0f, "%d" );
			draw_sld( cx1, y1, cw, "Max Distance", config::aim_max_dist, 30.0f, 600.0f, "%d m" );
			draw_sld( cx1, y1, cw, "Silent FOV", config::aim_silent_fov, 10.0f, 300.0f, "%d px" );
			draw_sld( cx1, y1, cw, "Hit Chance", config::aim_hit_chance, 10.0f, 100.0f, "%d%%" );
			draw_chk( cx1, y1, cw, "Silent Indicator", config::aim_silent_indicator );

			draw_chk( cx2, y2, cw, "Triggerbot", config::trig_enabled );
			draw_sld( cx2, y2, cw, "Trig Delay", config::trig_delay_ms, 0.0f, 250.0f, "%d ms" );
			draw_chk( cx2, y2, cw, "Trig Semi-Auto", config::trig_semi_auto );
			draw_chk( cx2, y2, cw, "2D RCS", config::rcs_enabled );
			draw_chk( cx2, y2, cw, "No Recoil", config::no_recoil );
			draw_chk( cx2, y2, cw, "No Spread", config::no_spread );
			draw_chk( cx2, y2, cw, "No Cam/Optics Shake", config::no_cam_shake );
			draw_chk( cx2, y2, cw, "Motion Prediction", config::pred_enabled );
			draw_chk( cx2, y2, cw, "Multipoint Hitboxes", config::aim_multipoint );
			draw_chk( cx2, y2, cw, "Visible Only (LoS)", config::aim_vis_check );
			draw_chk( cx2, y2, cw, "Weapon-Aware Aim", config::aim_weapon_aware );
			draw_chk( cx2, y2, cw, "Ignore Clan", config::aim_ignore_clan );
			draw_chk( cx2, y2, cw, "Ignore Police", config::aim_ignore_police );
			draw_chk( cx2, y2, cw, "Ignore Knocked", config::aim_ignore_knocked );
			draw_chk( cx2, y2, cw, "Ignore Friends", config::aim_ignore_friends );
			draw_chk( cx2, y2, cw, "Closest First", config::aim_closest_first );
			draw_chk( cx2, y2, cw, "Prefer Head", config::aim_prefer_head );
		} else if ( tab == 1 ) {
			draw_chk( cx1, y1, cw, "ESP Master", config::esp_master );
			draw_chk( cx1, y1, cw, "Profile: Police", config::esp_prof[ 0 ].enabled );
			draw_sld( cx1, y1, cw, "Police Dist", config::esp_prof[ 0 ].max_dist, 50.0f, 800.0f, "%d m" );
			draw_chk( cx1, y1, cw, "Profile: Robbers", config::esp_prof[ 1 ].enabled );
			draw_sld( cx1, y1, cw, "Robber Dist", config::esp_prof[ 1 ].max_dist, 50.0f, 800.0f, "%d m" );
			draw_chk( cx1, y1, cw, "Profile: Civilians", config::esp_prof[ 2 ].enabled );
			draw_sld( cx1, y1, cw, "Civilian Dist", config::esp_prof[ 2 ].max_dist, 50.0f, 800.0f, "%d m" );
			draw_chk( cx1, y1, cw, "Police Coworkers", config::esp_police_coworkers );
			draw_chk( cx1, y1, cw, "2D Box", config::esp_box );
			draw_chk( cx1, y1, cw, "Box Fill", config::esp_box_fill );
			draw_chk( cx1, y1, cw, "Skeleton", config::esp_skeleton );

			draw_chk( cx2, y2, cw, "3D Hitboxes", config::esp_hitboxes );
			draw_chk( cx2, y2, cw, "Off-screen Arrows", config::esp_oof_arrows );
			draw_sld( cx2, y2, cw, "Arrow Size", config::esp_oof_size, 5.0f, 18.0f, "%d px" );
			draw_chk( cx2, y2, cw, "Snaplines", config::esp_snaplines );
			draw_sld( cx2, y2, cw, "Snap Center Radius", config::esp_snap_radius, 0.0f, 200.0f, "%d px" );
			draw_chk( cx2, y2, cw, "Corpse ESP", config::esp_corpse );
			draw_chk( cx2, y2, cw, "Death Point", config::esp_death_point );
			draw_chk( cx2, y2, cw, "Nickname & Role", config::esp_name );
			draw_chk( cx2, y2, cw, "Clan Tag", config::esp_clan );
			draw_chk( cx2, y2, cw, "Distance", config::esp_distance );
			draw_chk( cx2, y2, cw, "Weapon", config::esp_weapon );
			draw_chk( cx2, y2, cw, "Armor / Crouch / Knock", config::esp_knocked );
			draw_chk( cx2, y2, cw, "8-Way Font Outline", config::esp_font_8way );
		} else if ( tab == 2 ) {
			draw_chk( cx1, y1, cw, "2D Radar", config::esp_radar );
			draw_sld( cx1, y1, cw, "Radar Range", config::radar_range, 40.0f, 400.0f, "%d m" );
			draw_sld( cx1, y1, cw, "Radar Size", config::radar_size, 110.0f, 250.0f, "%d px" );
			draw_chk( cx1, y1, cw, "World Markers on Radar", config::radar_world_markers );
			draw_chk( cx1, y1, cw, "World Master", config::world_master );
			draw_chk( cx1, y1, cw, "Loose Items", config::world_loose );
			draw_chk( cx1, y1, cw, "Loot Nodes", config::world_nodes );
			draw_chk( cx1, y1, cw, "Containers", config::world_containers );
			draw_chk( cx1, y1, cw, "Hide Empty Containers", config::world_hide_empty );

			draw_chk( cx2, y2, cw, "Dropped Loot", config::world_dropped );
			draw_sld( cx2, y2, cw, "Loot Render Dist", config::world_loot_dist, 40.0f, 500.0f, "%d m" );
			draw_chk( cx2, y2, cw, "Cars ESP", config::world_cars );
			draw_chk( cx2, y2, cw, "Helicopters ESP", config::world_helis );
			draw_chk( cx2, y2, cw, "Hide Local Vehicle", config::world_hide_local_veh );
			draw_sld( cx2, y2, cw, "Vehicle Render Dist", config::world_veh_dist, 50.0f, 800.0f, "%d m" );
		} else {
			static constexpr const char* k_fc_keys[] = { "F5", "V", "C", "X" };
			static constexpr const char* k_dr_keys[] = { "G", "E", "F", "T" };
			static constexpr const char* k_pl_keys[] = { "F6", "F7", "F9", "P" };
			static constexpr const char* k_ls_keys[] = { "TAB", "F2", "F3", "L" };
			static constexpr const char* k_prios[] = { "Z-Order", "Overlap Suppress", "Min Noise Fire/Aim" };

			draw_chk( cx1, y1, cw, "Free Cam", config::freecam_enabled );
			draw_cyc( cx1, y1, cw, "Free Cam Key", config::freecam_key, k_fc_keys, 4 );
			draw_sld( cx1, y1, cw, "Free Cam Speed", config::freecam_speed, 5.0f, 60.0f, "%d" );
			draw_chk( cx1, y1, cw, "FOV Changer", config::fov_changer_enabled );
			draw_sld( cx1, y1, cw, "Custom FOV", config::fov_value, 60.0f, 130.0f, "%d" );
			draw_chk( cx1, y1, cw, "Preserve ADS Zoom", config::fov_preserve_ads );
			draw_chk( cx1, y1, cw, "Skeleton Diag Panel", config::show_diag );

			draw_chk( cx2, y2, cw, "Instant Loot Reveal", config::instant_loot_reveal );
			draw_cyc( cx2, y2, cw, "Door Toggle Key (6B)", config::door_toggle_key, k_dr_keys, 4 );
			draw_chk( cx2, y2, cw, "Nearest Plot Panel", config::plot_panel_enabled );
			draw_cyc( cx2, y2, cw, "Plot Panel Key", config::plot_panel_key, k_pl_keys, 4 );
			draw_chk( cx2, y2, cw, "Player Roster Window", config::player_list_open );
			draw_cyc( cx2, y2, cw, "Player List Key", config::player_list_key, k_ls_keys, 4 );
			draw_cyc( cx2, y2, cw, "ESP Priority Mode", config::esp_priority_mode, k_prios, 3 );
		}

		const float max_y = ( y1 > y2 ? y1 : y2 ) + g_scroll_y[ tab ];
		g_content_h[ tab ] = max_y - body_y + 12.0f;
	}
}
