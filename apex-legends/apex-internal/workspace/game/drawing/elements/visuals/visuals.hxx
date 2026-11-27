#pragma once

namespace visuals {
	inline void draw_watermark( int player_count ) {
		if ( !config::watermark )
			return;

		char buf[ 96 ]{};
		wsprintfA( buf, "Nexus Apex | %d FPS | %d Players", d3d11_ui::g_fps, player_count );
		float tw = d3d11_ui::measure_text_width( buf, 0.88f );
		const float w = tw + 22.0f;
		const float h = 22.0f;
		const float x = ( d3d11_ui::g_width - w ) * 0.5f;
		const float y = 14.0f;

		d3d11_ui::add_filled_rect( x, y, w, h, { 0.045f, 0.048f, 0.075f, 0.82f } );
		d3d11_ui::add_rect( x, y, w, h, 1.0f, { 0.14f, 0.16f, 0.24f, 0.90f } );
		d3d11_ui::add_text( x + w * 0.5f, y + h * 0.5f, buf, { 0.88f, 0.90f, 0.96f, 1.0f }, 0.88f, true, true );
	}

	inline void draw_crosshair( ) {
		if ( !config::crosshair )
			return;

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;
		const float len = 6.0f;
		const d3d11_ui::color_t col{ 0.95f, 0.95f, 0.98f, 0.95f };

		d3d11_ui::add_line( cx - len, cy, cx + len, cy, 1.5f, col );
		d3d11_ui::add_line( cx, cy - len, cx, cy + len, 1.5f, col );
	}

	inline void draw_aim_fov( ) {
		if ( !config::aim_fov_circle )
			return;

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;
		d3d11_ui::add_circle( cx, cy, config::aim_fov_radius, 64, 1.2f, { 0.44f, 0.37f, 0.98f, 0.80f } );
	}

	inline void draw_corner_box( float x, float y, float w, float h, const d3d11_ui::color_t& col ) {
		float cw = w * 0.25f;
		float ch = h * 0.20f;
		if ( cw < 3.0f ) cw = 3.0f;
		if ( ch < 3.0f ) ch = 3.0f;
		const d3d11_ui::color_t bg{ 0.0f, 0.0f, 0.0f, 0.80f };
		auto seg = [&]( float x1, float y1, float x2, float y2 ) {
			d3d11_ui::add_line( x1, y1, x2, y2, 3.0f, bg );
			d3d11_ui::add_line( x1, y1, x2, y2, 1.2f, col );
		};
		seg( x, y, x + cw, y );             seg( x, y, x, y + ch );
		seg( x + w, y, x + w - cw, y );     seg( x + w, y, x + w, y + ch );
		seg( x, y + h, x + cw, y + h );     seg( x, y + h, x, y + h - ch );
		seg( x + w, y + h, x + w - cw, y + h ); seg( x + w, y + h, x + w, y + h - ch );
	}

	inline d3d11_ui::color_t get_shield_color( int armor_type, int max_sh ) {
		if ( armor_type == 5 || max_sh >= 125 ) return { 0.98f, 0.24f, 0.26f, 1.0f }; // Red
		if ( armor_type == 4 )                  return { 0.98f, 0.80f, 0.20f, 1.0f }; // Gold
		if ( armor_type == 3 || max_sh >= 100 ) return { 0.72f, 0.34f, 0.98f, 1.0f }; // Purple
		if ( armor_type == 2 || max_sh >= 75 )  return { 0.25f, 0.62f, 1.00f, 1.0f }; // Blue
		return { 0.92f, 0.94f, 0.98f, 1.0f };                                          // White
	}

	inline void draw_spectators( int spec_count, const char names[ 16 ][ 32 ] ) {
		if ( !config::esp_spectators || spec_count <= 0 )
			return;
		char buf[ 32 ]{};
		wsprintfA( buf, "SPEC: %d", spec_count );
		float tw = d3d11_ui::measure_text_width( buf, 0.88f );
		const float w = tw + 20.0f;
		const float h = 20.0f;
		const float x = ( d3d11_ui::g_width - w ) * 0.5f;
		const float y = 40.0f;
		d3d11_ui::add_filled_rect( x, y, w, h, { 0.16f, 0.04f, 0.05f, 0.86f } );
		d3d11_ui::add_rect( x, y, w, h, 1.0f, { 0.98f, 0.28f, 0.30f, 0.95f } );
		d3d11_ui::add_text( x + w * 0.5f, y + h * 0.5f, buf, { 0.98f, 0.45f, 0.45f, 1.0f }, 0.88f, true, true );
		int show = spec_count > 6 ? 6 : spec_count;
		for ( int s = 0; s < show; ++s ) {
			if ( names[ s ][ 0 ] ) {
				d3d11_ui::add_text_outlined( d3d11_ui::g_width * 0.5f, y + 23.0f + s * 13.0f, names[ s ], { 0.98f, 0.72f, 0.72f, 0.95f }, 0.76f, true, false );
			}
		}
	}

	inline void draw_weapon_icon( float x, float y, int cat, const d3d11_ui::color_t& col ) {
		if ( cat == 6 ) { // Melee / Knife
			d3d11_ui::add_line( x, y + 6.0f, x + 6.0f, y + 3.0f, 1.5f, col );
			d3d11_ui::add_line( x + 6.0f, y + 3.0f, x + 13.0f, y + 1.0f, 1.2f, col );
		}
		else if ( cat == 5 ) { // Pistol
			d3d11_ui::add_filled_rect( x + 2.0f, y + 1.0f, 9.0f, 2.5f, col );
			d3d11_ui::add_filled_rect( x + 2.0f, y + 3.5f, 2.5f, 4.0f, col );
		}
		else if ( cat == 2 ) { // Shotgun
			d3d11_ui::add_filled_rect( x, y + 2.0f, 14.0f, 2.0f, col );
			d3d11_ui::add_filled_rect( x + 4.0f, y + 4.0f, 6.0f, 1.5f, col );
			d3d11_ui::add_line( x, y + 3.0f, x - 3.0f, y + 6.0f, 1.5f, col );
		}
		else if ( cat == 3 ) { // Sniper
			d3d11_ui::add_filled_rect( x + 4.0f, y, 5.0f, 1.5f, col );
			d3d11_ui::add_filled_rect( x, y + 2.0f, 16.0f, 2.0f, col );
			d3d11_ui::add_filled_rect( x + 5.0f, y + 4.0f, 2.5f, 3.0f, col );
		}
		else if ( cat == 1 ) { // SMG
			d3d11_ui::add_filled_rect( x + 1.0f, y + 1.5f, 10.0f, 2.5f, col );
			d3d11_ui::add_filled_rect( x + 5.0f, y + 4.0f, 2.0f, 3.5f, col );
		}
		else { // Rifle / LMG
			d3d11_ui::add_filled_rect( x, y + 2.0f, 13.0f, 2.2f, col );
			d3d11_ui::add_filled_rect( x + 6.0f, y + 4.2f, 2.5f, 3.0f, col );
			d3d11_ui::add_line( x, y + 3.0f, x - 2.5f, y + 5.5f, 1.5f, col );
		}
	}

	inline void get_weapon_info( int id, char* out, int& cat ) {
		cat = 0;
		switch ( id ) {
		case 0:  wsprintfA( out, "R-301" ); cat = 0; break;
		case 1:  wsprintfA( out, "SENTINEL" ); cat = 3; break;
		case 2:  wsprintfA( out, "BOCEK" ); cat = 3; break;
		case 7:  wsprintfA( out, "RAMPAGE" ); cat = 4; break;
		case 80: wsprintfA( out, "FLATLINE" ); cat = 0; break;
		case 81: wsprintfA( out, "G7 SCOUT" ); cat = 3; break;
		case 82: wsprintfA( out, "HEMLOK" ); cat = 0; break;
		case 83: wsprintfA( out, "KRABER" ); cat = 3; break;
		case 84: wsprintfA( out, "LONGBOW" ); cat = 3; break;
		case 85: wsprintfA( out, "L-STAR" ); cat = 4; break;
		case 86: wsprintfA( out, "MASTIFF" ); cat = 2; break;
		case 87: wsprintfA( out, "MOZAMBIQUE" ); cat = 2; break;
		case 88: wsprintfA( out, "PEACEKEEPER" ); cat = 2; break;
		case 89: wsprintfA( out, "PROWLER" ); cat = 1; break;
		case 90: wsprintfA( out, "R-99" ); cat = 1; break;
		case 91: wsprintfA( out, "DEVOTION" ); cat = 4; break;
		case 92: wsprintfA( out, "RE-45" ); cat = 5; break;
		case 93: wsprintfA( out, "ALTERNATOR" ); cat = 1; break;
		case 94: wsprintfA( out, "SPITFIRE" ); cat = 4; break;
		case 95: wsprintfA( out, "TRIPLE TAKE" ); cat = 3; break;
		case 96: wsprintfA( out, "WINGMAN" ); cat = 5; break;
		case 97: wsprintfA( out, "VOLT" ); cat = 1; break;
		case 98: wsprintfA( out, "30-30" ); cat = 3; break;
		case 99: wsprintfA( out, "CAR" ); cat = 1; break;
		case 100: wsprintfA( out, "NEMESIS" ); cat = 0; break;
		case 105: wsprintfA( out, "P2020" ); cat = 5; break;
		case 106: wsprintfA( out, "EVA-8" ); cat = 2; break;
		case 107: wsprintfA( out, "HAVOC" ); cat = 0; break;
		case 108: wsprintfA( out, "CHARGE RIFLE" ); cat = 3; break;
		default: wsprintfA( out, "WEP #%d", id ); cat = 0; break;
		}
	}

