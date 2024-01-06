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
#include <cereal/types/list.hpp>
#include <cereal/types/vector.hpp>
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
	inline void prologue( cereal::JSONOutputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void prologue( cereal::JSONInputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONOutputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::JSONInputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}
	/// \}

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

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for generic types.
	///
	template<class T>
	inline ScribeWrapper<T> make_scribe_wrap( const std::string &t_name, T &&t_value, detail::GenericTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for smart pointer types pointing to polymorphic types.
	///
	template<class T>
	inline typename std::enable_if_t<std::is_polymorphic_v<typename std::remove_reference_t<T>::element_type>, ScribePointerWrapper<T>> make_scribe_wrap(
	    const std::string &t_name, T &&t_value, detail::SmartPointerTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for smart pointer types.
	/// \todo fix the serialization for this.
	///
	template<class T>
	inline typename std::enable_if_t<!std::is_polymorphic_v<typename std::remove_reference_t<T>::element_type>, ScribePointerWrapper<T>> make_scribe_wrap(
	    const std::string &t_name, T &&t_value, detail::SmartPointerTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for container types.
	///
	template<class T>
	inline ScribeContainerWrapper<T> make_scribe_wrap( const std::string &t_name, T &&t_value, detail::DynamicContainerTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name };
	}

	///
	/// \brief Factory function for poly-scribe wrappers.
	///
	/// The idea is to wrap members in a poly-scribe wrapper using this method before serializing.
	/// \code{.cpp}
	/// struct Object
	/// {
	/// 	int value;
	/// 	template<class Archive>
	/// 	void serialize( Archive &archive )
	/// 	{
	/// 		archive( poly_scribe::make_scribe_wrap( "name for the value", value ) );
	/// 	}
	/// };
	/// \endcode
	/// The object can then be serialized using the cereal archives.
	/// \code{.cpp}
	/// std::ostringstream out_stream;
	/// {
	/// 	cereal::JSONOutputArchive archive( out_stream );
	/// 	archive( poly_scribe::make_scribe_wrap( "cool name", Object{} ) );
	/// }
	/// \endcode
	/// \tparam T type to be wrapped
	/// \param t_name name for the wrapped value. Similar to cereal::NameValuePair.
	/// \param t_value value to be wrapped.
	/// \return the wrap object.
	///
	template<class T>
	inline auto make_scribe_wrap( const std::string &t_name, T &&t_value )
	{
		return make_scribe_wrap( t_name, std::forward<T>( t_value ), detail::GetWrapperTag<typename std::remove_reference_t<T>>::type( ) );
	}
} // namespace poly_scribe

#endif