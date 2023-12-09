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

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value;
		std::string m_name;

		ScribeWrapper( T&& t_value, std::string t_name ) : m_value( std::forward<T>( t_value ) ), m_name( std::move( t_name ) ) {}

		~ScribeWrapper( )                                    = default;
		ScribeWrapper( const ScribeWrapper& )                = delete;
		ScribeWrapper( ScribeWrapper&& ) noexcept            = delete;
		ScribeWrapper& operator=( ScribeWrapper const& )     = delete;
		ScribeWrapper& operator=( ScribeWrapper&& ) noexcept = delete;

		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive& t_archive ) const
		{
			t_archive( cereal::make_nvp( m_name, m_value ) );
		}

		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive& t_archive )
		{
			t_archive( cereal::make_nvp( m_name, m_value ) );
		}
	};

	template<class T>
	inline ScribeWrapper<T> make_scribe_wrap( const std::string& t_name, T&& t_value )
	{
		return { std::forward<T>( t_value ), t_name };
	}
} // namespace poly_scribe


#endif