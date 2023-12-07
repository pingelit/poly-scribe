#include "test-helper.hpp"

#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <poly-scribe/poly-scribe.hpp>

TEMPLATE_TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper][template]", bool, int, char, float, double, long )
{
	using namespace poly_scribe;
	SECTION( "Construction" )
	{
		TestType integer = 0;
		auto wrap        = make_scribe_wrap( "name", integer );
		REQUIRE( true );
	}
}

TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper]" )
{
	SECTION( "Binary" )
	{
		test_pod<cereal::BinaryInputArchive, cereal::BinaryOutputArchive>( );
	}
	SECTION( "JSON" )
	{
		test_pod<cereal::JSONInputArchive, cereal::JSONOutputArchive>( );
	}
	SECTION( "XML" )
	{
		test_pod<cereal::XMLInputArchive, cereal::XMLOutputArchive>( );
	}
}