/**
 * \file scribe-wrapper.hpp
 * \brief Header for the scribe wrapper for smart pointers.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_POINTER_WRAPPER_HPP
#define POLY_SCRIBE_POINTER_WRAPPER_HPP

#include "detail/poly-wrapper.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <memory>

namespace poly_scribe
{
	template<typename T, class Enable = void>
	class ScribePointerWrapper
	{
		///
		/// \brief value type of the pointer.
		///
		using value_type = typename std::remove_reference<T>::type::element_type;

		///
		/// \brief Helper wrapper for the pointer poly scribe wrapper.
		/// \tparam Ty wrapped type.
		/// \todo Find out, if this class is really necessary.
		///
		template<typename Ty>
		struct Wrapper
		{
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
			Ty &m_ptr;          ///< wrapped pointer.
			std::string m_type; ///< name of the type.

			///
			/// \brief Construct a new Wrapper object
			/// \param t_pointer pointer to wrap.
			/// \param t_type type string of the wrapped value.
			///
			Wrapper( Ty &t_pointer, std::string t_type ) : m_ptr( t_pointer ), m_type( t_type ) {}

			///
			/// \brief Save method for the object.
			/// \tparam Archive archive type to save to.
			/// \param t_archive archive to save to.
			/// \todo make the type string a variable, many other places as well.
			/// \todo inline serialization, to make sure it works for all user types.
			///
			template<class Archive>
			void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
			{
				t_archive( cereal::make_nvp( "type", m_type ) );
				m_ptr->serialize( t_archive );
			}

			///
			/// \brief Load method for the object.
			/// \tparam Archive archive type to load from.
			/// \param t_archive archive to load from.
			/// \todo this is currently only for shared_ptr!
			///
			template<class Archive>
			void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
			{
				std::string name;
				auto binding = detail::get_input_binding( t_archive, name );
				std::shared_ptr<void> result;

				binding.shared_ptr( &t_archive, result, typeid( value_type ), m_type, name );
				m_ptr = std::static_pointer_cast<value_type>( result );
			}
		};

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		T &m_ptr;           ///< Wrapped pointer.
		std::string m_name; ///< Name used to serialize the pointer.

		///
		/// \brief Construct a ScribePointerWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribePointerWrapper( T &t_value, std::string t_name ) : m_ptr( std::forward<T>( t_value ) ), m_name( std::move( t_name ) ) {}

		///
		/// \brief Save method for the object.
		/// \remark Not specialized for any specific archive.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			std::type_info const &ptrinfo      = typeid( *m_ptr.get( ) );
			static std::type_info const &tinfo = typeid( value_type );

			if( ptrinfo == tinfo )
			{
				t_archive( cereal::make_nvp( m_name, Wrapper<T>( m_ptr, detail::BindingName<value_type>::name( ) ) ) );
				return;
			}

			const auto &m = ::cereal::detail::StaticObject<detail::OutputMap>::getInstance( ).map;

			auto binding = m.find( std::type_index( ptrinfo ) );

			if( binding == m.end( ) )
				throw std::runtime_error( "TODO" );

			binding->second.shared_ptr( &t_archive, m_ptr.get( ), tinfo, m_name );
		}

		///
		/// \brief Load method for the object.
		/// \remark Not specialized for any specific archive.
		/// \tparam Archive archive type to load to.
		/// \param t_archive archive to load from.
		///
		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			t_archive( cereal::make_nvp( m_name, Wrapper<T>( m_ptr, detail::BindingName<value_type>::name( ) ) ) );
		}
	};

	template<typename T>
	class ScribePointerWrapper<T, typename std::enable_if_t<!std::is_polymorphic_v<typename std::remove_reference<T>::type::element_type>>>
	{
		///
		/// \brief value type of the pointer.
		///
		using value_type = typename std::remove_reference<T>::type::element_type;

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		T &m_ptr;           ///< Wrapped pointer.
		std::string m_name; ///< Name used to serialize the pointer.

		///
		/// \brief Construct a ScribePointerWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribePointerWrapper( T &t_value, std::string t_name ) : m_ptr( std::forward<T>( t_value ) ), m_name( std::move( t_name ) ) {}

		///
		/// \brief Save method for the object.
		/// \remark Not specialized for any specific archive.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_nvp( m_name, *m_ptr.get( ) ) );
		}

		///
		/// \brief Load method for the object.
		/// \remark Not specialized for any specific archive.
		/// \tparam Archive archive type to load to.
		/// \param t_archive archive to load from.
		///
		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			if( !m_ptr )
			{
				m_ptr = std::make_shared<value_type>( );
			}
			t_archive( cereal::make_nvp( m_name, *m_ptr.get( ) ) );
		}
	};

	///
	/// \brief Pro- and epilogue functions to ensure that the wrapper is serialized inline for JSON archives.
	/// \{
	///
	template<class T>
	inline void prologue( cereal::JSONOutputArchive & /*unused*/, ScribePointerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void prologue( cereal::JSONInputArchive & /*unused*/, ScribePointerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONOutputArchive & /*unused*/, ScribePointerWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONInputArchive & /*unused*/, ScribePointerWrapper<T> const & /*unused*/ )
	{
	}
	/// \}
} // namespace poly_scribe

#endif