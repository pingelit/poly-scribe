#include <catch2/catch_test_macros.hpp>
#include <poly-scribe/poly-scribe.hpp>

TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper]" )
{
	using namespace poly_scribe;
	SECTION( "Construction" )
	{
		int integer = 0;

		auto wrap = make_scribe_wrap( "name", integer );
	}
}