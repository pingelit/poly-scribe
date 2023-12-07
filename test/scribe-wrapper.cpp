#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <poly-scribe/poly-scribe.hpp>

TEMPLATE_TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper][template]", bool, int, char, float, double, long )
{
	using namespace poly_scribe;
	SECTION( "Construction" )
	{
		TestType integer = 0;
		auto wrap = make_scribe_wrap( "name", integer );
		REQUIRE( true );
	}
}