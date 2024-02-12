#include "test-helper.hpp"

#include <array>
#include <catch2/catch_all.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/external/rapidjson/document.h>
#include <poly-scribe/poly-scribe.hpp>
#include <string>

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEMPLATE_PRODUCT_TEST_CASE_SIG( "scribe-map-wrapper::xml-serialization", "[scribe-wrapper]", ( ( typename T, size_t S ), T, S ), ( std::map, std::unordered_map ),
                                ( (std::string, int), (std::string, double), ( std::string, std::string ), (std::string, std::shared_ptr<RegisteredDerived>),
                                  (int, int)) )
{
	std::stringstream string_stream;
	TestType object { };
	const auto name           = GENERATE_RANDOM_STRING( 10 );
	const auto container_size = GENERATE( 0, 1, 5 );

	if constexpr( std::is_same_v<std::string, std::remove_cv_t<typename TestType::key_type>> )
	{
		auto random_names = GENERATE_VECTOR_OF_STRINGS( 5, 5 );
		if constexpr( std::is_arithmetic_v<typename TestType::mapped_type> )
		{
			auto random_values = GENERATE( chunk(
			    5, take( 5, random( std::numeric_limits<typename TestType::mapped_type>::min( ), std::numeric_limits<typename TestType::mapped_type>::max( ) ) ) ) );

			for( auto i = 0; i < container_size; ++i )
			{
				object.emplace( random_names[i], random_values[i] );
			}
		}
		if constexpr( std::is_same_v<std::string, typename TestType::mapped_type> )
		{
			auto random_values = GENERATE_VECTOR_OF_STRINGS( 5, 10 );
			for( auto i = 0; i < container_size; ++i )
			{
				object.emplace( random_names[i], random_values[i] );
			}
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::mapped_type> )
		{
			for( auto i = 0; i < container_size; ++i )
			{
				auto value             = std::make_shared<RegisteredDerived>( );
				value->m_base_value    = GENERATE_RANDOM( double, 1 );
				value->m_derived_value = GENERATE_RANDOM( int, 1 );
				object.emplace( random_names[i], value );
			}
		}
	}
	if constexpr( std::is_arithmetic_v<std::remove_cv_t<typename TestType::key_type>> )
	{
		auto random_keys = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::key_type>::min( ), std::numeric_limits<typename TestType::key_type>::max( ) ) ) ) );
		auto random_values = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::mapped_type>::min( ), std::numeric_limits<typename TestType::mapped_type>::max( ) ) ) ) );

		for( auto i = 0; i < container_size; ++i )
		{
			object.emplace( random_keys[i], random_values[i] );
		}
	}

	{
		cereal::XMLOutputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		archive( poly_scribe::make_scribe_wrap( name, object ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::XMLInputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		TestType read_object { };
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );
		if constexpr( std::is_same_v<std::string, std::remove_cv_t<typename TestType::key_type>> )
		{
			if constexpr( std::is_arithmetic_v<typename TestType::mapped_type> || std::is_same_v<std::string, typename TestType::mapped_type> )
			{
				REQUIRE( read_object == object );
			}
			if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::mapped_type> )
			{
				REQUIRE( read_object.size( ) == object.size( ) );

				for( auto&& [key, value]: read_object )
				{
					auto iterator = object.find( key );
					REQUIRE( iterator != object.end( ) );
					REQUIRE( *( iterator->second ) == *value );
				}
			}
		}
		if constexpr( std::is_arithmetic_v<std::remove_cv_t<typename TestType::key_type>> )
		{
			REQUIRE( read_object == object );
		}
	}
}


