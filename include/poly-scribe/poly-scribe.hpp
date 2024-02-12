/**
 * \file poly-scribe.hpp
 * \brief Main header for the poly-scribe library.
 *
 * This includes the other header for the library.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_POLY_SCRIBE_HPP
#define POLY_SCRIBE_POLY_SCRIBE_HPP

/**
 * \brief Main namespace of the library.
 */
namespace poly_scribe
{
}

#include "detail/poly-bind.hpp"
#include "factory.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

// NOLINTBEGIN(cppcoreguidelines-macro-usage,bugprone-macro-parentheses)
//! Helper macro to omit unused warning
#if defined( __GNUC__ )
// GCC / clang don't want the function
#	define POLY_SCRIBE_BIND_TO_ARCHIVES_UNUSED_FUNCTION
#else
#	define POLY_SCRIBE_BIND_TO_ARCHIVES_UNUSED_FUNCTION \
		static void unused( )                            \
		{                                                \
			(void)binding;                               \
		}
#endif

#define POLY_SCRIBE_BIND_TO_ARCHIVES( Type )                                                                                                  \
	namespace poly_scribe::detail                                                                                                             \
	{                                                                                                                                         \
		template<>                                                                                                                            \
		struct init_binding<Type>                                                                                                             \
		{                                                                                                                                     \
			static inline BindToArchives<Type> const &binding = ::cereal::detail::StaticObject<BindToArchives<Type>>::getInstance( ).bind( ); \
			POLY_SCRIBE_BIND_TO_ARCHIVES_UNUSED_FUNCTION                                                                                      \
		};                                                                                                                                    \
	}

#define POLY_SCRIBE_REGISTER_TYPE( Type )        \
	namespace poly_scribe::detail                \
	{                                            \
		template<>                               \
		struct BindingName<Type>                 \
		{                                        \
			static constexpr char const *name( ) \
			{                                    \
				return #Type;                    \
			}                                    \
		};                                       \
	}                                            \
	CEREAL_REGISTER_TYPE( Type )                 \
	POLY_SCRIBE_BIND_TO_ARCHIVES( Type )

#define POLY_SCRIBE_REGISTER_POLYMORPHIC_RELATION( Base, Derived ) CEREAL_REGISTER_POLYMORPHIC_RELATION( Base, Derived )

#define POLY_SCRIBE_REGISTER_ARCHIVE( Archive )                                                                                   \
	namespace poly_scribe::detail                                                                                                 \
	{                                                                                                                             \
		template<class T, class BindingTag>                                                                                       \
		typename PolymorphicSerializationSupport<Archive, T>::type instantiate_polymorphic_binding( T *, Archive *, BindingTag ); \
	}

POLY_SCRIBE_REGISTER_ARCHIVE( cereal::JSONOutputArchive );
POLY_SCRIBE_REGISTER_ARCHIVE( cereal::JSONInputArchive );
POLY_SCRIBE_REGISTER_ARCHIVE( cereal::XMLOutputArchive );
POLY_SCRIBE_REGISTER_ARCHIVE( cereal::XMLInputArchive );
// NOLINTEND(cppcoreguidelines-macro-usage,bugprone-macro-parentheses)

#endif