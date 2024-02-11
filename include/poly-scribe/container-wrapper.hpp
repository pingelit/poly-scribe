/**
 * \file container-wrapper.hpp
 * \brief Header for the scribe wrapper for containers.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_CONTAINER_WRAPPER_HPP
#define POLY_SCRIBE_CONTAINER_WRAPPER_HPP

#include "detail/tags.hpp"
#include "pointer-wrapper.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/vector.hpp>

namespace poly_scribe
{
	namespace detail
	{
		template<class T>
		inline ScribePointerWrapper<T> make_scribe_pointer_wrap( T &&t_value )
		{
			return { std::forward<T>( t_value ) };
		}
	} // namespace detail

	template<typename T>
	class ScribeContainerWrapper
	{
		///
		/// \brief Get the correct type to hold.
		///
		/// See cereal::NameValuePair for more info.
		///
		using Type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value, typename std::remove_cv<T>::type,
		                                       typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type>::type;

		using element = typename std::remove_reference_t<T>::value_type;

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value; ///< Wrapped value.

		///
		/// \brief Construct a ScribeContainerWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribeContainerWrapper( T &&t_value ) : m_value( std::forward<T>( t_value ) ) {}

		~ScribeContainerWrapper( ) = default;

		///
		/// \brief Save method for the object.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_size_tag( m_value.size( ) ) );

			for( auto const &value: m_value )
			{
				// t_archive( make_scribe_wrap( "unused", value ) );
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<element>>::type, detail::GenericTag> )
				{
					t_archive( value );
				}
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<element>>::type, detail::SmartPointerTag> )
				{
					t_archive( detail::make_scribe_pointer_wrap( value ) );
				}
			}
		}

		///
		/// \brief Load method for the object.
		/// \tparam Archive archive type to load to.
		/// \param t_archive archive to load from.
		///
		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			size_t size = 0;
			t_archive( cereal::make_size_tag( size ) );

			m_value.resize( size );

			for( auto &value: m_value )
			{
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<element>>::type, detail::GenericTag> )
				{
					t_archive( value );
				}
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<element>>::type, detail::SmartPointerTag> )
				{
					t_archive( detail::make_scribe_pointer_wrap( value ) );
				}
			}
		}
	};
} // namespace poly_scribe
#endif