import re

import pytest

from poly_scribe_code_gen import py_gen
from poly_scribe_code_gen.parse_idl import _validate_and_parse


def test__transform_types_typedefs() -> None:
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
    assert result["typedefs"]["int_or_float"]["type"].replace(" ", "") == "Union[int,float]"
    assert (
        result["typedefs"]["int_seq_4"]["type"].replace(" ", "")
        == "Annotated[List[int],Len(min_length=4,max_length=4)]"
    )


def test__transform_types_structs() -> None:
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

    assert result["structs"]["FooBar"]["members"]["foo"]["type"].replace(" ", "") == "int"
    assert result["structs"]["FooBar"]["members"]["bar"]["type"].replace(" ", "") == "Optional[float]"
    assert result["structs"]["FooBar"]["members"]["baz"]["type"].replace(" ", "") == "Optional[List[int]]"
    assert result["structs"]["FooBar"]["members"]["qux"]["type"].replace(" ", "") == "Optional[Dict[str,int]]"
    assert (
        result["structs"]["FooBar"]["members"]["quux"]["type"].replace(" ", "")
        == "Optional[Annotated[List[int],Len(min_length=4,max_length=4)]]"
    )
    assert (
        result["structs"]["BazQux"]["members"]["union"]["type"].replace(" ", "")
        == "Optional[Union[int,float,bool,FooBar]]"
    )


def test__transform_types_unknown_type() -> None:
    type_data = {
        "type_name": "FooBar",
        "vector": False,
        "map": False,
        "union": False,
        "size": None,
    }

    with pytest.raises(ValueError, match="Unknown type:"):
        py_gen._transformer(type_data, {})


def test_render_template_typedefs() -> None:
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
    assert "int_seq_4 = Annotated[List[int],Len(min_length=4,max_length=4)]".replace(" ", "") in result.replace(" ", "")


def test_render_template_enums() -> None:
    idl = """
enum FooBar {
    "FOO",
    "BAR",
    "BAZ"
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    assert "class FooBar(StrEnum)".replace(" ", "") in result.replace(" ", "")
    assert 'FOO = "FOO"'.replace(" ", "") in result.replace(" ", "")
    assert 'BAR = "BAR"'.replace(" ", "") in result.replace(" ", "")
    assert 'BAZ = "BAZ"'.replace(" ", "") in result.replace(" ", "")


def test_render_template_structs() -> None:
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

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"class\s+(\w+)\(BaseModel\):\s*(.*?)\n\n", re.DOTALL)
    matches = pattern.findall(result)

    assert len(matches) == 2
    assert "FooBar" in [match[0] for match in matches]
    assert "BazQux" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "FooBar":
            assert "foo: int".replace(" ", "") in struct_body.replace(" ", "")
            assert "bar: Optional[float]".replace(" ", "") in struct_body.replace(" ", "")
            assert "baz: Optional[List[int]]".replace(" ", "") in struct_body.replace(" ", "")
            assert "qux: Optional[Dict[str, int]]".replace(" ", "") in struct_body.replace(" ", "")
            assert "quux: Optional[Annotated[List[int],Len(min_length=4,max_length=4)]]".replace(
                " ", ""
            ) in struct_body.replace(" ", "")
        elif match[0] == "BazQux":
            assert "union: Optional[Union[int, float, bool, FooBar]]".replace(" ", "") in struct_body.replace(" ", "")


def test_render_template_struct_with_inheritance() -> None:
    idl = """
dictionary FooBar {
    required int foo;
    required float bar;
};

dictionary BazQux : FooBar {
    sequence<int> baz;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"class\s+(\w+)\((\w*)\):\s*(.*?)\n\n", re.DOTALL)
    matches = pattern.findall(result)

    assert len(matches) == 2
    assert "FooBar" in [match[0] for match in matches]
    assert "BazQux" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[2]
        if match[0] == "FooBar":
            assert "foo: int".replace(" ", "") in struct_body.replace(" ", "")
            assert "bar: float".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "BazQux":
            assert "FooBar" in match[1]
            assert "baz: Optional[List[int]]".replace(" ", "") in struct_body.replace(" ", "")


