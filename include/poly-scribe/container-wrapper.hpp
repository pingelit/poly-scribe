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

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/vector.hpp>

namespace poly_scribe
{
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

		///
		/// \brief Helper wrapper for containers.
		/// \todo Find out, if this class is really necessary.
		///
		struct Wrapper
		{
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
			Type &m_value; ///< wrapped value.

			///
			/// \brief Construct a new Wrapper object
			/// \param t_value value to wrap.
			///
			Wrapper( Type &&t_value ) : m_value( t_value ) {}

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
					t_archive( make_scribe_wrap( "unused", value ) );
				}
			}

			///
			/// \brief Load method for the object.
			/// \tparam Archive archive type to load from.
			/// \param t_archive archive to load from.
			///
			template<class Archive>
			void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
			{
				size_t size;
				t_archive( cereal::make_size_tag( size ) );

				m_value.resize( size );

				for( auto &value: m_value )
				{
				}
			}
		};

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value;       ///< Wrapped value.
		std::string m_name; ///< Name used to serialize the value.

		///
		/// \brief Construct a ScribeContainerWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribeContainerWrapper( T &&t_value, std::string t_name ) : m_value( std::forward<T>( t_value ) ), m_name( std::move( t_name ) ) {}

		~ScribeContainerWrapper( )                                              = default;
		ScribeContainerWrapper( const ScribeContainerWrapper & )                = delete;
		ScribeContainerWrapper( ScribeContainerWrapper && ) noexcept            = delete;
		ScribeContainerWrapper &operator=( ScribeContainerWrapper const & )     = delete;
		ScribeContainerWrapper &operator=( ScribeContainerWrapper && ) noexcept = delete;

		///
		/// \brief Save method for the object.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_nvp( m_name, Wrapper( m_value ) ) );
		}

		///
		/// \brief Load method for the object.
		/// \tparam Archive archive type to load to.
		/// \param t_archive archive to load from.
		///
		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			t_archive( cereal::make_nvp( m_name, Wrapper( m_value ) ) );
		}
	};

	///
	/// \brief Pro- and epilogue functions to ensure that the wrapper is serialized inline for JSON archives.
	/// \{
	///
	template<class T>
	inline void prologue( cereal::JSONOutputArchive & /*unused*/, ScribeContainerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void prologue( cereal::JSONInputArchive & /*unused*/, ScribeContainerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONOutputArchive & /*unused*/, ScribeContainerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONInputArchive & /*unused*/, ScribeContainerWrapper<T> const & /*unused*/ )
	{
	}
	/// \}
} // namespace poly_scribe
#endif