/**
 * \file poly-wrapper.hpp
 * \brief Header containing the polymorphic wrapper.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_DETAIL_POLY_WRAPPER_HPP
#define POLY_SCRIBE_DETAIL_POLY_WRAPPER_HPP

#include <cereal/cereal.hpp>
#include <memory>

namespace poly_scribe::detail
{
	template<typename T, typename P>
	struct PolyScribeWrapper
	{
	private:
		using Type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value, typename std::remove_cv<T>::type,
		                                       typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type>::type;


	public:
		PolyScribeWrapper( char const *t_name, T &&t_value ) : name( t_name ), m_value( std::forward<T>( t_value ) ) {}

		// todo check against clang-tidy and see if rule of 5 is fulfilled.
		~PolyScribeWrapper( )                                         = default;
		PolyScribeWrapper( const PolyScribeWrapper & )                = default;
		PolyScribeWrapper( PolyScribeWrapper && ) noexcept            = default;
		PolyScribeWrapper &operator=( PolyScribeWrapper && ) noexcept = default;
		PolyScribeWrapper &operator=( PolyScribeWrapper const & )     = delete;

		char const *name;
		Type m_value;

		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_nvp( "type", std::string( BindingName<P>::name( ) ) ) );
			const_cast<P &>( *m_value ).CEREAL_SERIALIZE_FUNCTION_NAME( t_archive );
		}

		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			const_cast<P &>( *m_value ).CEREAL_SERIALIZE_FUNCTION_NAME( t_archive );
		}
	};

	template<typename P, class T>
	inline PolyScribeWrapper<T, P> make_poly_wrapper( std::string const &t_name, T &&t_value )
	{
		return { t_name.c_str( ), std::forward<T>( t_value ) };
	}
} // namespace poly_scribe::detail

#endif