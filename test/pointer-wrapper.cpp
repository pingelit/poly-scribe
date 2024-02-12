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

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST_CASE( "scribe-pointer-wrapper::base", "[scribe-wrapper]" )
{
	const std::ostringstream out_stream;
	auto object             = std::make_shared<RegisteredDerived>( );
	object->m_base_value    = GENERATE_RANDOM( double, 1 );
	object->m_derived_value = GENERATE_RANDOM( int, 1 );

	auto name = GENERATE_RANDOM_STRING( 10 );
	auto wrap = poly_scribe::make_scribe_wrap( name, object );
	REQUIRE( wrap.m_name == name );
	REQUIRE( wrap.m_value.m_ptr->m_base_value == object->m_base_value );
	REQUIRE( wrap.m_value.m_ptr->m_derived_value == object->m_derived_value );

	object->m_base_value    = GENERATE_RANDOM( double, 1 );
	object->m_derived_value = GENERATE_RANDOM( int, 1 );

	REQUIRE( wrap.m_value.m_ptr->m_base_value == object->m_base_value );
	REQUIRE( wrap.m_value.m_ptr->m_derived_value == object->m_derived_value );
}

TEST_CASE( "scribe-pointer-wrapper::correct-layout", "[scribe-wrapper]" )
{
	SECTION( "Pod ptr" )
	{
		std::ostringstream out_stream;
		auto object     = std::make_shared<int>( GENERATE_RANDOM( int, 1 ) );
		const auto name = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::JSONOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );

		rapidjson::Document document;
		document.Parse( out_stream.str( ).c_str( ) );

		REQUIRE( document[name.c_str( )] == *object );
	}

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
		cereal::JSONOutputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		// poly_scribe::make_scribe_wrap( name, object ).save( archive );
		archive( poly_scribe::make_scribe_wrap( name, object ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::JSONInputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		std::shared_ptr<TestType> read_object;
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );

		auto read_object_casted = std::dynamic_pointer_cast<RegisteredDerived>( read_object );

		REQUIRE( read_object_casted->m_base_value == object_casted->m_base_value );
		REQUIRE( read_object_casted->m_derived_value == object_casted->m_derived_value );
	}
}

TEST_CASE( "scribe-pointer-wrapper::correct-layout-xml", "[scribe-wrapper]" )
{
	SECTION( "Pod ptr" )
	{
		std::ostringstream out_stream;
		auto object     = std::make_shared<int>( GENERATE_RANDOM( int, 1 ) );
		const auto name = GENERATE_RANDOM_STRING( 10 );

		{
			cereal::XMLOutputArchive archive( out_stream );
			archive( poly_scribe::make_scribe_wrap( name, object ) );
		}
		INFO( out_stream.str( ) );
	}

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
	}
}

// NOLINTEND(readability-function-cognitive-complexity)