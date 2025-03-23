#include "poly-scribe-structs/integration_data.hpp"

#include <array>
#include <fstream>
#include <random>
#include <rfl/json.hpp>
#include <string>


constexpr int MAX_STRING_SIZE     = 5;
constexpr int RAND_LIMIT          = 10;
constexpr double RAND_LIMIT_FLOAT = 10.;


std::string random_string( )
{
	static const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string result;
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );
	std::uniform_int_distribution<> dis( 0, alphabet.size( ) - 1 );
	for( int i = 0; i < MAX_STRING_SIZE; ++i )
	{
		result += alphabet[dis( gen )];
	}
	return result;
}

int random_int( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );
	std::uniform_int_distribution dis( -RAND_LIMIT, RAND_LIMIT );
	return dis( gen );
}

double random_double( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );
	std::uniform_real_distribution dis( -RAND_LIMIT_FLOAT, RAND_LIMIT_FLOAT );
	return dis( gen );
}

// void gen_random_base( integration_space::Base_t& t_base )
// {
// 	std::random_device random_distribution;
// 	std::mt19937 gen( random_distribution( ) );

// 	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );
// 	std::uniform_real_distribution dis_real( -RAND_LIMIT_FLOAT, RAND_LIMIT_FLOAT );


// 	t_base->union_member = ( dis_int( gen ) % 2 == 0 ) ? dis_real( gen ) : dis_int( gen );
// }

integration_space::Base_t gen_random_derived_one( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );

	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );

	integration_space::DerivedOne derived_one { };

	derived_one.union_member = ( dis_int( gen ) % 2 == 0 ) ? random_double( ) : random_int( );

	for( int i = 0; i < 3; ++i )
	{
		derived_one.vec_3d[i] = random_double( );
	}

	int str_vec_size = random_int( ) % 3 + 1;
	for( int i = 0; i < str_vec_size; ++i )
	{
		derived_one.str_vec.push_back( random_string( ) );
	}

	std::unordered_map<std::string, std::string> string_map;
	int string_map_size = dis_int( gen ) % 3 + 1;
	for( int i = 0; i < string_map_size; ++i )
	{
		derived_one.string_map[random_string( )] = random_string( );
	}

	return derived_one;
}

integration_space::Base_t gen_random_derived_two( )
{
	integration_space::DerivedTwo derived_two { };

	derived_two.union_member = ( random_int( ) % 2 == 0 ) ? random_double( ) : random_int( );

	for( int i = 0; i < 3; ++i )
	{
		derived_two.vec_3d[i] = random_double( );
	}

	int str_vec_size = random_int( ) % 3 + 1;
	for( int i = 0; i < str_vec_size; ++i )
	{
		derived_two.str_vec.push_back( random_string( ) );
	}

	return derived_two;
}

integration_space::NonPolyDerived gen_random_non_poly_derived( )
{
	integration_space::NonPolyDerived object { };
	object.value = random_int( );
	return object;
}


integration_space::IntegrationTest gen_random_integration_test( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );

	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );

	integration_space::IntegrationTest object;

	object.object_map.emplace( "one", gen_random_derived_one( ) );
	object.object_map.emplace( "two", gen_random_derived_two( ) );

	object.object_vec.push_back( gen_random_derived_one( ) );
	object.object_vec.push_back( gen_random_derived_two( ) );

	object.object_array = { gen_random_derived_one( ), gen_random_derived_two( ) };

	object.enum_value = ( dis_int( gen ) % 2 == 0 ) ? integration_space::Enumeration::value1 : integration_space::Enumeration::value2;

	object.non_poly_derived = gen_random_non_poly_derived( );

	return object;
}

int main( int argc, char* argv[] )
{
	if( argc == 1 )
	{
		std::cerr << "Usage: " << argv[0] << " <output file>\n";
		std::cerr << " or  : " << argv[0] << " <output file> <input file>\n";
		return 1;
	}

	if( argc == 2 )
	{
		auto data = gen_random_integration_test( );

		rfl::json::save( argv[1], data, rfl::json::pretty );

		return 0;
	}

	if( argc == 3 )
	{
		auto data = rfl::json::load<integration_space::IntegrationTest>( argv[2] );

		rfl::json::save( argv[1], data, rfl::json::pretty );

		return 0;
	}

	return 1;
}