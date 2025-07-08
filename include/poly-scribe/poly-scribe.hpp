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
#include <rfl/cbor.hpp>
#include <rfl/json.hpp>
#include <rfl/ubjson.hpp>
#include <rfl/yaml.hpp>


/**
 * \brief Main namespace of the library.
 */
namespace poly_scribe
{
	/**
	 * \brief Load a file.
	 *
	 * This function loads a file from the file system and tries to parse it as a given type.
	 *
	 * \tparam T The type to parse the file as.
	 * \param input_file The path to the file to load.
	 * \return A result containing the parsed data or an error.
	 */
	template<typename T>
	rfl::Result<T> load( const std::filesystem::path& input_file )
	{
		if( !std::filesystem::exists( input_file ) )
		{
			return rfl::error( "Input file does not exist" );
		}

		if( std::filesystem::is_directory( input_file ) )
		{
			return rfl::error( "Input file is a directory" );
		}

		if( input_file.extension( ) == ".yaml" )
		{
			return rfl::yaml::load<T>( input_file.string( ) );
		}
		else if( input_file.extension( ) == ".json" )
		{
			return rfl::json::load<T>( input_file.string( ) );
		}
		else if( input_file.extension( ) == ".cbor" )
		{
			return rfl::cbor::load<T>( input_file.string( ) );
		}
		else if( input_file.extension( ) == ".ubjson" )
		{
			return rfl::ubjson::load<T>( input_file.string( ) );
		}
		else
		{
			return rfl::error( "Input file extension is not supported" );
		}
	}

	/**
	 * \brief Save a file.
	 *
	 * This function saves a data structure to the file system.
	 *
	 * \tparam T The type of the data to save.
	 * \param output_file The path to the file to save.
	 * \param data The data to save.
	 * \return A result containing nothing or an error.
	 */
	template<typename T>
	rfl::Result<rfl::Nothing> save( const std::filesystem::path& output_file, const T& data )
	{
		if( std::filesystem::is_directory( output_file ) )
		{
			return rfl::error( "Output file is a directory" );
		}

		if( output_file.extension( ) == ".yaml" )
		{
			return rfl::yaml::save( output_file.string( ), data );
		}
		else if( output_file.extension( ) == ".json" )
		{
			return rfl::json::save( output_file.string( ), data, rfl::json::pretty );
		}
		else if( output_file.extension( ) == ".cbor" )
		{
			return rfl::cbor::save( output_file.string( ), data );
		}
		else if( output_file.extension( ) == ".ubjson" )
		{
			return rfl::ubjson::save( output_file.string( ), data );
		}
		else
		{
			return rfl::error( "Output file extension is not supported" );
		}
	}
} // namespace poly_scribe

#endif