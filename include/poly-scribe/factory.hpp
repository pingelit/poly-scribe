/**
 * \file factory.hpp
 * \brief Header containing the factories for the scribe wrapper.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_FACTORY_HPP
#define POLY_SCRIBE_FACTORY_HPP

#include "container-wrapper.hpp"
#include "map-wrapper.hpp"
#include "detail/tags.hpp"
#include "pointer-wrapper.hpp"
#include "scribe-wrapper.hpp"

#include <type_traits>

namespace poly_scribe
{
	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for generic types.
	///
	template<class T>
	inline ScribeWrapper<T> make_scribe_wrap( const std::string &t_name, T &&t_value, bool t_optional, detail::GenericTag /*unused*/ )
	{
		return { std::forward<T>( t_value ), t_name, t_optional };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for smart pointer types pointing to polymorphic types.
	///
	template<class T>
	inline typename std::enable_if_t<std::is_polymorphic_v<typename std::remove_reference_t<T>::element_type>, ScribeWrapper<ScribePointerWrapper<T>>> make_scribe_wrap(
	    const std::string &t_name, T &&t_value, bool t_optional, detail::SmartPointerTag /*unused*/ )
	{
		return { { std::forward<T>( t_value ) }, t_name, t_optional };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for smart pointer types.
	/// \todo fix the serialization for this.
	///
	template<class T>
	inline typename std::enable_if_t<!std::is_polymorphic_v<typename std::remove_reference_t<T>::element_type>, ScribeWrapper<ScribePointerWrapper<T>>> make_scribe_wrap(
	    const std::string &t_name, T &&t_value, bool t_optional, detail::SmartPointerTag /*unused*/ )
	{
		return { { std::forward<T>( t_value ) }, t_name, t_optional };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for container types.
	///
	template<class T>
	inline ScribeWrapper<ScribeContainerWrapper<T>> make_scribe_wrap( const std::string &t_name, T &&t_value, bool t_optional, detail::DynamicContainerTag /*unused*/ )
	{
		return { { std::forward<T>( t_value ) }, t_name, t_optional };
	}

	///
	/// \copybrief make_scribe_wrap( const std::string &t_name, T &&t_value )
	/// Specialized for map container types.
	///
	template<class T>
	inline ScribeWrapper<ScribeMapWrapper<T>> make_scribe_wrap( const std::string &t_name, T &&t_value, bool t_optional, detail::MapContainerTag /*unused*/ )
	{
		return { { std::forward<T>( t_value ) }, t_name, t_optional };
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
	/// \param t_optional if true, the value is optional.
	/// \return the wrap object.
	/// \warning The wrapped object is only in a valid state if loaded or all contained values are optional and have a valid value.
	/// \todo using validation methods it could also be checked if the values are in a valid state.
	///
	template<class T>
	inline auto make_scribe_wrap( const std::string &t_name, T &&t_value, bool t_optional = false )
	{
		return make_scribe_wrap( t_name, std::forward<T>( t_value ), t_optional, typename detail::GetWrapperTag<typename std::remove_reference_t<T>>::type( ) );
	}
} // namespace poly_scribe
#endif