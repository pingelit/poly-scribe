#include "integration_data.h"
#include "test-helper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/json.hpp>
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


void generate_random_base( const std::shared_ptr<integration_space::Base>& t_ptr )
{
	t_ptr->vec_3d[0]    = GENERATE_RANDOM( double, 1 );
	t_ptr->vec_3d[1]    = GENERATE_RANDOM( double, 1 );
	t_ptr->vec_3d[2]    = GENERATE_RANDOM( double, 1 );
	t_ptr->union_member = GENERATE( 42, 3.141 ); // NOLINT
	auto vec_size       = GENERATE( take( 1, random( 1, 5 ) ) );
	for( int i = 0; i < vec_size; ++i )
	{
		t_ptr->str_vec.push_back( GENERATE_RANDOM_STRING( 10 ) );
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
	integration_space::NonPolyDerived object;
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
}

// NOLINTEND(readability-function-cognitive-complexity)