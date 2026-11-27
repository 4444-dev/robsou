// orelia-internal/workspace/game/unity_il2cpp.hxx
#pragma once

namespace unity {
	struct vec3_t { float x, y, z; };
	struct mat4_t { float m[ 16 ]; };

	using domain_get_fn = void*( * )( );
	using thread_attach_fn = void*( * )( void* );
	using domain_get_assemblies_fn = void**( * )( void*, size_t* );
	using assembly_get_image_fn = void*( * )( void* );
	using image_get_name_fn = const char*( * )( void* );
	using class_from_name_fn = void*( * )( void*, const char*, const char* );
	using class_get_method_fn = void*( * )( void*, const char*, int );
	using runtime_invoke_fn = void*( * )( void*, void*, void**, void** );
	using object_unbox_fn = void*( * )( void* );
	using object_get_class_fn = void*( * )( void* );
	using class_get_name_fn = const char*( * )( void* );
	using class_get_type_fn = void*( * )( void* );
	using type_get_obj_fn = void*( * )( void* );

	// Direct native IL2CPP method signatures (zero boxing / zero GC allocation)
	using cam_get_main_fn = void*( * )( void* method );
	using cam_mat_inj_fn = void( * )( uintptr_t native_cam, mat4_t* out, void* method );
	using cam_get_fov_fn = float( * )( void* cam, void* method );
	using cam_set_fov_fn = void( * )( void* cam, float fov, void* method );
	using comp_get_tr_fn = void*( * )( void* comp, void* method );
	using tr_pos_inj_fn = void( * )( uintptr_t native_tr, vec3_t* out, void* method );
	using tr_set_pos_inj_fn = void( * )( uintptr_t native_tr, const vec3_t* val, void* method );
	using anim_get_bone_fn = void*( * )( void* animator, int bone_id, void* method );
	using tr_get_parent_fn = void*( * )( void* tr, void* method );
	using tr_child_count_fn = int( * )( void* tr, void* method );
	using tr_get_child_fn = void*( * )( void* tr, int index, void* method );
	using phys_linecast_fn = bool( * )( const vec3_t* start, const vec3_t* end, int layer_mask, int query_trigger, void* method );

	inline bool g_ready = false;
	inline bool g_scene_ready_cached = false;
	inline HMODULE g_game_assembly = nullptr;

	// Core IL2CPP API
	inline domain_get_fn f_domain_get = nullptr;
	inline thread_attach_fn f_thread_attach = nullptr;
	inline domain_get_assemblies_fn f_domain_asms = nullptr;
	inline assembly_get_image_fn f_asm_image = nullptr;
	inline image_get_name_fn f_img_name = nullptr;
	inline class_from_name_fn f_cls_from_name = nullptr;
	inline class_get_method_fn f_cls_method = nullptr;
	inline runtime_invoke_fn f_invoke = nullptr;
	inline object_unbox_fn f_unbox = nullptr;
	inline object_get_class_fn f_obj_class = nullptr;
	inline class_get_name_fn f_cls_name = nullptr;
	inline class_get_type_fn f_cls_type = nullptr;
	inline type_get_obj_fn f_type_obj = nullptr;

	// MethodInfo* descriptors
	inline void* m_cam_get_main = nullptr;
	inline void* m_cam_w2c_inj = nullptr;
	inline void* m_cam_proj_inj = nullptr;
	inline void* m_cam_get_fov = nullptr;
	inline void* m_cam_set_fov = nullptr;
	inline void* m_comp_get_transform = nullptr;
	inline void* m_tr_pos_inj = nullptr;
	inline void* m_tr_set_pos_inj = nullptr;
	inline void* m_obj_find_all = nullptr;
	inline void* m_anim_get_bone = nullptr;
	inline void* m_tr_get_parent = nullptr;
	inline void* m_tr_child_count = nullptr;
	inline void* m_tr_get_child = nullptr;
	inline void* m_phys_linecast = nullptr;

