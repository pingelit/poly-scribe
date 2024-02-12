/**
 * \file helper.hpp
 * \brief Header containing some detail helpers.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_DETAIL_HELPER_HPP
#define POLY_SCRIBE_DETAIL_HELPER_HPP

#include "../pointer-wrapper.hpp"

#include <cereal/cereal.hpp>
#include <memory>


namespace poly_scribe::detail
{

	template<class T>
	inline ScribePointerWrapper<T> make_scribe_pointer_wrap( T &&t_value )
	{
		return { std::forward<T>( t_value ) };
	}
} // namespace poly_scribe::detail

#endif