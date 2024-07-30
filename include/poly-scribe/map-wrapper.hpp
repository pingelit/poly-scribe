/**
 * \file map-wrapper.hpp
 * \brief Header for the scribe wrapper for map like containers.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_MAP_WRAPPER_HPP
#define POLY_SCRIBE_MAP_WRAPPER_HPP

#include "detail/helper.hpp"
#include "detail/tags.hpp"
#include "pointer-wrapper.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <type_traits>

namespace poly_scribe
{
	template<typename T>
	class ScribeMapWrapper
	{
		///
		/// \brief Get the correct type to hold.
		///
		/// See cereal::NameValuePair for more info.
		///
		using Type = typename std::conditional_t<std::is_array_v<typename std::remove_reference_t<T>>, typename std::remove_cv_t<T>,
		                                       typename std::conditional_t<std::is_lvalue_reference_v<T>, T, typename std::decay_t<T>>>;

		using key_type    = typename std::remove_reference_t<T>::key_type;
		using mapped_type = typename std::remove_reference_t<T>::mapped_type;

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value; ///< Wrapped value.

		///
		/// \brief Construct a ScribeContainerWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribeMapWrapper( T &&t_value ) : m_value( std::forward<T>( t_value ) ) {}

		~ScribeMapWrapper( ) = default;

		///
		/// \brief Save method for the object.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_size_tag( m_value.size( ) ) );

			for( auto &&[key, value]: m_value )
			{
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::GenericTag> )
				{
					t_archive( cereal::make_map_item( key, value ) );
				}
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::SmartPointerTag> )
				{
					t_archive( cereal::make_map_item( key, detail::make_scribe_pointer_wrap( value ) ) );
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

			m_value.clear( );

			auto hint = m_value.begin( );
			for( size_t i = 0; i < size; ++i )
			{
				key_type key;
				mapped_type value;

				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::GenericTag> )
				{
					t_archive( cereal::make_map_item( key, value ) );
				}
				if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::SmartPointerTag> )
				{
					t_archive( cereal::make_map_item( key, detail::make_scribe_pointer_wrap( value ) ) );
				}

				hint = m_value.emplace_hint( hint, std::move( key ), std::move( value ) );
			}
		}

		template<>
		void CEREAL_SAVE_FUNCTION_NAME( cereal::JSONOutputArchive &t_archive ) const
		{
			if constexpr( std::is_same_v<std::string, key_type> )
			{
				for( auto &&[key, value]: m_value )
				{
					if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::GenericTag> )
					{
						t_archive( cereal::make_nvp( key, value ) );
					}
					if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::SmartPointerTag> )
					{
						t_archive( cereal::make_nvp( key, detail::make_scribe_pointer_wrap( value ) ) );
					}
				}
			}
			else
			{
				cereal::CEREAL_SAVE_FUNCTION_NAME( t_archive, m_value );
			}
		}

		template<>
		void CEREAL_LOAD_FUNCTION_NAME( cereal::JSONInputArchive &t_archive )
		{
			if constexpr( std::is_same_v<std::string, key_type> )
			{
				m_value.clear( );

				while( true )
				{
					const char *raw_key = t_archive.getNodeName( );
					if( raw_key == nullptr )
					{
						break;
					}
					const std::string key = raw_key;
					mapped_type value;

					if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::GenericTag> )
					{
						t_archive( cereal::make_nvp( key, value ) );
					}
					if constexpr( std::is_same_v<typename detail::GetWrapperTag<std::remove_reference_t<mapped_type>>::type, detail::SmartPointerTag> )
					{
						t_archive( cereal::make_nvp( key, detail::make_scribe_pointer_wrap( value ) ) );
					}

					m_value.emplace( key, value );
				}
			}
			else
			{
				cereal::CEREAL_LOAD_FUNCTION_NAME( t_archive, m_value );
			}
		}
	};
} // namespace poly_scribe
#endif