	// Direct native function pointers
	inline cam_get_main_fn fn_cam_get_main = nullptr;
	inline cam_mat_inj_fn fn_cam_w2c_inj = nullptr;
	inline cam_mat_inj_fn fn_cam_proj_inj = nullptr;
	inline cam_get_fov_fn fn_cam_get_fov = nullptr;
	inline cam_set_fov_fn fn_cam_set_fov = nullptr;
	inline comp_get_tr_fn fn_comp_get_tr = nullptr;
	inline tr_pos_inj_fn fn_tr_pos_inj = nullptr;
	inline tr_set_pos_inj_fn fn_tr_set_pos_inj = nullptr;
	inline anim_get_bone_fn fn_anim_get_bone = nullptr;
	inline tr_get_parent_fn fn_tr_get_parent = nullptr;
	inline tr_child_count_fn fn_tr_child_count = nullptr;
	inline tr_get_child_fn fn_tr_get_child = nullptr;
	inline phys_linecast_fn fn_phys_linecast = nullptr;

	// Cached RuntimeType objects
	inline void* g_char_type_obj = nullptr;
	inline void* g_entry_type_obj = nullptr;

	// Cached UnityEntry singleton
	inline uintptr_t g_unity_entry_obj = 0;

	// Thread attachment tracking
	inline DWORD g_attached_tids[ 128 ]{};
	inline int g_attached_tid_count = 0;
	inline volatile LONG g_attach_lock = 0;

	// Page access cache: bit 0 = readable, bit 1 = writable.
	// VirtualQuery is a kernel transition and the ESP path probes thousands of
	// managed pointers per frame, so it is only called once per page.
	constexpr uint8_t page_readable = 0x1;
	constexpr uint8_t page_writable = 0x2;
	inline uintptr_t g_page_cache_keys[ 8192 ]{};
	inline uint8_t g_page_cache_vals[ 8192 ]{};

	inline uint8_t page_flags( uintptr_t page ) {
		page &= ~static_cast< uintptr_t >( 0xFFF );
		if ( page < 0x10000ULL || page >= 0x00007FFFFFFF0000ULL ) return 0;
		const uintptr_t idx = ( page >> 12 ) & 8191ULL;
		if ( g_page_cache_keys[ idx ] == page ) return g_page_cache_vals[ idx ];

		uint8_t flags = 0;
		MEMORY_BASIC_INFORMATION mbi{};
		if ( VirtualQuery( reinterpret_cast< LPCVOID >( page ), &mbi, sizeof( mbi ) ) == sizeof( mbi ) &&
			 mbi.State == MEM_COMMIT && !( mbi.Protect & PAGE_GUARD ) ) {
			const DWORD p = mbi.Protect & 0xFF;
			if ( p == PAGE_READONLY || p == PAGE_READWRITE || p == PAGE_WRITECOPY ||
				 p == PAGE_EXECUTE_READ || p == PAGE_EXECUTE_READWRITE || p == PAGE_EXECUTE_WRITECOPY ) {
				flags |= page_readable;
			}
			if ( p == PAGE_READWRITE || p == PAGE_WRITECOPY ||
				 p == PAGE_EXECUTE_READWRITE || p == PAGE_EXECUTE_WRITECOPY ) {
				flags |= page_writable;
			}
		}
		g_page_cache_keys[ idx ] = page;
		g_page_cache_vals[ idx ] = flags;
		return flags;
	}

	inline char lower_ch( char c ) {
		return ( c >= 'A' && c <= 'Z' ) ? static_cast< char >( c + 32 ) : c;
	}

	inline bool str_eq( const char* a, const char* b ) {
		if ( !a || !b ) return false;
		while ( *a && *b ) {
			if ( *a != *b ) return false;
			++a;
			++b;
		}
		return *a == '\0' && *b == '\0';
	}

	// Case-insensitive compare, used for assembly / image names which come from
	// the loader and are not guaranteed to keep the casing of the file on disk.
	inline bool str_ieq( const char* a, const char* b ) {
		if ( !a || !b ) return false;
		while ( *a && *b ) {
			if ( lower_ch( *a ) != lower_ch( *b ) ) return false;
			++a;
			++b;
		}
		return *a == '\0' && *b == '\0';
	}

	inline bool str_icontains( const char* hay, const char* needle ) {
		if ( !hay || !needle || !*needle ) return false;
		auto to_low = []( char c ) -> char {
			return ( c >= 'A' && c <= 'Z' ) ? static_cast< char >( c + 32 ) : c;
		};
		for ( int i = 0; hay[ i ]; ++i ) {
			int j = 0;
			while ( needle[ j ] && to_low( hay[ i + j ] ) == to_low( needle[ j ] ) ) ++j;
			if ( !needle[ j ] ) return true;
		}
		return false;
	}

