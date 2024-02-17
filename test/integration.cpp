#include "integration_data.h"
#include "test-helper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/external/rapidjson/document.h>
#include <memory>

// NOLINTBEGIN(readability-function-cognitive-complexity)

///
/// \brief This function compares two instances of DerivedOne.
/// \param t_lhs left hand side
/// \param t_rhs right hand side
///
void compare_derived_one( const integration_space::DerivedOne& t_lhs, const integration_space::DerivedOne& t_rhs )
{
	REQUIRE( t_lhs.string_map == t_rhs.string_map );
}

///
/// \brief This function compares two instances of DerivedTwo.
/// \param t_lhs left hand side
/// \param t_rhs right hand side
///
void compare_derived_two( const integration_space::DerivedTwo& t_lhs, const integration_space::DerivedTwo& t_rhs )
{
	REQUIRE( t_lhs.optional_value == t_rhs.optional_value );
}

///
/// \brief This function compares two pointers to the base type.
/// \param t_lhs left hand side pointer
/// \param t_rhs right hand side pointer
///
void compare_pointers_to_base_type( const std::shared_ptr<integration_space::Base>& t_lhs, const std::shared_ptr<integration_space::Base>& t_rhs )
{
	REQUIRE( t_lhs->vec_3d == t_rhs->vec_3d );
	REQUIRE( t_lhs->union_member == t_rhs->union_member );
	REQUIRE( t_lhs->str_vec == t_rhs->str_vec );

	if( auto lhs = std::dynamic_pointer_cast<integration_space::DerivedOne>( t_lhs ) )
	{
		auto rhs = std::dynamic_pointer_cast<integration_space::DerivedOne>( t_rhs );
		REQUIRE( rhs );
		compare_derived_one( *lhs, *rhs );
	}
	else if( auto lhs = std::dynamic_pointer_cast<integration_space::DerivedTwo>( t_lhs ) )
	{
		auto rhs = std::dynamic_pointer_cast<integration_space::DerivedTwo>( t_rhs );
		REQUIRE( rhs );
		compare_derived_two( *lhs, *rhs );
	}
	else
	{
		FAIL( "Invalid pointer type" );
	}
}

///
/// \brief This function compares a JSON value with a pointers to the Base.
/// \param t_lhs left hand side json value
/// \param t_rhs right hand side pointer
///
void compare_json_to_base( const rapidjson::Value& t_lhs, const std::shared_ptr<integration_space::Base>& t_rhs )
{
	REQUIRE( t_lhs.HasMember( "vec_3d" ) );
	REQUIRE( t_lhs["vec_3d"].IsArray( ) );
	REQUIRE( t_lhs["vec_3d"].Size( ) == 3 );
	REQUIRE( t_lhs["vec_3d"][0].IsDouble( ) );
	REQUIRE( t_lhs["vec_3d"][0].GetDouble( ) == t_rhs->vec_3d[0] );
	REQUIRE( t_lhs["vec_3d"][1].IsDouble( ) );
	REQUIRE( t_lhs["vec_3d"][1].GetDouble( ) == t_rhs->vec_3d[1] );
	REQUIRE( t_lhs["vec_3d"][2].IsDouble( ) );
	REQUIRE( t_lhs["vec_3d"][2].GetDouble( ) == t_rhs->vec_3d[2] );

	const auto index = static_cast<std::int32_t>( t_rhs->union_member.index( ) );
	REQUIRE( t_lhs.HasMember( "union_member" ) );
	REQUIRE( t_lhs["union_member"].HasMember( "index" ) );
	REQUIRE( t_lhs["union_member"]["index"].IsInt( ) );
	REQUIRE( t_lhs["union_member"]["index"].GetInt( ) == index );

	if( std::holds_alternative<int>( t_rhs->union_member ) )
	{
		REQUIRE( t_lhs["union_member"]["data"].IsInt( ) );
		REQUIRE( t_lhs["union_member"]["data"].GetInt( ) == std::get<int>( t_rhs->union_member ) );
	}
	else if( std::holds_alternative<double>( t_rhs->union_member ) )
	{
		REQUIRE( t_lhs["union_member"]["data"].IsDouble( ) );
		REQUIRE( t_lhs["union_member"]["data"].GetDouble( ) == std::get<double>( t_rhs->union_member ) );
	}

	REQUIRE( t_lhs.HasMember( "str_vec" ) );
	REQUIRE( t_lhs["str_vec"].IsArray( ) );
	REQUIRE( t_lhs["str_vec"].Size( ) == t_rhs->str_vec.size( ) );
	for( size_t i = 0; i < t_rhs->str_vec.size( ); ++i )
	{
		REQUIRE( t_lhs["str_vec"][i].IsString( ) );
		REQUIRE( t_lhs["str_vec"][i].GetString( ) == t_rhs->str_vec[i] );
	}
}

