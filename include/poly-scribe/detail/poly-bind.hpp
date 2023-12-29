/**
 * \file poly-bind.hpp
 * \brief Header containing the logic behind the polymorphic binding.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_DETAIL_POLY_BIND_HPP
#define POLY_SCRIBE_DETAIL_POLY_BIND_HPP

#include "poly-wrapper.hpp"

#include <cereal/types/polymorphic.hpp>
#include <map>

namespace poly_scribe::detail
{
	//! Binds a compile time type with a user defined string
	template<class T>
	struct BindingName
	{
		static char const *name( ) { return "unknown"; }
	};

	template<class T>
	struct EmptyDeleter
	{
		void operator( )( T * /*unused*/ ) const {}
	};

	struct InputMap
	{
		using SharedCaster = std::function<void( void *, std::shared_ptr<void> &, std::type_info const &, const std::string &, const std::string & )>;
		using UniqueCaster = std::function<void( void *, std::unique_ptr<void, EmptyDeleter<void>> &, std::type_info const &, const std::string & )>;

		struct Casters
		{
			SharedCaster shared_ptr;
			UniqueCaster unique_ptr;
		};

		std::map<std::string, Casters> map;
	};

	struct OutputMap
	{
		using Caster = std::function<void( void *, void const *, std::type_info const &, const std::string & )>;

		struct Casters
		{
			Caster shared_ptr;
			Caster unique_ptr;
		};

		std::map<std::type_index, Casters> map;
	};

	template<typename Archive, typename T>
	struct InputBindingCreator
	{
		InputBindingCreator( )
		{
			auto &map = ::cereal::detail::StaticObject<InputMap>::getInstance( ).map;
			auto lock = ::cereal::detail::StaticObject<InputMap>::lock( );
			auto key  = std::string( BindingName<T>::name( ) );
			auto lb   = map.lower_bound( key );

			if( lb != map.end( ) && lb->first == key )
			{
				return;
			}

			typename InputMap::Casters casters;

			casters.shared_ptr = []( void *arptr, std::shared_ptr<void> &dptr, std::type_info const &baseInfo, std::string type, std::string name )
			{
				Archive &ar            = *static_cast<Archive *>( arptr );
				std::shared_ptr<T> ptr = std::make_shared<T>( );

				ptr->CEREAL_SERIALIZE_FUNCTION_NAME( ar );

				if( type != name )
				{
					dptr = cereal::detail::PolymorphicCasters::template upcast<T>( ptr, baseInfo );
				}
				else
				{
					dptr = ptr;
				}
			};

			casters.unique_ptr = []( void *arptr, std::unique_ptr<void, EmptyDeleter<void>> &dptr, std::type_info const &baseInfo, std::string name )
			{
				Archive &ar = *static_cast<Archive *>( arptr );
				std::unique_ptr<T, EmptyDeleter<void>> ptr;

				ar( cereal::make_nvp( name, make_poly_wrapper<T>( name, ptr ) ) );

				dptr.reset( cereal::detail::PolymorphicCasters::template upcast<T>( ptr.release( ), baseInfo ) );
			};

			map.insert( lb, { std::move( key ), std::move( casters ) } );
		}
	};

	template<typename Archive, typename T>
	struct OutputBindingCreator
	{
		OutputBindingCreator( )
		{
			auto &map = ::cereal::detail::StaticObject<OutputMap>::getInstance( ).map;
			auto key  = std::type_index( typeid( T ) );
			auto lb   = map.lower_bound( key );

			if( lb != map.end( ) && lb->first == key )
			{
				return;
			}

			OutputMap::Casters casters;

			casters.shared_ptr = [&]( void *arptr, void const *dptr, std::type_info const &baseInfo, const std::string &name )
			{
				Archive &ar = *static_cast<Archive *>( arptr );

				auto ptr = cereal::detail::PolymorphicCasters::template downcast<T>( dptr, baseInfo );

				ar( cereal::make_nvp( name, make_poly_wrapper<T>( name, ptr ) ) );
			};

			casters.unique_ptr = [&]( void *arptr, void const *dptr, std::type_info const &baseInfo, const std::string &name )
			{
				Archive &ar = *static_cast<Archive *>( arptr );

				std::unique_ptr<T const> const ptr( cereal::detail::PolymorphicCasters::template downcast<T>( dptr, baseInfo ) );

				ar( cereal::make_nvp( name, make_poly_wrapper<T>( name, ptr ) ) );
			};

			map.insert( { std::move( key ), std::move( casters ) } );
		}
	};

	template<class Archive, class T>
	struct CreateBindings
	{
		static const InputBindingCreator<Archive, T> &load( std::true_type /*unsused*/ )
		{
			return cereal::detail::StaticObject<InputBindingCreator<Archive, T>>::getInstance( );
		}

		static const OutputBindingCreator<Archive, T> &save( std::true_type /*unsused*/ )
		{
			return cereal::detail::StaticObject<OutputBindingCreator<Archive, T>>::getInstance( );
		}

		inline static void load( std::false_type /*unsused*/ ) {}
		inline static void save( std::false_type /*unsused*/ ) {}
	};

	template<class Archive, class T>
	struct PolymorphicSerializationSupport
	{
#if defined( _MSC_VER ) && !defined( __INTEL_COMPILER )
		//! Creates the appropriate bindings depending on whether the archive supports
		//! saving or loading
		virtual CEREAL_DLL_EXPORT void instantiate( ) CEREAL_USED;
#else  // NOT _MSC_VER
       //! Creates the appropriate bindings depending on whether the archive supports
       //! saving or loading
		static CEREAL_DLL_EXPORT void instantiate( ) CEREAL_USED;
		//! This typedef causes the compiler to instantiate this static function
		typedef instantiate_function<instantiate> unused;
#endif // _MSC_VER
	};

	template<class Archive, class T>
	CEREAL_DLL_EXPORT void PolymorphicSerializationSupport<Archive, T>::instantiate( )
	{
		CreateBindings<Archive, T>::save( std::integral_constant < bool,
		                                  std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value &&cereal::traits::is_output_serializable<T, Archive>::value >
		                                      { } );

		CreateBindings<Archive, T>::load( std::integral_constant < bool,
		                                  std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value &&cereal::traits::is_input_serializable<T, Archive>::value >
		                                      { } );
	}

	template<class T, class Tag = cereal::detail::polymorphic_binding_tag>
	struct BindToArchives
	{
		void bind( std::false_type /*unsused*/ ) const { instantiate_polymorphic_binding( static_cast<T *>( nullptr ), 0, Tag { } ); }

		void bind( std::true_type /*unsused*/ ) const {}

		BindToArchives const &bind( ) const
		{
			static_assert( std::is_polymorphic<T>::value, "Attempting to register non polymorphic type" );
			bind( std::is_abstract<T>( ) );
			return *this;
		}
	};

	template<class T, class Tag = cereal::detail::polymorphic_binding_tag>
	struct init_binding;

	template<class T, typename BindingTag>
	void instantiate_polymorphic_binding( T * /*unsused*/, int /*unsused*/, BindingTag /*unsused*/ )
	{
	}

	template<class Archive>
	inline InputMap::Casters get_input_binding( Archive &t_archive, std::string &t_name )
	{
		t_archive( cereal::make_nvp( "type", t_name ) );

		auto const &bindingMap = cereal::detail::StaticObject<InputMap>::getInstance( ).map;

		auto binding = bindingMap.find( t_name );
		if( binding == bindingMap.end( ) )
		{
			throw std::exception( "bar" );
		}

		return binding->second;
	}
} // namespace poly_scribe::detail

#endif