	inline bool is_page_readable( uintptr_t page ) {
		return ( page_flags( page ) & page_readable ) != 0;
	}

	inline bool is_page_writable( uintptr_t page ) {
		return ( page_flags( page ) & page_writable ) != 0;
	}

	inline bool is_mem_readable( uintptr_t addr, size_t size ) {
		if ( addr < 0x10000ULL || size == 0 || size > 0x1000000ULL || ( addr + size ) >= 0x00007FFFFFFF0000ULL )
			return false;
		const uintptr_t first_page = addr & ~0xFFFULL;
		const uintptr_t last_page = ( addr + size - 1 ) & ~0xFFFULL;
		for ( uintptr_t p = first_page; p <= last_page; p += 0x1000ULL ) {
			if ( !is_page_readable( p ) ) return false;
		}
		return true;
	}

	inline void reset_page_cache( ) {
		for ( int i = 0; i < 8192; ++i ) {
			g_page_cache_keys[ i ] = 0;
			g_page_cache_vals[ i ] = 0;
		}
	}

	template< typename T >
	inline bool safe_read( uintptr_t addr, T& out ) {
		if ( !is_mem_readable( addr, sizeof( T ) ) ) return false;
		out = *reinterpret_cast< const T* >( addr );
		return true;
	}

	template< typename T >
	inline bool safe_write( uintptr_t addr, const T& val ) {
		if ( !is_mem_readable( addr, sizeof( T ) ) ) return false;
		if ( !is_page_writable( addr & ~0xFFFULL ) ) return false;
		*reinterpret_cast< T* >( addr ) = val;
		return true;
	}

	inline uintptr_t read_u64( uintptr_t addr ) {
		uintptr_t val = 0;
		safe_read( addr, val );
		return val;
	}

	inline int32_t read_i32( uintptr_t addr ) {
		int32_t val = 0;
		safe_read( addr, val );
		return val;
	}

	inline float read_f32( uintptr_t addr ) {
		float val = 0.0f;
		safe_read( addr, val );
		return val;
	}

	inline uint8_t read_u8( uintptr_t addr ) {
		uint8_t val = 0;
		safe_read( addr, val );
		return val;
	}

	inline bool is_managed_ptr( uintptr_t addr ) {
		if ( addr < 0x10000 || addr > 0x00007FFFFFFFFFFF || ( addr & 0x7 ) != 0 )
			return false;
		if ( !is_mem_readable( addr, 0x10 ) )
			return false;
		uintptr_t klass = *reinterpret_cast< uintptr_t* >( addr );
		if ( klass < 0x10000 || klass > 0x00007FFFFFFFFFFF || ( klass & 0x7 ) != 0 )
			return false;
		if ( !is_mem_readable( klass, 0x18 ) )
			return false;
		uintptr_t image = *reinterpret_cast< uintptr_t* >( klass );
		return image >= 0x10000 && image <= 0x00007FFFFFFFFFFF && ( image & 0x7 ) == 0;
	}

	inline bool is_managed_ptr( void* obj ) {
		return is_managed_ptr( reinterpret_cast< uintptr_t >( obj ) );
	}

	inline bool is_valid_obj( void* obj ) {
		if ( !is_managed_ptr( obj ) ) return false;
		if ( !is_mem_readable( reinterpret_cast< uintptr_t >( obj ) + 0x10, sizeof( void* ) ) ) return false;
		const uintptr_t cached = *reinterpret_cast< uintptr_t* >( reinterpret_cast< uintptr_t >( obj ) + 0x10 );
		return cached >= 0x10000ULL && cached < 0x00007FFFFFFF0000ULL;
	}