	inline void draw_radar_frame( float rx, float ry, float rsize ) {
		d3d11_ui::add_filled_rect( rx, ry, rsize, rsize, { 0.04f, 0.045f, 0.07f, 0.80f } );
		d3d11_ui::add_rect( rx, ry, rsize, rsize, 1.0f, { 0.18f, 0.20f, 0.30f, 0.92f } );
		float rcx = rx + rsize * 0.5f;
		float rcy = ry + rsize * 0.5f;
		d3d11_ui::add_line( rx, rcy, rx + rsize, rcy, 1.0f, { 0.14f, 0.16f, 0.24f, 0.65f } );
		d3d11_ui::add_line( rcx, ry, rcx, ry + rsize, 1.0f, { 0.14f, 0.16f, 0.24f, 0.65f } );
		d3d11_ui::add_line( rcx, rcy, rcx - rsize * 0.28f, ry, 1.0f, { 0.44f, 0.37f, 0.98f, 0.45f } );
		d3d11_ui::add_line( rcx, rcy, rcx + rsize * 0.28f, ry, 1.0f, { 0.44f, 0.37f, 0.98f, 0.45f } );
		d3d11_ui::add_filled_rect( rcx - 2.0f, rcy - 2.0f, 4.0f, 4.0f, { 0.96f, 0.97f, 1.0f, 1.0f } );
	}

	inline void draw_radar_blip( float rx, float ry, float rsize, float dx_m, float dy_m, float yaw_deg, const d3d11_ui::color_t& col ) {
		float rad = ( 90.0f - yaw_deg ) * ( 3.14159265f / 180.0f );
		float cs = cosf( rad );
		float sn = sinf( rad );
		float rot_x = dx_m * cs - dy_m * sn;
		float rot_y = dx_m * sn + dy_m * cs;
		float scale = ( rsize * 0.5f ) / 150.0f;
		float px = rot_x * scale;
		float py = -rot_y * scale;
		float half = rsize * 0.5f - 4.0f;
		if ( px < -half ) px = -half;
		if ( px > half ) px = half;
		if ( py < -half ) py = -half;
		if ( py > half ) py = half;
		d3d11_ui::add_filled_rect( rx + rsize * 0.5f + px - 2.0f, ry + rsize * 0.5f + py - 2.0f, 4.0f, 4.0f, col );
	}

	inline void draw_oof_arrow( float cx, float cy, float dx_w, float dy_w, float yaw_deg, const d3d11_ui::color_t& col ) {
		float rad = ( yaw_deg - 90.0f ) * ( 3.14159265f / 180.0f );
		float ang = atan2f( dy_w, dx_w ) - rad;
		float dir_x = cosf( ang );
		float dir_y = -sinf( ang );
		float radius = 210.0f;
		float tx = cx + dir_x * radius;
		float ty = cy + dir_y * radius;
		float px = -dir_y;
		float py = dir_x;
		float bx = tx - dir_x * 11.0f;
		float by = ty - dir_y * 11.0f;
		d3d11_ui::add_line( tx, ty, bx + px * 6.0f, by + py * 6.0f, 1.8f, col );
		d3d11_ui::add_line( tx, ty, bx - px * 6.0f, by - py * 6.0f, 1.8f, col );
		d3d11_ui::add_line( bx + px * 6.0f, by + py * 6.0f, bx - px * 6.0f, by - py * 6.0f, 1.2f, col );
	}

	inline void get_loot_info( int item_id, int w_id, char* out, d3d11_ui::color_t& col, int& loot_cat ) {
		const d3d11_ui::color_t c_common{ 0.85f, 0.87f, 0.90f, 0.92f };
		const d3d11_ui::color_t c_rare{ 0.25f, 0.62f, 1.00f, 1.0f };
		const d3d11_ui::color_t c_epic{ 0.72f, 0.34f, 0.98f, 1.0f };
		const d3d11_ui::color_t c_gold{ 0.98f, 0.80f, 0.20f, 1.0f };
		const d3d11_ui::color_t c_mythic{ 0.98f, 0.24f, 0.26f, 1.0f };

		if ( w_id > 0 && w_id < 256 ) {
			int cat = 0;
			get_weapon_info( w_id, out, cat );
			if ( !( out[ 0 ] == 'W' && out[ 1 ] == 'E' && out[ 2 ] == 'P' && out[ 3 ] == ' ' && out[ 4 ] == '#' ) ) {
				col = ( w_id == 83 ) ? c_mythic : d3d11_ui::color_t{ 0.35f, 0.90f, 0.98f, 1.0f };
				loot_cat = 0;
				return;
			}
		}

		loot_cat = 1;
		switch ( item_id ) {
		case 1:   wsprintfA( out, "KRABER" );        col = c_mythic; loot_cat = 0; break;
		case 2:   wsprintfA( out, "MASTIFF" );       col = c_rare;   loot_cat = 0; break;
		case 126: wsprintfA( out, "NEMESIS" );       col = c_rare;   loot_cat = 0; break;
		case 140: wsprintfA( out, "LIGHT AMMO" );    col = { 0.95f, 0.62f, 0.22f, 0.95f }; loot_cat = 3; break;
		case 141: wsprintfA( out, "ENERGY AMMO" );   col = { 0.45f, 0.92f, 0.32f, 0.95f }; loot_cat = 3; break;
		case 142: wsprintfA( out, "SHOTGUN AMMO" );  col = { 0.95f, 0.28f, 0.28f, 0.95f }; loot_cat = 3; break;
		case 143: wsprintfA( out, "HEAVY AMMO" );    col = { 0.28f, 0.80f, 0.72f, 0.95f }; loot_cat = 3; break;
		case 144: wsprintfA( out, "SNIPER AMMO" );   col = { 0.68f, 0.45f, 0.98f, 0.95f }; loot_cat = 3; break;
		case 182: wsprintfA( out, "ACCEL" );         col = c_rare;   loot_cat = 2; break;
		case 183: wsprintfA( out, "PHOENIX KIT" );   col = c_epic;   loot_cat = 2; break;
		case 184: wsprintfA( out, "MED KIT" );       col = c_rare;   loot_cat = 2; break;
		case 185: wsprintfA( out, "SYRINGE" );       col = c_common; loot_cat = 2; break;
		case 186: wsprintfA( out, "SHIELD BAT" );    col = c_rare;   loot_cat = 2; break;
		case 187: wsprintfA( out, "SHIELD CELL" );   col = c_common; loot_cat = 2; break;
		case 188: wsprintfA( out, "HELMET LV1" );    col = c_common; loot_cat = 1; break;
		case 189: wsprintfA( out, "HELMET LV2" );    col = c_rare;   loot_cat = 1; break;
		case 190: wsprintfA( out, "HELMET LV3" );    col = c_epic;   loot_cat = 1; break;
		case 191: wsprintfA( out, "HELMET LV4" );    col = c_gold;   loot_cat = 1; break;
		case 193: wsprintfA( out, "ARMOR LV1" );     col = c_common; loot_cat = 1; break;
		case 194: wsprintfA( out, "ARMOR LV2" );     col = c_rare;   loot_cat = 1; break;
		case 195: wsprintfA( out, "ARMOR LV3" );     col = c_epic;   loot_cat = 1; break;
		case 196: wsprintfA( out, "ARMOR LV4" );     col = c_gold;   loot_cat = 1; break;
		case 197: wsprintfA( out, "EVO SHIELD" );    col = c_mythic; loot_cat = 1; break;
		case 203: wsprintfA( out, "KNOCK LV1" );     col = c_common; loot_cat = 1; break;
		case 204: wsprintfA( out, "KNOCK LV2" );     col = c_rare;   loot_cat = 1; break;
		case 205: wsprintfA( out, "KNOCK LV3" );     col = c_epic;   loot_cat = 1; break;
		case 206: wsprintfA( out, "KNOCK LV4" );     col = c_gold;   loot_cat = 1; break;
		case 207: wsprintfA( out, "BACKPACK LV1" );  col = c_common; loot_cat = 1; break;
		case 208: wsprintfA( out, "BACKPACK LV2" );  col = c_rare;   loot_cat = 1; break;
		case 209: wsprintfA( out, "BACKPACK LV3" );  col = c_epic;   loot_cat = 1; break;
		case 210: wsprintfA( out, "BACKPACK LV4" );  col = c_gold;   loot_cat = 1; break;
		default:
			out[ 0 ] = '\0';
			col = c_common;
			loot_cat = -1;
			break;
		}
	}

