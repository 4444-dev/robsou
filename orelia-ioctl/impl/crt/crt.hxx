#define ULONG_MAX 0xFFFFFFFFUL
namespace crt
{
    unsigned long strtoul(
        const char* str,
        char** endptr,
        int base
    ) {
        while ( *str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' )
            str++;

        bool negative = false;
        if ( *str == '-' ) {
            negative = true;
            str++;
        }
        else if ( *str == '+' ) {
            str++;
        }

        if ( base == 0 ) {
            if ( *str == '0' ) {
                str++;
                if ( *str == 'x' || *str == 'X' ) {
                    base = 16;
                    str++;
                }
                else {
                    base = 8;
                }
            }
            else {
                base = 10;
            }
        }
        else if ( base == 16 ) {
            if ( *str == '0' && ( *( str + 1 ) == 'x' || *( str + 1 ) == 'X' ) ) {
                str += 2;
            }
        }

        unsigned long result = 0;
        bool valid_digit_found = false;

        while ( *str ) {
            int digit;

            if ( *str >= '0' && *str <= '9' ) {
                digit = *str - '0';
            }
            else if ( *str >= 'a' && *str <= 'z' ) {
                digit = *str - 'a' + 10;
            }
            else if ( *str >= 'A' && *str <= 'Z' ) {
                digit = *str - 'A' + 10;
            }
            else {
                break;
            }

            if ( digit >= base ) {
                break;
            }

            valid_digit_found = true;

            if ( result > ( ULONG_MAX - digit ) / base ) {
                result = ULONG_MAX;
                break;
            }

            result = result * base + digit;
            str++;
        }

        if ( endptr ) {
            *endptr = const_cast< char* >( valid_digit_found ? str : str - valid_digit_found );
        }

        return negative ? static_cast< unsigned long >( -static_cast< long >( result ) ) : result;
    }

    [[ nodiscard ]]
    const char* str_str( 
        const char* haystack,
        const char* needle
    ) {
        if ( !haystack || !needle )
            return nullptr;

        if ( !*needle )
            return haystack;

        const char* p1 = haystack;
        while ( *p1 ) {
            const char* p1_begin = p1;
            const char* p2 = needle;

            while ( *p1 && *p2 && ( *p1 == *p2 ) ) {
                p1++;
                p2++;
            }

            if ( !*p2 )
                return p1_begin;

            p1 = p1_begin + 1;
        }

        return nullptr;
    }

    [[nodiscard]]
    int tolower( int c ) {
        if ( c >= 'A' && c <= 'Z' ) {
            return c + ( 'a' - 'A' );
        }
        return c;
    }

    [[nodiscard]]
    char* strtolower( char* str ) {
        for ( char* p = str; *p != '\0'; ++p ) {
            *p = static_cast< char >( tolower( *p ) );
        }
        return str;
    }

    [[ nodiscard ]] 
    void* memcpy ( 
        void* dest ,
        const void* src , 
        size_t len 
    ) {
        char* d = ( char* ) dest;
        const char* s = ( const char* ) src;
        while ( len-- )
            *d++ = *s++;
        return dest;
    }

    [[ nodiscard ]]
    int wcscmp (
        const wchar_t* s1 ,
        const wchar_t* s2
    ) {
        if ( !s1 && !s2 ) return 0;
        if ( !s1 ) return -1;
        if ( !s2 ) return 1;

        while ( *s1 && ( *s1 == *s2 ) ) {
            ++s1;
            ++s2;
        }

        return static_cast< int >( static_cast< std::uint16_t >( *s1 ) ) -
               static_cast< int >( static_cast< std::uint16_t >( *s2 ) );
    }

    [[ nodiscard ]]
    int wcsicmp (
        const wchar_t* s1 ,
        const wchar_t* s2
    ) {
        if ( !s1 && !s2 ) return 0;
        if ( !s1 ) return -1;
        if ( !s2 ) return 1;

        auto to_lower_w = [ ] ( std::uint16_t ch ) -> std::uint16_t {
            if ( ch >= L'A' && ch <= L'Z' )
                return ch + ( L'a' - L'A' );
            return ch;
        };

        std::uint16_t c1 = 0;
        std::uint16_t c2 = 0;
        do {
            c1 = to_lower_w( static_cast< std::uint16_t >( *s1++ ) );
            c2 = to_lower_w( static_cast< std::uint16_t >( *s2++ ) );
        } while ( c1 && c1 == c2 );

        return static_cast< int >( c1 ) - static_cast< int >( c2 );
    }

    [[ nodiscard ]]
    std::int32_t strcmp (
        const char* string,
        const char* string_cmp
    ) {
        while ( *string != '\0' )
        {
            if ( *string != *string_cmp )
                break;
            string++;
            string_cmp++;
        }
        return *string - *string_cmp;
    }

    [[ nodiscard ]] 
    std::size_t strlen (
        const char* str 
    ) {
        const char* s;
        for ( s = str; *s; ++s );
        return ( s - str );
    }
}