	inline bool get_class_name( uintptr_t obj, char* out, size_t max_len ) {
		if ( !out || max_len == 0 ) return false;
		out[ 0 ] = '\0';
		if ( !is_managed_ptr( obj ) ) return false;
		uintptr_t klass = *reinterpret_cast< uintptr_t* >( obj );
		uintptr_t name_ptr = read_u64( klass + 0x10 );
		if ( name_ptr < 0x10000 || name_ptr > 0x00007FFFFFFFFFFF ) return false;
		if ( !is_page_readable( name_ptr & ~0xFFFULL ) ) return false;
		const char* s = reinterpret_cast< const char* >( name_ptr );
		size_t i = 0;
		while ( i + 1 < max_len ) {
			if ( ( ( name_ptr + i ) & 0xFFFULL ) == 0 && !is_page_readable( ( name_ptr + i ) & ~0xFFFULL ) )
				break;
			char c = s[ i ];
			if ( c == '\0' ) {
				out[ i ] = '\0';
				return i > 0;
			}
			if ( static_cast< unsigned char >( c ) < 32 || static_cast< unsigned char >( c ) > 126 )
				return false;
			out[ i ] = c;
			++i;
		}
		out[ i ] = '\0';
		return i > 0;
	}

	inline bool obj_is( uintptr_t obj, const char* want ) {
		char nm[ 96 ]{};
		if ( !get_class_name( obj, nm, sizeof( nm ) ) ) return false;
		return str_eq( nm, want );
	}

	// Substring match on the class name: il2cpp_class_get_name drops the namespace,
	// so the exact name of a model is not knowable in advance, but the word that
	// identifies it usually is.
	inline bool obj_contains( uintptr_t obj, const char* want ) {
		char nm[ 96 ]{};
		if ( !get_class_name( obj, nm, sizeof( nm ) ) ) return false;
		return str_icontains( nm, want );
	}

	inline int read_il2cpp_string( uintptr_t str_obj, char* out, size_t max_len ) {
		if ( !out || max_len == 0 ) return 0;
		out[ 0 ] = '\0';
		if ( !is_managed_ptr( str_obj ) || !is_mem_readable( str_obj + 0x10, 8 ) ) return 0;
		int32_t len = *reinterpret_cast< int32_t* >( str_obj + 0x10 );
		if ( len <= 0 || len > 256 ) return 0;
		if ( !is_mem_readable( str_obj + 0x14, static_cast< size_t >( len ) * 2 ) ) return 0;
		const wchar_t* wbuf = reinterpret_cast< const wchar_t* >( str_obj + 0x14 );
		int written = WideCharToMultiByte( CP_UTF8, 0, wbuf, len, out, static_cast< int >( max_len ) - 1, nullptr, nullptr );
		if ( written <= 0 ) {
			out[ 0 ] = '\0';
			return 0;
		}
		out[ written < static_cast< int >( max_len ) - 1 ? written : static_cast< int >( max_len ) - 1 ] = '\0';
		return written;
	}

	template< typename T >
	inline bool read_prim_data( uintptr_t pdata, T& out ) {
		if ( !is_managed_ptr( pdata ) ) return false;
		return safe_read( pdata + 0x18, out );
	}

	template< typename T >
	inline bool read_prim_model( uintptr_t pmodel, T& out ) {
		if ( !is_managed_ptr( pmodel ) ) return false;
		uintptr_t pdata = read_u64( pmodel + 0x10 );
		return read_prim_data( pdata, out );
	}

	inline int read_enum_id( uintptr_t enum_obj, char* out, size_t max_len ) {
		if ( !out || max_len == 0 ) return 0;
		out[ 0 ] = '\0';
		if ( !is_managed_ptr( enum_obj ) ) return 0;
		uintptr_t id_str = read_u64( enum_obj + 0x10 );
		return read_il2cpp_string( id_str, out, max_len );
	}

	inline int read_dict( uintptr_t dict, uintptr_t& entries_data ) {
		entries_data = 0;
		if ( !is_managed_ptr( dict ) || !is_mem_readable( dict + 0x18, 0x10 ) ) return 0;
		uintptr_t ent_arr = *reinterpret_cast< uintptr_t* >( dict + 0x18 );
		int32_t count = *reinterpret_cast< int32_t* >( dict + 0x20 );
		if ( count <= 0 || count > 4096 || !is_managed_ptr( ent_arr ) ) return 0;
		if ( !is_mem_readable( ent_arr + 0x18, 8 ) ) return 0;
		uintptr_t cap = *reinterpret_cast< uintptr_t* >( ent_arr + 0x18 );
		if ( cap < static_cast< uintptr_t >( count ) || cap > 16384 ) return 0;
		if ( !is_mem_readable( ent_arr + 0x20, static_cast< size_t >( count ) * 0x18 ) ) return 0;
		entries_data = ent_arr + 0x20;
		return count;
	}