	inline void draw_skeleton( uintptr_t ent, uintptr_t bone_arr, bool add_origin, const apex_sdk::vec3_t& origin, const apex_sdk::matrix4x4_t& vm, const d3d11_ui::color_t& col ) {
		if ( !bone_arr )
			return;

		bool drew_studio = false;
		uintptr_t hdr_ptr = 0;
		if ( apex_sdk::safe_read( ent + apex_sdk::g_studio_hdr_off, hdr_ptr ) && apex_sdk::is_ptr_valid( hdr_ptr ) ) {
			uintptr_t studio = 0;
			uintptr_t cand = 0;
			int num_bones = 0, bone_idx = 0;
			if ( apex_sdk::safe_read( hdr_ptr + 0x8, cand ) && apex_sdk::is_ptr_valid( cand ) ) {
				apex_sdk::safe_read( cand + 0x9C, num_bones );
				apex_sdk::safe_read( cand + 0xA0, bone_idx );
				if ( num_bones > 1 && num_bones <= 256 && bone_idx > 0 && bone_idx <= 0x20000 )
					studio = cand;
			}
			if ( !studio ) {
				num_bones = 0;
				bone_idx = 0;
				apex_sdk::safe_read( hdr_ptr + 0x9C, num_bones );
				apex_sdk::safe_read( hdr_ptr + 0xA0, bone_idx );
				if ( num_bones > 1 && num_bones <= 256 && bone_idx > 0 && bone_idx <= 0x20000 )
					studio = hdr_ptr;
			}
			if ( studio ) {
				static constexpr std::uint32_t k_strides[ 3 ] = { 0xB4, 0xB0, 0xD8 };
				std::uint32_t stride = 0xB4;
				for ( int si = 0; si < 3; ++si ) {
					std::uint32_t st = k_strides[ si ];
					uintptr_t mb0 = studio + static_cast< uintptr_t >( bone_idx );
					uintptr_t mb1 = mb0 + st;
					int p0 = 0, p1 = -99;
					if ( apex_sdk::safe_read( mb0 + 0x04, p0 ) && apex_sdk::safe_read( mb1 + 0x04, p1 ) ) {
						if ( p0 == -1 && p1 >= 0 && p1 < num_bones ) {
							stride = st;
							break;
						}
					}
				}
				int limit = num_bones > 96 ? 96 : num_bones;
				float sx[ 96 ]{}, sy[ 96 ]{};
				bool vis[ 96 ]{};
				for ( int b = 0; b < limit; ++b ) {
					uintptr_t mb = studio + static_cast< uintptr_t >( bone_idx ) + static_cast< uintptr_t >( b ) * stride;
					int parent = -1;
					if ( !apex_sdk::safe_read( mb + 0x04, parent ) || parent < 0 || parent >= limit )
						continue;
					apex_sdk::vec3_t wp{}, pp{};
					if ( !apex_sdk::read_bone_pos( bone_arr, b, origin, add_origin, wp ) ||
						 !apex_sdk::read_bone_pos( bone_arr, parent, origin, add_origin, pp ) )
						continue;
					if ( fabsf( wp.x - pp.x ) > 55.0f || fabsf( wp.y - pp.y ) > 55.0f || fabsf( wp.z - pp.z ) > 55.0f )
						continue;
					if ( !vis[ b ] )
						vis[ b ] = apex_sdk::world_to_screen( wp, vm, d3d11_ui::g_width, d3d11_ui::g_height, sx[ b ], sy[ b ] );
					if ( !vis[ parent ] )
						vis[ parent ] = apex_sdk::world_to_screen( pp, vm, d3d11_ui::g_width, d3d11_ui::g_height, sx[ parent ], sy[ parent ] );
					if ( vis[ b ] && vis[ parent ] ) {
						d3d11_ui::add_line( sx[ parent ], sy[ parent ], sx[ b ], sy[ b ], 1.2f, col );
						drew_studio = true;
					}
				}
			}
		}

		if ( !drew_studio ) {
			static constexpr int k_pairs[][ 2 ] = {
				{ 8, 7 }, { 7, 5 }, { 5, 3 }, { 3, 2 },
				{ 7, 11 }, { 11, 12 }, { 12, 13 },
				{ 7, 35 }, { 35, 36 }, { 36, 37 },
				{ 2, 57 }, { 57, 58 }, { 58, 59 },
				{ 2, 62 }, { 62, 63 }, { 63, 64 }
			};
			for ( int p = 0; p < 16; ++p ) {
				apex_sdk::vec3_t p1{}, p2{};
				if ( !apex_sdk::read_bone_pos( bone_arr, k_pairs[ p ][ 0 ], origin, add_origin, p1 ) ||
					 !apex_sdk::read_bone_pos( bone_arr, k_pairs[ p ][ 1 ], origin, add_origin, p2 ) )
					continue;
				if ( fabsf( p1.x - origin.x ) > 110.0f || fabsf( p1.z - origin.z ) > 110.0f ||
					 fabsf( p2.x - origin.x ) > 110.0f || fabsf( p2.z - origin.z ) > 110.0f )
					continue;
				float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
				if ( apex_sdk::world_to_screen( p1, vm, d3d11_ui::g_width, d3d11_ui::g_height, x1, y1 ) &&
					 apex_sdk::world_to_screen( p2, vm, d3d11_ui::g_width, d3d11_ui::g_height, x2, y2 ) ) {
					d3d11_ui::add_line( x1, y1, x2, y2, 1.2f, col );
				}
			}
		}
	}

