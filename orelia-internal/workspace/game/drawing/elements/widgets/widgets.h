// orelia-internal/workspace/game/drawing/elements/widgets/widgets.h
#pragma once
#include <workspace/game/drawing/d3d11_renderer.hxx>
#include <workspace/game/features/features.h>

namespace widgets {
	struct ui_input_t {
		float mx = 0.0f;
		float my = 0.0f;
		bool lmb_down = false;
		bool lmb_click = false;
		int active_slider_id = 0;

		bool hovered( float x, float y, float w, float h ) const {
			return mx >= x && mx <= x + w && my >= y && my <= y + h;
		}
	};

	struct palette_t {
		d3d11_ui::color_t bg          { 0.043f, 0.051f, 0.078f, 0.97f };
		d3d11_ui::color_t sidebar     { 0.059f, 0.071f, 0.110f, 0.99f };
		d3d11_ui::color_t card        { 0.078f, 0.094f, 0.145f, 0.96f };
		d3d11_ui::color_t card_hdr    { 0.098f, 0.118f, 0.180f, 1.00f };
		d3d11_ui::color_t ctrl_bg     { 0.062f, 0.074f, 0.114f, 1.00f };
		d3d11_ui::color_t ctrl_hover  { 0.098f, 0.118f, 0.180f, 1.00f };
		d3d11_ui::color_t border      { 0.145f, 0.173f, 0.263f, 0.95f };
		d3d11_ui::color_t text_hi     { 0.949f, 0.961f, 0.996f, 1.00f };
		d3d11_ui::color_t text_mid    { 0.690f, 0.725f, 0.820f, 1.00f };
		d3d11_ui::color_t text_dim    { 0.435f, 0.475f, 0.580f, 1.00f };
		d3d11_ui::color_t accent      { 0.424f, 0.361f, 0.906f, 1.00f };
		d3d11_ui::color_t accent_hi   { 0.545f, 0.490f, 0.996f, 1.00f };
		d3d11_ui::color_t cyan        { 0.133f, 0.827f, 0.933f, 1.00f };
		d3d11_ui::color_t ok          { 0.204f, 0.827f, 0.600f, 1.00f };
		d3d11_ui::color_t warn        { 0.984f, 0.749f, 0.141f, 1.00f };
		d3d11_ui::color_t err         { 0.973f, 0.443f, 0.443f, 1.00f };
	};

	inline palette_t get_palette( ) {
		palette_t p{};
		switch ( config::theme_accent & 3 ) {
		case 1:
			p.accent    = { 0.082f, 0.722f, 0.851f, 1.00f };
			p.accent_hi = { 0.220f, 0.855f, 0.965f, 1.00f };
			break;
		case 2:
			p.accent    = { 0.125f, 0.745f, 0.502f, 1.00f };
			p.accent_hi = { 0.255f, 0.875f, 0.620f, 1.00f };
			break;
		case 3:
			p.accent    = { 0.925f, 0.345f, 0.388f, 1.00f };
			p.accent_hi = { 0.985f, 0.475f, 0.510f, 1.00f };
			break;
		default:
			break;
		}
		return p;
	}

	inline void draw_card( float x, float y, float w, float h, const char* title, const palette_t& pal ) {
		d3d11_ui::add_filled_rounded_rect( x, y, w, h, 5.0f, pal.card );
		d3d11_ui::add_rounded_rect( x, y, w, h, 5.0f, 1.0f, pal.border );
		d3d11_ui::add_filled_rect( x + 1.0f, y + 26.0f, w - 2.0f, 1.0f, pal.border );
		d3d11_ui::add_filled_rect( x + 10.0f, y + 7.0f, 3.0f, 12.0f, pal.accent );
		d3d11_ui::add_text( x + 18.0f, y + 13.0f, title, pal.text_hi, 0.84f, false, true );
	}

	inline bool draw_toggle( float x, float y, float w, const char* label, bool& val, const ui_input_t& in, const palette_t& pal ) {
		constexpr float row_h = 24.0f;
		const bool hov = in.hovered( x, y, w, row_h );
		if ( hov ) {
			d3d11_ui::add_filled_rounded_rect( x, y, w, row_h, 3.0f, pal.ctrl_hover );
		}
		d3d11_ui::add_text( x + 6.0f, y + row_h * 0.5f, label, val ? pal.text_hi : pal.text_mid, 0.82f, false, true );

		const float sw_w = 30.0f;
		const float sw_h = 14.0f;
		const float sx = x + w - sw_w - 6.0f;
		const float sy = y + ( row_h - sw_h ) * 0.5f;
		d3d11_ui::add_filled_rounded_rect( sx, sy, sw_w, sw_h, 7.0f, val ? pal.accent : pal.ctrl_bg );
		d3d11_ui::add_rounded_rect( sx, sy, sw_w, sw_h, 7.0f, 1.0f, val ? pal.accent_hi : pal.border );

		const float knob_x = val ? ( sx + sw_w - 11.5f ) : ( sx + 2.5f );
		d3d11_ui::add_filled_rounded_rect( knob_x, sy + 2.5f, 9.0f, 9.0f, 4.5f, pal.text_hi );

		if ( hov && in.lmb_click ) {
			val = !val;
			return true;
		}
		return false;
	}