def test_render_template_struct_with_poly_inheritance() -> None:
    idl = """
dictionary X {
    required int foo;
};

dictionary B : X {
    required int bar;
};

dictionary C : X {
    required float baz;
};

dictionary Y {
    required B content;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"class\s+(\w+)\((\w*)\):\s*(.*?)\n\n", re.DOTALL)
    matches = pattern.findall(result)

    assert len(matches) == 4
    assert "X" in [match[0] for match in matches]
    assert "B" in [match[0] for match in matches]
    assert "C" in [match[0] for match in matches]
    assert "Y" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[2]
        if match[0] == "X":
            assert "foo: int".replace(" ", "") in struct_body.replace(" ", "")
            assert 'type: Literal["X"] = "X"'.replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "B":
            assert "bar: int".replace(" ", "") in struct_body.replace(" ", "")
            assert 'type: Literal["B"] = "B"'.replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "C":
            assert "baz: float".replace(" ", "") in struct_body.replace(" ", "")
            assert 'type: Literal["C"] = "C"'.replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "Y":
            assert 'content: Annotated[Union[B,C,X],Field(discriminator="type")]'.replace(
                " ", ""
            ) in struct_body.replace(" ", "")


def test_render_template_default_member_values() -> None:
    idl = """
dictionary Foo {
    int foo = 42;
    int bar;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "test"})

    assert "foo: Optional[int] = 42".replace(" ", "") in result.replace(" ", "")
    assert "bar: Optional[int] = None".replace(" ", "") in result.replace(" ", "")


def test_generate_py(tmp_path) -> None:
    idl = """
dictionary Foo {
    int foo;
    float bar;
};

dictionary Bar : Foo {
    sequence<int> baz;
};

enum FooBarEnum {
    "FOO",
    "BAR",
    "BAZ"
};

typedef int my_int;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;

dictionary Baz : Foo {
    (int or float or bool or Foo ) union;
    int_map map;
};
"""
    parsed_idl = _validate_and_parse(idl)

    out_file = tmp_path / "test.py"

    py_gen.generate_python(parsed_idl, {}, out_file)

    assert out_file.exists()
    with open(out_file) as f:
        content = f.read()

        assert "class Foo(BaseModel):" in content
        assert "foo: Optional[int]" in content
        assert "bar: Optional[float]" in content


def test__render_template_poly_have_type_member() -> None:
    idl = """
dictionary X {
    int type = 42;
};

dictionary B : X {
    int bar;
};

dictionary C : X {
    float baz;
};
"""
    parsed_idl = _validate_and_parse(idl)

    with pytest.raises(ValueError, match="Struct X already has a member named 'type'"):
        py_gen._render_template(parsed_idl, {})

    idl = """
dictionary X {
    int foo;
};

dictionary B : X {
    int bar;
};

dictionary C : X {
    float type;
};
"""
    parsed_idl = _validate_and_parse(idl)

    with pytest.raises(ValueError, match="Struct C already has a member named 'type'"):
        py_gen._render_template(parsed_idl, {})


def test__render_template_poly_derived_in_decl() -> None:
    idl = """
dictionary X {
};

dictionary B : X {
};

dictionary C : X {
};

dictionary Y {
    C content;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {})

    pattern = re.compile(r"class\s+(\w+)\((\w*)\):\s*(.*?)\n\n", re.DOTALL)

    matches = pattern.findall(result)

    assert len(matches) == 4
    assert "X" in [match[0] for match in matches]
    assert "B" in [match[0] for match in matches]
    assert "C" in [match[0] for match in matches]
    assert "Y" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[2]
        if match[0] == "Y":
            assert 'content: Optional[Annotated[Union[B, C, X], Field(discriminator="type")]] = None'.replace(
                " ", ""
            ) in struct_body.replace(" ", "")
