/**
 * \file tags.hpp
 * \brief Header for tags.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_DETAIL_TAGS_HPP
#define POLY_SCRIBE_DETAIL_TAGS_HPP

#include <memory>

namespace poly_scribe::detail
{
	struct SmartPointerTag
	{
	};

	struct GenericTag
	{
	};

	template<typename T>
	struct GetWrapperTag
	{
		using type = GenericTag;
	};

	template<typename T>
	struct GetWrapperTag<T&> : public GetWrapperTag<T>
	{
	};

	template<typename T>
	struct GetWrapperTag<std::shared_ptr<T>>
	{
		using type = SmartPointerTag;
	};

	template<typename T>
	struct GetWrapperTag<std::weak_ptr<T>>
	{
		using type = SmartPointerTag;
	};

	template<typename T>
	struct GetWrapperTag<std::unique_ptr<T>>
	{
		using type = SmartPointerTag;
	};


} // namespace poly_scribe::detail

#endif