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

///
/// \brief Namespace for implementation details.
///
/// These are typically not used by user code.
///
namespace poly_scribe::detail
{
	///
	/// \brief Binds a compile time type with a user defined string
	///
	/// Default is unkown.
	///
	template<class T>
	struct BindingName
	{
		static char const *name( ) { return "unknown"; }
	};

	///
	/// \brief Empty deleter for loading unique_ptr.
	/// \tparam T type of the pointed value to delete.
	///
	template<class T>
	struct EmptyDeleter
	{
		void operator( )( T * /*unused*/ ) const {}
	};

	///
	/// \brief Struct that holds casters for loading polymorphic values.
	/// Static objects of this will be created for each archive.
	/// \sa https://uscilab.github.io/cereal/assets/doxygen/structcereal_1_1detail_1_1InputBindingMap.html
	///
	struct InputMap
	{
		///
		/// \brief Function specifications for input casters.
		/// \param[in] p1 archive pointer.
		/// \param[in] p2 smart pointer to load into.
		/// \param[in] p3 std::type_id of the base type of pointe type.
		/// \param[in] p4
		/// \todo check if all arguments are actually used.
		/// \{
		///
		using SharedCaster = std::function<void( void *, std::shared_ptr<void> &, std::type_info const &, const std::string &, const std::string & )>;
		using UniqueCaster = std::function<void( void *, std::unique_ptr<void, EmptyDeleter<void>> &, std::type_info const &, const std::string & )>;
		/// \}

		///
		/// \brief Helper struct carrying the casters.
		///
		struct Casters
		{
			SharedCaster shared_ptr;
			UniqueCaster unique_ptr;
		};

		///
		/// \brief Maping from type string to a input caster.
		///
		std::map<std::string, Casters> map;
	};

	///
	/// \brief Struct that holds casters for saving polymorphic values.
	/// Static objects of this will be created for each archive.
	/// \sa https://uscilab.github.io/cereal/assets/doxygen/structcereal_1_1detail_1_1OutputBindingMap.html
	///
	struct OutputMap
	{
		///
		/// \brief Function specification for output casters.
		/// \param[in] p1 archive pointer.
		/// \param[in] p2 pointer to the data to be saved.
		/// \param[in] p3 std::type_info of the owning smart pointer.
		/// \param[in]
		///
		using Caster = std::function<void( void *, void const *, std::type_info const &, const std::string & )>;

		///
		/// \brief Helper struct carrying the casters.
		///
		struct Casters
		{
			Caster shared_ptr;
			Caster unique_ptr;
		};

		///
		/// \brief Maping from std::type_index to a output caster.
		///
		std::map<std::type_index, Casters> map;
	};

	///
	/// \brief Create an entry in the static InputMap for the given types.
	/// \sa https://uscilab.github.io/cereal/assets/doxygen/structcereal_1_1detail_1_1InputBindingCreator.html
	/// \tparam Archive the binding should be created for.
	/// \tparam T polymorphic type the binding should be created for.
	///
	template<typename Archive, typename T>
	struct InputBindingCreator
	{
		///
		/// \brief Initialize the input binding.
		///
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

	///
	/// \brief Create an entry in the static OutputMap for the given types.
	/// \sa https://uscilab.github.io/cereal/assets/doxygen/structcereal_1_1detail_1_1OutputBindingCreator.html
	/// \tparam Archive the binding should be created for.
	/// \tparam T polymorphic type the binding should be created for.
	///
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

	///
	/// \brief Logic to create and bind types using the input and output binding.
	///
	/// This is based on the work of the cereal authors. Which in turn is heavily inspired by the boost serialization implementation.
	/// OutputBindingCreator, InputBindingCreator, OutputMap and, InputMap are based on this concept as well but were adapted to work in this scenario.
	/// For further information see the documentation of cereal.
	/// \sa https://uscilab.github.io/cereal/assets/doxygen/polymorphic__impl_8hpp_source.html
	/// \sa https://uscilab.github.io/cereal/index.html
	/// \copyright Copyright (c) 2013-2022, Randolph Voorhies, Shane Grant
	/// \copyright Copyright (c) 2002 Robert Ramey
	/// \copyright Copyright (c) 2006 David Abrahams
	/// \{
	///
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
		virtual CEREAL_DLL_EXPORT void instantiate( ) CEREAL_USED;
#else  // NOT _MSC_VER
		static CEREAL_DLL_EXPORT void instantiate( ) CEREAL_USED;
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
	///
	/// \}
	///

	///
	/// \brief Get the input binding object for the given archive.
	///
	/// \tparam Archive archive to get the binding for.
	/// \param t_archive archive to load from.
	/// \param t_name name of the to be loaded objects type.
	/// \return the InputMap::Casters.
	/// \todo fix exception.
	/// \todo make type string a variable.
	///
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