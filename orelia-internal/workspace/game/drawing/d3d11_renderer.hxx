// orelia-internal/workspace/game/drawing/d3d11_renderer.hxx
#pragma once
#include <d3d11.h>
#include <intrin.h>

#include <workspace/game/drawing/shaders/vs.h>
#include <workspace/game/drawing/shaders/ps_color.h>
#include <workspace/game/drawing/shaders/ps_tex.h>

#pragma comment(lib, "d3d11.lib")

namespace d3d11_ui {
	struct vertex_t {
		float x, y, z;
		float u, v;
		float r, g, b, a;
	};

	struct color_t {
		float r, g, b, a;
	};

	inline float clampf( float v, float lo, float hi ) {
		return v < lo ? lo : ( v > hi ? hi : v );
	}

	inline float lerpf( float a, float b, float t ) {
		return a + ( b - a ) * t;
	}

	inline color_t lerp_col( const color_t& a, const color_t& b, float t ) {
		return {
			lerpf( a.r, b.r, t ),
			lerpf( a.g, b.g, t ),
			lerpf( a.b, b.b, t ),
			lerpf( a.a, b.a, t )
		};
	}

	inline float fast_sqrt( float x ) {
		if ( x <= 0.0f ) return 0.0f;
		return _mm_cvtss_f32( _mm_sqrt_ss( _mm_set_ss( x ) ) );
	}

	inline float fast_sin( float x ) {
		constexpr float pi = 3.14159265f;
		constexpr float two_pi = 6.28318530f;
		while ( x > pi ) x -= two_pi;
		while ( x < -pi ) x += two_pi;
		float x2 = x * x;
		return x * ( 1.0f - x2 / 6.0f + ( x2 * x2 ) / 120.0f - ( x2 * x2 * x2 ) / 5040.0f );
	}

	inline float fast_cos( float x ) {
		return fast_sin( x + 1.57079632f );
	}

	inline int str_len( const char* s ) {
		int n = 0;
		while ( s && s[ n ] ) ++n;
		return n;
	}

	inline void mem_cpy( void* dst, const void* src, size_t n ) {
		if ( !dst || !src || !n ) return;
		auto* d64 = static_cast< uint64_t* >( dst );
		const auto* s64 = static_cast< const uint64_t* >( src );
		size_t q = n >> 3;
		for ( size_t i = 0; i < q; ++i ) d64[ i ] = s64[ i ];
		size_t tail = q << 3;
		auto* d8 = static_cast< unsigned char* >( dst );
		const auto* s8 = static_cast< const unsigned char* >( src );
		for ( size_t i = tail; i < n; ++i ) d8[ i ] = s8[ i ];
	}

	inline IDXGISwapChain* g_swap_chain = nullptr;
	inline ID3D11Device* g_device = nullptr;
	inline ID3D11DeviceContext* g_context = nullptr;
	inline ID3D11RenderTargetView* g_rtv = nullptr;
	inline HWND g_hwnd = nullptr;
	inline float g_width = 1920.0f;
	inline float g_height = 1080.0f;
	inline int g_fps = 0;
	inline DWORD g_fps_tick = 0;
	inline int g_fps_frames = 0;

	inline ID3D11VertexShader* g_vs = nullptr;
	inline ID3D11PixelShader* g_ps_color = nullptr;
	inline ID3D11PixelShader* g_ps_tex = nullptr;
	inline ID3D11InputLayout* g_layout = nullptr;
	inline ID3D11Buffer* g_vb = nullptr;
	inline ID3D11BlendState* g_blend = nullptr;
	inline ID3D11RasterizerState* g_rast = nullptr;
	inline ID3D11DepthStencilState* g_depth = nullptr;
	inline ID3D11SamplerState* g_sampler = nullptr;
	inline ID3D11ShaderResourceView* g_font_srv = nullptr;
	inline bool g_pipeline_ready = false;

	constexpr int max_vertices = 131072;
	inline vertex_t g_color_verts[ max_vertices ]{};
	inline int g_color_count = 0;
	inline vertex_t g_text_verts[ max_vertices ]{};
	inline int g_text_count = 0;

	struct tex_quad_t {
		ID3D11ShaderResourceView* srv;
		vertex_t verts[ 6 ];
	};
	constexpr int max_tex_quads = 512;
	inline tex_quad_t g_tex_quads[ max_tex_quads ]{};
	inline int g_tex_quad_count = 0;
	inline vertex_t g_tex_upload_verts[ max_tex_quads * 6 ]{};

	constexpr int atlas_w = 256;
	constexpr int atlas_h = 128;
	constexpr int cell_w = 16;
	constexpr int cell_h = 20;
	inline unsigned int g_atlas_pixels[ atlas_w * atlas_h ]{};
	inline float g_char_adv[ 95 ]{};
	inline float g_font_height = 15.0f;

