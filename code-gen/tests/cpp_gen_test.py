import re
from pathlib import Path

import pytest
from docstring_parser import parse

from poly_scribe_code_gen import cpp_gen
from poly_scribe_code_gen._types import AdditionalData, ParsedIDL
from poly_scribe_code_gen.parse_idl import _validate_and_parse


def test_render_template_additional_data() -> None:
    parsed_idl: ParsedIDL = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data: AdditionalData = {
        "out_file": "test.hpp",
        "author_name": "John Doe",
        "author_email": "johndoe@foo.baz",
        "licence": "MIT",
        "package": "test",
        "year": "2025",
    }

    result = cpp_gen._render_template(parsed_idl, additional_data)

    assert "\\brief" in result
    assert "/**" in result
    assert "#include <poly-scribe/poly-scribe.hpp>" in result
    assert "test.hpp" in result
    assert "John Doe" in result
    assert "johndoe@foo.baz" in result
    assert "MIT" in result
    assert "namespace test" in result


def test_render_template_additional_data_missing() -> None:
    parsed_idl: ParsedIDL = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data: AdditionalData = {"package": "test"}  # type: ignore

    result = cpp_gen._render_template(parsed_idl, additional_data)

    assert "\\brief" in result
    assert "/**" in result
    assert "#include <poly-scribe/poly-scribe.hpp>" in result
    assert "namespace test" in result


def test_render_template_namespace_missing() -> None:
    parsed_idl: ParsedIDL = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data: AdditionalData = {}  # type: ignore

    with pytest.raises(ValueError, match="Missing package name in additional data"):
        cpp_gen._render_template(parsed_idl, additional_data)


