#pragma once

/*____________________________________________________________________________________________________________

Original Author: skadro
Github: https://github.com/skadro-official
License: See end of file

skCrypter
		Compile-time, Usermode + Kernelmode, safe and lightweight string crypter library for C++11+

							*Not removing this part is appreciated*
____________________________________________________________________________________________________________*/

#include <intrin.h>

#ifdef _KERNEL_MODE
namespace std
{
	// STRUCT TEMPLATE remove_reference
	template <class _Ty>
	struct remove_reference {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&> {
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&&> {
		using type = _Ty;
	};

	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	// STRUCT TEMPLATE remove_const
	template <class _Ty>
	struct remove_const { // remove top-level const qualifier
		using type = _Ty;
	};

	template <class _Ty>
	struct remove_const<const _Ty> {
		using type = _Ty;
	};

	template <class _Ty>
	using remove_const_t = typename remove_const<_Ty>::type;
}
#else
#include <type_traits>
#endif

namespace skc
{
	template<class _Ty>
	using clean_type = typename std::remove_const_t<std::remove_reference_t<_Ty>>;

	__forceinline unsigned char runtime_entropy_mask ( )
	{
		unsigned long long mix = __rdtsc( );
#ifdef _KERNEL_MODE
		mix ^= *reinterpret_cast< const volatile unsigned int* >( 0xFFFFF78000000008ULL );
		mix ^= __readgsqword( 0x188 );
#elif defined(_M_X64)
		mix ^= *reinterpret_cast< const volatile unsigned int* >( 0x7FFE0008ULL );
		mix ^= __readgsqword( 0x40 );
		const unsigned long long peb = __readgsqword( 0x60 );
		if ( peb )
			mix ^= *reinterpret_cast< const unsigned long long* >( peb + 0x10 );
#endif
		unsigned char b = static_cast< unsigned char >( mix ^ ( mix >> 13 ) ^ ( mix >> 27 ) );
		return b ? b : 0x5A;
	}

	template <int _size , char _key1 , char _key2 , typename T>
	class skCrypter
	{
	public:
		__forceinline constexpr skCrypter ( T* data )
		{
			crypt ( data );
		}

		__forceinline void rekey_runtime ( )
		{
			if ( _rt_applied || !isEncrypted ( ) )
				return;

			unsigned char m = runtime_entropy_mask ( );
			if ( ( static_cast< unsigned char >( _storage[ _size - 1 ] ) ^ m ) == 0 )
				m ^= 0x01;

			for ( int i = 0; i < _size; i++ )
			{
				_storage[ i ] ^= static_cast< T >( m );
			}
			_rt_mask = m;
			_rt_applied = true;
		}

		__forceinline T* get ( )
		{
			return _storage;
		}

		__forceinline int size ( ) // (w)char count
		{
			return _size;
		}

		__forceinline  char key ( )
		{
			return _key1;
		}

		__forceinline  T* encrypt ( )
		{
			if ( !isEncrypted ( ) )
				crypt ( _storage );

			return _storage;
		}

		__forceinline  T* decrypt ( )
		{
			if ( isEncrypted ( ) )
			{
				if ( _rt_applied )
				{
					for ( int i = 0; i < _size; i++ )
					{
						_storage[ i ] ^= static_cast< T >( _rt_mask );
					}
					_rt_applied = false;
					_rt_mask = 0;
				}
				crypt ( _storage );
			}

			return _storage;
		}

		__forceinline bool isEncrypted ( )
		{
			return _storage [ _size - 1 ] != 0;
		}

		__forceinline void clear ( ) // set full storage to 0
		{
			for ( int i = 0; i < _size; i++ )
			{
				_storage [ i ] = 0;
			}
			_rt_applied = false;
			_rt_mask = 0;
		}

		__forceinline operator T* ( )
		{
			decrypt ( );

			return _storage;
		}

	private:
		__forceinline constexpr void crypt ( T* data )
		{
			for ( int i = 0; i < _size; i++ )
			{
				_storage [ i ] = data [ i ] ^ ( _key1 + i % ( 1 + _key2 ) );
			}
		}

		T _storage [ _size ] {};
		unsigned char _rt_mask { 0 };
		bool _rt_applied { false };
	};
}

#define encrypt(str) skCrypt_key(str, static_cast<char>(((__LINE__ * 17 + __COUNTER__ * 31) & 0x7E) | 1), static_cast<char>(((__LINE__ ^ __COUNTER__) & 0x3F) + 1))
#define skCrypt_key(str, key1, key2) []() { \
			constexpr auto crypted_init = skc::skCrypter \
				<sizeof(str) / sizeof(str[0]), key1, key2, skc::clean_type<decltype(str[0])>>((skc::clean_type<decltype(str[0])>*)str); \
			auto crypted = crypted_init; \
			crypted.rekey_runtime(); \
			return crypted; }()

/*________________________________________________________________________________

MIT License

Copyright (c) 2020 skadro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

________________________________________________________________________________*/
