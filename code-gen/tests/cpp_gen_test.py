import poly_scribe_code_gen.cpp_gen as cpp_gen
from poly_scribe_code_gen.parse_idl import _validate_and_parse

import pytest
from jinja2.exceptions import TemplateError


def test_render_template_additional_data():
    parsed_idl = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data = {
        "out_file": "test.hpp",
        "author_name": "John Doe",
        "author_email": "johndoe@foo.baz",
        "licence": "MIT",
        "package": "test",
    }

    result = cpp_gen._render_template(parsed_idl, additional_data)

    assert "\\brief" in result
    assert "/**" in result
    assert "#include <rfl.hpp>" in result
    assert "test.hpp" in result
    assert "John Doe" in result
    assert "johndoe@foo.baz" in result
    assert "MIT" in result
    assert "namespace test" in result


def test_render_template_additional_data_missing():
    parsed_idl = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data = {"package": "test"}

    result = cpp_gen._render_template(parsed_idl, additional_data)

    assert "\\brief" in result
    assert "/**" in result
    assert "#include <rfl.hpp>" in result
    assert "namespace test" in result


def test_render_template_namespace_missing():
    parsed_idl = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data = {}

    with pytest.raises(ValueError, match="Missing package name in additional data"):
        cpp_gen._render_template(parsed_idl, additional_data)


def test__transform_types_typedefs():
    idl = """
typedef int my_int;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;
typedef (int or float) int_or_float;
typedef [Size=4] sequence<int> int_seq_4;
    """
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._transform_types(parsed_idl)

    assert "my_int" in result["typedefs"]
    assert "int_seq" in result["typedefs"]
    assert "int_map" in result["typedefs"]
    assert "int_or_float" in result["typedefs"]
    assert "int_seq_4" in result["typedefs"]

    assert result["typedefs"]["my_int"]["type"].replace(" ", "") == "int"
    assert result["typedefs"]["int_seq"]["type"].replace(" ", "") == "std::vector<int>"
    assert result["typedefs"]["int_map"]["type"].replace(" ", "") == "std::unordered_map<std::string,int>"
    assert result["typedefs"]["int_or_float"]["type"].replace(" ", "") == "std::variant<int,float>"
    assert result["typedefs"]["int_seq_4"]["type"].replace(" ", "") == "std::array<int,4>"


def test__transform_types_structs():
    idl = """
dictionary FooBar {
    int foo;
    float bar;
    sequence<int> baz;
    record<ByteString, int> qux;
    [Size=4] sequence<int> quux;
};

dictionary BazQux {
    (int or float or bool or FooBar ) union;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._transform_types(parsed_idl)

    assert "FooBar" in result["structs"]
    assert "BazQux" in result["structs"]

    assert result["structs"]["FooBar"]["members"]["foo"]["type"].replace(" ", "") == "int"
    assert result["structs"]["FooBar"]["members"]["bar"]["type"].replace(" ", "") == "float"
    assert result["structs"]["FooBar"]["members"]["baz"]["type"].replace(" ", "") == "std::vector<int>"
    assert (
        result["structs"]["FooBar"]["members"]["qux"]["type"].replace(" ", "") == "std::unordered_map<std::string,int>"
    )
    assert result["structs"]["FooBar"]["members"]["quux"]["type"].replace(" ", "") == "std::array<int,4>"
    assert (
        result["structs"]["BazQux"]["members"]["union"]["type"].replace(" ", "")
        == "std::variant<int,float,bool,FooBar>"
    )


def test__transform_types_unknown_type():
    type_data = {
        "type_name": "FooBar",
        "vector": False,
        "map": False,
        "union": False,
        "size": None,
    }

    with pytest.raises(ValueError, match="Unknown type:"):
        cpp_gen._transformer(type_data)


def test_render_template_typedefs():
    idl = """
typedef int my_int;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;
typedef (int or float) int_or_float;
typedef [Size=4] sequence<int> int_seq_4;
    """
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    assert "using my_int = int;".replace(" ", "") in result.replace(" ", "")
    assert "using int_seq = std::vector<int>;".replace(" ", "") in result.replace(" ", "")
    assert "using int_map = std::unordered_map<std::string, int>;".replace(" ", "") in result.replace(" ", "")
    assert "using int_or_float = std::variant<int, float>;".replace(" ", "") in result.replace(" ", "")
    assert "using int_seq_4 = std::array<int, 4>;".replace(" ", "") in result.replace(" ", "")
    assert "namespace test" in result
