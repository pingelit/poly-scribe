/**
 * \file scribe-wrapper.hpp
 * \brief Header for the scribe wrapper.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_SCRIBE_WRAPPER_HPP
#define POLY_SCRIBE_SCRIBE_WRAPPER_HPP

#include <cereal/cereal.hpp>
#include <string>
#include <type_traits>

namespace poly_scribe
{
	template<typename T>
	class ScribeWrapper
	{
		using Type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value, typename std::remove_cv<T>::type,
		                                       typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type>::type;

		Type value;
		std::string name;

	public:
		ScribeWrapper( T &&val, const std::string &n ) : value( std::forward<T>( val ) ), name( n ) {}

		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &ar ) const
		{
			ar( cereal::make_nvp( name, value ) );
		}

		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &ar )
		{
			ar( cereal::make_nvp( name, value ) );
		}
	};

	template<class T>
	inline ScribeWrapper<T> make_scribe_wrap( const std::string &name, T &&value )
	{
		return { std::forward<T>( value ), name };
	}
} // namespace poly_scribe


#endif