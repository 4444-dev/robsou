#pragma once

namespace crt {
	inline int strcmp( const char* str1, const char* str2 ) {
		if ( !str1 && !str2 ) return 0;
		if ( !str1 ) return -1;
		if ( !str2 ) return 1;

		while ( *str1 && ( *str1 == *str2 ) ) {
			str1++;
			str2++;
		}

		return *( unsigned char* )str1 - *( unsigned char* )str2;
	}

	inline void* memcpy(
		void* dest,
		const void* src,
		size_t len
	) {
		if ( dest && src && len ) {
			__movsb( static_cast< unsigned char* >( dest ), static_cast< const unsigned char* >( src ), len );
		}
		return dest;
	}

	inline int memcmp(
		const void* s1,
		const void* s2,
		size_t n
	) {
		const unsigned char* p1 = ( const unsigned char* )s1;
		const unsigned char* end1 = p1 + n;
		const unsigned char* p2 = ( const unsigned char* )s2;
		int                   d = 0;
		for ( ;;) {
			if ( d || p1 >= end1 ) break;
			d = ( int )*p1++ - ( int )*p2++;
			if ( d || p1 >= end1 ) break;
			d = ( int )*p1++ - ( int )*p2++;
			if ( d || p1 >= end1 ) break;
			d = ( int )*p1++ - ( int )*p2++;
			if ( d || p1 >= end1 ) break;
			d = ( int )*p1++ - ( int )*p2++;
		}
		return d;
	}
}