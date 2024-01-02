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

#include "detail/poly-bind.hpp"
#include "detail/tags.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <string>
#include <type_traits>

namespace poly_scribe
{
	///
	/// \brief Wraps a "normal" value to be used with the poly-scribe environment.
	///
	/// It holds a reference to a value that will be serialized using the cereal archives.
	/// This basically is a cereal::NameValuePair but also is the basis for the poly-scribe eco system.
	/// It should be mainly created using the ::make_scribe_wrap function.
	/// \tparam T type of the value the wrapper holds.
	///
	template<typename T>
	class ScribeWrapper
	{
		///
		/// \brief Get the correct type to hold.
		///
		/// See cereal::NameValuePair for more info.
		///
		using Type = typename std::conditional<std::is_array<typename std::remove_reference<T>::type>::value, typename std::remove_cv<T>::type,
		                                       typename std::conditional<std::is_lvalue_reference<T>::value, T, typename std::decay<T>::type>::type>::type;

	public:
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value;       ///< Wrapped value.
		std::string m_name; ///< Name used to serialize the value.

		///
		/// \brief Construct a ScribeWrapper object
		///
		/// \param t_value rvalue reference to the value to wrap.
		/// \param t_name name the value should to be serialized with.
		///
		ScribeWrapper( T &&t_value, std::string t_name ) : m_value( std::forward<T>( t_value ) ), m_name( std::move( t_name ) ) {}

		~ScribeWrapper( )                                     = default;
		ScribeWrapper( const ScribeWrapper & )                = delete;
		ScribeWrapper( ScribeWrapper && ) noexcept            = delete;
		ScribeWrapper &operator=( ScribeWrapper const & )     = delete;
		ScribeWrapper &operator=( ScribeWrapper && ) noexcept = delete;

		///
		/// \brief Save method for the object.
		/// \remark Not specialized for any specific archive.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_nvp( m_name, m_value ) );
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
			t_archive( cereal::make_nvp( m_name, m_value ) );
		}
	};

	///
	/// \brief Pro- and epilogue functions to ensure that the wrapper is serialized inline for JSON archives.
	/// \{
	///
	template<class T>
	inline void prologue( cereal::JSONOutputArchive &, ScribeWrapper<T> const & )
	{
	}

	template<class T>
	inline void prologue( cereal::JSONInputArchive &, ScribeWrapper<T> const & )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONOutputArchive &, ScribeWrapper<T> const & )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONInputArchive &, ScribeWrapper<T> const & )
	{
	}
	/// \}

	template<typename T>
	class ScribePointerWrapper
	{
		using value_type = typename std::remove_reference<T>::type::element_type;

		template<typename Ty>
		struct Wrapper
		{
			// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
			Ty &m_value;
			std::string m_type;

			Wrapper( Ty &t_pointer, std::string t_type ) : m_value( t_pointer ), m_type( t_type ) {}

			template<class Archive>
			void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
			{
				// todo make the type string a variable
				t_archive( cereal::make_nvp( "type", m_type ) );
				m_value->serialize( t_archive );
			}

			template<class Archive>
			void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
			{
				std::string name;
				auto binding = detail::get_input_binding( t_archive, name );
				std::shared_ptr<void> result;

				binding.shared_ptr( &t_archive, result, typeid( value_type ), m_type, name );
				m_value = std::static_pointer_cast<value_type>( result );
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


	///
	/// \brief Pro- and epilogue functions to ensure that the wrapper is serialized inline for JSON archives.
	/// \{
	///
	template<class T>
	inline void prologue( cereal::JSONOutputArchive &, ScribePointerWrapper<T> const & )
	{
	}

	template<class T>
	inline void prologue( cereal::JSONInputArchive &, ScribePointerWrapper<T> const & )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONOutputArchive &, ScribePointerWrapper<T> const & )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONInputArchive &, ScribePointerWrapper<T> const & )
	{
	}
	/// \}

	template<class T>
	inline ScribeWrapper<T> make_scribe_wrap( const std::string &t_name, T &&t_value, detail::GenericTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	template<class T>
	inline ScribePointerWrapper<T> make_scribe_wrap( const std::string &t_name, T &&t_value, detail::SmartPointerTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	template<class T>
	inline auto make_scribe_wrap( const std::string &t_name, T &&t_value )
	{
		return make_scribe_wrap( t_name, std::forward<T>( t_value ), detail::GetWrapperTag<typename std::remove_reference_t<T>>::type( ) );
	}
} // namespace poly_scribe

#endif