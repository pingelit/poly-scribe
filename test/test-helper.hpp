/**
 * \file test-helper.hpp
 * \brief Helper header for tests.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_TEST_HELPER_HPP
#define POLY_SCRIBE_TEST_HELPER_HPP

#include <catch2/catch_all.hpp>
#include <poly-scribe/poly-scribe.hpp>

static constexpr int MAX_REPS = 2;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define GENERATE_RANDOM( type, reps ) GENERATE( take( reps, random( std::numeric_limits<type>::min( ), std::numeric_limits<type>::max( ) ) ) )

#define GENERATE_RANDOM_STRING( length ) \
	GENERATE( map( []( const std::vector<int>& i ) { return std::string( i.begin( ), i.end( ) ); }, chunk( length, take( length, random( 32, 122 ) ) ) ) )
// NOLINTEND(cppcoreguidelines-macro-usage)

template<class IArchive, class OArchive>
inline void test_pod( )
{
	bool const o_bool                     = GENERATE( true, false );
	char const o_char                     = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	unsigned char const o_uchar           = GENERATE( take( MAX_REPS, random( 0, 255 ) ) );
	uint8_t const o_uint8                 = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	int8_t const o_int8                   = GENERATE( take( MAX_REPS, random( 0, 255 ) ) );
	uint16_t const o_uint16               = GENERATE_RANDOM( uint16_t, MAX_REPS );
	int16_t const o_int16                 = GENERATE_RANDOM( int16_t, MAX_REPS );
	uint32_t const o_uint32               = GENERATE_RANDOM( uint32_t, 1 );
	int32_t const o_int32                 = GENERATE_RANDOM( int32_t, 1 );
	uint64_t const o_uint64               = GENERATE_RANDOM( uint64_t, 1 );
	int64_t const o_int64                 = GENERATE_RANDOM( int64_t, 1 );
	float const o_float                   = GENERATE_RANDOM( float, 1 );
	double const o_double                 = GENERATE_RANDOM( double, 1 );
	long double const o_long_double       = GENERATE_RANDOM( long double, 1 );
	long const o_long                     = GENERATE_RANDOM( long, 1 );
	unsigned long const o_ulong           = GENERATE_RANDOM( unsigned long, 1 );
	long long const o_long_long           = GENERATE_RANDOM( long long, 1 );
	unsigned long long const o_ulong_long = GENERATE_RANDOM( unsigned long long, 1 );

	std::ostringstream os;
	{
		OArchive oar( os );
		oar( poly_scribe::make_scribe_wrap( "bool", o_bool ) );
		oar( poly_scribe::make_scribe_wrap( "char", o_char ) );
		oar( poly_scribe::make_scribe_wrap( "uchar", o_uchar ) );
		oar( poly_scribe::make_scribe_wrap( "uint8", o_uint8 ) );
		oar( poly_scribe::make_scribe_wrap( "int8", o_int8 ) );
		oar( poly_scribe::make_scribe_wrap( "uint16", o_uint16 ) );
		oar( poly_scribe::make_scribe_wrap( "int16", o_int16 ) );
		oar( poly_scribe::make_scribe_wrap( "uint32", o_uint32 ) );
		oar( poly_scribe::make_scribe_wrap( "int32", o_int32 ) );
		oar( poly_scribe::make_scribe_wrap( "uint64", o_uint64 ) );
		oar( poly_scribe::make_scribe_wrap( "int64", o_int64 ) );
		oar( poly_scribe::make_scribe_wrap( "float", o_float ) );
		oar( poly_scribe::make_scribe_wrap( "double", o_double ) );
		oar( poly_scribe::make_scribe_wrap( "long_double", o_long_double ) );
		oar( poly_scribe::make_scribe_wrap( "long", o_long ) );
		oar( poly_scribe::make_scribe_wrap( "ulong", o_ulong ) );
		oar( poly_scribe::make_scribe_wrap( "long_long", o_long_long ) );
		oar( poly_scribe::make_scribe_wrap( "ulong_long", o_ulong_long ) );
	}

	bool i_bool                     = false;
	char i_char                     = 0;
	unsigned char i_uchar           = 0;
	uint8_t i_uint8                 = 0;
	int8_t i_int8                   = 0;
	uint16_t i_uint16               = 0;
	int16_t i_int16                 = 0;
	uint32_t i_uint32               = 0;
	int32_t i_int32                 = 0;
	uint64_t i_uint64               = 0;
	int64_t i_int64                 = 0;
	float i_float                   = 0;
	double i_double                 = 0;
	long double i_long_double       = 0;
	long i_long                     = 0;
	unsigned long i_ulong           = 0;
	long long i_long_long           = 0;
	unsigned long long i_ulong_long = 0;

	std::istringstream is( os.str( ) );
	{
		IArchive iar( is );
		iar( poly_scribe::make_scribe_wrap( "bool", i_bool ) );
		iar( poly_scribe::make_scribe_wrap( "char", i_char ) );
		iar( poly_scribe::make_scribe_wrap( "uchar", i_uchar ) );
		iar( poly_scribe::make_scribe_wrap( "uint8", i_uint8 ) );
		iar( poly_scribe::make_scribe_wrap( "int8", i_int8 ) );
		iar( poly_scribe::make_scribe_wrap( "uint16", i_uint16 ) );
		iar( poly_scribe::make_scribe_wrap( "int16", i_int16 ) );
		iar( poly_scribe::make_scribe_wrap( "uint32", i_uint32 ) );
		iar( poly_scribe::make_scribe_wrap( "int32", i_int32 ) );
		iar( poly_scribe::make_scribe_wrap( "uint64", i_uint64 ) );
		iar( poly_scribe::make_scribe_wrap( "int64", i_int64 ) );
		iar( poly_scribe::make_scribe_wrap( "float", i_float ) );
		iar( poly_scribe::make_scribe_wrap( "double", i_double ) );
		iar( poly_scribe::make_scribe_wrap( "long_double", i_long_double ) );
		iar( poly_scribe::make_scribe_wrap( "long", i_long ) );
		iar( poly_scribe::make_scribe_wrap( "ulong", i_ulong ) );
		iar( poly_scribe::make_scribe_wrap( "long_long", i_long_long ) );
		iar( poly_scribe::make_scribe_wrap( "ulong_long", i_ulong_long ) );
	}

	REQUIRE( i_bool == o_bool );
	REQUIRE( i_char == o_char );
	REQUIRE( i_uchar == o_uchar );
	REQUIRE( i_uint8 == o_uint8 );
	REQUIRE( i_int8 == o_int8 );
	REQUIRE( i_uint16 == o_uint16 );
	REQUIRE( i_int16 == o_int16 );
	REQUIRE( i_uint32 == o_uint32 );
	REQUIRE( i_int32 == o_int32 );
	REQUIRE( i_uint64 == o_uint64 );
	REQUIRE( i_int64 == o_int64 );
	REQUIRE( i_long == o_long );
	REQUIRE( i_ulong == o_ulong );
	REQUIRE( i_long_long == o_long_long );
	REQUIRE( i_ulong_long == o_ulong_long );
	REQUIRE_THAT( i_float, Catch::Matchers::WithinAbs( o_float, 1e-5F ) );
	REQUIRE_THAT( i_double, Catch::Matchers::WithinAbs( o_double, 1e-5 ) );
	REQUIRE_THAT( i_long_double, Catch::Matchers::WithinAbs( o_long_double, 1e-5L ) );
}


#endif