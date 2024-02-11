#include "test-helper.hpp"

#include <array>
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

TEMPLATE_PRODUCT_TEST_CASE( "scribe-container-wrapper::correct-layout", "[scribe-wrapper]", ( std::vector, std::list ), (int, double, std::shared_ptr<RegisteredDerived>))
{
	std::stringstream string_stream;
	TestType object { };
	const auto name           = GENERATE_RANDOM_STRING( 10 );
	const auto container_size = GENERATE( 0, 1, 5 );
	object.resize( container_size );

	if constexpr( std::is_arithmetic_v<typename TestType::value_type> )
	{
		auto random_values = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::value_type>::min( ), std::numeric_limits<typename TestType::value_type>::max( ) ) ) ) );
		auto counter = 0;
		for( auto&& value: object )
		{
			value = random_values[counter++];
		}
	}
	if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
	{
		for( auto&& value: object )
		{
			value                  = std::make_shared<RegisteredDerived>( );
			value->m_base_value    = GENERATE_RANDOM( double, 1 );
			value->m_derived_value = GENERATE_RANDOM( int, 1 );
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
		if constexpr( std::is_arithmetic_v<typename TestType::value_type> )
		{
			REQUIRE_THAT( read_object, Catch::Matchers::RangeEquals( object ) );
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
		{
			REQUIRE_THAT( read_object, Catch::Matchers::RangeEquals( object, []( const std::shared_ptr<RegisteredDerived>& lhs,
			                                                                     const std::shared_ptr<RegisteredDerived>& rhs ) { return *lhs == *rhs; } ) );
		}
	}

	rapidjson::Document document;
	document.Parse( string_stream.str( ).c_str( ) );
	rapidjson::Value json_array;
	REQUIRE_NOTHROW( json_array = document[name.c_str( )] );
	REQUIRE( json_array.IsArray( ) );
	REQUIRE( json_array.Size( ) == container_size );


	const double accuracy = 0.001;
	auto counter          = 0;
	for( const auto& value: object )
	{
		if constexpr( std::is_integral_v<typename TestType::value_type> )
		{
			REQUIRE( json_array[counter++].GetInt64( ) == value );
		}
		if constexpr( std::is_floating_point_v<typename TestType::value_type> )
		{
			REQUIRE_THAT( json_array[counter++].GetDouble( ), Catch::Matchers::WithinRel( value, accuracy ) );
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
		{
			REQUIRE( json_array[counter]["type"] == "RegisteredDerived" );
			REQUIRE_THAT( json_array[counter]["base_value"].GetDouble( ), Catch::Matchers::WithinRel( value->m_base_value, accuracy ) );
			REQUIRE( json_array[counter++]["derived_value"].GetInt( ) == value->m_derived_value );
		}
	}
}

TEMPLATE_PRODUCT_TEST_CASE_SIG( "scribe-container-wrapper::correct-layout", "[scribe-wrapper]", ( ( typename T, size_t S ), T, S ), ( std::array ),
                                ( ( int, 0 ), ( int, 1 ), ( int, 5 ), ( double, 0 ), ( double, 1 ), ( double, 5 ), ( std::shared_ptr<RegisteredDerived>, 0 ),
                                  ( std::shared_ptr<RegisteredDerived>, 1 ), ( std::shared_ptr<RegisteredDerived>, 5 ) ) )
{
	std::stringstream string_stream;
	TestType object { };
	const auto name = GENERATE_RANDOM_STRING( 10 );

	if constexpr( std::is_arithmetic_v<typename TestType::value_type> )
	{
		auto random_values = GENERATE(
		    chunk( 5, take( 5, random( std::numeric_limits<typename TestType::value_type>::min( ), std::numeric_limits<typename TestType::value_type>::max( ) ) ) ) );
		auto counter = 0;
		for( auto&& value: object )
		{
			value = random_values[counter++];
		}
	}
	if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
	{
		for( auto&& value: object )
		{
			value                  = std::make_shared<RegisteredDerived>( );
			value->m_base_value    = GENERATE_RANDOM( double, 1 );
			value->m_derived_value = GENERATE_RANDOM( int, 1 );
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
		if constexpr( std::is_arithmetic_v<typename TestType::value_type> )
		{
			REQUIRE_THAT( read_object, Catch::Matchers::RangeEquals( object ) );
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
		{
			REQUIRE_THAT( read_object, Catch::Matchers::RangeEquals( object, []( const std::shared_ptr<RegisteredDerived>& lhs,
			                                                                     const std::shared_ptr<RegisteredDerived>& rhs ) { return *lhs == *rhs; } ) );
		}
	}

	rapidjson::Document document;
	document.Parse( string_stream.str( ).c_str( ) );
	rapidjson::Value json_array;
	REQUIRE_NOTHROW( json_array = document[name.c_str( )] );
	REQUIRE( json_array.IsArray( ) );


	const double accuracy = 0.001;
	auto counter          = 0;
	for( const auto& value: object )
	{
		if constexpr( std::is_integral_v<typename TestType::value_type> )
		{
			REQUIRE( json_array[counter++].GetInt64( ) == value );
		}
		if constexpr( std::is_floating_point_v<typename TestType::value_type> )
		{
			REQUIRE_THAT( json_array[counter++].GetDouble( ), Catch::Matchers::WithinRel( value, accuracy ) );
		}
		if constexpr( std::is_same_v<std::shared_ptr<RegisteredDerived>, typename TestType::value_type> )
		{
			REQUIRE( json_array[counter]["type"] == "RegisteredDerived" );
			REQUIRE_THAT( json_array[counter]["base_value"].GetDouble( ), Catch::Matchers::WithinRel( value->m_base_value, accuracy ) );
			REQUIRE( json_array[counter++]["derived_value"].GetInt( ) == value->m_derived_value );
		}
	}
}

// NOLINTEND(readability-function-cognitive-complexity)