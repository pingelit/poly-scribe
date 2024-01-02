#include "test-helper.hpp"

#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
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

TEMPLATE_TEST_CASE( "scribe-pointer-wrapper::base", "[scribe-wrapper][template]", bool, char, int, float, double, long, std::string )
{
	// todo this does not test the pointer wrapper anymore.
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
	REQUIRE( wrap.m_value == value );
	REQUIRE( *wrap.m_value == *value );

	std::ostringstream out_stream;
	{
		cereal::JSONOutputArchive archive( out_stream );
		archive( poly_scribe::make_scribe_wrap( name, wrap ) );
	}
	INFO( out_stream.str( ) );

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

	REQUIRE( wrap.m_value == value );
	REQUIRE( *wrap.m_value == *value );
}

TEST_CASE( "scribe-pointer-wrapper::correct-layout", "[scribe-wrapper]" )
{
	SECTION( "unregistered, derived ptr" )
	{
		std::ostringstream out_stream;
		auto object             = std::make_shared<UnregisteredDerived>( );
		object->m_base_value    = GENERATE_RANDOM( double, 1 );
		object->m_derived_value = GENERATE_RANDOM( int, 1 );
		const auto name         = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::JSONOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );

		rapidjson::Document document;
		document.Parse( out_stream.str( ).c_str( ) );

		rapidjson::Value json_object;
		REQUIRE_NOTHROW( json_object = document[name.c_str( )] );
		REQUIRE( json_object["type"] == "unknown" );
		REQUIRE( json_object["base_value"] == object->m_base_value );
		REQUIRE( json_object["derived_value"] == object->m_derived_value );
	}

	SECTION( "Registered, derived ptr" )
	{
		std::ostringstream out_stream;
		auto object             = std::make_shared<RegisteredDerived>( );
		object->m_base_value    = GENERATE_RANDOM( double, 1 );
		object->m_derived_value = GENERATE_RANDOM( int, 1 );
		const auto name         = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::JSONOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );

		rapidjson::Document document;
		document.Parse( out_stream.str( ).c_str( ) );

		rapidjson::Value json_object;
		REQUIRE_NOTHROW( json_object = document[name.c_str( )] );
		REQUIRE( json_object["type"] == "RegisteredDerived" );
		REQUIRE( json_object["base_value"] == object->m_base_value );
		REQUIRE( json_object["derived_value"] == object->m_derived_value );
	}

	SECTION( "unregistered, base ptr" )
	{
		std::ostringstream out_stream;
		std::shared_ptr<Base> object   = std::make_shared<RegisteredDerived>( );
		auto object_casted             = std::dynamic_pointer_cast<RegisteredDerived>( object );
		object_casted->m_base_value    = GENERATE_RANDOM( double, 1 );
		object_casted->m_derived_value = GENERATE_RANDOM( int, 1 );
		const auto name                = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::JSONOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );

		rapidjson::Document document;
		document.Parse( out_stream.str( ).c_str( ) );

		rapidjson::Value json_object;
		REQUIRE_NOTHROW( json_object = document[name.c_str( )] );
		REQUIRE( json_object["type"] == "RegisteredDerived" );
		REQUIRE( json_object["base_value"] == object_casted->m_base_value );
		REQUIRE( json_object["derived_value"] == object_casted->m_derived_value );
	}

	SECTION( "Registered, base ptr" )
	{
		std::ostringstream out_stream;
		std::shared_ptr<Base> object   = std::make_shared<RegisteredDerived>( );
		auto object_casted             = std::dynamic_pointer_cast<RegisteredDerived>( object );
		object_casted->m_base_value    = GENERATE_RANDOM( double, 1 );
		object_casted->m_derived_value = GENERATE_RANDOM( int, 1 );
		const auto name                = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::JSONOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );

		rapidjson::Document document;
		document.Parse( out_stream.str( ).c_str( ) );

		rapidjson::Value json_object;
		REQUIRE_NOTHROW( json_object = document[name.c_str( )] );
		REQUIRE( json_object["type"] == "RegisteredDerived" );
		REQUIRE( json_object["base_value"] == object_casted->m_base_value );
		REQUIRE( json_object["derived_value"] == object_casted->m_derived_value );
	}
}

TEMPLATE_TEST_CASE( "scribe-pointer-wrapper::casting", "[scribe-wrapper]", Base, RegisteredDerived )
{
	std::stringstream string_stream;
	std::shared_ptr<TestType> object = std::make_shared<RegisteredDerived>( );
	auto object_casted               = std::dynamic_pointer_cast<RegisteredDerived>( object );
	object_casted->m_base_value      = GENERATE_RANDOM( double, 1 );
	object_casted->m_derived_value   = GENERATE_RANDOM( int, 1 );
	const auto name                  = GENERATE_RANDOM_STRING( 10 );

	{
		cereal::JSONOutputArchive archive( string_stream );
		// poly_scribe::make_scribe_wrap( name, object ).save( archive );
		archive( poly_scribe::make_scribe_wrap( name, object ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::JSONInputArchive archive( string_stream );
		std::shared_ptr<TestType> read_object;
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );

		auto read_object_casted = std::dynamic_pointer_cast<RegisteredDerived>( read_object );

		REQUIRE( read_object_casted->m_base_value == object_casted->m_base_value );
		REQUIRE( read_object_casted->m_derived_value == object_casted->m_derived_value );
	}
}