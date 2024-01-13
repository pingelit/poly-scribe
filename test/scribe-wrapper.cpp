#include "test-helper.hpp"

#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/external/rapidjson/document.h>
#include <poly-scribe/poly-scribe.hpp>
#include <string>

TEMPLATE_TEST_CASE( "scribe-wrapper::base", "[scribe-wrapper][template]", bool, char, int, float, double, long, std::string )
{
	using namespace poly_scribe;
	TestType value { };

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

	std::stringstream string_stream;
	{
		cereal::JSONOutputArchive archive( string_stream );
		archive( poly_scribe::make_scribe_wrap( name, wrap ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::JSONInputArchive archive( string_stream );
		TestType read_object { };
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );

		REQUIRE( value == read_object );
	}

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

TEST_CASE( "scribe-wrapper::pod", "[scribe-wrapper]" )
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

TEST_CASE( "scribe-wrapper::correct-layout", "[scribe-wrapper]" )
{
	std::ostringstream out_stream;
	const auto value = GENERATE_RANDOM( int, 1 );
	const auto name  = GENERATE_RANDOM_STRING( 10 );

	{
		cereal::JSONOutputArchive archive( out_stream );
		archive( poly_scribe::make_scribe_wrap( name, value ) );
	}

	INFO( out_stream.str( ) );

	rapidjson::Document document;
	document.Parse( out_stream.str( ).c_str( ) );

	REQUIRE( document[name.c_str( )] == value );
}

TEMPLATE_TEST_CASE( "scribe-wrapper::base.pointer", "[scribe-wrapper][template]", bool, char, int, float, double, long, std::string )
{
	using namespace poly_scribe;
	auto value = std::make_shared<TestType>( );

	if constexpr( std::is_same_v<bool, TestType> )
	{
		*value = GENERATE( true, false );
	}
	else if constexpr( std::is_same_v<char, TestType> )
	{
		*value = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	}
	else if constexpr( std::is_same_v<std::string, TestType> )
	{
		*value = GENERATE_RANDOM_STRING( 10 );
	}
	else if constexpr( !std::is_same_v<char, TestType> && !std::is_same_v<bool, TestType> && !std::is_same_v<std::string, TestType> )
	{
		*value = GENERATE_RANDOM( TestType, MAX_REPS );
	}

	auto name = GENERATE_RANDOM_STRING( 10 );
	auto wrap = make_scribe_wrap( name, value );
	REQUIRE( wrap.m_name == name );
	REQUIRE( wrap.m_value.m_ptr == value );
	REQUIRE( *wrap.m_value.m_ptr == *value );

	std::stringstream string_stream;
	{
		cereal::JSONOutputArchive archive( string_stream );
		archive( poly_scribe::make_scribe_wrap( name, wrap ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::JSONInputArchive archive( string_stream );
		std::shared_ptr<TestType> read_object;
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );

		REQUIRE( *wrap.m_value.m_ptr == *read_object );
	}

	if constexpr( std::is_same_v<bool, TestType> )
	{
		*value = GENERATE( true, false );
	}
	else if constexpr( std::is_same_v<char, TestType> )
	{
		*value = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	}
	else if constexpr( std::is_same_v<std::string, TestType> )
	{
		*value = GENERATE_RANDOM_STRING( 10 );
	}
	else if constexpr( !std::is_same_v<char, TestType> && !std::is_same_v<bool, TestType> && !std::is_same_v<std::string, TestType> )
	{
		*value = GENERATE_RANDOM( TestType, MAX_REPS );
	}

	REQUIRE( wrap.m_value.m_ptr == value );
	REQUIRE( *wrap.m_value.m_ptr == *value );
}