	inline int draw_players( int& out_spectators, char out_spec_names[ 16 ][ 32 ] ) {
		out_spectators = 0;
		bool any_vis = config::esp_boxes || config::esp_skeleton || config::esp_names || config::esp_weapon ||
			config::esp_distance || config::esp_health || config::esp_shield || config::esp_hp_text ||
			config::esp_head_dot || config::esp_status || config::esp_snaplines || config::esp_spectators ||
			config::esp_world || config::esp_grenades || config::esp_radar || config::esp_oof_arrows || config::esp_aim_warning;
		if ( ( !any_vis && !config::aim_enabled && !config::aim_silent && !config::trigger_enabled ) || !apex_sdk::g_initialized )
			return 0;

		uintptr_t local = apex_sdk::get_local_player( );
		static uintptr_t s_prev_local = 0;

		static constexpr int k_max_scan = 16384;
		static constexpr int k_direct_max = 256;
		static constexpr int k_step_per_frame = 64;
		static int scan_cursor = k_direct_max + 1;
		static bool is_cached[ k_max_scan + 1 ]{};
		static int npc_cached[ 256 ]{};
		static int npc_count = 0;
		static bool is_world_cached[ k_max_scan + 1 ]{};
		static int world_cached[ 512 ]{};
		static apex_sdk::ent_kind world_kinds[ 512 ]{};
		static int world_count = 0;

		if ( local != s_prev_local ) {
			s_prev_local = local;
			npc_count = 0;
			world_count = 0;
			scan_cursor = k_direct_max + 1;
			for ( int k = 0; k <= k_max_scan; ++k ) {
				is_cached[ k ] = false;
				is_world_cached[ k ] = false;
			}
			apex_sdk::clear_page_cache( );
			return 0;
		}

		if ( !local || !apex_sdk::is_mem_readable( local, 0x100 ) )
			return 0;

		apex_sdk::matrix4x4_t vm{};
		if ( !apex_sdk::get_view_matrix( vm ) )
			return 0;

		std::uint8_t local_life = 0;
		int local_health = 0;
		apex_sdk::safe_read( local + apex_sdk::offsets::life_state, local_life );
		apex_sdk::safe_read( local + apex_sdk::offsets::health, local_health );
		bool local_alive = ( local_life == 0 && local_health > 0 );

		apex_sdk::vec3_t local_pos{};
		if ( !apex_sdk::get_entity_origin( local, local_pos ) )
			apex_sdk::safe_read( local + apex_sdk::offsets::camera_origin, local_pos );
		apex_sdk::vec3_t local_ang{};
		apex_sdk::safe_read( local + apex_sdk::offsets::view_angles, local_ang );
		int local_team = 0;
		apex_sdk::safe_read( local + apex_sdk::offsets::team_num, local_team );

		const float radar_x = 16.0f;
		const float radar_y = 44.0f;
		const float radar_sz = 136.0f;
		if ( config::esp_radar )
			draw_radar_frame( radar_x, radar_y, radar_sz );

		for ( int s = 0; s < k_step_per_frame; ++s ) {
			uintptr_t ent = apex_sdk::get_entity( scan_cursor );
			apex_sdk::ent_kind kind = ( ent && ent != local ) ? apex_sdk::classify_entity( ent ) : apex_sdk::ent_kind::none;
			bool valid_npc = ( kind == apex_sdk::ent_kind::player || kind == apex_sdk::ent_kind::npc );
			bool valid_world = ( kind == apex_sdk::ent_kind::loot || kind == apex_sdk::ent_kind::deathbox || kind == apex_sdk::ent_kind::projectile );

			if ( valid_npc && !is_cached[ scan_cursor ] ) {
				if ( npc_count < 256 ) {
					is_cached[ scan_cursor ] = true;
					npc_cached[ npc_count++ ] = scan_cursor;
				}
			}
			else if ( !valid_npc && is_cached[ scan_cursor ] ) {
				is_cached[ scan_cursor ] = false;
				for ( int k = 0; k < npc_count; ++k ) {
					if ( npc_cached[ k ] == scan_cursor ) {
						npc_cached[ k ] = npc_cached[ --npc_count ];
						break;
					}
				}
			}

			if ( valid_world && !is_world_cached[ scan_cursor ] ) {
				if ( world_count < 512 ) {
					is_world_cached[ scan_cursor ] = true;
					world_cached[ world_count ] = scan_cursor;
					world_kinds[ world_count ] = kind;
					++world_count;
				}
			}
			else if ( !valid_world && is_world_cached[ scan_cursor ] ) {
				is_world_cached[ scan_cursor ] = false;
				for ( int k = 0; k < world_count; ++k ) {
					if ( world_cached[ k ] == scan_cursor ) {
						--world_count;
						world_cached[ k ] = world_cached[ world_count ];
						world_kinds[ k ] = world_kinds[ world_count ];
						break;
					}
				}
			}

			if ( ++scan_cursor > k_max_scan )
				scan_cursor = k_direct_max + 1;
		}

		if ( config::esp_world || config::esp_grenades ) {
			int loot_drawn = 0;
			for ( int w = 0; w < world_count; ++w ) {
				uintptr_t went = apex_sdk::get_entity( world_cached[ w ] );
				if ( !went || apex_sdk::classify_entity( went ) != world_kinds[ w ] ) {
					is_world_cached[ world_cached[ w ] ] = false;
					--world_count;
					world_cached[ w ] = world_cached[ world_count ];
					world_kinds[ w ] = world_kinds[ world_count ];
					--w;
					continue;
				}
				apex_sdk::vec3_t wpos{};
				if ( !apex_sdk::get_entity_origin( went, wpos ) )
					continue;
				float wdx = wpos.x - local_pos.x;
				float wdy = wpos.y - local_pos.y;
				float wdz = wpos.z - local_pos.z;
				float wdist = d3d11_ui::fast_sqrt( wdx * wdx + wdy * wdy + wdz * wdz ) * 0.0254f;
				if ( wdist > config::esp_world_dist )
					continue;
				float wx = 0.0f, wy = 0.0f;
				if ( !apex_sdk::world_to_screen( wpos, vm, d3d11_ui::g_width, d3d11_ui::g_height, wx, wy ) )
					continue;
				if ( wx < -40.0f || wx > d3d11_ui::g_width + 40.0f || wy < -20.0f || wy > d3d11_ui::g_height + 20.0f )
					continue;

				char wbuf[ 48 ]{};
				if ( world_kinds[ w ] == apex_sdk::ent_kind::projectile ) {
					if ( config::esp_grenades ) {
						wsprintfA( wbuf, "GRENADE! [%dm]", static_cast< int >( wdist ) );
						d3d11_ui::add_text( wx, wy, wbuf, { 1.0f, 0.22f, 0.22f, 1.0f }, 0.84f, true, true );
					}
				}
				else if ( world_kinds[ w ] == apex_sdk::ent_kind::deathbox ) {
					if ( config::esp_world && config::loot_deathbox && loot_drawn < 40 ) {
						wsprintfA( wbuf, "DEATHBOX [%dm]", static_cast< int >( wdist ) );
						d3d11_ui::add_text( wx, wy, wbuf, { 0.98f, 0.80f, 0.22f, 1.0f }, 0.78f, true, true );
						++loot_drawn;
					}
				}
				else if ( config::esp_world && loot_drawn < 40 ) {
					if ( wdist < 1.5f )
						continue;
					int item_id = 0;
					int w_id = 0;
					int grd = 0;
					apex_sdk::safe_read( went + apex_sdk::offsets::prop_custom_script_int, item_id );
					apex_sdk::safe_read( went + apex_sdk::offsets::prop_weapon_index, w_id );
					apex_sdk::safe_read( went + apex_sdk::offsets::grade, grd );
					char lname[ 32 ]{};
					d3d11_ui::color_t lcol{};
					int lcat = 1;
					get_loot_info( item_id, w_id, lname, lcol, lcat );
					if ( lname[ 0 ] == '\0' && item_id > 0 && item_id < 400 ) {
						char raw_name[ 32 ]{};
						if ( apex_sdk::is_sig_ptr_readable( went + apex_sdk::offsets::ent_name ) ) {
							const char* rn = reinterpret_cast< const char* >( went + apex_sdk::offsets::ent_name );
							int ni = 0;
							for ( ; ni < 24 && rn[ ni ] >= 32 && rn[ ni ] <= 126; ++ni )
								raw_name[ ni ] = rn[ ni ];
							raw_name[ ni ] = '\0';
							if ( ni >= 3 ) {
								for ( int k = 0; k <= ni; ++k ) lname[ k ] = raw_name[ k ];
								lcat = 1;
							}
						}
						if ( grd == 2 ) lcol = { 0.25f, 0.62f, 1.00f, 1.0f };
						else if ( grd == 3 ) lcol = { 0.72f, 0.34f, 0.98f, 1.0f };
						else if ( grd == 4 ) lcol = { 0.98f, 0.80f, 0.20f, 1.0f };
						else if ( grd >= 5 ) lcol = { 0.98f, 0.24f, 0.26f, 1.0f };
					}
					if ( lname[ 0 ] == '\0' || lcat < 0 )
						continue;
					bool show = ( lcat == 0 && config::loot_weapons ) ||
								( lcat == 1 && config::loot_armor ) ||
								( lcat == 2 && config::loot_heals ) ||
								( lcat == 3 && config::loot_ammo );
					if ( show ) {
						wsprintfA( wbuf, "%s [%dm]", lname, static_cast< int >( wdist ) );
						d3d11_ui::add_text( wx, wy, wbuf, lcol, 0.74f, true, true );
						++loot_drawn;
					}
				}
			}
		}

		const float cx = d3d11_ui::g_width * 0.5f;
		const float cy = d3d11_ui::g_height * 0.5f;
		float best_score = 1e9f;
		float best_dx = 0.0f;
		float best_dy = 0.0f;
		apex_sdk::vec3_t best_world_aim{};
		bool has_target = false;

		int drawn = 0;
		bool any_trig_hovered = false;
		static ULONGLONG s_trig_hover = 0;
		static ULONGLONG s_trig_last = 0;
		for ( int c = 0; c < k_direct_max + npc_count; ++c ) {
			bool is_cached_slot = ( c >= k_direct_max );
			int i = is_cached_slot ? npc_cached[ c - k_direct_max ] : ( c + 1 );
			uintptr_t ent = apex_sdk::get_entity( i );
			if ( !ent || ent == local || !apex_sdk::is_player_entity( ent ) ) {
				if ( is_cached_slot ) {
					is_cached[ i ] = false;
					npc_cached[ c - k_direct_max ] = npc_cached[ --npc_count ];
					--c;
				}
				continue;
			}

			std::uint8_t life_state = 0;
			int health = 0;
			apex_sdk::safe_read( ent + apex_sdk::offsets::life_state, life_state );
			apex_sdk::safe_read( ent + apex_sdk::offsets::health, health );
			if ( life_state != 0 || health <= 0 ) {
				if ( config::esp_spectators && i <= 128 ) {
					if ( apex_sdk::is_spectating_local( i, ent, local ) ) {
						if ( out_spectators < 16 ) {
							if ( !apex_sdk::get_player_name( i, out_spec_names[ out_spectators ], 32 ) )
								wsprintfA( out_spec_names[ out_spectators ], "Spectator #%d", i );
						}
						++out_spectators;
					}
				}
				continue;
			}

			int team = 0;
			apex_sdk::safe_read( ent + apex_sdk::offsets::team_num, team );
			if ( config::esp_team_check && team == local_team )
				continue;

			apex_sdk::vec3_t pos{};
			if ( !apex_sdk::get_entity_origin( ent, pos ) )
				continue;

			float dx = pos.x - local_pos.x;
			float dy = pos.y - local_pos.y;
			float dz = pos.z - local_pos.z;
			float dist_m = d3d11_ui::fast_sqrt( dx * dx + dy * dy + dz * dz ) * 0.0254f;
			if ( dist_m > config::esp_max_dist )
				continue;

			apex_sdk::ent_kind ent_k = apex_sdk::classify_entity( ent );
			bool is_human = ( ent_k == apex_sdk::ent_kind::player );
			int bleed = 0;
			int duck = 0;
			if ( is_human ) {
				apex_sdk::safe_read( ent + apex_sdk::offsets::bleedout_state, bleed );
				apex_sdk::safe_read( ent + apex_sdk::offsets::duck_state, duck );
			}
			bool is_vis = ( config::esp_vis_check || config::aim_vis_only ) ? apex_sdk::is_entity_visible( i, ent ) : false;

			d3d11_ui::color_t box_col = bleed > 0
				? d3d11_ui::color_t{ 0.98f, 0.72f, 0.20f, 1.0f }
				: ( ( config::esp_vis_check && is_vis )
					? d3d11_ui::color_t{ 0.22f, 0.96f, 0.48f, 1.0f }
					: d3d11_ui::color_t{ 1.0f, 0.30f, 0.32f, 1.0f } );

			if ( config::esp_radar ) {
				draw_radar_blip( radar_x, radar_y, radar_sz, dx * 0.0254f, dy * 0.0254f, local_ang.y, box_col );
			}

			apex_sdk::vec3_t maxs{};
			float height_z = ( bleed > 0 ) ? 28.0f : ( ( duck > 0 ) ? 46.0f : 70.0f );
			if ( apex_sdk::safe_read( ent + apex_sdk::offsets::collision_maxs, maxs ) && maxs.z >= 24.0f && maxs.z <= 76.0f )
				height_z = maxs.z;

			uintptr_t bone_arr = apex_sdk::get_bone_array( ent, pos );
			bool add_origin = false;
			bool has_head_bone = false;
			apex_sdk::vec3_t head_bone{};
			float min_bz = ( bleed > 0 ) ? 20.0f : ( ( duck > 0 ) ? 34.0f : 48.0f );
			if ( bone_arr && apex_sdk::find_head_bone( bone_arr, pos, add_origin, head_bone ) ) {
				float bz = head_bone.z - pos.z + 6.0f;
				if ( bz >= min_bz && bz <= 84.0f ) {
					height_z = bz;
					has_head_bone = true;
				}
			}

			apex_sdk::vec3_t head{ pos.x, pos.y, pos.z + height_z };
			float fx = 0.0f, fy = 0.0f, hx = 0.0f, hy = 0.0f;
			bool on_feet = apex_sdk::world_to_screen( pos, vm, d3d11_ui::g_width, d3d11_ui::g_height, fx, fy );
			bool on_head = apex_sdk::world_to_screen( head, vm, d3d11_ui::g_width, d3d11_ui::g_height, hx, hy );
			bool on_screen = on_feet && on_head && fx >= -50.0f && fx <= d3d11_ui::g_width + 50.0f && fy >= -50.0f && hy <= d3d11_ui::g_height + 50.0f;
			if ( !on_screen ) {
				if ( config::esp_oof_arrows ) {
					draw_oof_arrow( cx, cy, dx, dy, local_ang.y, box_col );
				}
				continue;
			}

			if ( config::esp_skeleton && bone_arr && dist_m <= 150.0f ) {
				draw_skeleton( ent, bone_arr, add_origin, pos, vm, { 0.94f, 0.96f, 1.0f, 0.85f } );
			}

			float bh = fy - hy;
			if ( bh < 4.0f || bh > d3d11_ui::g_height )
				continue;
			float bw = bh * 0.55f;
			float bx = ( fx + hx ) * 0.5f - bw * 0.5f;
			float by = hy;

			if ( config::esp_boxes ) {
				if ( config::esp_corner_box ) {
					draw_corner_box( bx, by, bw, bh, box_col );
				}
				else {
					d3d11_ui::add_rect( bx - 1.0f, by - 1.0f, bw + 2.0f, bh + 2.0f, 1.0f, { 0.0f, 0.0f, 0.0f, 0.75f } );
					d3d11_ui::add_rect( bx, by, bw, bh, 1.0f, box_col );
				}
			}

			if ( config::esp_head_dot ) {
				float r = bh * 0.075f;
				if ( r < 2.5f ) r = 2.5f;
				if ( r > 8.0f ) r = 8.0f;
				float hdx = hx, hdy = hy + bh * 0.14f;
				if ( has_head_bone )
					apex_sdk::world_to_screen( head_bone, vm, d3d11_ui::g_width, d3d11_ui::g_height, hdx, hdy );
				d3d11_ui::add_circle( hdx, hdy, r, 16, 1.2f, box_col );
			}

			if ( config::esp_snaplines ) {
				d3d11_ui::add_line( cx, d3d11_ui::g_height, bx + bw * 0.5f, by + bh, 1.0f, { box_col.r, box_col.g, box_col.b, 0.55f } );
			}

			if ( config::esp_health ) {
				int max_hp = 100;
				apex_sdk::safe_read( ent + apex_sdk::offsets::max_health, max_hp );
				if ( max_hp <= 0 ) max_hp = 100;
				float frac = static_cast< float >( health ) / static_cast< float >( max_hp );
				if ( frac > 1.0f ) frac = 1.0f;
				if ( frac < 0.0f ) frac = 0.0f;
				float bar_h = bh * frac;
				d3d11_ui::add_filled_rect( bx - 5.0f, by, 3.0f, bh, { 0.0f, 0.0f, 0.0f, 0.70f } );
				d3d11_ui::add_filled_rect( bx - 5.0f, by + ( bh - bar_h ), 3.0f, bar_h, { 0.25f, 0.92f, 0.45f, 1.0f } );
			}

			int sh = 0, max_sh = 0, armor_type = 0;
			if ( config::esp_shield || config::esp_hp_text || config::aim_priority >= 2 ) {
				apex_sdk::safe_read( ent + apex_sdk::offsets::shield, sh );
				apex_sdk::safe_read( ent + apex_sdk::offsets::max_shield, max_sh );
				apex_sdk::safe_read( ent + apex_sdk::offsets::armor_type, armor_type );
			}

			if ( config::esp_shield && max_sh > 0 && sh > 0 ) {
				float frac = static_cast< float >( sh ) / static_cast< float >( max_sh );
				if ( frac > 1.0f ) frac = 1.0f;
				float bar_h = bh * frac;
				d3d11_ui::color_t sh_col = get_shield_color( armor_type, max_sh );
				d3d11_ui::add_filled_rect( bx - 9.0f, by, 3.0f, bh, { 0.0f, 0.0f, 0.0f, 0.70f } );
				d3d11_ui::add_filled_rect( bx - 9.0f, by + ( bh - bar_h ), 3.0f, bar_h, sh_col );
			}

			if ( config::esp_names ) {
				static char cached_names[ 512 ][ 32 ]{};
				static int cached_name_id[ 512 ]{};
				char nbuf[ 32 ]{};
				if ( c < 512 && cached_name_id[ c ] == i && cached_names[ c ][ 0 ] ) {
					for ( int k = 0; k < 31 && cached_names[ c ][ k ]; ++k ) nbuf[ k ] = cached_names[ c ][ k ];
				}
				else {
					if ( !apex_sdk::get_player_name( i, nbuf, sizeof( nbuf ) ) ) {
						nbuf[ 0 ] = 'D'; nbuf[ 1 ] = 'u'; nbuf[ 2 ] = 'm'; nbuf[ 3 ] = 'm'; nbuf[ 4 ] = 'y'; nbuf[ 5 ] = '\0';
					}
					if ( c < 512 ) {
						cached_name_id[ c ] = i;
						for ( int k = 0; k < 32; ++k ) cached_names[ c ][ k ] = nbuf[ k ];
					}
				}
				d3d11_ui::add_text_outlined( bx + bw * 0.5f, by - 14.0f, nbuf, { 0.96f, 0.97f, 1.0f, 1.0f }, 0.84f, true, false );
			}

			float bot_y = by + bh + 2.0f;
			if ( config::esp_distance ) {
				char dbuf[ 24 ]{};
				wsprintfA( dbuf, "%dm", static_cast< int >( dist_m ) );
				d3d11_ui::add_text_outlined( bx + bw * 0.5f, bot_y, dbuf, { 0.96f, 0.97f, 1.0f, 1.0f }, 0.82f, true, false );
				bot_y += 12.0f;
			}

			if ( config::esp_hp_text ) {
				char hbuf[ 32 ]{};
				if ( sh > 0 )
					wsprintfA( hbuf, "%d + %d", health, sh );
				else
					wsprintfA( hbuf, "%d HP", health );
				d3d11_ui::add_text_outlined( bx + bw * 0.5f, bot_y, hbuf, { 0.78f, 0.94f, 0.84f, 1.0f }, 0.78f, true, false );
				bot_y += 12.0f;
			}

			if ( config::esp_weapon ) {
				std::uint32_t ewep_h = 0;
				char wname[ 32 ] = "MELEE";
				int wcat = 6;
				if ( apex_sdk::safe_read( ent + apex_sdk::offsets::latest_primary_weapons, ewep_h ) && ewep_h != 0xFFFFFFFF ) {
					uintptr_t ewep = apex_sdk::get_entity( ewep_h & 0xFFFF );
					if ( ewep && apex_sdk::is_sig_ptr_readable( ewep + apex_sdk::offsets::weapon_name_index ) ) {
						int ewid = -1;
						apex_sdk::safe_read( ewep + apex_sdk::offsets::weapon_name_index, ewid );
						if ( ewid >= 0 )
							get_weapon_info( ewid, wname, wcat );
					}
				}
				float tw = d3d11_ui::measure_text_width( wname, 0.74f );
				float total_w = 18.0f + tw;
				float wx = bx + bw * 0.5f - total_w * 0.5f;
				draw_weapon_icon( wx, bot_y + 1.0f, wcat, { 0.45f, 0.88f, 0.98f, 1.0f } );
				d3d11_ui::add_text_outlined( wx + 18.0f, bot_y, wname, { 0.92f, 0.95f, 1.0f, 1.0f }, 0.74f, false, false );
			}

			if ( config::esp_status || config::esp_aim_warning ) {
				float sy = by;
				const float sx = bx + bw + 4.0f;
				if ( config::esp_aim_warning && dist_m <= 120.0f ) {
					apex_sdk::vec3_t eang{};
					if ( apex_sdk::safe_read( ent + apex_sdk::offsets::view_angles, eang ) ) {
						float pr = eang.x * ( 3.14159265f / 180.0f );
						float yr = eang.y * ( 3.14159265f / 180.0f );
						float cp = cosf( pr );
						float fx_v = cp * cosf( yr );
						float fy_v = cp * sinf( yr );
						float fz_v = -sinf( pr );
						float inv_l = 1.0f / ( d3d11_ui::fast_sqrt( dx * dx + dy * dy + dz * dz ) + 0.001f );
						float dot = ( -dx * inv_l ) * fx_v + ( -dy * inv_l ) * fy_v + ( -dz * inv_l ) * fz_v;
						if ( dot > 0.988f ) {
							d3d11_ui::add_text_outlined( sx, sy, "AIMING!", { 1.0f, 0.18f, 0.42f, 1.0f }, 0.78f, false, false );
							sy += 12.0f;
						}
					}
				}
				if ( is_human && config::esp_status ) {
					if ( bleed > 0 ) {
						d3d11_ui::add_text_outlined( sx, sy, "KNOCK", { 0.98f, 0.72f, 0.20f, 1.0f }, 0.74f, false, false );
						sy += 11.0f;
					}
					int skydive = 0;
					apex_sdk::safe_read( ent + apex_sdk::offsets::skydive_state, skydive );
					if ( skydive > 0 ) {
						d3d11_ui::add_text_outlined( sx, sy, "FLY", { 0.35f, 0.88f, 0.98f, 1.0f }, 0.74f, false, false );
						sy += 11.0f;
					}
					std::uint8_t zooming = 0;
					apex_sdk::safe_read( ent + apex_sdk::offsets::zooming, zooming );
					if ( zooming != 0 ) {
						d3d11_ui::add_text_outlined( sx, sy, "ADS", { 0.98f, 0.90f, 0.30f, 1.0f }, 0.74f, false, false );
						sy += 11.0f;
					}
					if ( duck > 0 ) {
						d3d11_ui::add_text_outlined( sx, sy, "DUCK", { 0.70f, 0.78f, 0.92f, 1.0f }, 0.74f, false, false );
						sy += 11.0f;
					}
					int squad = -1;
					apex_sdk::safe_read( ent + apex_sdk::offsets::squad_id, squad );
					if ( squad >= 0 && squad < 100 ) {
						char sqbuf[ 16 ]{};
						wsprintfA( sqbuf, "SQ %d", squad );
						d3d11_ui::add_text_outlined( sx, sy, sqbuf, { 0.76f, 0.78f, 0.86f, 0.90f }, 0.72f, false, false );
					}
				}
			}

			if ( config::trigger_enabled && local_alive && !menu::g_open && ( !config::aim_vis_only || is_vis ) ) {
				if ( cx >= bx && cx <= bx + bw && cy >= by && cy <= by + bh ) {
					any_trig_hovered = true;
					ULONGLONG now = GetTickCount64( );
					if ( s_trig_hover == 0 )
						s_trig_hover = now;
					if ( now - s_trig_hover >= static_cast< ULONGLONG >( config::trigger_delay_ms ) &&
						 now - s_trig_last >= 110 ) {
						s_trig_last = now;
						INPUT tin[ 2 ]{};
						tin[ 0 ].type = INPUT_MOUSE;
						tin[ 0 ].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						tin[ 1 ].type = INPUT_MOUSE;
						tin[ 1 ].mi.dwFlags = MOUSEEVENTF_LEFTUP;
						SendInput( 2, tin, sizeof( INPUT ) );
					}
				}
			}

			if ( ( config::aim_enabled || config::aim_silent ) && local_alive && ( !config::aim_vis_only || is_vis ) ) {
				float bone_frac = ( config::aim_bone == 0 ) ? 0.90f : ( ( config::aim_bone == 1 ) ? 0.78f : 0.62f );
				apex_sdk::vec3_t aim_pos{ pos.x, pos.y, pos.z + height_z * bone_frac };
				if ( has_head_bone ) {
					aim_pos = head_bone;
					if ( config::aim_bone == 1 ) {
						apex_sdk::vec3_t neck{};
						if ( apex_sdk::read_bone_pos( bone_arr, 7, pos, add_origin, neck ) && fabsf( head_bone.z - neck.z ) < 16.0f )
							aim_pos = neck;
						else
							aim_pos.z -= 6.0f;
					}
					else if ( config::aim_bone == 2 ) {
						apex_sdk::vec3_t chest{};
						if ( apex_sdk::read_bone_pos( bone_arr, 5, pos, add_origin, chest ) && fabsf( head_bone.z - chest.z ) < 28.0f )
							aim_pos = chest;
						else
							aim_pos.z -= 16.0f;
					}
				}
				if ( config::aim_predict && dist_m > 5.0f ) {
					apex_sdk::vec3_t evel{};
					if ( apex_sdk::safe_read( ent + apex_sdk::offsets::abs_velocity, evel ) ) {
						float dist_u = d3d11_ui::fast_sqrt( dx * dx + dy * dy + dz * dz );
						float t = dist_u / 24000.0f;
						aim_pos.x += evel.x * t;
						aim_pos.y += evel.y * t;
						aim_pos.z += evel.z * t + 0.5f * 750.0f * t * t;
					}
				}
				float ax = 0.0f, ay = 0.0f;
				if ( apex_sdk::world_to_screen( aim_pos, vm, d3d11_ui::g_width, d3d11_ui::g_height, ax, ay ) ) {
					float adx = ax - cx;
					float ady = ay - cy;
					float dist_px = d3d11_ui::fast_sqrt( adx * adx + ady * ady );
					if ( dist_px <= config::aim_fov_radius ) {
						float score = dist_px;
						if ( config::aim_priority == 1 )
							score = dist_m;
						else if ( config::aim_priority == 2 )
							score = static_cast< float >( health + sh );
						else if ( config::aim_priority == 3 )
							score = dist_px + dist_m * 1.5f + static_cast< float >( health + sh ) * 0.25f;

						if ( score < best_score ) {
							best_score = score;
							best_dx = adx;
							best_dy = ady;
							best_world_aim = aim_pos;
							has_target = true;
						}
					}
				}
			}

			++drawn;
		}

		if ( !any_trig_hovered )
			s_trig_hover = 0;

		bool lmb_down = !menu::g_open && ( ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 );
		bool rmb_down = !menu::g_open && ( ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) != 0 );