	inline bool create_font_atlas( ) {
		HMODULE gdi = GetModuleHandleA( "GDI32.dll" );
		if ( !gdi ) gdi = LoadLibraryA( "GDI32.dll" );
		if ( !gdi ) return false;

		using create_dc_fn = HDC( WINAPI* )( HDC );
		using delete_dc_fn = BOOL( WINAPI* )( HDC );
		using create_dib_fn = HBITMAP( WINAPI* )( HDC, const BITMAPINFO*, UINT, VOID**, HANDLE, DWORD );
		using create_font_fn = HFONT( WINAPI* )( int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR );
		using select_obj_fn = HGDIOBJ( WINAPI* )( HDC, HGDIOBJ );
		using delete_obj_fn = BOOL( WINAPI* )( HGDIOBJ );
		using set_bk_mode_fn = int( WINAPI* )( HDC, int );
		using set_text_col_fn = COLORREF( WINAPI* )( HDC, COLORREF );
		using text_out_fn = BOOL( WINAPI* )( HDC, int, int, LPCSTR, int );
		using get_extent_fn = BOOL( WINAPI* )( HDC, LPCSTR, int, LPSIZE );

		auto f_create_dc = reinterpret_cast< create_dc_fn >( GetProcAddress( gdi, "CreateCompatibleDC" ) );
		auto f_delete_dc = reinterpret_cast< delete_dc_fn >( GetProcAddress( gdi, "DeleteDC" ) );
		auto f_create_dib = reinterpret_cast< create_dib_fn >( GetProcAddress( gdi, "CreateDIBSection" ) );
		auto f_create_font = reinterpret_cast< create_font_fn >( GetProcAddress( gdi, "CreateFontA" ) );
		auto f_select_obj = reinterpret_cast< select_obj_fn >( GetProcAddress( gdi, "SelectObject" ) );
		auto f_delete_obj = reinterpret_cast< delete_obj_fn >( GetProcAddress( gdi, "DeleteObject" ) );
		auto f_set_bk = reinterpret_cast< set_bk_mode_fn >( GetProcAddress( gdi, "SetBkMode" ) );
		auto f_set_col = reinterpret_cast< set_text_col_fn >( GetProcAddress( gdi, "SetTextColor" ) );
		auto f_text_out = reinterpret_cast< text_out_fn >( GetProcAddress( gdi, "TextOutA" ) );
		auto f_get_extent = reinterpret_cast< get_extent_fn >( GetProcAddress( gdi, "GetTextExtentPoint32A" ) );

		if ( !f_create_dc || !f_delete_dc || !f_create_dib || !f_create_font || !f_select_obj ||
			 !f_delete_obj || !f_set_bk || !f_set_col || !f_text_out || !f_get_extent )
			return false;

		HDC hdc = f_create_dc( nullptr );
		if ( !hdc ) return false;

		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmi.bmiHeader.biWidth = atlas_w;
		bmi.bmiHeader.biHeight = -atlas_h;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		void* dib_bits = nullptr;
		HBITMAP bmp = f_create_dib( hdc, &bmi, DIB_RGB_COLORS, &dib_bits, nullptr, 0 );
		if ( !bmp || !dib_bits ) {
			f_delete_dc( hdc );
			return false;
		}

		HFONT font = f_create_font(
			-14, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI"
		);

		HGDIOBJ old_bmp = f_select_obj( hdc, bmp );
		HGDIOBJ old_font = font ? f_select_obj( hdc, font ) : nullptr;

		auto* raw = static_cast< unsigned int* >( dib_bits );
		for ( int i = 0; i < atlas_w * atlas_h; ++i )
			raw[ i ] = 0x00000000;

		f_set_bk( hdc, TRANSPARENT );
		f_set_col( hdc, RGB( 255, 255, 255 ) );

		for ( int ch = 0; ch < 95; ++ch ) {
			char c = static_cast< char >( 32 + ch );
			int col = ch % 16;
			int row = ch / 16;
			int ox = col * cell_w;
			int oy = row * cell_h;

			SIZE sz{};
			f_get_extent( hdc, &c, 1, &sz );
			g_char_adv[ ch ] = static_cast< float >( sz.cx > 0 ? sz.cx : 4 );
			f_text_out( hdc, ox, oy, &c, 1 );
		}

		for ( int i = 0; i < atlas_w * atlas_h; ++i ) {
			unsigned int px = raw[ i ];
			unsigned int r = ( px >> 16 ) & 0xFF;
			unsigned int g = ( px >> 8 ) & 0xFF;
			unsigned int b = px & 0xFF;
			unsigned int alpha = ( r * 77 + g * 150 + b * 29 ) >> 8;
			if ( alpha > 255 ) alpha = 255;
			g_atlas_pixels[ i ] = ( alpha << 24 ) | 0x00FFFFFF;
		}

		if ( old_font ) f_select_obj( hdc, old_font );
		f_select_obj( hdc, old_bmp );
		if ( font ) f_delete_obj( font );
		f_delete_obj( bmp );
		f_delete_dc( hdc );

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = atlas_w;
		desc.Height = atlas_h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA init{};
		init.pSysMem = g_atlas_pixels;
		init.SysMemPitch = atlas_w * 4;

		ID3D11Texture2D* tex = nullptr;
		if ( FAILED( g_device->CreateTexture2D( &desc, &init, &tex ) ) )
			return false;

		HRESULT hr = g_device->CreateShaderResourceView( tex, nullptr, &g_font_srv );
		tex->Release( );
		return SUCCEEDED( hr );
	}

