
#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <memory>
#include <poly-scribe/detail/tags.hpp>

TEMPLATE_TEST_CASE( "tags", "[tags][template]", bool, char, int, float, double, long, std::string )
{
	SECTION( "Generic type" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<TestType>::type, poly_scribe::detail::GenericTag>::value );
	}

	SECTION( "Generic type, reference" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<TestType&>::type, poly_scribe::detail::GenericTag>::value );
	}

	SECTION( "shared_ptr" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<std::shared_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "shared_ptr, reference" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<std::shared_ptr<TestType>&>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "weak_ptr" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<std::weak_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
	}

	SECTION( "unique_ptr" )
	{
		STATIC_REQUIRE( std::is_same<poly_scribe::detail::GetWrapperTag<std::unique_ptr<TestType>>::type, poly_scribe::detail::SmartPointerTag>::value );
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