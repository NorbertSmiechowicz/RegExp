
#ifndef _CONCEPTS_H_
# define _CONCEPTS_H_

# include <concepts>

////////////////////////////////////////////////
//  Concepts

template< typename FromT, typename ToT>
concept non_narrowing_cast =
(
    requires { static_cast< ToT>( std::declval< FromT>()); } &&
    (sizeof( ToT) >= sizeof( FromT))
);

#endif//_CONCEPTS_H_