		if ( ( config::aim_enabled || config::aim_silent ) && local_alive && has_target ) {
			if ( config::aim_target_line ) {
				d3d11_ui::add_line( cx, cy, cx + best_dx, cy + best_dy, 1.1f, { 0.44f, 0.37f, 0.98f, 0.70f } );
			}
			bool did_silent = false;
			if ( config::aim_silent && !config::safe_mode && lmb_down ) {
				apex_sdk::vec3_t cam_pos{};
				if ( !apex_sdk::safe_read( local + apex_sdk::offsets::camera_origin, cam_pos ) )
					cam_pos = { local_pos.x, local_pos.y, local_pos.z + 64.0f };
				apex_sdk::vec3_t aim_ang = apex_sdk::calc_angle( cam_pos, best_world_aim );
				apex_sdk::vec3_t punch{};
				if ( apex_sdk::safe_read( local + apex_sdk::offsets::punch_angle, punch ) ) {
					aim_ang.x -= punch.x;
					aim_ang.y -= punch.y;
					if ( aim_ang.x > 89.0f ) aim_ang.x = 89.0f;
					if ( aim_ang.x < -89.0f ) aim_ang.x = -89.0f;
					while ( aim_ang.y > 180.0f ) aim_ang.y -= 360.0f;
					while ( aim_ang.y < -180.0f ) aim_ang.y += 360.0f;
				}
				did_silent = apex_sdk::apply_silent_usercmd( aim_ang );
			}
			if ( !did_silent && config::aim_enabled && ( rmb_down || lmb_down ) ) {
				static float rem_x = 0.0f;
				static float rem_y = 0.0f;
				float smooth = config::aim_smooth < 1.0f ? 1.0f : config::aim_smooth;
				float step_x = ( best_dx / smooth ) + rem_x;
				float step_y = ( best_dy / smooth ) + rem_y;
				int ix = static_cast< int >( step_x );
				int iy = static_cast< int >( step_y );
				rem_x = step_x - static_cast< float >( ix );
				rem_y = step_y - static_cast< float >( iy );
				if ( ix != 0 || iy != 0 ) {
					INPUT in{};
					in.type = INPUT_MOUSE;
					in.mi.dx = ix;
					in.mi.dy = iy;
					in.mi.dwFlags = MOUSEEVENTF_MOVE;
					SendInput( 1, &in, sizeof( in ) );
				}
			}
		}