	inline void ensure_thread_attached( ) {
		if ( !f_domain_get || !f_thread_attach ) return;
		DWORD tid = GetCurrentThreadId( );
		while ( InterlockedCompareExchange( &g_attach_lock, 1, 0 ) != 0 ) {
			YieldProcessor( );
		}
		for ( int i = 0; i < g_attached_tid_count; ++i ) {
			if ( g_attached_tids[ i ] == tid ) {
				InterlockedExchange( &g_attach_lock, 0 );
				return;
			}
		}
		InterlockedExchange( &g_attach_lock, 0 );

		void* dom = f_domain_get( );
		if ( dom ) {
			f_thread_attach( dom );
			while ( InterlockedCompareExchange( &g_attach_lock, 1, 0 ) != 0 ) {
				YieldProcessor( );
			}
			bool found = false;
			for ( int i = 0; i < g_attached_tid_count; ++i ) {
				if ( g_attached_tids[ i ] == tid ) { found = true; break; }
			}
			if ( !found && g_attached_tid_count < 128 ) {
				g_attached_tids[ g_attached_tid_count++ ] = tid;
			}
			InterlockedExchange( &g_attach_lock, 0 );
		}
	}

	inline void* resolve_type_obj( void* img, const char* ns, const char* name ) {
		if ( !img || !f_cls_from_name || !f_cls_type || !f_type_obj ) return nullptr;
		void* cls = f_cls_from_name( img, ns, name );
		if ( !cls ) return nullptr;
		void* t = f_cls_type( cls );
		return t ? f_type_obj( t ) : nullptr;
	}

	template< typename Fn >
	inline Fn extract_native_fn( void* method_info ) {
		if ( !method_info || !is_mem_readable( reinterpret_cast< uintptr_t >( method_info ), sizeof( void* ) ) )
			return nullptr;
		void* ptr = *reinterpret_cast< void** >( method_info );
		uintptr_t addr = reinterpret_cast< uintptr_t >( ptr );
		if ( addr < 0x10000 || addr > 0x00007FFFFFFFFFFF ) return nullptr;
		return reinterpret_cast< Fn >( ptr );
	}

	// Zero-GC direct native helpers
	inline void* get_main_camera( ) {
		if ( !fn_cam_get_main || !m_cam_get_main ) return nullptr;
		return fn_cam_get_main( m_cam_get_main );
	}

	inline void* get_component_transform( void* comp ) {
		if ( !fn_comp_get_tr || !m_comp_get_transform || !is_valid_obj( comp ) ) return nullptr;
		return fn_comp_get_tr( comp, m_comp_get_transform );
	}

	inline bool get_transform_pos_native( uintptr_t native_tr, vec3_t& out ) {
		if ( !fn_tr_pos_inj || !m_tr_pos_inj || native_tr < 0x10000 ) return false;
		fn_tr_pos_inj( native_tr, &out, m_tr_pos_inj );
		return true;
	}

	inline bool get_transform_pos( void* managed_tr, vec3_t& out ) {
		if ( !is_valid_obj( managed_tr ) ) return false;
		const uintptr_t native_tr = read_u64( reinterpret_cast< uintptr_t >( managed_tr ) + 0x10 );
		return get_transform_pos_native( native_tr, out );
	}

	inline bool get_camera_matrices( void* cam, mat4_t& out_view, mat4_t& out_proj ) {
		if ( !fn_cam_w2c_inj || !fn_cam_proj_inj || !m_cam_w2c_inj || !m_cam_proj_inj || !is_valid_obj( cam ) )
			return false;
		const uintptr_t native_cam = read_u64( reinterpret_cast< uintptr_t >( cam ) + 0x10 );
		if ( native_cam < 0x10000 ) return false;
		fn_cam_w2c_inj( native_cam, &out_view, m_cam_w2c_inj );
		fn_cam_proj_inj( native_cam, &out_proj, m_cam_proj_inj );
		return true;
	}

	inline void* tr_get_parent( void* tr ) {
		if ( !fn_tr_get_parent || !m_tr_get_parent || !is_valid_obj( tr ) ) return nullptr;
		void* parent = fn_tr_get_parent( tr, m_tr_get_parent );
		return is_valid_obj( parent ) ? parent : nullptr;
	}

