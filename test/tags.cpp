
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