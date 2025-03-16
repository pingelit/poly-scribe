/**
 * \file poly-scribe.hpp
 * \brief Main header for the poly-scribe library.
 *
 * This includes the other header for the library.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_POLY_SCRIBE_HPP
#define POLY_SCRIBE_POLY_SCRIBE_HPP

#include <filesystem>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <rfl/yaml.hpp>


/**
 * \brief Main namespace of the library.
 */
namespace poly_scribe
{
	template<typename T>
	rfl::Result<T> load( const std::filesystem::path input_file )
	{
		if( !std::filesystem::exists( input_file ) )
		{
			return rfl::Error( "Input file does not exist" );
		}

		if( std::filesystem::is_directory( input_file ) )
		{
			return rfl::Error( "Input file is a directory" );
		}

		if( input_file.extension( ) == ".yaml" )
		{
			return rfl::yaml::load<T>( input_file.string( ) );
		}
		else if( input_file.extension( ) == ".json" )
		{
			return rfl::json::load<T>( input_file.string( ) );
		}
		else
		{
			return rfl::Error( "Input file extension is not supported" );
		}
	}

	template<typename T>
	rfl::Result<rfl::Nothing> save( const std::filesystem::path output_file, const T& data )
	{
		if( std::filesystem::is_directory( output_file ) )
		{
			return rfl::Error( "Output file is a directory" );
		}

		if( output_file.extension( ) == ".yaml" )
		{
			return rfl::yaml::save( output_file.string( ), data );
		}
		else if( output_file.extension( ) == ".json" )
		{
			return rfl::json::save( output_file.string( ), data, rfl::json::pretty );
		}
		else
		{
			return rfl::Error( "Output file extension is not supported" );
		}
	}
} // namespace poly_scribe

#endif