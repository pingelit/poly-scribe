///
/// Simple type def for a 3D vector.
///
typedef [Size=3] sequence<double> Vector;

///
/// Enumeration for testing purposes.
///
enum Enumeration { 
    "value1", ///< First value
    "value2"  ///< Second value
};

///
/// Base dictionary to test a polymorphic structure.
///
dictionary Base
{
    required Vector vec_3d;
    (double or int) union_member;
    sequence<string> str_vec;
};

/// DerivedOne is a dictionary that inherits from Base and adds a string_map member.
dictionary DerivedOne : Base
{
    record<ByteString, string> string_map;
};

/// DerivedTwo is a dictionary that inherits from Base and adds an optional double member.
dictionary DerivedTwo : Base
{
    double optional_value = 3.141;
};

///
/// NonPolyBase is used to test a non-polymorphic inheritance structure.
///
dictionary NonPolyBase
{
};

/// NonPolyDerived is a dictionary that inherits from NonPolyBase and adds an int member.
dictionary NonPolyDerived : NonPolyBase
{
    int value;
};

///
/// IntegrationTest is used to test various data structures and types.
///
/// This is a more pythonic docstring, lets try this:
/// Raises:
///     ValueError: If the value is not valid.
///
dictionary IntegrationTest
{
    record<ByteString, Base> object_map;

    sequence<Base> object_vec;

    sequence<int> opt_vec;

    [Size=2] sequence<Base> object_array;

    Enumeration enum_value;

    NonPolyDerived non_poly_derived;

    string string_value_with_default = "default";
};