	inline int tr_child_count( void* tr ) {
		if ( !fn_tr_child_count || !m_tr_child_count || !is_valid_obj( tr ) ) return 0;
		const int count = fn_tr_child_count( tr, m_tr_child_count );
		return ( count > 0 && count < 64 ) ? count : 0;
	}

	inline void* tr_get_child( void* tr, int index ) {
		if ( !fn_tr_get_child || !m_tr_get_child || !is_valid_obj( tr ) ) return nullptr;
		void* child = fn_tr_get_child( tr, index, m_tr_get_child );
		return is_valid_obj( child ) ? child : nullptr;
	}

	inline void* get_animator_bone( void* animator, int human_bone_id ) {
		if ( !fn_anim_get_bone || !m_anim_get_bone || !is_valid_obj( animator ) ) return nullptr;
		void* tr = fn_anim_get_bone( animator, human_bone_id, m_anim_get_bone );
		return is_valid_obj( tr ) ? tr : nullptr;
	}

	inline bool set_transform_pos( void* managed_tr, const vec3_t& pos ) {
		if ( !fn_tr_set_pos_inj || !m_tr_set_pos_inj || !is_valid_obj( managed_tr ) ) return false;
		const uintptr_t native_tr = read_u64( reinterpret_cast< uintptr_t >( managed_tr ) + 0x10 );
		if ( native_tr < 0x10000 ) return false;
		fn_tr_set_pos_inj( native_tr, &pos, m_tr_set_pos_inj );
		return true;
	}

	inline bool unity_linecast( vec3_t a, vec3_t b, int mask = -5 ) {
		if ( !m_phys_linecast || !f_invoke ) return false;
		int qti = 1; // QueryTriggerInteraction.Ignore
		void* args[ 4 ] = { &a, &b, &mask, &qti };
		void* exc = nullptr;
		void* res = f_invoke( m_phys_linecast, nullptr, args, &exc );
		if ( exc || !res || !f_unbox ) return false;
		return *reinterpret_cast< bool* >( f_unbox( res ) );
	}

