#pragma once
#include <Windows.h>
#include <winternl.h>
#include <cstdint>

namespace sec {
	constexpr std::uint64_t fnv_offset = 14695981039346656037ULL;
	constexpr std::uint64_t fnv_prime  = 1099511628211ULL;

	constexpr std::uint64_t hash_a( const char* s, std::uint64_t h = fnv_offset ) {
		return *s ? hash_a( s + 1, ( h ^ static_cast< std::uint8_t >( *s ) ) * fnv_prime ) : h;
	}

	constexpr std::uint64_t hash_w_lower( const wchar_t* s, std::uint64_t h = fnv_offset ) {
		if ( !*s ) return h;
		wchar_t c = *s;
		if ( c >= L'A' && c <= L'Z' ) c += 32;
		return hash_w_lower( s + 1, ( h ^ static_cast< std::uint8_t >( c & 0xFF ) ) * fnv_prime );
	}

	inline std::uint64_t rt_hash_a( const char* s ) {
		std::uint64_t h = fnv_offset;
		while ( s && *s ) {
			h ^= static_cast< std::uint8_t >( *s++ );
			h *= fnv_prime;
		}
		return h;
	}

	inline std::uint64_t rt_hash_w_lower( const wchar_t* s, std::uint16_t byte_len ) {
		std::uint64_t h = fnv_offset;
		const std::uint16_t chars = byte_len / sizeof( wchar_t );
		for ( std::uint16_t i = 0; i < chars; ++i ) {
			wchar_t c = s[ i ];
			if ( c >= L'A' && c <= L'Z' ) c += 32;
			h ^= static_cast< std::uint8_t >( c & 0xFF );
			h *= fnv_prime;
		}
		return h;
	}

	struct ldr_entry_t {
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		LIST_ENTRY InInitializationOrderLinks;
		PVOID DllBase;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
	};

	inline HMODULE find_module_by_hash( std::uint64_t mod_hash ) {
#ifdef _M_X64
		const auto* peb = reinterpret_cast< const PEB* >( __readgsqword( 0x60 ) );
		if ( !peb || !peb->Ldr ) return nullptr;
		const LIST_ENTRY* head = &peb->Ldr->InMemoryOrderModuleList;
		for ( const LIST_ENTRY* cur = head->Flink; cur && cur != head; cur = cur->Flink ) {
			const auto* ent = CONTAINING_RECORD( cur, ldr_entry_t, InMemoryOrderLinks );
			if ( ent->DllBase && ent->BaseDllName.Buffer && ent->BaseDllName.Length ) {
				if ( rt_hash_w_lower( ent->BaseDllName.Buffer, ent->BaseDllName.Length ) == mod_hash ) {
					return reinterpret_cast< HMODULE >( ent->DllBase );
				}
			}
		}
#endif
		return nullptr;
	}

	inline void* find_export_by_hash( std::uint64_t mod_hash, std::uint64_t fn_hash ) {
		HMODULE mod = find_module_by_hash( mod_hash );
		if ( !mod ) return nullptr;
		const auto base = reinterpret_cast< std::uintptr_t >( mod );
		const auto* dos = reinterpret_cast< const IMAGE_DOS_HEADER* >( base );
		if ( dos->e_magic != IMAGE_DOS_SIGNATURE ) return nullptr;
		const auto* nt = reinterpret_cast< const IMAGE_NT_HEADERS64* >( base + dos->e_lfanew );
		if ( nt->Signature != IMAGE_NT_SIGNATURE ) return nullptr;
		const auto& exp_dir = nt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];
		if ( !exp_dir.VirtualAddress || !exp_dir.Size ) return nullptr;

		const auto* exp = reinterpret_cast< const IMAGE_EXPORT_DIRECTORY* >( base + exp_dir.VirtualAddress );
		const auto* funcs = reinterpret_cast< const std::uint32_t* >( base + exp->AddressOfFunctions );
		const auto* names = reinterpret_cast< const std::uint32_t* >( base + exp->AddressOfNames );
		const auto* ords  = reinterpret_cast< const std::uint16_t* >( base + exp->AddressOfNameOrdinals );

		for ( std::uint32_t i = 0; i < exp->NumberOfNames; ++i ) {
			const char* name = reinterpret_cast< const char* >( base + names[ i ] );
			if ( rt_hash_a( name ) == fn_hash ) {
				const std::uint32_t rva = funcs[ ords[ i ] ];
				if ( rva >= exp_dir.VirtualAddress && rva < exp_dir.VirtualAddress + exp_dir.Size )
					return nullptr;
				return reinterpret_cast< void* >( base + rva );
			}
		}
		return nullptr;
	}

	template < typename Fn >
	inline Fn resolve( std::uint64_t mod_hash, std::uint64_t fn_hash ) {
		return reinterpret_cast< Fn >( find_export_by_hash( mod_hash, fn_hash ) );
	}

	__forceinline bool opaque_true( ) {
		volatile int x = 7;
		volatile int y = 13;
		return ( ( x * x + y ) ^ 0x3A ) != 0;
	}

	inline void hide_thread( ) {
		using nt_set_info_thread_t = NTSTATUS( NTAPI* )( HANDLE, ULONG, PVOID, ULONG );
		static auto fn = resolve< nt_set_info_thread_t >(
			hash_w_lower( L"ntdll.dll" ), hash_a( "NtSetInformationThread" ) );
		if ( fn ) {
			fn( reinterpret_cast< HANDLE >( -2 ), 0x11, nullptr, 0 );
		}
	}

	inline bool has_sw_breakpoint( const void* addr, std::size_t len ) {
		const auto* p = static_cast< const std::uint8_t* >( addr );
		for ( std::size_t i = 0; i < len; ++i ) {
			if ( p[ i ] == 0xCC ) return true;
		}
		return false;
	}

	inline void scrub_pe_sections( ) {
		HMODULE self_mod = GetModuleHandleW( nullptr );
		if ( !self_mod ) return;
		const auto base = reinterpret_cast< std::uintptr_t >( self_mod );
		auto* dos = reinterpret_cast< PIMAGE_DOS_HEADER >( base );
		if ( dos->e_magic != IMAGE_DOS_SIGNATURE ) return;
		auto* nt = reinterpret_cast< PIMAGE_NT_HEADERS64 >( base + dos->e_lfanew );
		if ( nt->Signature != IMAGE_NT_SIGNATURE ) return;
		DWORD old_p = 0;
		if ( VirtualProtect( reinterpret_cast< void* >( base ), 0x1000, PAGE_READWRITE, &old_p ) ) {
			nt->FileHeader.TimeDateStamp = 0;
			auto* sec = IMAGE_FIRST_SECTION( nt );
			for ( WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i ) {
				for ( int k = 0; k < IMAGE_SIZEOF_SHORT_NAME; ++k ) {
					sec[ i ].Name[ k ] = static_cast< BYTE >( ( i * 17 + k * 31 ) ^ 0x5A );
				}
			}
			VirtualProtect( reinterpret_cast< void* >( base ), 0x1000, old_p, &old_p );
		}
	}
}

#define LAZY_FN( mod, fn ) \
	sec::resolve< decltype( &fn ) >( sec::hash_w_lower( mod ), sec::hash_a( #fn ) )