TEMPLATE_PRODUCT_TEST_CASE_SIG( "scribe-map-wrapper::json-correct-layout", "[scribe-wrapper]", ( ( typename T, size_t S ), T, S ), ( std::map, std::unordered_map ),
                                ( (std::string, int), (std::string, double), ( std::string, std::string ), (std::string, std::shared_ptr<RegisteredDerived>),
                                  (int, int)) )
{
	std::stringstream string_stream;
	TestType object { };
	const auto name           = GENERATE_RANDOM_STRING( 10 );
	const auto container_size = GENERATE( 0, 1, 5 );

	if constexpr( std::is_same_v<std::string, std::remove_cv_t<typename TestType::key_type>> )
	{
		auto random_names = GENERATE_VECTOR_OF_STRINGS( 5, 5 );
		if constexpr( std::is_arithmetic_v<typename TestType::mapped_type> )
		{
			auto random_values = GENERATE( chunk(
			    5, take( 5, random( std::numeric_limits<typename TestType::mapped_type>::min( ), std::numeric_limits<typename TestType::mapped_type>::max( ) ) ) ) );

			for( auto i = 0; i < container_size; ++i )
			{
				object.emplace( random_names[i], random_values[i] );
			}
		}
		if constexpr( std::is_same_v<std::string, typename TestType::mapped_type> )
		{
			auto random_values = GENERATE_VECTOR_OF_STRINGS( 5, 10 );
			for( auto i = 0; i < container_size; ++i )
			{
				object.emplace( random_names[i], random_values[i] );
			}
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::mapped_type> )
		{
			for( auto i = 0; i < container_size; ++i )
			{
				auto value             = std::make_shared<RegisteredDerived>( );
				value->m_base_value    = GENERATE_RANDOM( double, 1 );
				value->m_derived_value = GENERATE_RANDOM( int, 1 );
				object.emplace( random_names[i], value );
			}
		}
	}
	if constexpr( std::is_arithmetic_v<std::remove_cv_t<typename TestType::key_type>> )
	{
		auto random_keys = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::key_type>::min( ), std::numeric_limits<typename TestType::key_type>::max( ) ) ) ) );
		auto random_values = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::mapped_type>::min( ), std::numeric_limits<typename TestType::mapped_type>::max( ) ) ) ) );

		for( auto i = 0; i < container_size; ++i )
		{
			object.emplace( random_keys[i], random_values[i] );
		}
	}

	{
		cereal::JSONOutputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		archive( poly_scribe::make_scribe_wrap( name, object ) );
	}
	INFO( string_stream.str( ) );

	{
		cereal::JSONInputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		TestType read_object { };
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );
		if constexpr( std::is_same_v<std::string, std::remove_cv_t<typename TestType::key_type>> )
		{
			if constexpr( std::is_arithmetic_v<typename TestType::mapped_type> || std::is_same_v<std::string, typename TestType::mapped_type> )
			{
				REQUIRE( read_object == object );
			}
			if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::mapped_type> )
			{
				REQUIRE( read_object.size( ) == object.size( ) );

				for( auto&& [key, value]: read_object )
				{
					auto iterator = object.find( key );
					REQUIRE( iterator != object.end( ) );
					REQUIRE( *( iterator->second ) == *value );
				}
			}
		}
		if constexpr( std::is_arithmetic_v<std::remove_cv_t<typename TestType::key_type>> )
		{
			REQUIRE( read_object == object );
		}
	}

	rapidjson::Document document;
	document.Parse( string_stream.str( ).c_str( ) );
	rapidjson::Value json_map;
	REQUIRE_NOTHROW( json_map = document[name.c_str( )] );

	const double accuracy = 0.001;

	if constexpr( std::is_same_v<std::string, std::remove_cv_t<typename TestType::key_type>> )
	{
		REQUIRE( json_map.IsObject( ) );
		REQUIRE( json_map.MemberCount( ) == container_size );

		for( auto&& [key, value]: object )
		{
			if constexpr( std::is_integral_v<typename TestType::mapped_type> )
			{
				REQUIRE( json_map[key.c_str( )].GetInt( ) == value );
			}
			if constexpr( std::is_floating_point_v<typename TestType::mapped_type> )
			{
				REQUIRE_THAT( json_map[key.c_str( )].GetDouble( ), Catch::Matchers::WithinRel( value, accuracy ) );
			}
			if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::mapped_type> )
			{
				REQUIRE( json_map[key.c_str( )]["type"] == "RegisteredDerived" );
				REQUIRE_THAT( json_map[key.c_str( )]["base_value"].GetDouble( ), Catch::Matchers::WithinRel( value->m_base_value, accuracy ) );
				REQUIRE( json_map[key.c_str( )]["derived_value"].GetInt( ) == value->m_derived_value );
			}
		}
	}
	if constexpr( std::is_arithmetic_v<std::remove_cv_t<typename TestType::key_type>> )
	{
		REQUIRE( json_map.IsArray( ) );
		REQUIRE( json_map.Size( ) == container_size );

		auto counter = 0;

		for( auto&& [key, value]: object )
		{
			REQUIRE( json_map[counter].IsObject( ) );
			REQUIRE( json_map[counter]["key"] == key );
			REQUIRE( json_map[counter++]["value"] == value );
		}
	}
}

// NOLINTEND(readability-function-cognitive-complexity)