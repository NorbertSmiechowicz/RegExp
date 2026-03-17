
#ifndef _INTRO_H_
# define _INTRO_H_

# include <concepts>

////////////////////////////////////////////////
// Common Macros

#define RC( T, X) reinterpret_cast< T>( X)
#define SC( T, X) static_cast< T>( X)
#define CC( T, X) const_cast< T>( X)

////////////////////////////////////////////////
//  Concepts

template< typename FromT, typename ToT>
concept non_narrowing_cast =
(
    requires { static_cast< ToT>( std::declval< FromT>()); } &&
    (sizeof( ToT) >= sizeof( FromT))
);

#endif//_INTRO_H_