	inline bool draw_slider( int id, float x, float y, float w, const char* label, float& val, float vmin, float vmax, const char* fmt, ui_input_t& in, const palette_t& pal ) {
		constexpr float row_h = 34.0f;
		const float bar_x = x + 6.0f;
		const float bar_w = w - 12.0f;
		const float bar_y = y + 22.0f;
		const float bar_h = 5.0f;

		const bool hov = in.hovered( x, y, w, row_h );
		if ( hov && in.lmb_click ) {
			in.active_slider_id = id;
		}
		bool changed = false;
		if ( in.active_slider_id == id ) {
			if ( in.lmb_down ) {
				float t = d3d11_ui::clampf( ( in.mx - bar_x ) / ( bar_w > 1.0f ? bar_w : 1.0f ), 0.0f, 1.0f );
				val = vmin + ( vmax - vmin ) * t;
				changed = true;
			} else {
				in.active_slider_id = 0;
			}
		}

		d3d11_ui::add_text( x + 6.0f, y + 9.0f, label, pal.text_mid, 0.80f, false, true );

		char vbuf[ 32 ]{};
		wsprintfA( vbuf, fmt, static_cast< int >( val + 0.5f ) );
		const float vw = d3d11_ui::measure_text_width( vbuf, 0.80f );
		d3d11_ui::add_text( x + w - vw - 6.0f, y + 9.0f, vbuf, pal.accent_hi, 0.80f, false, true );

		const float frac = d3d11_ui::clampf( ( val - vmin ) / ( ( vmax - vmin ) > 0.001f ? ( vmax - vmin ) : 1.0f ), 0.0f, 1.0f );
		d3d11_ui::add_filled_rounded_rect( bar_x, bar_y, bar_w, bar_h, 2.5f, pal.ctrl_bg );
		if ( frac > 0.01f ) {
			d3d11_ui::add_filled_rounded_rect( bar_x, bar_y, bar_w * frac, bar_h, 2.5f, pal.accent );
		}
		const float kx = bar_x + bar_w * frac - 4.5f;
		d3d11_ui::add_filled_rounded_rect( kx, bar_y - 2.5f, 9.0f, 10.0f, 4.5f, pal.text_hi );
		return changed;
	}

	inline bool draw_cycle( float x, float y, float w, const char* label, int& idx, const char* const* items, int count, const ui_input_t& in, const palette_t& pal ) {
		constexpr float row_h = 26.0f;
		const bool hov = in.hovered( x, y, w, row_h );
		d3d11_ui::add_filled_rounded_rect( x, y, w, row_h, 3.0f, hov ? pal.ctrl_hover : pal.ctrl_bg );
		d3d11_ui::add_rounded_rect( x, y, w, row_h, 3.0f, 1.0f, pal.border );

		d3d11_ui::add_text( x + 8.0f, y + row_h * 0.5f, label, pal.text_mid, 0.80f, false, true );
		const char* val_txt = ( count > 0 ) ? items[ ( ( idx % count ) + count ) % count ] : "-";
		const float vw = d3d11_ui::measure_text_width( val_txt, 0.80f );
		d3d11_ui::add_text( x + w - vw - 8.0f, y + row_h * 0.5f, val_txt, pal.accent_hi, 0.80f, false, true );

		if ( hov && in.lmb_click && count > 0 ) {
			idx = ( idx + 1 ) % count;
			return true;
		}
		return false;
	}

	inline bool draw_button( float x, float y, float w, float h, const char* label, const ui_input_t& in, const palette_t& pal, bool primary = false ) {
		const bool hov = in.hovered( x, y, w, h );
		const d3d11_ui::color_t bg = primary
			? ( hov ? pal.accent_hi : pal.accent )
			: ( hov ? pal.ctrl_hover : pal.ctrl_bg );
		d3d11_ui::add_filled_rounded_rect( x, y, w, h, 4.0f, bg );
		if ( !primary ) {
			d3d11_ui::add_rounded_rect( x, y, w, h, 4.0f, 1.0f, pal.border );
		}
		d3d11_ui::add_text( x + w * 0.5f, y + h * 0.5f, label, pal.text_hi, 0.82f, true, true );
		return hov && in.lmb_click;
	}

	inline void draw_status_row( float x, float y, float w, const char* label, const char* val, const d3d11_ui::color_t& val_col, const palette_t& pal ) {
		constexpr float row_h = 24.0f;
		d3d11_ui::add_filled_rounded_rect( x, y, w, row_h, 3.0f, pal.ctrl_bg );
		d3d11_ui::add_text( x + 8.0f, y + row_h * 0.5f, label, pal.text_mid, 0.80f, false, true );
		const float vw = d3d11_ui::measure_text_width( val, 0.80f );
		d3d11_ui::add_text( x + w - vw - 8.0f, y + row_h * 0.5f, val, val_col, 0.80f, false, true );
	}
}
