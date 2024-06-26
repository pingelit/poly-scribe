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

#include <array>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>


namespace poly_scribe::detail
{
	// NOLINTBEGIN(readability-identifier-naming)

	///
	/// \brief  SFINAE check if container type.
	///
	/// std::string here is not considered to be a container.
	/// Based on: https://stackoverflow.com/questions/61063470/c-test-for-containers-via-sfinae
	/// \{
	template<class N, class Enable = void>
	struct is_container
	{
		static const bool value = false;
	};

	template<typename Key, typename T, typename Compare, typename Allocator>
	struct is_container<std::map<Key, T, Compare, Allocator>>
	{
		static constexpr bool value = false;
	};

	template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
	struct is_container<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
	{
		static constexpr bool value = false;
	};

	template<class N>
	struct is_container<N, std::void_t<typename N::value_type>>
	{
		static const bool value = true;
	};

	template<>
	struct is_container<std::string>
	{
		static const bool value = false;
	};

	template<class N>
	static constexpr bool is_container_v = is_container<std::remove_reference_t<N>>::value;
	/// \}

	///
	/// \brief SFINAE check if smart pointer type.
	/// \{
	template<class N, class Enable = void>
	struct is_smart_ptr
	{
		static const bool value = false;
	};

	template<class N>
	struct is_smart_ptr<N, std::void_t<typename N::element_type>>
	{
		static const bool value = true;
	};

	template<class N>
	static constexpr bool is_smart_ptr_v = is_smart_ptr<std::remove_reference_t<N>>::value;
	/// \}

	///
	/// \brief Check if std::array.
	/// \{
	template<class N>
	struct is_array : std::false_type
	{
	};

	template<typename T, std::size_t N>
	struct is_array<std::array<T, N>> : std::true_type
	{
	};

	template<class N>
	static constexpr bool is_array_v = is_array<std::remove_reference_t<N>>::value;
	/// \}

	///
	/// \brief SFINAE check if map like type.
	/// \{
	template<typename Container, typename Enable = void>
	struct is_map_like
	{
		static constexpr bool value = false;
	};

	template<typename Container>
	struct is_map_like<Container, std::void_t<typename Container::key_type, typename Container::mapped_type>>
	{
		static constexpr bool value = true;
	};

	template<class Container>
	static constexpr bool is_map_like_v = is_map_like<std::remove_reference_t<Container>>::value;
	/// \}

	// NOLINTEND(readability-identifier-naming)

	///
	/// \brief Tag system for ::make_scribe_wrap.
	///
	/// Get the correct tag for the given type using GetWrapperTag.
	/// \{
	struct DynamicContainerTag
	{
	};

	struct MapContainerTag
	{
	};

	struct SmartPointerTag
	{
	};

	struct GenericTag
	{
	};

	template<typename T, class Enable = void>
	struct GetWrapperTag
	{
		using type = GenericTag;
	};

	template<typename T>
	struct GetWrapperTag<T, std::enable_if_t<is_smart_ptr_v<T>>>
	{
		using type = SmartPointerTag;
	};

	template<typename T>
	struct GetWrapperTag<T, std::enable_if_t<is_container_v<T>>>
	{
		using type = DynamicContainerTag;
	};

	template<typename T>
	struct GetWrapperTag<T, std::enable_if_t<is_map_like_v<T>>>
	{
		using type = MapContainerTag;
	};
	/// \}
} // namespace poly_scribe::detail

#endif