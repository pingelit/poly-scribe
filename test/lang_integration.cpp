#include "poly-scribe-structs/integration_data.hpp"

#include <array>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <random>
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

void gen_random_base( const std::shared_ptr<integration_space::Base>& t_ptr )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );

	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );
	std::uniform_real_distribution dis_real( -RAND_LIMIT_FLOAT, RAND_LIMIT_FLOAT );

	for( int i = 0; i < 3; ++i )
	{
		t_ptr->vec_3d[i] = dis_real( gen );
	}

	// t_ptr->union_member = ( dis_int( gen ) % 2 == 0 ) ? dis_real( gen ) : dis_int( gen );

	int str_vec_size = dis_int( gen ) % 3 + 1;
	for( int i = 0; i < str_vec_size; ++i )
	{
		t_ptr->str_vec.push_back( random_string( ) );
	}
}

std::shared_ptr<integration_space::DerivedOne> gen_random_derived_one( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );

	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );

	auto ptr = std::make_shared<integration_space::DerivedOne>( );

	gen_random_base( ptr );

	std::unordered_map<std::string, std::string> string_map;
	int string_map_size = dis_int( gen ) % 3 + 1;
	for( int i = 0; i < string_map_size; ++i )
	{
		ptr->string_map[random_string( )] = random_string( );
	}

	return ptr;
}

std::shared_ptr<integration_space::DerivedTwo> gen_random_derived_two( )
{
	auto ptr = std::make_shared<integration_space::DerivedTwo>( );

	gen_random_base( ptr );

	return ptr;
}

integration_space::NonPolyDerived gen_random_non_poly_derived( )
{
	std::random_device random_distribution;
	std::mt19937 gen( random_distribution( ) );

	std::uniform_int_distribution dis_int( -RAND_LIMIT, RAND_LIMIT );

	integration_space::NonPolyDerived object { };
	int value = dis_int( gen );
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

	object.object_array[0] = gen_random_derived_one( );
	object.object_array[1] = gen_random_derived_two( );

	object.enum_value = ( dis_int( gen ) % 2 == 0 ) ? integration_space::Enumeration::value1 : integration_space::Enumeration::value2;

	object.non_poly_derived = gen_random_non_poly_derived( );

	return object;
}

int main( int argc, char* argv[] )
{
	if( argc != 2 )
	{
		std::cerr << "Usage: " << argv[0] << " <output file>\n";
		return 1;
	}

	auto data = gen_random_integration_test( );

	std::stringstream string_stream;
	{
		cereal::JSONOutputArchive archive( string_stream );
		data.serialize( archive );
	}

	std::ofstream out_file_stream( argv[1] );

	out_file_stream << string_stream.str( );
	out_file_stream.close( );

	return 0;
}