	inline void release_pipeline( ) {
		g_pipeline_ready = false;
		if ( g_rtv ) { g_rtv->Release( ); g_rtv = nullptr; }
		if ( g_font_srv ) { g_font_srv->Release( ); g_font_srv = nullptr; }
		if ( g_sampler ) { g_sampler->Release( ); g_sampler = nullptr; }
		if ( g_depth ) { g_depth->Release( ); g_depth = nullptr; }
		if ( g_rast ) { g_rast->Release( ); g_rast = nullptr; }
		if ( g_blend ) { g_blend->Release( ); g_blend = nullptr; }
		if ( g_vb ) { g_vb->Release( ); g_vb = nullptr; }
		if ( g_layout ) { g_layout->Release( ); g_layout = nullptr; }
		if ( g_ps_tex ) { g_ps_tex->Release( ); g_ps_tex = nullptr; }
		if ( g_ps_color ) { g_ps_color->Release( ); g_ps_color = nullptr; }
		if ( g_vs ) { g_vs->Release( ); g_vs = nullptr; }
		if ( g_context ) { g_context->Release( ); g_context = nullptr; }
		if ( g_device ) { g_device->Release( ); g_device = nullptr; }
		g_swap_chain = nullptr;
	}

	inline bool init_pipeline( ) {
		if ( g_pipeline_ready )
			return true;
		if ( !g_device || !g_context )
			return false;

		if ( FAILED( g_device->CreateVertexShader( g_vs_bytecode, sizeof( g_vs_bytecode ), nullptr, &g_vs ) ) )
			return false;
		if ( FAILED( g_device->CreatePixelShader( g_ps_color_bytecode, sizeof( g_ps_color_bytecode ), nullptr, &g_ps_color ) ) )
			return false;
		if ( FAILED( g_device->CreatePixelShader( g_ps_tex_bytecode, sizeof( g_ps_tex_bytecode ), nullptr, &g_ps_tex ) ) )
			return false;

		D3D11_INPUT_ELEMENT_DESC ied[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if ( FAILED( g_device->CreateInputLayout( ied, 3, g_vs_bytecode, sizeof( g_vs_bytecode ), &g_layout ) ) )
			return false;

		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( vertex_t ) * max_vertices;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if ( FAILED( g_device->CreateBuffer( &bd, nullptr, &g_vb ) ) )
			return false;

		D3D11_BLEND_DESC bdesc{};
		bdesc.RenderTarget[ 0 ].BlendEnable = TRUE;
		bdesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bdesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bdesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
		bdesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
		bdesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		bdesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bdesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		g_device->CreateBlendState( &bdesc, &g_blend );

		D3D11_RASTERIZER_DESC rdesc{};
		rdesc.FillMode = D3D11_FILL_SOLID;
		rdesc.CullMode = D3D11_CULL_NONE;
		rdesc.DepthClipEnable = TRUE;
		g_device->CreateRasterizerState( &rdesc, &g_rast );

		D3D11_DEPTH_STENCIL_DESC ddesc{};
		ddesc.DepthEnable = FALSE;
		ddesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		g_device->CreateDepthStencilState( &ddesc, &g_depth );

		D3D11_SAMPLER_DESC sdesc{};
		sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sdesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		g_device->CreateSamplerState( &sdesc, &g_sampler );

		if ( !create_font_atlas( ) )
			return false;

		g_pipeline_ready = true;
		return true;
	}

	inline void to_ndc( float px, float py, float& nx, float& ny ) {
		nx = ( px / g_width ) * 2.0f - 1.0f;
		ny = 1.0f - ( py / g_height ) * 2.0f;
	}

	inline void push_tri( vertex_t* dst, int& count, float x1, float y1, float x2, float y2, float x3, float y3, const color_t& c ) {
		if ( count + 3 > max_vertices ) return;
		float ax, ay, bx, by, cx, cy;
		to_ndc( x1, y1, ax, ay );
		to_ndc( x2, y2, bx, by );
		to_ndc( x3, y3, cx, cy );
		dst[ count++ ] = { ax, ay, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { bx, by, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { cx, cy, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
	}

	inline void push_quad( vertex_t* dst, int& count, float x, float y, float w, float h,
						   float u0, float v0, float u1, float v1, const color_t& c ) {
		if ( count + 6 > max_vertices )
			return;
		float x0, y0, x1, y1;
		to_ndc( x, y, x0, y0 );
		to_ndc( x + w, y + h, x1, y1 );
		dst[ count++ ] = { x0, y0, 0.f, u0, v0, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { x1, y0, 0.f, u1, v0, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { x0, y1, 0.f, u0, v1, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { x1, y0, 0.f, u1, v0, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { x1, y1, 0.f, u1, v1, c.r, c.g, c.b, c.a };
		dst[ count++ ] = { x0, y1, 0.f, u0, v1, c.r, c.g, c.b, c.a };
	}

	inline void add_filled_triangle( float x1, float y1, float x2, float y2, float x3, float y3, const color_t& c ) {
		push_tri( g_color_verts, g_color_count, x1, y1, x2, y2, x3, y3, c );
	}

	inline void add_filled_rect( float x, float y, float w, float h, const color_t& c ) {
		push_quad( g_color_verts, g_color_count, x, y, w, h, 0.f, 0.f, 1.f, 1.f, c );
	}

	inline void add_gradient_rect_v( float x, float y, float w, float h, const color_t& top, const color_t& bot ) {
		if ( g_color_count + 6 > max_vertices ) return;
		float x0, y0, x1, y1;
		to_ndc( x, y, x0, y0 );
		to_ndc( x + w, y + h, x1, y1 );
		g_color_verts[ g_color_count++ ] = { x0, y0, 0.f, 0.f, 0.f, top.r, top.g, top.b, top.a };
		g_color_verts[ g_color_count++ ] = { x1, y0, 0.f, 1.f, 0.f, top.r, top.g, top.b, top.a };
		g_color_verts[ g_color_count++ ] = { x0, y1, 0.f, 0.f, 1.f, bot.r, bot.g, bot.b, bot.a };
		g_color_verts[ g_color_count++ ] = { x1, y0, 0.f, 1.f, 0.f, top.r, top.g, top.b, top.a };
		g_color_verts[ g_color_count++ ] = { x1, y1, 0.f, 1.f, 1.f, bot.r, bot.g, bot.b, bot.a };
		g_color_verts[ g_color_count++ ] = { x0, y1, 0.f, 0.f, 1.f, bot.r, bot.g, bot.b, bot.a };
	}

	inline void add_gradient_rect_h( float x, float y, float w, float h, const color_t& left, const color_t& right ) {
		if ( g_color_count + 6 > max_vertices ) return;
		float x0, y0, x1, y1;
		to_ndc( x, y, x0, y0 );
		to_ndc( x + w, y + h, x1, y1 );
		g_color_verts[ g_color_count++ ] = { x0, y0, 0.f, 0.f, 0.f, left.r, left.g, left.b, left.a };
		g_color_verts[ g_color_count++ ] = { x1, y0, 0.f, 1.f, 0.f, right.r, right.g, right.b, right.a };
		g_color_verts[ g_color_count++ ] = { x0, y1, 0.f, 0.f, 1.f, left.r, left.g, left.b, left.a };
		g_color_verts[ g_color_count++ ] = { x1, y0, 0.f, 1.f, 0.f, right.r, right.g, right.b, right.a };
		g_color_verts[ g_color_count++ ] = { x1, y1, 0.f, 1.f, 1.f, right.r, right.g, right.b, right.a };
		g_color_verts[ g_color_count++ ] = { x0, y1, 0.f, 0.f, 1.f, left.r, left.g, left.b, left.a };
	}

	inline void add_rect( float x, float y, float w, float h, float t, const color_t& c ) {
		add_filled_rect( x, y, w, t, c );
		add_filled_rect( x, y + h - t, w, t, c );
		add_filled_rect( x, y, t, h, c );
		add_filled_rect( x + w - t, y, t, h, c );
	}

	inline void add_rect_outlined( float x, float y, float w, float h, float t, const color_t& c, const color_t& outline ) {
		add_rect( x - 1.0f, y - 1.0f, w + 2.0f, h + 2.0f, t + 1.0f, outline );
		add_rect( x, y, w, h, t, c );
	}

	inline void add_line( float x1, float y1, float x2, float y2, float t, const color_t& c ) {
		if ( g_color_count + 6 > max_vertices )
			return;
		float dx = x2 - x1;
		float dy = y2 - y1;
		float len = fast_sqrt( dx * dx + dy * dy );
		if ( len < 0.001f )
			return;
		float nx = -dy / len * ( t * 0.5f );
		float ny = dx / len * ( t * 0.5f );

		float ax, ay, bx, by, cx, cy, dxp, dyp;
		to_ndc( x1 + nx, y1 + ny, ax, ay );
		to_ndc( x1 - nx, y1 - ny, bx, by );
		to_ndc( x2 + nx, y2 + ny, cx, cy );
		to_ndc( x2 - nx, y2 - ny, dxp, dyp );

		g_color_verts[ g_color_count++ ] = { ax, ay, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		g_color_verts[ g_color_count++ ] = { cx, cy, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		g_color_verts[ g_color_count++ ] = { bx, by, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		g_color_verts[ g_color_count++ ] = { cx, cy, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		g_color_verts[ g_color_count++ ] = { dxp, dyp, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
		g_color_verts[ g_color_count++ ] = { bx, by, 0.f, 0.f, 0.f, c.r, c.g, c.b, c.a };
	}

	inline void add_circle( float cx, float cy, float radius, int segments, float t, const color_t& c ) {
		constexpr float two_pi = 6.283185307f;
		float step = two_pi / static_cast< float >( segments );
		for ( int i = 0; i < segments; ++i ) {
			float a1 = step * i;
			float a2 = step * ( i + 1 );
			add_line(
				cx + fast_cos( a1 ) * radius,
				cy + fast_sin( a1 ) * radius,
				cx + fast_cos( a2 ) * radius,
				cy + fast_sin( a2 ) * radius,
				t,
				c
			);
		}
	}

	inline void add_filled_circle( float cx, float cy, float radius, int segments, const color_t& c ) {
		constexpr float two_pi = 6.283185307f;
		float step = two_pi / static_cast< float >( segments );
		for ( int i = 0; i < segments; ++i ) {
			float a1 = step * i;
			float a2 = step * ( i + 1 );
			add_filled_triangle(
				cx, cy,
				cx + fast_cos( a1 ) * radius, cy + fast_sin( a1 ) * radius,
				cx + fast_cos( a2 ) * radius, cy + fast_sin( a2 ) * radius,
				c
			);
		}
	}

	inline void add_filled_rounded_rect( float x, float y, float w, float h, float r, const color_t& c ) {
		float max_r = ( w < h ? w : h ) * 0.5f;
		if ( r <= 0.5f ) {
			add_filled_rect( x, y, w, h, c );
			return;
		}
		if ( r > max_r ) r = max_r;

		add_filled_rect( x + r, y, w - r * 2.0f, h, c );
		add_filled_rect( x, y + r, r, h - r * 2.0f, c );
		add_filled_rect( x + w - r, y + r, r, h - r * 2.0f, c );

		constexpr float half_pi = 1.57079632f;
		constexpr int segs = 2;
		auto draw_corner = [&]( float ccx, float ccy, float start_ang ) {
			float step = half_pi / static_cast< float >( segs );
			for ( int i = 0; i < segs; ++i ) {
				float a1 = start_ang + step * i;
				float a2 = start_ang + step * ( i + 1 );
				add_filled_triangle(
					ccx, ccy,
					ccx + fast_cos( a1 ) * r, ccy + fast_sin( a1 ) * r,
					ccx + fast_cos( a2 ) * r, ccy + fast_sin( a2 ) * r,
					c
				);
			}
		};
		draw_corner( x + w - r, y + r, -half_pi );
		draw_corner( x + w - r, y + h - r, 0.0f );
		draw_corner( x + r, y + h - r, half_pi );
		draw_corner( x + r, y + r, half_pi * 2.0f );
	}

	inline void add_rounded_rect( float x, float y, float w, float h, float r, float t, const color_t& c ) {
		float max_r = ( w < h ? w : h ) * 0.5f;
		if ( r <= 0.5f ) {
			add_rect( x, y, w, h, t, c );
			return;
		}
		if ( r > max_r ) r = max_r;

		add_filled_rect( x + r, y, w - r * 2.0f, t, c );
		add_filled_rect( x + r, y + h - t, w - r * 2.0f, t, c );
		add_filled_rect( x, y + r, t, h - r * 2.0f, c );
		add_filled_rect( x + w - t, y + r, t, h - r * 2.0f, c );

		constexpr float half_pi = 1.57079632f;
		constexpr int segs = 2;
		auto draw_arc = [&]( float ccx, float ccy, float start_ang ) {
			float step = half_pi / static_cast< float >( segs );
			for ( int i = 0; i < segs; ++i ) {
				float a1 = start_ang + step * i;
				float a2 = start_ang + step * ( i + 1 );
				add_line(
					ccx + fast_cos( a1 ) * r, ccy + fast_sin( a1 ) * r,
					ccx + fast_cos( a2 ) * r, ccy + fast_sin( a2 ) * r,
					t, c
				);
			}
		};
		draw_arc( x + w - r, y + r, -half_pi );
		draw_arc( x + w - r, y + h - r, 0.0f );
		draw_arc( x + r, y + h - r, half_pi );
		draw_arc( x + r, y + r, half_pi * 2.0f );
	}

	inline float measure_text_width( const char* text, float scale ) {
		float w = 0.0f;
		for ( int i = 0; text && text[ i ]; ++i ) {
			unsigned char ch = static_cast< unsigned char >( text[ i ] );
			if ( ch >= 32 && ch <= 126 ) {
				w += g_char_adv[ ch - 32 ] * scale;
			}
		}
		return w;
	}

	inline void add_text( float x, float y, const char* text, const color_t& c, float scale = 1.0f, bool center_x = false, bool center_y = false ) {
		if ( !text || !*text )
			return;

		float total_w = measure_text_width( text, scale );
		float sx = center_x ? ( x - total_w * 0.5f ) : x;
		float sy = center_y ? ( y - ( g_font_height * scale ) * 0.5f ) : y;

		float cur_x = sx;
		for ( int i = 0; text[ i ]; ++i ) {
			unsigned char ch = static_cast< unsigned char >( text[ i ] );
			if ( ch >= 32 && ch <= 126 ) {
				int idx = ch - 32;
				int col = idx % 16;
				int row = idx / 16;
				float adv = g_char_adv[ idx ];
				float u0 = static_cast< float >( col * cell_w ) / static_cast< float >( atlas_w );
				float v0 = static_cast< float >( row * cell_h ) / static_cast< float >( atlas_h );
				float u1 = static_cast< float >( col * cell_w + adv + 1.0f ) / static_cast< float >( atlas_w );
				float v1 = static_cast< float >( row * cell_h + cell_h ) / static_cast< float >( atlas_h );
				push_quad( g_text_verts, g_text_count, cur_x, sy, ( adv + 1.0f ) * scale, static_cast< float >( cell_h ) * scale, u0, v0, u1, v1, c );
				cur_x += adv * scale;
			}
		}
	}

	inline void add_text_outlined( float x, float y, const char* text, const color_t& c, float scale = 1.0f, bool center_x = false, bool center_y = false ) {
		if ( !text || !*text )
			return;
		const color_t shadow{ 0.0f, 0.0f, 0.0f, c.a * 0.92f };
		add_text( x + 1.0f, y + 1.0f, text, shadow, scale, center_x, center_y );
		add_text( x, y, text, c, scale, center_x, center_y );
	}

	inline void add_text_outlined_8way( float x, float y, const char* text, const color_t& c, float scale = 1.0f, bool center_x = false, bool center_y = false ) {
		if ( !text || !*text )
			return;
		const color_t out{ 0.0f, 0.0f, 0.0f, c.a * 0.90f };
		add_text( x - 1.0f, y - 1.0f, text, out, scale, center_x, center_y );
		add_text( x + 1.0f, y + 1.0f, text, out, scale, center_x, center_y );
		add_text( x, y, text, c, scale, center_x, center_y );
	}

	inline void add_shadow_rect( float x, float y, float w, float h, float r, float alpha = 0.28f ) {
		add_filled_rounded_rect( x - 6.0f, y - 4.0f, w + 12.0f, h + 12.0f, r + 4.0f, { 0.0f, 0.0f, 0.0f, alpha * 0.35f } );
		add_filled_rounded_rect( x - 3.0f, y - 2.0f, w + 6.0f, h + 6.0f, r + 2.0f, { 0.0f, 0.0f, 0.0f, alpha * 0.65f } );
	}

	inline void add_texture( float x, float y, float w, float h, float u0, float v0, float u1, float v1, ID3D11ShaderResourceView* srv, const color_t& c ) {
		if ( !srv || g_tex_quad_count >= max_tex_quads )
			return;
		srv->AddRef( );
		auto& q = g_tex_quads[ g_tex_quad_count++ ];
		q.srv = srv;
		float x0, y0, x1, y1;
		to_ndc( x, y, x0, y0 );
		to_ndc( x + w, y + h, x1, y1 );
		q.verts[ 0 ] = { x0, y0, 0.f, u0, v0, c.r, c.g, c.b, c.a };
		q.verts[ 1 ] = { x1, y0, 0.f, u1, v0, c.r, c.g, c.b, c.a };
		q.verts[ 2 ] = { x1, y1, 0.f, u1, v1, c.r, c.g, c.b, c.a };
		q.verts[ 3 ] = { x0, y0, 0.f, u0, v0, c.r, c.g, c.b, c.a };
		q.verts[ 4 ] = { x1, y1, 0.f, u1, v1, c.r, c.g, c.b, c.a };
		q.verts[ 5 ] = { x0, y1, 0.f, u0, v1, c.r, c.g, c.b, c.a };
	}

	inline void flush_batch( const vertex_t* verts, int count, ID3D11PixelShader* ps ) {
		if ( count <= 0 || !g_vb )
			return;

		D3D11_MAPPED_SUBRESOURCE mapped{};
		if ( SUCCEEDED( g_context->Map( g_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) ) {
			mem_cpy( mapped.pData, verts, static_cast< size_t >( count ) * sizeof( vertex_t ) );
			g_context->Unmap( g_vb, 0 );
		}

		UINT stride = sizeof( vertex_t );
		UINT offset = 0;
		g_context->IASetVertexBuffers( 0, 1, &g_vb, &stride, &offset );
		g_context->IASetInputLayout( g_layout );
		g_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		g_context->VSSetShader( g_vs, nullptr, 0 );
		g_context->PSSetShader( ps, nullptr, 0 );
		g_context->Draw( static_cast< UINT >( count ), 0 );
	}

	inline void clear_batches( ) {
		for ( int i = 0; i < g_tex_quad_count; ++i ) {
			if ( g_tex_quads[ i ].srv ) {
				g_tex_quads[ i ].srv->Release( );
				g_tex_quads[ i ].srv = nullptr;
			}
		}
		g_color_count = 0;
		g_text_count = 0;
		g_tex_quad_count = 0;
	}

	inline void render_frame( IDXGISwapChain* swap_chain ) {
		if ( !swap_chain ) {
			clear_batches( );
			return;
		}

		++g_fps_frames;
		DWORD now_tick = GetTickCount( );
		if ( g_fps_tick == 0 ) {
			g_fps_tick = now_tick;
		}
		else if ( now_tick - g_fps_tick >= 500 ) {
			g_fps = static_cast< int >( ( g_fps_frames * 1000 ) / ( now_tick - g_fps_tick ) );
			g_fps_frames = 0;
			g_fps_tick = now_tick;
		}

		DXGI_SWAP_CHAIN_DESC sd{};
		if ( SUCCEEDED( swap_chain->GetDesc( &sd ) ) ) {
			float w = static_cast< float >( sd.BufferDesc.Width );
			float h = static_cast< float >( sd.BufferDesc.Height );
			if ( w > 0.f && h > 0.f && ( w != g_width || h != g_height || g_swap_chain != swap_chain ) ) {
				if ( g_rtv ) {
					g_rtv->Release( );
					g_rtv = nullptr;
				}
				g_width = w;
				g_height = h;
			}
			g_hwnd = sd.OutputWindow;
		}

		if ( !g_device || g_swap_chain != swap_chain ) {
			if ( g_device || g_swap_chain ) {
				release_pipeline( );
			}
			if ( FAILED( swap_chain->GetDevice( __uuidof( ID3D11Device ), reinterpret_cast< void** >( &g_device ) ) ) ) {
				clear_batches( );
				return;
			}
			g_device->GetImmediateContext( &g_context );
			g_swap_chain = swap_chain;
			if ( g_width <= 0.f ) g_width = 1920.f;
			if ( g_height <= 0.f ) g_height = 1080.f;
		}

		if ( !g_rtv ) {
			ID3D11Texture2D* back_buffer = nullptr;
			if ( SUCCEEDED( swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &back_buffer ) ) ) && back_buffer ) {
				g_device->CreateRenderTargetView( back_buffer, nullptr, &g_rtv );
				back_buffer->Release( );
			}
			if ( SUCCEEDED( swap_chain->GetDesc( &sd ) ) ) {
				if ( sd.BufferDesc.Width > 0 && sd.BufferDesc.Height > 0 ) {
					g_width = static_cast< float >( sd.BufferDesc.Width );
					g_height = static_cast< float >( sd.BufferDesc.Height );
				}
			}
		}

		if ( !g_rtv || !init_pipeline( ) ) {
			clear_batches( );
			return;
		}

		if ( g_color_count == 0 && g_text_count == 0 && g_tex_quad_count == 0 )
			return;

		ID3D11RenderTargetView* prev_rtv = nullptr;
		ID3D11DepthStencilView* prev_dsv = nullptr;
		ID3D11BlendState* prev_blend = nullptr;
		float prev_blend_factor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
		UINT prev_sample_mask = 0xFFFFFFFF;
		ID3D11DepthStencilState* prev_depth = nullptr;
		UINT prev_stencil_ref = 0;
		ID3D11RasterizerState* prev_rast = nullptr;
		D3D11_VIEWPORT prev_vps[ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE ]{};
		UINT prev_vp_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
		ID3D11InputLayout* prev_layout = nullptr;
		D3D11_PRIMITIVE_TOPOLOGY prev_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ID3D11VertexShader* prev_vs = nullptr;
		ID3D11PixelShader* prev_ps = nullptr;
		ID3D11ShaderResourceView* prev_srv = nullptr;
		ID3D11SamplerState* prev_sampler = nullptr;

		g_context->OMGetRenderTargets( 1, &prev_rtv, &prev_dsv );
		g_context->OMGetBlendState( &prev_blend, prev_blend_factor, &prev_sample_mask );
		g_context->OMGetDepthStencilState( &prev_depth, &prev_stencil_ref );
		g_context->RSGetState( &prev_rast );
		g_context->RSGetViewports( &prev_vp_count, prev_vps );
		g_context->IAGetInputLayout( &prev_layout );
		g_context->IAGetPrimitiveTopology( &prev_topology );
		g_context->VSGetShader( &prev_vs, nullptr, nullptr );
		g_context->PSGetShader( &prev_ps, nullptr, nullptr );
		g_context->PSGetShaderResources( 0, 1, &prev_srv );
		g_context->PSGetSamplers( 0, 1, &prev_sampler );

		g_context->OMSetRenderTargets( 1, &g_rtv, nullptr );
		D3D11_VIEWPORT vp{ 0.f, 0.f, g_width, g_height, 0.f, 1.f };
		g_context->RSSetViewports( 1, &vp );
		g_context->RSSetState( g_rast );
		g_context->OMSetDepthStencilState( g_depth, 0 );
		float blend_factor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
		g_context->OMSetBlendState( g_blend, blend_factor, 0xFFFFFFFF );

		flush_batch( g_color_verts, g_color_count, g_ps_color );
		g_context->PSSetShaderResources( 0, 1, &g_font_srv );
		g_context->PSSetSamplers( 0, 1, &g_sampler );
		flush_batch( g_text_verts, g_text_count, g_ps_tex );

		for ( int i = 1; i < g_tex_quad_count; ++i ) {
			tex_quad_t key = g_tex_quads[ i ];
			int j = i - 1;
			while ( j >= 0 && reinterpret_cast< uintptr_t >( g_tex_quads[ j ].srv ) > reinterpret_cast< uintptr_t >( key.srv ) ) {
				g_tex_quads[ j + 1 ] = g_tex_quads[ j ];
				--j;
			}
			g_tex_quads[ j + 1 ] = key;
		}
		for ( int i = 0; i < g_tex_quad_count; ++i ) {
			mem_cpy( &g_tex_upload_verts[ i * 6 ], g_tex_quads[ i ].verts, sizeof( vertex_t ) * 6 );
		}
		if ( g_tex_quad_count > 0 && g_vb ) {
			D3D11_MAPPED_SUBRESOURCE mapped{};
			if ( SUCCEEDED( g_context->Map( g_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped ) ) ) {
				mem_cpy( mapped.pData, g_tex_upload_verts, static_cast< size_t >( g_tex_quad_count * 6 ) * sizeof( vertex_t ) );
				g_context->Unmap( g_vb, 0 );
			}
			UINT stride = sizeof( vertex_t );
			UINT offset = 0;
			g_context->IASetVertexBuffers( 0, 1, &g_vb, &stride, &offset );
			g_context->IASetInputLayout( g_layout );
			g_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			g_context->VSSetShader( g_vs, nullptr, 0 );
			g_context->PSSetShader( g_ps_tex, nullptr, 0 );
			ID3D11ShaderResourceView* last_srv = nullptr;
			int batch_start = 0;
			for ( int i = 0; i <= g_tex_quad_count; ++i ) {
				ID3D11ShaderResourceView* cur_srv = ( i < g_tex_quad_count ) ? g_tex_quads[ i ].srv : nullptr;
				if ( i == g_tex_quad_count || cur_srv != last_srv ) {
					if ( last_srv && i > batch_start ) {
						g_context->PSSetShaderResources( 0, 1, &last_srv );
						g_context->Draw( static_cast< UINT >( ( i - batch_start ) * 6 ), static_cast< UINT >( batch_start * 6 ) );
					}
					last_srv = cur_srv;
					batch_start = i;
				}
			}
		}

		clear_batches( );

		g_context->OMSetRenderTargets( 1, &prev_rtv, prev_dsv );
		g_context->OMSetBlendState( prev_blend, prev_blend_factor, prev_sample_mask );
		g_context->OMSetDepthStencilState( prev_depth, prev_stencil_ref );
		g_context->RSSetState( prev_rast );
		if ( prev_vp_count > 0 ) g_context->RSSetViewports( prev_vp_count, prev_vps );
		g_context->IASetInputLayout( prev_layout );
		g_context->IASetPrimitiveTopology( prev_topology );
		g_context->VSSetShader( prev_vs, nullptr, 0 );
		g_context->PSSetShader( prev_ps, nullptr, 0 );
		g_context->PSSetShaderResources( 0, 1, &prev_srv );
		g_context->PSSetSamplers( 0, 1, &prev_sampler );

		if ( prev_rtv ) prev_rtv->Release( );
		if ( prev_dsv ) prev_dsv->Release( );
		if ( prev_blend ) prev_blend->Release( );
		if ( prev_depth ) prev_depth->Release( );
		if ( prev_rast ) prev_rast->Release( );
		if ( prev_layout ) prev_layout->Release( );
		if ( prev_vs ) prev_vs->Release( );
		if ( prev_ps ) prev_ps->Release( );
		if ( prev_srv ) prev_srv->Release( );
		if ( prev_sampler ) prev_sampler->Release( );
	}
}