///
/// \brief This function compares a JSON value with a pointers to the DerivedOne.
/// \param t_lhs left hand side json value
/// \param t_rhs right hand side pointer
///
void compare_json_to_derived_one( const rapidjson::Value& t_lhs, const std::shared_ptr<integration_space::DerivedOne>& t_rhs )
{
	REQUIRE( t_lhs.IsObject( ) );
	REQUIRE( t_lhs.HasMember( "type" ) );
	REQUIRE( t_lhs["type"].IsString( ) );
	REQUIRE( t_lhs["type"].GetString( ) == std::string( "DerivedOne" ) );

	compare_json_to_base( t_lhs, t_rhs );

	REQUIRE( t_lhs.HasMember( "string_map" ) );
	REQUIRE( t_lhs["string_map"].IsObject( ) );
	REQUIRE( t_lhs["string_map"].MemberCount( ) == t_rhs->string_map.size( ) );
	for( const auto& [key, value]: t_rhs->string_map )
	{
		REQUIRE( t_lhs["string_map"].HasMember( key.c_str( ) ) );
		REQUIRE( t_lhs["string_map"][key.c_str( )].IsString( ) );
		REQUIRE( t_lhs["string_map"][key.c_str( )].GetString( ) == value );
	}
}

///
/// \brief This function compares a JSON value with a pointers to the DerivedTwo.
/// \param t_lhs left hand side json value
/// \param t_rhs right hand side pointer
///
void compare_json_to_derived_two( const rapidjson::Value& t_lhs, const std::shared_ptr<integration_space::DerivedTwo>& t_rhs )
{
	REQUIRE( t_lhs.IsObject( ) );
	REQUIRE( t_lhs.HasMember( "type" ) );
	REQUIRE( t_lhs["type"].IsString( ) );
	REQUIRE( t_lhs["type"].GetString( ) == std::string( "DerivedTwo" ) );

	compare_json_to_base( t_lhs, t_rhs );

	REQUIRE( t_lhs["optional_value"].IsDouble( ) );
	REQUIRE( t_lhs["optional_value"].GetDouble( ) == t_rhs->optional_value );
}

///
/// \brief This function compares a JSON value with a pointers to the base type.
/// \param t_lhs left hand side json value
/// \param t_rhs right hand side pointer
///
void compare_json_to_base_type( const rapidjson::Value& t_lhs, const std::shared_ptr<integration_space::Base>& t_rhs )
{
	if( auto derived_one = std::dynamic_pointer_cast<integration_space::DerivedOne>( t_rhs ) )
	{
		compare_json_to_derived_one( t_lhs, derived_one );
	}
	else if( auto derived_two = std::dynamic_pointer_cast<integration_space::DerivedTwo>( t_rhs ) )
	{
		compare_json_to_derived_two( t_lhs, derived_two );
	}
	else
	{
		FAIL( "Invalid pointer type" );
	}
}

///
/// \brief This function compares a JSON value with a IntegrationTest.
/// \param t_lhs left hand side json value
/// \param t_rhs right hand side object
///
void compare_json_to_integration_test( rapidjson::Value& t_json_value, const integration_space::IntegrationTest& t_data )
{
	REQUIRE( t_json_value.IsObject( ) );
	REQUIRE( t_json_value.HasMember( "object_map" ) );
	REQUIRE( t_json_value.HasMember( "object_vec" ) );
	REQUIRE( t_json_value.HasMember( "object_array" ) );
	REQUIRE( t_json_value.HasMember( "enum_value" ) );
	REQUIRE( t_json_value.HasMember( "non_poly_derived" ) );

	rapidjson::Value json_object_map;
	rapidjson::Value json_object_vec;
	rapidjson::Value json_object_array;
	rapidjson::Value json_enum_value;
	rapidjson::Value json_non_poly_derived;

	REQUIRE_NOTHROW( json_object_map = t_json_value["object_map"] );
	REQUIRE_NOTHROW( json_object_vec = t_json_value["object_vec"] );
	REQUIRE_NOTHROW( json_object_array = t_json_value["object_array"] );
	REQUIRE_NOTHROW( json_enum_value = t_json_value["enum_value"] );
	REQUIRE_NOTHROW( json_non_poly_derived = t_json_value["non_poly_derived"] );

	REQUIRE( json_object_map.IsObject( ) );

	REQUIRE( json_object_vec.IsArray( ) );
	REQUIRE( json_object_vec.Size( ) == t_data.object_vec.size( ) );

	REQUIRE( json_object_array.IsArray( ) );
	REQUIRE( json_object_array.Size( ) == t_data.object_array.size( ) );

	REQUIRE( json_enum_value.IsInt( ) );
	REQUIRE( json_enum_value.GetInt( ) == static_cast<int>( t_data.enum_value ) );

	REQUIRE( json_non_poly_derived.IsObject( ) );
	REQUIRE( json_non_poly_derived.HasMember( "value" ) );
	REQUIRE( json_non_poly_derived["value"].IsInt( ) );
	REQUIRE( json_non_poly_derived["value"].GetInt( ) == t_data.non_poly_derived.value );

	for( const auto& [key, value]: t_data.object_map )
	{
		REQUIRE( json_object_map.HasMember( key.c_str( ) ) );
		rapidjson::Value json_ptr;
		REQUIRE_NOTHROW( json_ptr = json_object_map[key.c_str( )] );
		compare_json_to_base_type( json_ptr, value );
	}

	for( size_t i = 0; i < t_data.object_vec.size( ); ++i )
	{
		rapidjson::Value json_ptr;
		REQUIRE_NOTHROW( json_ptr = json_object_vec[i] );
		compare_json_to_base_type( json_ptr, t_data.object_vec[i] );
	}

	for( size_t i = 0; i < t_data.object_array.size( ); ++i )
	{
		rapidjson::Value json_ptr;
		REQUIRE_NOTHROW( json_ptr = json_object_array[i] );
		compare_json_to_base_type( json_ptr, t_data.object_array[i] );
	}
}

