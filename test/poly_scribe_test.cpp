#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <iostream>
#include <poly-scribe/poly-scribe.hpp>


TEST_CASE( "load_error_returns", "[poly-scribe]" )
{
	SECTION( "File does not exist" )
	{
		const auto result = poly_scribe::load<int>( "file_that_does_not_exist.txt" );
		REQUIRE( !result );
		REQUIRE( result.error( ).what( ) == "Input file does not exist" );
	}

	SECTION( "File is a directory" )
	{
		const auto result = poly_scribe::load<int>( "." );
		REQUIRE( !result );
		REQUIRE( result.error( ).what( ) == "Input file is a directory" );
	}

	SECTION( "Unsupported file extension" )
	{
		// create a file with an unsupported extension
		std::filesystem::path unsupported_file = "file_with_unsupported_extension.xyz";
		std::ofstream ofs( unsupported_file );
		ofs << "This is a test file with an unsupported extension." << std::endl;
		ofs.close( );

		const auto result = poly_scribe::load<int>( unsupported_file );
		REQUIRE( !result );
		REQUIRE( result.error( ).what( ) == "Input file extension is not supported" );

		// Clean up the created file
		std::filesystem::remove( unsupported_file );
	}
}

TEST_CASE( "save_error_returns", "[poly-scribe]" )
{
	SECTION( "File is a directory" )
	{
		const auto result = poly_scribe::save( ".", 42 );
		REQUIRE( !result );
		REQUIRE( result.error( ).what( ) == "Output file is a directory" );
	}

	SECTION( "Unsupported file extension" )
	{
		const auto result = poly_scribe::save( "file_with_unsupported_extension.xyz", 42 );
		REQUIRE( !result );
		REQUIRE( result.error( ).what( ) == "Output file extension is not supported" );
	}
}
