/**
 * \file poly-wrapper.hpp
 * \brief Header containing the polymorphic wrapper.
 * \author Pascal Palenda ppa@akustik.rwth-aachen.de
 * \copyright
 * Copyright (c) 2023-present Pascal Palenda
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#ifndef POLY_SCRIBE_DETAIL_POLY_WRAPPER_HPP
#define POLY_SCRIBE_DETAIL_POLY_WRAPPER_HPP

#include <cereal/cereal.hpp>

namespace poly_scribe::detail
{
	// forward declare
	template<typename T>
	struct BindingName;

	///
	/// \brief Thin wrapper used for the polymorphic binding logic.
	///
	/// This is used to cast to and from the correct type.
	/// \tparam T type of the wrapped value.
	/// \tparam P type of the expected return value.
	/// \todo check if this class is necessary.
	///
	template<typename T, typename P>
	struct PolyScribeWrapper
	{
	private:
		using Type = typename std::conditional_t<std::is_array_v<typename std::remove_reference_t<T>>, typename std::remove_cv_t<T>,
		                                       typename std::conditional_t<std::is_lvalue_reference_v<T>, T, typename std::decay_t<T>>>;

	public:
		PolyScribeWrapper( char const *t_name, T &&t_value ) : name( t_name ), m_value( std::forward<T>( t_value ) ) {}

		~PolyScribeWrapper( ) = default;

		// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
		Type m_value;     ///< wrapped value.
		char const *name; ///< name of the value \todo this is not used!

		///
		/// \brief Save method for the object.
		/// \tparam Archive archive type to save to.
		/// \param t_archive archive to save to.
		/// \todo make the type string a variable, many other places as well.
		/// \todo inline serialization, to make sure it works for all user types.
		///
		template<class Archive>
		void CEREAL_SAVE_FUNCTION_NAME( Archive &t_archive ) const
		{
			t_archive( cereal::make_nvp( "type", std::string( BindingName<P>::name( ) ) ) );
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			const_cast<P &>( *m_value ).CEREAL_SERIALIZE_FUNCTION_NAME( t_archive ); // todo necessary?
		}

		///
		/// \brief Load method for the object.
		/// \tparam Archive archive type to load from.
		/// \param t_archive archive to load from.
		/// \todo inline serialization, to make sure it works for all user types.
		///
		template<class Archive>
		void CEREAL_LOAD_FUNCTION_NAME( Archive &t_archive )
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			const_cast<P &>( *m_value ).CEREAL_SERIALIZE_FUNCTION_NAME( t_archive ); // todo necessary?
		}
	};

	///
	/// \brief Factory for the PolyScribeWrapper.
	/// \tparam P type of the expected return value.
	/// \tparam T type of the wrapped value.
	/// \param t_name name of the wrapped value. Not used in the wrapper!
	/// \param t_value value to wrap.
	/// \return the wrapper.
	///
	template<typename P, class T>
	inline PolyScribeWrapper<T, P> make_poly_wrapper( std::string const &t_name, T &&t_value )
	{
		return { t_name.c_str( ), std::forward<T>( t_value ) };
	}
} // namespace poly_scribe::detail

#endif