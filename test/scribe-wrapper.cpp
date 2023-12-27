#include "test-helper.hpp"

#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <poly-scribe/poly-scribe.hpp>
#include <string>

TEMPLATE_TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper][template]", bool, char, int, float, double, long, std::string )
{
	using namespace poly_scribe;
	TestType value{};

	if constexpr( std::is_same_v<bool, TestType> )
	{
		value = GENERATE( true, false );
	}
	else if constexpr( std::is_same_v<char, TestType> )
	{
		value = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	}
	else if constexpr( std::is_same_v<std::string, TestType> )
	{
		value = GENERATE_RANDOM_STRING( 10 );
	}
	else if constexpr( !std::is_same_v<char, TestType> && !std::is_same_v<bool, TestType> && !std::is_same_v<std::string, TestType> )
	{
		value = GENERATE_RANDOM( TestType, MAX_REPS );
	}

	auto name = GENERATE_RANDOM_STRING( 10 );
	auto wrap = make_scribe_wrap( name, value );
	REQUIRE( wrap.m_name == name );
	REQUIRE( wrap.m_value == value );

	if constexpr( std::is_same_v<bool, TestType> )
	{
		value = GENERATE( true, false );
	}
	else if constexpr( std::is_same_v<char, TestType> )
	{
		value = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	}
	else if constexpr( std::is_same_v<std::string, TestType> )
	{
		value = GENERATE_RANDOM_STRING( 10 );
	}
	else if constexpr( !std::is_same_v<char, TestType> && !std::is_same_v<bool, TestType> && !std::is_same_v<std::string, TestType> )
	{
		value = GENERATE_RANDOM( TestType, MAX_REPS );
	}

	REQUIRE( wrap.m_value == value );
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