def test__transform_types_typedefs() -> None:
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

    result = cpp_gen._transform_types(parsed_idl)

    assert "FooBar" in result["structs"]
    assert "BazQux" in result["structs"]

    assert result["structs"]["FooBar"]["members"]["foo"]["type"].replace(" ", "") == "int"
    assert result["structs"]["FooBar"]["members"]["bar"]["type"].replace(" ", "") == "std::optional<float>"
    assert result["structs"]["FooBar"]["members"]["baz"]["type"].replace(" ", "") == "std::optional<std::vector<int>>"
    assert (
        result["structs"]["FooBar"]["members"]["qux"]["type"].replace(" ", "")
        == "std::optional<std::unordered_map<std::string,int>>"
    )
    assert result["structs"]["FooBar"]["members"]["quux"]["type"].replace(" ", "") == "std::optional<std::array<int,4>>"
    assert (
        result["structs"]["BazQux"]["members"]["union"]["type"].replace(" ", "")
        == "std::optional<std::variant<int,float,bool,FooBar>>"
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
        cpp_gen._transformer(type_data)


def test_render_template_typedefs() -> None:
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


def test_render_template_enums() -> None:
    idl = """
enum FooBar {
    "FOO",
    "BAR",
    "BAZ"
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    assert "enum class FooBar {".replace(" ", "") in result.replace(" ", "")
    assert "FOO,".replace(" ", "") in result.replace(" ", "")
    assert "BAR,".replace(" ", "") in result.replace(" ", "")
    assert "BAZ".replace(" ", "") in result.replace(" ", "")
    assert "};".replace(" ", "") in result.replace(" ", "")
    assert "namespace test" in result


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

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"struct (\w+) \{([^}]*)\};", re.MULTILINE)
    matches = pattern.findall(result)

    assert len(matches) == 2
    assert "FooBar" in [match[0] for match in matches]
    assert "BazQux" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "FooBar":
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
            assert "std::optional<float> bar;".replace(" ", "") in struct_body.replace(" ", "")
            assert "std::optional<std::vector<int>> baz;".replace(" ", "") in struct_body.replace(" ", "")
            assert "std::optional<std::unordered_map<std::string, int>> qux;".replace(" ", "") in struct_body.replace(
                " ", ""
            )
            assert "std::optional<std::array<int, 4>> quux;".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "BazQux":
            assert "std::optional<std::variant<int, float, bool, FooBar>> union;".replace(
                " ", ""
            ) in struct_body.replace(" ", "")


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

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"struct (\w+) \{([^}]*)\};", re.MULTILINE)
    matches = pattern.findall(result)

    assert len(matches) == 2
    assert "FooBar" in [match[0] for match in matches]
    assert "BazQux" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "FooBar":
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
            assert "float bar;".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "BazQux":
            assert "std::optional<std::vector<int>> baz;".replace(" ", "") in struct_body.replace(" ", "")
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
            assert "float bar;".replace(" ", "") in struct_body.replace(" ", "")


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

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    pattern = re.compile(r"struct (\w+) \{([^}]*)\};", re.MULTILINE)
    matches = pattern.findall(result)

    assert len(matches) == 4
    assert "X" in [match[0] for match in matches]
    assert "B" in [match[0] for match in matches]
    assert "C" in [match[0] for match in matches]
    assert "Y" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "X":
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "B":
            assert "int bar;".replace(" ", "") in struct_body.replace(" ", "")
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "C":
            assert "float baz;".replace(" ", "") in struct_body.replace(" ", "")
            assert "int foo;".replace(" ", "") in struct_body.replace(" ", "")
        elif match[0] == "Y":
            assert "X_t content;".replace(" ", "") in struct_body.replace(" ", "")

    assert 'using X_t = rfl::TaggedUnion<"type", X, B, C>;'.replace(" ", "") in result.replace(" ", "")


def test__sort_inheritance_data() -> None:
    inheritance_data = {
        "X": ["B", "C"],
        "B": ["D"],
        "Y": ["X"],
    }

    result = cpp_gen._sort_inheritance_data(inheritance_data)

    assert len(result) == 3
    assert result[0] == ("Y", ["X"])
    assert result[1] == ("X", ["B", "C"])
    assert result[2] == ("B", ["D"])


def test__flatten_struct_inheritance() -> None:
    idl = """
dictionary FooBar {
    int foo;
    float bar;
};

dictionary BazQux : FooBar {
    sequence<int> baz;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._flatten_struct_inheritance(parsed_idl)

    assert "FooBar" in result["structs"]
    assert "BazQux" in result["structs"]

    assert "foo" in result["structs"]["BazQux"]["members"]
    assert "bar" in result["structs"]["BazQux"]["members"]
    assert "baz" in result["structs"]["BazQux"]["members"]

    assert "foo" in result["structs"]["FooBar"]["members"]
    assert "bar" in result["structs"]["FooBar"]["members"]


def test_generate_cpp(tmp_path: Path) -> None:
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

    additional_data: AdditionalData = {
        "out_file": "test.hpp",
        "author_name": "John Doe",
        "author_email": "johndoe@foo.baz",
        "licence": "MIT",
        "package": "test",
    }
    out_file = tmp_path / "test.hpp"

    cpp_gen.generate_cpp(parsed_idl, additional_data, out_file)

    assert out_file.exists()
    with open(out_file) as f:
        content = f.read()
        assert "\\brief" in content
        assert "/**" in content
        assert "#include <poly-scribe/poly-scribe.hpp>" in content
        assert "test.hpp" in content
        assert "John Doe" in content
        assert "johndoe@foo.baz" in content
        assert "MIT" in content
        assert "namespace test" in content


def test_generate_cpp_missing_package(tmp_path: Path) -> None:
    parsed_idl: ParsedIDL = {"structs": {}, "inheritance_data": {}, "typedefs": {}, "enums": {}}
    additional_data: AdditionalData = {  # type: ignore
        "out_file": "test.hpp",
        "author_name": "John Doe",
        "author_email": "johndoe@foo.baz",
        "licence": "MIT",
    }
    out_file = tmp_path / "test.hpp"

    with pytest.raises(ValueError, match="Missing package name in additional data"):
        cpp_gen.generate_cpp(parsed_idl, additional_data, out_file)


def test_render_template_default_member_values() -> None:
    idl = """
dictionary Foo {
    int foo = 42;
    int bar;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    assert "std::optional<int> foo = 42;".replace(" ", "") in result.replace(" ", "")
    assert "std::optional<int> bar;".replace(" ", "") in result.replace(" ", "")
    assert "namespace test" in result


def test__trasform_types_change_base_inheritance_type() -> None:
    idl = """
dictionary Foo {
};

dictionary Bar : Foo {
};

dictionary A {
};

dictionary B : A {
};

dictionary C : A {
};

typedef Bar Baz;
typedef C Qux;
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._transform_types(parsed_idl)

    assert "Foo_t" in result["typedefs"]["Baz"]["type"]  # non polymorphic
    assert "A_t" in result["typedefs"]["Qux"]["type"]  # polymorphic

    render_result = cpp_gen._render_template(result, AdditionalData({"package": "test"}))

    assert 'using A_t = rfl::TaggedUnion<"type", A, B, C>;'.replace(" ", "") in render_result.replace(" ", "")
    assert 'using Foo_t = rfl::TaggedUnion<"type", Foo, Bar>;'.replace(" ", "") in render_result.replace(" ", "")


def test__render_doxystring() -> None:
    doc_string = """
short description

long description
with multiple lines

Args:
    arg1: description for arg1
    arg2: description for arg2

Returns:
    description for return value

Raises:
    Exception: description for exception
        with multiple lines on the exception
    AnotherException: description for another exception
"""

    doc_string_parsed = parse(doc_string)

    result = cpp_gen._render_doxystring(doc_string_parsed)

    assert "/// \\brief short description" in result
    assert "///\n" in result
    assert "/// long description" in result
    assert "/// with multiple lines" in result
    assert "/// \\param arg1 description for arg1" in result
    assert "/// \\param arg2 description for arg2" in result
    assert "/// \\return description for return value" in result
    assert "/// \\throws Exception description for exception" in result
    assert "/// with multiple lines on the exception" in result
    assert "/// \\throws AnotherException description for another exception" in result


def test_render_template_comments() -> None:
    idl = """
///
/// This is a comment
///
/// This is a second line of the comment
///
/// Args:
///     foo: This is a comment for foo
///     bar: This is a comment for bar
///
dictionary Foo { ///< inline comment
    /// Short comment for foo
    int foo;
    /// Short comment for bar
    float bar;
};

/// Typedef comment
typedef int my_int; ///< inline typedef comment

/// My Enum comment
enum MyEnum {
    /// Enum value 1 comment
    "VALUE_1",
    /// Enum value 2 comment
    "VALUE_2",
    "VALUE_3" ///< Enum value 3 comment only inline
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._render_template(parsed_idl, {"package": "test"})

    regex = re.compile(r"((?:[ \t]*?///.*\n)+)", re.MULTILINE)
    matches = regex.findall(result)

    assert len(matches) == 8
    assert "My Enum comment" in matches[0]
    assert "Enum value 1 comment" in matches[1]
    assert "Enum value 2 comment" in matches[2]
    assert "Enum value 3 comment only inline" in matches[3]
    assert "Typedef comment" in matches[4]
    assert "inline typedef comment" in matches[4]
    assert "This is a comment\n" in matches[5]
    assert "This is a second line of the comment" in matches[5]
    assert "\\param foo This is a comment for foo" in matches[5]
    assert "\\param bar This is a comment for bar" in matches[5]
    assert "inline comment" in matches[5]
    assert "Short comment for foo" in matches[6]
    assert "Short comment for bar" in matches[7]


def test__render_template_string_default_value() -> None:
    idl = """
dictionary Foo {
    string foo = "bar";
};
"""
    parsed_idl = _validate_and_parse(idl)
    result = cpp_gen._render_template(parsed_idl, {"package": "foo"})

    pattern = re.compile(r"struct (\w+) \{([^}]*)\};", re.MULTILINE)
    matches = pattern.findall(result)

    assert len(matches) == 1
    assert "Foo" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "Foo":
            assert 'std::optional<std::string> foo = "bar";'.replace(" ", "") in struct_body.replace(" ", "")


def test__render_template_boolean_default_value() -> None:
    idl = """
dictionary Foo {
    bool foo = true;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = cpp_gen._render_template(parsed_idl, {"package": "foo"})

    pattern = re.compile(r"struct (\w+) \{([^}]*)\};", re.MULTILINE)
    matches = pattern.findall(result)

    assert len(matches) == 1
    assert "Foo" in [match[0] for match in matches]

    for match in matches:
        struct_body = match[1]
        if match[0] == "Foo":
            assert "std::optional<bool> foo = true;".replace(" ", "") in struct_body.replace(" ", "")
