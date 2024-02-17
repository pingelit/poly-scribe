typedef [Size=3] sequence<double> Vector;

enum Enumeration { 
    "value1",
    "value2"
};

dictionary Base
{
    Vector vec_3d;
    (double or int) union_member;
    sequence<string> str_vec;
};

dictionary DerivedOne : Base
{
    record<ByteString, string> string_map;
};

dictionary DerivedTwo : Base
{
    double optional_value = 3.141;
};

dictionary IntegrationTest
{
    record<ByteString, Base> object_map;

    sequence<Base> object_vec;

    [Size=2] sequence<Base> object_array;

    Enumeration enum_value;
};