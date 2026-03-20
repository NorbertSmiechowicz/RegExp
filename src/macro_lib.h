
#ifndef _MACRO_LIB_H_
# define _MACRO_LIB_H_

# ifdef __cplusplus
#  include <stdbool.h>
# endif

# ifdef __cplusplus
#  define RC( T, X) reinterpret_cast< T>( X)
#  define SC( T, X) static_cast< T>( X)
#  define CC( T, X) const_cast< T>( X)
# else
#  define SC( T, X) (T)(X)
#  define RC( T, X) (T)(X)
#  define CC( T, X) (T)(X)
# endif

# if __cplusplus >= 202300L
#  define NODISCARD [[nodiscard]]
# elif __STDC_VERSION__ >= 201700L
#  define NODISCARD [[nodiscard]]
# else
#  define NODISCARD
# endif

# if defined( __cplusplus) && (defined( __GNUC__) || defined ( __clang__))
#  define RESTRICT __restrict__
# elif __STDC_VERSION__ >= 199900L
#  define RESTRICT restrict
# else
#  define RESTRICT
# endif

#endif//_MACRO_LIB_H_
