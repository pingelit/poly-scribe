
#include <array>
#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <list>
#include <map>
#include <memory>
#include <poly-scribe/detail/tags.hpp>
#include <string>
#include <unordered_map>
#include <vector>


TEMPLATE_TEST_CASE( "tags", "[tags][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<TestType>::type, poly_scribe::detail::GenericTag>::value );
	}

	SECTION( "Generic type, reference" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<TestType&>::type, poly_scribe::detail::GenericTag>::value );
	}

	SECTION( "shared_ptr" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<std::shared_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "shared_ptr, reference" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<std::shared_ptr<TestType>&>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "weak_ptr" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<std::weak_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "unique_ptr" )
	{
		STATIC_REQUIRE( std::is_same<typename poly_scribe::detail::GetWrapperTag<std::unique_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
	}
}

TEMPLATE_TEST_CASE( "is_container", "[sfinae][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_container_v<TestType> );
	}

	SECTION( "std::vector" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::vector<TestType>> );
	}

	SECTION( "std::list" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::list<TestType>> );
	}

	SECTION( "std::array" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::array<TestType,1>> );
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::array<TestType,2>> );
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::array<TestType,4>> );
		STATIC_REQUIRE( poly_scribe::detail::is_container_v<std::array<TestType,8>> );
	}

	SECTION( "std::map" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_container_v<std::map<std::string, TestType>> );
		STATIC_REQUIRE( !poly_scribe::detail::is_container_v<std::unordered_map<std::string, TestType>> );
	}
}

TEMPLATE_TEST_CASE( "is_smart_ptr", "[sfinae][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_smart_ptr_v<TestType> );
	}

	SECTION( "Raw ptr" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_smart_ptr_v<TestType*> );
	}

	SECTION( "std::shared_ptr" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_smart_ptr_v<std::shared_ptr<TestType>> );
	}

	SECTION( "std::weak_ptr" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_smart_ptr_v<std::weak_ptr<TestType>> );
	}

	SECTION( "std::unique_ptr" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_smart_ptr_v<std::unique_ptr<TestType>> );
	}
}

TEMPLATE_TEST_CASE( "is_array", "[sfinae][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_array_v<TestType> );
	}

	SECTION( "std::vector" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_array_v<std::vector<TestType>> );
	}

	SECTION( "std::list" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_array_v<std::list<TestType>> );
	}

	SECTION( "std::array" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_array_v<std::array<TestType, 1>> );
		STATIC_REQUIRE( poly_scribe::detail::is_array_v<std::array<TestType, 2>> );
		STATIC_REQUIRE( poly_scribe::detail::is_array_v<std::array<TestType, 4>> );
		STATIC_REQUIRE( poly_scribe::detail::is_array_v<std::array<TestType, 8>> );
	}

	SECTION( "raw" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_array_v<TestType[]> ); // NOLINT
	}
}

TEMPLATE_TEST_CASE( "is_map_like", "[sfinae][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<TestType> );
	}

	SECTION( "std::vector" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::vector<TestType>> );
	}

	SECTION( "std::list" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::list<TestType>> );
	}

	SECTION( "std::array" )
	{
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::array<TestType, 1>> );
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::array<TestType, 2>> );
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::array<TestType, 4>> );
		STATIC_REQUIRE( !poly_scribe::detail::is_map_like_v<std::array<TestType, 8>> );
	}

	SECTION( "maps" )
	{
		STATIC_REQUIRE( poly_scribe::detail::is_map_like_v<std::map<std::string, TestType>> );
		STATIC_REQUIRE( poly_scribe::detail::is_map_like_v<std::unordered_map<std::string, TestType>> );
	}
}