	inline bool init( ) {
		if ( g_ready ) {
			ensure_thread_attached( );
			return true;
		}

		HMODULE ga = GetModuleHandleA( "GameAssembly.dll" );
		if ( !ga ) return false;
		g_game_assembly = ga;

		f_domain_get = reinterpret_cast< domain_get_fn >( GetProcAddress( ga, "il2cpp_domain_get" ) );
		f_thread_attach = reinterpret_cast< thread_attach_fn >( GetProcAddress( ga, "il2cpp_thread_attach" ) );
		f_domain_asms = reinterpret_cast< domain_get_assemblies_fn >( GetProcAddress( ga, "il2cpp_domain_get_assemblies" ) );
		f_asm_image = reinterpret_cast< assembly_get_image_fn >( GetProcAddress( ga, "il2cpp_assembly_get_image" ) );
		f_img_name = reinterpret_cast< image_get_name_fn >( GetProcAddress( ga, "il2cpp_image_get_name" ) );
		f_cls_from_name = reinterpret_cast< class_from_name_fn >( GetProcAddress( ga, "il2cpp_class_from_name" ) );
		f_cls_method = reinterpret_cast< class_get_method_fn >( GetProcAddress( ga, "il2cpp_class_get_method_from_name" ) );
		f_invoke = reinterpret_cast< runtime_invoke_fn >( GetProcAddress( ga, "il2cpp_runtime_invoke" ) );
		f_unbox = reinterpret_cast< object_unbox_fn >( GetProcAddress( ga, "il2cpp_object_unbox" ) );
		f_obj_class = reinterpret_cast< object_get_class_fn >( GetProcAddress( ga, "il2cpp_object_get_class" ) );
		f_cls_name = reinterpret_cast< class_get_name_fn >( GetProcAddress( ga, "il2cpp_class_get_name" ) );
		f_cls_type = reinterpret_cast< class_get_type_fn >( GetProcAddress( ga, "il2cpp_class_get_type" ) );
		f_type_obj = reinterpret_cast< type_get_obj_fn >( GetProcAddress( ga, "il2cpp_type_get_object" ) );

		if ( !f_domain_get || !f_thread_attach || !f_domain_asms || !f_asm_image ||
			 !f_img_name || !f_cls_from_name || !f_cls_method || !f_invoke || !f_unbox ) {
			return false;
		}

		ensure_thread_attached( );

		void* dom = f_domain_get( );
		if ( !dom ) return false;

		size_t count = 0;
		void** asms = f_domain_asms( dom, &count );
		if ( !asms || count == 0 ) return false;

		void* core_img = nullptr;
		void* anim_img = nullptr;
		void* phys_img = nullptr;
		void* battle_img = nullptr;
		void* start_img = nullptr;
		for ( size_t i = 0; i < count; ++i ) {
			void* img = f_asm_image( asms[ i ] );
			if ( !img ) continue;
			const char* n = f_img_name( img );
			if ( str_eq( n, "UnityEngine.CoreModule.dll" ) ) core_img = img;
			else if ( str_eq( n, "UnityEngine.AnimationModule.dll" ) ) anim_img = img;
			else if ( str_eq( n, "UnityEngine.PhysicsModule.dll" ) ) phys_img = img;
			else if ( str_eq( n, "Battle.dll" ) || str_eq( n, "Game.Battle.dll" ) ) battle_img = img;
			else if ( str_eq( n, "Start.dll" ) || str_eq( n, "Bootstrap.dll" ) || str_eq( n, "Game.dll" ) || str_eq( n, "Assembly-CSharp.dll" ) ) {
				if ( !start_img && f_cls_from_name( img, "Start", "UnityEntry" ) )
					start_img = img;
			}
		}
		if ( !core_img ) return false;

		if ( !start_img ) {
			for ( size_t i = 0; i < count; ++i ) {
				void* img = f_asm_image( asms[ i ] );
				if ( !img ) continue;
				if ( f_cls_from_name( img, "Start", "UnityEntry" ) ) {
					start_img = img;
					break;
				}
			}
		}

		void* cam_cls = f_cls_from_name( core_img, "UnityEngine", "Camera" );
		void* comp_cls = f_cls_from_name( core_img, "UnityEngine", "Component" );
		void* tr_cls = f_cls_from_name( core_img, "UnityEngine", "Transform" );
		void* obj_cls = f_cls_from_name( core_img, "UnityEngine", "Object" );

		if ( cam_cls ) {
			m_cam_get_main = f_cls_method( cam_cls, "get_main", 0 );
			m_cam_w2c_inj = f_cls_method( cam_cls, "get_worldToCameraMatrix_Injected", 2 );
			m_cam_proj_inj = f_cls_method( cam_cls, "get_projectionMatrix_Injected", 2 );
			m_cam_get_fov = f_cls_method( cam_cls, "get_fieldOfView", 0 );
			m_cam_set_fov = f_cls_method( cam_cls, "set_fieldOfView", 1 );
			fn_cam_get_main = extract_native_fn< cam_get_main_fn >( m_cam_get_main );
			fn_cam_w2c_inj = extract_native_fn< cam_mat_inj_fn >( m_cam_w2c_inj );
			fn_cam_proj_inj = extract_native_fn< cam_mat_inj_fn >( m_cam_proj_inj );
			fn_cam_get_fov = extract_native_fn< cam_get_fov_fn >( m_cam_get_fov );
			fn_cam_set_fov = extract_native_fn< cam_set_fov_fn >( m_cam_set_fov );
		}
		if ( comp_cls ) {
			m_comp_get_transform = f_cls_method( comp_cls, "get_transform", 0 );
			fn_comp_get_tr = extract_native_fn< comp_get_tr_fn >( m_comp_get_transform );
		}
		if ( tr_cls ) {
			m_tr_pos_inj = f_cls_method( tr_cls, "get_position_Injected", 2 );
			m_tr_set_pos_inj = f_cls_method( tr_cls, "set_position_Injected", 2 );
			fn_tr_pos_inj = extract_native_fn< tr_pos_inj_fn >( m_tr_pos_inj );
			fn_tr_set_pos_inj = extract_native_fn< tr_set_pos_inj_fn >( m_tr_set_pos_inj );

			// Hierarchy access: how the skeleton is read without any offset guess.
			m_tr_get_parent = f_cls_method( tr_cls, "get_parent", 0 );
			m_tr_child_count = f_cls_method( tr_cls, "get_childCount", 0 );
			m_tr_get_child = f_cls_method( tr_cls, "GetChild", 1 );
			fn_tr_get_parent = extract_native_fn< tr_get_parent_fn >( m_tr_get_parent );
			fn_tr_child_count = extract_native_fn< tr_child_count_fn >( m_tr_child_count );
			fn_tr_get_child = extract_native_fn< tr_get_child_fn >( m_tr_get_child );
		}
		if ( phys_img ) {
			void* phys_cls = f_cls_from_name( phys_img, "UnityEngine", "Physics" );
			if ( phys_cls ) {
				m_phys_linecast = f_cls_method( phys_cls, "Linecast", 4 );
				fn_phys_linecast = extract_native_fn< phys_linecast_fn >( m_phys_linecast );
			}
		}
		if ( obj_cls ) {
			m_obj_find_all = f_cls_method( obj_cls, "FindObjectsOfType", 1 );
		}

		if ( anim_img ) {
			void* anim_cls = f_cls_from_name( anim_img, "UnityEngine", "Animator" );
			if ( anim_cls ) {
				m_anim_get_bone = f_cls_method( anim_cls, "GetBoneTransformInternal", 1 );
				if ( !m_anim_get_bone ) {
					m_anim_get_bone = f_cls_method( anim_cls, "GetBoneTransform", 1 );
				}
				fn_anim_get_bone = extract_native_fn< anim_get_bone_fn >( m_anim_get_bone );
			}
		}

		if ( battle_img ) {
			g_char_type_obj = resolve_type_obj( battle_img, "Game.Battle.EntityModel.BattleCharacter.Views", "BattleCharacterEntityView" );
		}
		if ( start_img ) {
			g_entry_type_obj = resolve_type_obj( start_img, "Start", "UnityEntry" );
		}

		g_ready = ( fn_cam_get_main != nullptr && fn_cam_w2c_inj != nullptr && fn_cam_proj_inj != nullptr && fn_tr_pos_inj != nullptr );
		return g_ready;
	}

