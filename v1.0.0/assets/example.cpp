#include "plugin_data.hpp"

// This is required so that gcc compiles the code correctly.
template<typename T>
[[noreturn]] void unknown_plugin_type_static_assert( )
{
	static_assert( sizeof( T ) == 0, "Unknown plugin type" );
}

int main( int argc, char** argv )
{
	if( argc < 2 )
	{
		std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	std::filesystem::path input_file = argv[1];

	auto result = poly_scribe::load<plugin_namespace::PluginSystem>( input_file );
	if( !result )
	{
		std::cerr << "Error loading file: " << result.error( ).what( ) << std::endl;
		return 1;
	}

	plugin_namespace::PluginSystem& plugin_system = result.value( );

	if( plugin_system.plugin_map )
	{
		for( const auto& [name, plugin]: *plugin_system.plugin_map )
		{
			std::cout << "Plugin Key: " << name << std::endl;

			auto visitor = []( const auto& plugin )
			{
				using Type = std::decay_t<decltype( plugin )>;

				if constexpr( std::is_same_v<Type, plugin_namespace::PluginA> )
				{
					std::cout << "Type: PluginA" << std::endl;
					std::cout << "Name: " << plugin.name << std::endl;
					std::cout << "Description: " << plugin.description << std::endl;
					std::cout << "Param A: " << plugin.paramA.value_or( 0 ) << std::endl;
					if( plugin.paramVector )
					{
						std::cout << "Param Vector: ";
						for( const auto& v: *plugin.paramVector )
						{
							std::cout << v << " ";
						}
						std::cout << std::endl;
					}
				}
				else if constexpr( std::is_same_v<Type, plugin_namespace::PluginB> )
				{
					std::cout << "Type: PluginB" << std::endl;
					std::cout << "Name: " << plugin.name << std::endl;
					std::cout << "Description: " << plugin.description << std::endl;
					std::cout << "Param B: " << plugin.paramB.value_or( 0.0f ) << std::endl;
					if( plugin.paramEnum )
					{
						std::cout << "Param Enum: " << static_cast<int>( *plugin.paramEnum ) << std::endl;
					}
				}
				else if constexpr( std::is_same_v<Type, plugin_namespace::PluginBase> )
				{
					std::cout << "Type: PluginBase" << std::endl;
					std::cout << "Name: " << plugin.name << std::endl;
					std::cout << "Description: " << plugin.description << std::endl;
				}
				else
				{
					unknown_plugin_type_static_assert<Type>( );
				}
			};

			plugin.visit( visitor );
		}
	}
	else
	{
		std::cout << "No plugins loaded." << std::endl;
	}

	return 0;
}