void generate_random_base( const std::shared_ptr<integration_space::Base>& t_ptr )
{
	t_ptr->vec_3d[0]    = GENERATE_RANDOM( double, 1 );
	t_ptr->vec_3d[1]    = GENERATE_RANDOM( double, 1 );
	t_ptr->vec_3d[2]    = GENERATE_RANDOM( double, 1 );
	t_ptr->union_member = GENERATE( 42, 3.141 ); // NOLINT
	const auto values   = GENERATE_VECTOR_OF_STRINGS( 5, 10 );
	auto vec_size       = GENERATE( take( 1, random( 1, 5 ) ) );
	for( int i = 0; i < vec_size; ++i )
	{
		t_ptr->str_vec.push_back( values[i] );
	}
}

std::shared_ptr<integration_space::DerivedOne> generate_random_one( )
{
	auto ptr = std::make_shared<integration_space::DerivedOne>( );

	generate_random_base( ptr );

	const auto keys   = GENERATE_VECTOR_OF_STRINGS( 5, 5 );
	const auto values = GENERATE_VECTOR_OF_STRINGS( 5, 5 );

	auto size = GENERATE( take( 1, random( 1, 5 ) ) );

	for( int i = 0; i < size; ++i )
	{
		ptr->string_map.emplace( keys[i], values[i] );
	}

	return ptr;
}

std::shared_ptr<integration_space::DerivedTwo> generate_random_two( )
{
	auto ptr = std::make_shared<integration_space::DerivedTwo>( );

	generate_random_base( ptr );

	return ptr;
}

integration_space::NonPolyDerived generate_random_non_poly_derived( )
{
	integration_space::NonPolyDerived object { };
	object.value = GENERATE_RANDOM( int, 1 );
	return object;
}


integration_space::IntegrationTest generate_random_integration_dict( )
{
	integration_space::IntegrationTest object;

	object.object_map.emplace( "one", generate_random_one( ) );
	object.object_map.emplace( "two", generate_random_two( ) );

	object.object_vec.push_back( generate_random_one( ) );
	object.object_vec.push_back( generate_random_two( ) );

	object.object_array[0] = generate_random_one( );
	object.object_array[1] = generate_random_two( );

	object.enum_value = GENERATE( integration_space::Enumeration::value1, integration_space::Enumeration::value2 );

	object.non_poly_derived = generate_random_non_poly_derived( );

	return object;
}

TEST_CASE( "integration", "[integration]" )
{
	auto data = generate_random_integration_dict( );
	auto name = GENERATE_RANDOM_STRING( 10 );

	std::stringstream string_stream;
	{
		cereal::JSONOutputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		archive( poly_scribe::make_scribe_wrap( name, data ) );
	}
	INFO( string_stream.str( ) );

	integration_space::IntegrationTest read_object;
	{
		cereal::JSONInputArchive archive( string_stream ); // NOLINT(misc-const-correctness)
		archive( poly_scribe::make_scribe_wrap( name, read_object ) );
	}

	// test that all members of read_object are equal the members in data
	REQUIRE( data.object_map.size( ) == read_object.object_map.size( ) );
	REQUIRE( data.object_vec.size( ) == read_object.object_vec.size( ) );
	REQUIRE( data.object_array.size( ) == read_object.object_array.size( ) );
	REQUIRE( data.enum_value == read_object.enum_value );
	REQUIRE( data.non_poly_derived.value == read_object.non_poly_derived.value );

	for( const auto& [key, value]: data.object_map )
	{
		REQUIRE( read_object.object_map.count( key ) == 1 );
		compare_pointers_to_base_type( value, read_object.object_map.at( key ) );
	}

	for( size_t i = 0; i < data.object_vec.size( ); ++i )
	{
		compare_pointers_to_base_type( data.object_vec.at( i ), read_object.object_vec.at( i ) );
	}

	for( size_t i = 0; i < data.object_array.size( ); ++i )
	{
		compare_pointers_to_base_type( data.object_array.at( i ), read_object.object_array.at( i ) );
	}

	rapidjson::Document document;
	document.Parse( string_stream.str( ).c_str( ) );
	rapidjson::Value json_value;
	REQUIRE_NOTHROW( json_value = document[name.c_str( )] );

	compare_json_to_integration_test( json_value, data );
}

// NOLINTEND(readability-function-cognitive-complexity)