	inline bool is_scene_ready( ) {
		if ( !g_ready || !fn_cam_get_main ) {
			g_scene_ready_cached = false;
			return false;
		}
		void* cam = get_main_camera( );
		g_scene_ready_cached = is_valid_obj( cam );
		return g_scene_ready_cached;
	}

	inline bool resolve_battle_models( uintptr_t& out_entities_model, uintptr_t& out_slots_model, uintptr_t* out_battle_model = nullptr ) {
		out_entities_model = 0;
		out_slots_model = 0;
		if ( out_battle_model ) *out_battle_model = 0;
		if ( !g_ready || !g_entry_type_obj || !m_obj_find_all || !f_invoke ) return false;

		if ( !is_valid_obj( reinterpret_cast< void* >( g_unity_entry_obj ) ) ) {
			g_unity_entry_obj = 0;
			void* args[ 1 ] = { g_entry_type_obj };
			void* exc = nullptr;
			void* arr = f_invoke( m_obj_find_all, nullptr, args, &exc );
			if ( !exc && is_managed_ptr( arr ) ) {
				struct il2cpp_arr_t { void* k; void* m; uintptr_t b; uintptr_t len; void* data[ 1 ]; };
				auto* a = reinterpret_cast< il2cpp_arr_t* >( arr );
				if ( a->len > 0 && is_valid_obj( a->data[ 0 ] ) ) {
					g_unity_entry_obj = reinterpret_cast< uintptr_t >( a->data[ 0 ] );
				}
			}
		}
		if ( !g_unity_entry_obj ) return false;

		const uintptr_t game_obj = read_u64( g_unity_entry_obj + 0x28 );
		if ( !is_managed_ptr( game_obj ) ) return false;

		const uintptr_t game_model = read_u64( game_obj + 0xD8 );
		if ( !is_managed_ptr( game_model ) ) return false;

		const uintptr_t battle_model = read_u64( game_model + 0x10 );
		if ( !is_managed_ptr( battle_model ) ) return false;

		const uintptr_t entities_model = read_u64( battle_model + 0x90 );
		uintptr_t slots_model = read_u64( battle_model + 0x4D0 );
		if ( !is_managed_ptr( slots_model ) ) slots_model = read_u64( battle_model + 0x248 );
		if ( !is_managed_ptr( entities_model ) ) return false;

		if ( out_battle_model ) *out_battle_model = battle_model;
		out_entities_model = entities_model;
		out_slots_model = is_managed_ptr( slots_model ) ? slots_model : 0;
		return true;
	}
}
