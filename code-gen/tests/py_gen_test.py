import pytest

from poly_scribe_code_gen import py_gen
from poly_scribe_code_gen.parse_idl import _validate_and_parse


def test__transform_types_typedefs():
    idl = """
typedef int my_int;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;
typedef (int or float) int_or_float;
typedef [Size=4] sequence<int> int_seq_4;
    """
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._transform_types(parsed_idl)

    assert "my_int" in result["typedefs"]
    assert "int_seq" in result["typedefs"]
    assert "int_map" in result["typedefs"]
    assert "int_or_float" in result["typedefs"]
    assert "int_seq_4" in result["typedefs"]

    assert result["typedefs"]["my_int"]["type"].replace(" ", "") == "int"
    assert result["typedefs"]["int_seq"]["type"].replace(" ", "") == "List[int]"
    assert result["typedefs"]["int_map"]["type"].replace(" ", "") == "Dict[str,int]"
    assert (
        result["typedefs"]["int_or_float"]["type"].replace(" ", "")
        == "Union[int,float]"
    )
    assert (
        result["typedefs"]["int_seq_4"]["type"].replace(" ", "")
        == "Annotated[List[int],Len(min_length=4,max_length=4)]"
    )


def test__transform_types_structs():
    idl = """
dictionary FooBar {
    required int foo;
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

    result = py_gen._transform_types(parsed_idl)

    assert "FooBar" in result["structs"]
    assert "BazQux" in result["structs"]

    assert (
        result["structs"]["FooBar"]["members"]["foo"]["type"].replace(" ", "") == "int"
    )
    assert (
        result["structs"]["FooBar"]["members"]["bar"]["type"].replace(" ", "")
        == "float"
    )
    assert (
        result["structs"]["FooBar"]["members"]["baz"]["type"].replace(" ", "")
        == "List[int]"
    )
    assert (
        result["structs"]["FooBar"]["members"]["qux"]["type"].replace(" ", "")
        == "Dict[str,int]"
    )
    assert (
        result["structs"]["FooBar"]["members"]["quux"]["type"].replace(" ", "")
        == "Annotated[List[int],Len(min_length=4,max_length=4)]"
    )
    assert (
        result["structs"]["BazQux"]["members"]["union"]["type"].replace(" ", "")
        == "Union[int,float,bool,FooBar]"
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
        py_gen._transformer(type_data)


def test_render_template_typedefs():
    idl = """
typedef int my_int;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;
typedef (int or float) int_or_float;
typedef [Size=4] sequence<int> int_seq_4;
    """
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    assert "my_int = int".replace(" ", "") in result.replace(" ", "")
    assert "int_seq = List[int]".replace(" ", "") in result.replace(" ", "")
    assert "int_map = Dict[str, int]".replace(" ", "") in result.replace(" ", "")
    assert "int_or_float =Union[int, float]".replace(" ", "") in result.replace(" ", "")
    assert "int_seq_4 = Annotated[List[int],Len(min_length=4,max_length=4)]".replace(
        " ", ""
    ) in result.replace(" ", "")
