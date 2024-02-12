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
	GENERATE( map( []( const std::vector<int>& out ) { return std::string( out.begin( ), out.end( ) ); }, chunk( length, take( length, random( 32, 122 ) ) ) ) )

#define GENERATE_VECTOR_OF_STRINGS( size, length )                                                                     \
	GENERATE( chunk( size, map( []( const std::vector<int>& out ) { return std::string( out.begin( ), out.end( ) ); }, \
	                            chunk( length, take( ( size * length ), random( 32, 122 ) ) ) ) ) )
// NOLINTEND(cppcoreguidelines-macro-usage)

// NOLINTBEGIN(readability-function-cognitive-complexity)
template<class IArchive, class OArchive>
inline void test_pod( )
{
	bool const o_bool               = GENERATE( true, false );
	char const o_char               = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	unsigned char const o_uchar     = GENERATE( take( MAX_REPS, random( 0, 255 ) ) );
	uint8_t const o_uint8           = GENERATE( take( MAX_REPS, random( -128, 127 ) ) );
	int8_t const o_int8             = GENERATE( take( MAX_REPS, random( 0, 255 ) ) );
	uint16_t const o_uint16         = GENERATE_RANDOM( uint16_t, MAX_REPS );
	int16_t const o_int16           = GENERATE_RANDOM( int16_t, MAX_REPS );
	uint32_t const o_uint32         = GENERATE_RANDOM( uint32_t, 1 );
	int32_t const o_int32           = GENERATE_RANDOM( int32_t, 1 );
	float const o_float             = GENERATE_RANDOM( float, 1 );
	double const o_double           = GENERATE_RANDOM( double, 1 );
	long double const o_long_double = GENERATE_RANDOM( long double, 1 );
	long const o_long               = GENERATE_RANDOM( long, 1 );
	unsigned long const o_ulong     = GENERATE_RANDOM( unsigned long, 1 );

	std::ostringstream out_stream;
	{
		OArchive oar( out_stream );
		oar( poly_scribe::make_scribe_wrap( "bool", o_bool ) );
		oar( poly_scribe::make_scribe_wrap( "char", o_char ) );
		oar( poly_scribe::make_scribe_wrap( "uchar", o_uchar ) );
		oar( poly_scribe::make_scribe_wrap( "uint8", o_uint8 ) );
		oar( poly_scribe::make_scribe_wrap( "int8", o_int8 ) );
		oar( poly_scribe::make_scribe_wrap( "uint16", o_uint16 ) );
		oar( poly_scribe::make_scribe_wrap( "int16", o_int16 ) );
		oar( poly_scribe::make_scribe_wrap( "uint32", o_uint32 ) );
		oar( poly_scribe::make_scribe_wrap( "int32", o_int32 ) );
		oar( poly_scribe::make_scribe_wrap( "float", o_float ) );
		oar( poly_scribe::make_scribe_wrap( "double", o_double ) );
		oar( poly_scribe::make_scribe_wrap( "long_double", o_long_double ) );
		oar( poly_scribe::make_scribe_wrap( "long", o_long ) );
		oar( poly_scribe::make_scribe_wrap( "ulong", o_ulong ) );
	}

	bool i_bool               = false;
	char i_char               = 0;
	unsigned char i_uchar     = 0;
	uint8_t i_uint8           = 0;
	int8_t i_int8             = 0;
	uint16_t i_uint16         = 0;
	int16_t i_int16           = 0;
	uint32_t i_uint32         = 0;
	int32_t i_int32           = 0;
	float i_float             = 0;
	double i_double           = 0;
	long double i_long_double = 0;
	long i_long               = 0;
	unsigned long i_ulong     = 0;


	std::istringstream in_stream( out_stream.str( ) );
	{
		IArchive iar( in_stream );
		iar( poly_scribe::make_scribe_wrap( "bool", i_bool ) );
		iar( poly_scribe::make_scribe_wrap( "char", i_char ) );
		iar( poly_scribe::make_scribe_wrap( "uchar", i_uchar ) );
		iar( poly_scribe::make_scribe_wrap( "uint8", i_uint8 ) );
		iar( poly_scribe::make_scribe_wrap( "int8", i_int8 ) );
		iar( poly_scribe::make_scribe_wrap( "uint16", i_uint16 ) );
		iar( poly_scribe::make_scribe_wrap( "int16", i_int16 ) );
		iar( poly_scribe::make_scribe_wrap( "uint32", i_uint32 ) );
		iar( poly_scribe::make_scribe_wrap( "int32", i_int32 ) );
		iar( poly_scribe::make_scribe_wrap( "float", i_float ) );
		iar( poly_scribe::make_scribe_wrap( "double", i_double ) );
		iar( poly_scribe::make_scribe_wrap( "long_double", i_long_double ) );
		iar( poly_scribe::make_scribe_wrap( "long", i_long ) );
		iar( poly_scribe::make_scribe_wrap( "ulong", i_ulong ) );
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
	REQUIRE( i_long == o_long );
	REQUIRE( i_ulong == o_ulong );

	const auto float_margin       = 1e-5F;
	const auto double_margin      = 1e-5;
	const auto long_double_margin = 1e-5L;

	REQUIRE_THAT( i_float, Catch::Matchers::WithinAbs( o_float, float_margin ) );
	REQUIRE_THAT( i_double, Catch::Matchers::WithinAbs( o_double, double_margin ) );
	REQUIRE_THAT( i_long_double, Catch::Matchers::WithinAbs( o_long_double, long_double_margin ) );
}
// NOLINTEND(readability-function-cognitive-complexity)


struct Base
{
	double m_base_value { 0.0 };

	virtual ~Base( ) = default;

	template<class Archive>
	void CEREAL_SERIALIZE_FUNCTION_NAME( Archive& t_archive )
	{
		t_archive( poly_scribe::make_scribe_wrap( "base_value", m_base_value ) );
	}
};

struct UnregisteredDerived : public Base
{
	int m_derived_value { 0 };

	template<class Archive>
	void CEREAL_SERIALIZE_FUNCTION_NAME( Archive& t_archive )
	{
		cereal::base_class<Base>( this ).base_ptr->serialize( t_archive );
		t_archive( poly_scribe::make_scribe_wrap( "derived_value", m_derived_value ) );
	}
};

struct RegisteredDerived : public Base
{
	int m_derived_value { 0 };

	template<class Archive>
	void CEREAL_SERIALIZE_FUNCTION_NAME( Archive& t_archive )
	{
		cereal::base_class<Base>( this ).base_ptr->serialize( t_archive );
		t_archive( poly_scribe::make_scribe_wrap( "derived_value", m_derived_value ) );
	}
};

inline bool operator==( const RegisteredDerived& t_lhs, const RegisteredDerived& t_rhs )
{
	return t_lhs.m_base_value == t_rhs.m_base_value && t_lhs.m_derived_value == t_rhs.m_derived_value;
}

POLY_SCRIBE_REGISTER_TYPE( RegisteredDerived );

#endif