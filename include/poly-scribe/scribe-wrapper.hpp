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

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <string>

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

		~ScribeWrapper( ) = default;

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

	///
	/// \brief Pro- and epilogue functions to ensure that the wrapper is serialized inline for XML archives.
	/// \{
	///
	template<class T>
	inline void prologue( cereal::XMLOutputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void prologue( cereal::XMLInputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::XMLOutputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}

	template<class T>
	inline void epilogue( cereal::XMLInputArchive & /*unused*/, ScribeWrapper<T> const & /*unused*/ )
	{
	}
	/// \}
} // namespace poly_scribe

#endif