		return drawn;
	}

	inline void apply_local_features( ) {
		if ( !apex_sdk::g_initialized )
			return;
		uintptr_t local = apex_sdk::get_local_player( );
		if ( !local || !apex_sdk::is_mem_readable( local, 0x100 ) )
			return;

		std::uint8_t local_life = 0;
		int local_health = 0;
		apex_sdk::safe_read( local + apex_sdk::offsets::life_state, local_life );
		apex_sdk::safe_read( local + apex_sdk::offsets::health, local_health );
		if ( local_life != 0 || local_health <= 0 ) {
			config::current_held_weapon_id = -1;
			return;
		}

		static float s_orig_fov_scale = 0.0f;
		static bool s_fov_active = false;
		if ( apex_sdk::g_cl_fov_scale_ptr ) {
			if ( !config::safe_mode && config::fov_changer ) {
				if ( !s_fov_active ) {
					apex_sdk::safe_read( apex_sdk::g_cl_fov_scale_ptr, s_orig_fov_scale );
					s_fov_active = true;
				}
				float scale = config::fov_value / 70.0f;
				apex_sdk::safe_write< float >( apex_sdk::g_cl_fov_scale_ptr, scale );
			}
			else if ( s_fov_active ) {
				if ( s_orig_fov_scale >= 0.9f && s_orig_fov_scale <= 2.0f )
					apex_sdk::safe_write< float >( apex_sdk::g_cl_fov_scale_ptr, s_orig_fov_scale );
				s_fov_active = false;
			}
		}

		static bool s_prev_tp = false;
		if ( !config::safe_mode && config::third_person ) {
			if ( apex_sdk::g_thirdperson_ptr ) {
				apex_sdk::safe_write< float >( apex_sdk::g_thirdperson_ptr, 1.0f );
				apex_sdk::safe_write< int >( apex_sdk::g_thirdperson_ptr + 4, 1 );
			}
			apex_sdk::safe_write< std::uint8_t >( local + apex_sdk::offsets::third_person_shoulder, 1 );
			s_prev_tp = true;
		}
		else if ( s_prev_tp ) {
			if ( apex_sdk::g_thirdperson_ptr ) {
				apex_sdk::safe_write< float >( apex_sdk::g_thirdperson_ptr, -1.0f );
				apex_sdk::safe_write< int >( apex_sdk::g_thirdperson_ptr + 4, -1 );
			}
			apex_sdk::safe_write< std::uint8_t >( local + apex_sdk::offsets::third_person_shoulder, 0 );
			s_prev_tp = false;
		}

		uintptr_t wep = 0;
		std::uint32_t wep_handle = 0;
		if ( apex_sdk::safe_read( local + apex_sdk::offsets::latest_primary_weapons, wep_handle ) && wep_handle != 0xFFFFFFFF ) {
			uintptr_t cand = apex_sdk::get_entity( wep_handle & 0xFFFF );
			if ( cand && apex_sdk::is_sig_ptr_readable( cand ) )
				wep = cand;
		}

		int held_id = -1;
		int ammo_check = -1;
		if ( wep && apex_sdk::is_sig_ptr_readable( wep + apex_sdk::offsets::weapon_name_index ) ) {
			apex_sdk::safe_read( wep + apex_sdk::offsets::weapon_name_index, held_id );
			apex_sdk::safe_read( wep + apex_sdk::offsets::weapon_ammo, ammo_check );
			config::current_held_weapon_id = ( held_id >= 0 && held_id < 256 ) ? held_id : -1;

			if ( !config::safe_mode && config::skin_changer && held_id >= 0 && held_id < 256 && config::weapon_skin_enabled[ held_id ] ) {
				int sid = config::weapon_skins[ held_id ];
				apex_sdk::safe_write< int >( wep + apex_sdk::offsets::weapon_skin_override, sid );
				apex_sdk::safe_write< std::uint8_t >( wep + apex_sdk::offsets::weapon_skin_valid, 1 );
				apex_sdk::safe_write< int >( wep + apex_sdk::offsets::weapon_skin, sid );
			}
			if ( !config::safe_mode && config::no_spread ) {
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::weapon_player_data + apex_sdk::offsets::move_spread, 0.0f );
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::weapon_player_data + apex_sdk::offsets::spread_hip, 0.0f );
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::weapon_player_data + apex_sdk::offsets::spread_ads, 0.0f );
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::weapon_player_data + apex_sdk::offsets::kick_spread_hip, 0.0f );
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::weapon_player_data + apex_sdk::offsets::kick_spread_ads, 0.0f );
				apex_sdk::safe_write< float >( wep + apex_sdk::offsets::laser_spread, 0.0f );
			}
		}
		else {
			config::current_held_weapon_id = -1;
		}

		std::uint32_t vm_handle = 0;
		if ( !config::safe_mode && config::heirloom_enabled && apex_sdk::safe_read( local + apex_sdk::offsets::view_models, vm_handle ) && vm_handle != 0xFFFFFFFF ) {
			uintptr_t vm_ent = apex_sdk::get_entity( vm_handle & 0xFFFF );
			if ( vm_ent && apex_sdk::is_sig_ptr_readable( vm_ent ) && apex_sdk::is_sig_ptr_readable( vm_ent + apex_sdk::offsets::weapon_skin ) ) {
				int hskin = static_cast< int >( config::heirloom_skin + 0.5f );
				apex_sdk::safe_write< int >( vm_ent + apex_sdk::offsets::weapon_skin, hskin );
			}
		}

		if ( config::rcs_enabled && !menu::g_open ) {
			static apex_sdk::vec3_t s_prev_punch{};
			static float s_rcs_rem_x = 0.0f;
			static float s_rcs_rem_y = 0.0f;
			bool lmb = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
			if ( lmb ) {
				apex_sdk::vec3_t punch{};
				if ( apex_sdk::safe_read( local + apex_sdk::offsets::punch_angle, punch ) ) {
					float dp_x = punch.x - s_prev_punch.x;
					float dp_y = punch.y - s_prev_punch.y;
					float scale = config::rcs_strength * 0.01f;
					if ( !config::safe_mode ) {
						apex_sdk::vec3_t ang{};
						if ( apex_sdk::safe_read( local + apex_sdk::offsets::view_angles, ang ) ) {
							ang.x -= dp_x * scale;
							ang.y -= dp_y * scale;
							if ( ang.x > 89.0f ) ang.x = 89.0f;
							if ( ang.x < -89.0f ) ang.x = -89.0f;
							while ( ang.y > 180.0f ) ang.y -= 360.0f;
							while ( ang.y < -180.0f ) ang.y += 360.0f;
							apex_sdk::safe_write( local + apex_sdk::offsets::view_angles, ang );
						}
					}
					else if ( ( fabsf( dp_x ) > 0.005f || fabsf( dp_y ) > 0.005f ) && ( fabsf( dp_x ) < 15.0f && fabsf( dp_y ) < 15.0f ) ) {
						float step_x = dp_y * scale * 22.0f + s_rcs_rem_x;
						float step_y = -dp_x * scale * 22.0f + s_rcs_rem_y;
						int ix = static_cast< int >( step_x );
						int iy = static_cast< int >( step_y );
						s_rcs_rem_x = step_x - static_cast< float >( ix );
						s_rcs_rem_y = step_y - static_cast< float >( iy );
						if ( ix != 0 || iy != 0 ) {
							INPUT rcs_in{};
							rcs_in.type = INPUT_MOUSE;
							rcs_in.mi.dx = ix;
							rcs_in.mi.dy = iy;
							rcs_in.mi.dwFlags = MOUSEEVENTF_MOVE;
							SendInput( 1, &rcs_in, sizeof( rcs_in ) );
						}
					}
					s_prev_punch = punch;
				}
			}
			else {
				s_prev_punch = {};
				s_rcs_rem_x = 0.0f;
				s_rcs_rem_y = 0.0f;
			}
		}

		if ( config::superglide && !menu::g_open ) {
			static float s_prev_trav = 0.0f;
			static bool s_sg_crouch_next = false;
			if ( s_sg_crouch_next ) {
				s_sg_crouch_next = false;
				INPUT cin[ 2 ]{};
				cin[ 0 ].type = INPUT_KEYBOARD;
				cin[ 0 ].ki.wVk = VK_LCONTROL;
				cin[ 1 ].type = INPUT_KEYBOARD;
				cin[ 1 ].ki.wVk = VK_LCONTROL;
				cin[ 1 ].ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput( 2, cin, sizeof( INPUT ) );
			}
			float trav = 0.0f;
			if ( apex_sdk::safe_read( local + apex_sdk::offsets::traversal_progress, trav ) ) {
				if ( trav > 0.87f && s_prev_trav <= 0.87f && s_prev_trav > 0.05f ) {
					INPUT jin[ 2 ]{};
					jin[ 0 ].type = INPUT_KEYBOARD;
					jin[ 0 ].ki.wVk = VK_SPACE;
					jin[ 1 ].type = INPUT_KEYBOARD;
					jin[ 1 ].ki.wVk = VK_SPACE;
					jin[ 1 ].ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput( 2, jin, sizeof( INPUT ) );
					s_sg_crouch_next = true;
				}
				s_prev_trav = trav;
			}
		}

		if ( config::neo_strafe && !menu::g_open &&
			 ( GetAsyncKeyState( VK_SPACE ) & 0x8000 ) && ( GetAsyncKeyState( VK_LSHIFT ) & 0x8000 ) ) {
			int f = 0;
			if ( apex_sdk::safe_read( local + apex_sdk::offsets::flags, f ) && ( f & 1 ) == 0 ) {
				static ULONGLONG s_last_lurch = 0;
				static int s_lurch_step = 0;
				ULONGLONG now = GetTickCount64( );
				if ( now - s_last_lurch >= 18 ) {
					s_last_lurch = now;
					WORD vk_dir = ( s_lurch_step & 1 ) ? 'A' : 'D';
					++s_lurch_step;
					INPUT nin[ 3 ]{};
					nin[ 0 ].type = INPUT_MOUSE;
					nin[ 0 ].mi.dwFlags = MOUSEEVENTF_WHEEL;
					nin[ 0 ].mi.mouseData = 120;
					nin[ 1 ].type = INPUT_KEYBOARD;
					nin[ 1 ].ki.wVk = vk_dir;
					nin[ 2 ].type = INPUT_KEYBOARD;
					nin[ 2 ].ki.wVk = vk_dir;
					nin[ 2 ].ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput( 3, nin, sizeof( INPUT ) );
				}
			}
		}

		if ( config::bhop && !menu::g_open && ( GetAsyncKeyState( VK_SPACE ) & 0x8000 ) ) {
			int f = 0;
			if ( apex_sdk::safe_read( local + apex_sdk::offsets::flags, f ) && ( f & 1 ) ) {
				static ULONGLONG s_last_jump = 0;
				ULONGLONG now = GetTickCount64( );
				if ( now - s_last_jump >= 25 ) {
					s_last_jump = now;
					INPUT in[ 2 ]{};
					in[ 0 ].type = INPUT_KEYBOARD;
					in[ 0 ].ki.wVk = VK_SPACE;
					in[ 1 ].type = INPUT_KEYBOARD;
					in[ 1 ].ki.wVk = VK_SPACE;
					in[ 1 ].ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput( 2, in, sizeof( INPUT ) );
				}
			}
		}

		if ( config::hud_info ) {
			apex_sdk::vec3_t vel{};
			apex_sdk::safe_read( local + apex_sdk::offsets::abs_velocity, vel );
			int spd = static_cast< int >( d3d11_ui::fast_sqrt( vel.x * vel.x + vel.y * vel.y ) );
			int ammo = -1;
			if ( wep )
				apex_sdk::safe_read( wep + apex_sdk::offsets::weapon_ammo, ammo );
			char hbuf[ 64 ]{};
			if ( ammo >= 0 )
				wsprintfA( hbuf, "VEL: %d | AMMO: %d", spd, ammo );
			else
				wsprintfA( hbuf, "VEL: %d", spd );
			float tw = d3d11_ui::measure_text_width( hbuf, 0.86f );
			float w = tw + 20.0f;
			d3d11_ui::add_filled_rect( 16.0f, 16.0f, w, 22.0f, { 0.045f, 0.048f, 0.075f, 0.82f } );
			d3d11_ui::add_rect( 16.0f, 16.0f, w, 22.0f, 1.0f, { 0.14f, 0.16f, 0.24f, 0.90f } );
			d3d11_ui::add_text( 26.0f, 27.0f, hbuf, { 0.88f, 0.92f, 0.98f, 1.0f }, 0.86f, false, true );
		}
	}

	inline void draw_render_thread( ) {
		if ( config::panic_active ) {
			static bool s_panic_cleaned = false;
			if ( !s_panic_cleaned && apex_sdk::g_initialized ) {
				uintptr_t local = apex_sdk::get_local_player( );
				if ( local && apex_sdk::is_sig_ptr_readable( local ) )
					apex_sdk::safe_write< std::uint8_t >( local + apex_sdk::offsets::third_person_shoulder, 0 );
				if ( apex_sdk::g_cl_fov_scale_ptr )
					apex_sdk::safe_write< float >( apex_sdk::g_cl_fov_scale_ptr, 1.0f );
				s_panic_cleaned = true;
			}
			return;
		}

		apex_sdk::reset_page_cache( );

		int count = 0;
		int specs = 0;
		char spec_names[ 16 ][ 32 ]{};
		__try {
			count = draw_players( specs, spec_names );
			apply_local_features( );
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
		}
		draw_watermark( count );
		draw_spectators( specs, spec_names );
		draw_crosshair( );
		draw_aim_fov( );
	}
}
