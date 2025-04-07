from typing import TYPE_CHECKING, Any

import pytest

import poly_scribe_code_gen.parse_idl as parsing

if TYPE_CHECKING:
    from poly_scribe_code_gen._types import ParsedIDL


def test_parse_idl(mocker: Any) -> None:
    mocker.patch("builtins.open", mocker.mock_open(read_data="dummy"))
    validate_mock = mocker.patch("poly_scribe_code_gen.parse_idl._validate_and_parse", return_value={})

    parsed_idl = parsing.parse_idl("dummy")  # type: ignore

    assert parsed_idl == {}
    validate_mock.assert_called_once_with("dummy")  # type: ignore


def test__validate_and_parse_empty_idl() -> None:
    idl = ""
    parsed_idl = parsing._validate_and_parse(idl)

    assert parsed_idl == {"typedefs": {}, "enums": {}, "structs": {}, "inheritance_data": {}}


def test__validate_and_parse_typedef() -> None:
    idl = """
typedef int foobar;
typedef sequence<int> int_seq;
typedef record<ByteString, int> int_map;
typedef (int or float) int_or_float;
typedef [Size=4] sequence<int> int_seq_4;
    """
    parsed_idl = parsing._validate_and_parse(idl)

    assert "foobar" in parsed_idl["typedefs"]
    assert "int_seq" in parsed_idl["typedefs"]
    assert "int_map" in parsed_idl["typedefs"]
    assert "int_or_float" in parsed_idl["typedefs"]
    assert "int_seq_4" in parsed_idl["typedefs"]

    type_def_data = parsed_idl["typedefs"]["foobar"]
    assert type_def_data == {"type": "int"}

    type_def_data = parsed_idl["typedefs"]["int_seq"]["type"]
    assert type_def_data["type_name"] == "int"
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is True
    assert type_def_data["union"] is False

    type_def_data = parsed_idl["typedefs"]["int_map"]["type"]
    assert type_def_data["type_name"] == {"key": "string", "value": "int"}
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is True
    assert type_def_data["vector"] is False
    assert type_def_data["union"] is False

    type_def_data = parsed_idl["typedefs"]["int_or_float"]["type"]
    assert type_def_data["type_name"] == ["int", "float"]
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is False
    assert type_def_data["union"] is True

    type_def_data = parsed_idl["typedefs"]["int_seq_4"]["type"]
    assert type_def_data["type_name"] == "int"
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] == 4
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is True
    assert type_def_data["union"] is False


def test__validate_and_parse_enum() -> None:
    idl = """
enum FooBar {
    "foo",
    "bar",
    "baz"
};
    """
    parsed_idl = parsing._validate_and_parse(idl)

    assert "FooBar" in parsed_idl["enums"]
    assert parsed_idl["enums"]["FooBar"] == {"values": [{"name": "foo"}, {"name": "bar"}, {"name": "baz"}]}


def test__validate_and_parse_struct() -> None:
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
    parsed_idl = parsing._validate_and_parse(idl)

    assert "FooBar" in parsed_idl["structs"]
    assert "BazQux" in parsed_idl["structs"]

    struct_data = parsed_idl["structs"]["FooBar"]
    assert struct_data["inheritance"] is None
    struct_members = struct_data["members"]
    assert struct_members["foo"] == {"type": "int", "default": None, "required": False}
    assert struct_members["bar"] == {"type": "float", "default": None, "required": False}
    assert struct_members["baz"] == {
        "type": {"type_name": "int", "map": False, "union": False, "vector": True, "size": None, "ext_attrs": []},
        "default": None,
        "required": False,
    }
    assert struct_members["qux"] == {
        "type": {
            "type_name": {"key": "string", "value": "int"},
            "map": True,
            "union": False,
            "vector": False,
            "size": None,
            "ext_attrs": [],
        },
        "default": None,
        "required": False,
    }
    assert struct_members["quux"] == {  # Fails due to ext attrs!
        "type": {
            "type_name": "int",
            "map": False,
            "union": False,
            "vector": True,
            "size": 4,
            "ext_attrs": [],
        },
        "default": None,
        "required": False,
    }

    struct_data = parsed_idl["structs"]["BazQux"]
    assert struct_data["inheritance"] is None
    struct_members = struct_data["members"]
    assert struct_members["union"] == {
        "type": {
            "type_name": ["int", "float", "bool", "FooBar"],
            "map": False,
            "union": True,
            "vector": False,
            "size": None,
            "ext_attrs": [],
        },
        "default": None,
        "required": False,
    }


def test__validate_and_parse_struct_inheritance() -> None:
    idl = """
dictionary Foo{
};
dictionary Bar : Foo {
};
dictionary Baz : Bar {
};
dictionary Qux: Foo {
};
dictionary Quux {
    Qux qux;
};
typedef sequence<Bar> Quuz;
    """

    parsed_idl = parsing._validate_and_parse(idl)

    assert "Foo" in parsed_idl["structs"]
    assert "Bar" in parsed_idl["structs"]
    assert "Baz" in parsed_idl["structs"]
    assert "Qux" in parsed_idl["structs"]
    assert "Quux" in parsed_idl["structs"]
    assert "Quuz" in parsed_idl["typedefs"]

    struct_data = parsed_idl["structs"]["Foo"]
    assert struct_data["inheritance"] is None

    struct_data = parsed_idl["structs"]["Bar"]
    assert struct_data["inheritance"] == "Foo"

    struct_data = parsed_idl["structs"]["Baz"]
    assert struct_data["inheritance"] == "Bar"

    struct_data = parsed_idl["structs"]["Qux"]
    assert struct_data["inheritance"] == "Foo"

    assert parsed_idl["inheritance_data"] == {
        "Foo": ["Bar", "Qux"],
        "Bar": ["Baz"],
    }

    struct_data = parsed_idl["structs"]["Quux"]
    struct_members = struct_data["members"]
    assert struct_members["qux"]["type"] == "Foo"

    type_def_data = parsed_idl["typedefs"]["Quuz"]
    assert type_def_data["type"]["type_name"] == "Foo"


def test__validate_and_parse_struct_default_values_and_required() -> None:
    idl = """
dictionary Foo{
    int default_int = 42;
    float default_float = 3.14;
    required int required_int;
};
    """

    parsed_idl = parsing._validate_and_parse(idl)

    struct_data = parsed_idl["structs"]["Foo"]
    struct_members = struct_data["members"]
    assert struct_members["default_int"] == {"type": "int", "default": "42", "required": False}
    assert struct_members["default_float"] == {"type": "float", "default": "3.14", "required": False}
    assert struct_members["required_int"] == {"type": "int", "default": None, "required": True}


def test__flatten_dictionaries_partial_raises_error() -> None:
    dictionary_definition: dict[str, Any] = {
        "members": [],
        "inheritance": None,
        "partial": True,
        "ext_attrs": [],
    }

    with pytest.raises(RuntimeError, match="Partial dictionaries are not supported."):
        parsing._flatten_dictionaries(dictionary_definition)


def test__flatten_dictionaries_ext_attrs_raises_error() -> None:
    dictionary_definition = {
        "members": [],
        "inheritance": None,
        "partial": False,
        "ext_attrs": [{"name": "SomeAttr"}],
    }

    with pytest.raises(RuntimeError, match="Dictionary ext_attrs are not supported."):
        parsing._flatten_dictionaries(dictionary_definition)


def test__flatten_enums_non_enum_values_raises_error() -> None:
    enum_definition = {
        "values": [
            {"type": "foo"},
        ],
    }

    with pytest.raises(RuntimeError, match="Unsupported WebIDL type 'foo' in enum."):
        parsing._flatten_enums(enum_definition)


def test__flatten_raises_unsupported_type() -> None:
    with pytest.raises(RuntimeError, match="Unsupported WebIDL type 'foo'."):
        parsing._flatten({"definitions": [{"type": "foo"}]})


def test__flatten_type_raises_unrecognized_type() -> None:
    input_data = {
        "generic": True,
        "union": True,
    }

    with pytest.raises(RuntimeError, match="Unrecognised WebIDL type structure."):
        parsing._flatten_type(input_data)


def test__flatten_type_raises_record_element_count() -> None:
    input_data = {
        "generic": "record",
        "union": False,
        "idl_type": [1, 2, 3],
    }

    with pytest.raises(RuntimeError, match="Record must have two elements."):
        parsing._flatten_type(input_data)


def test__flatten_type_raises_sequence_element_count() -> None:
    input_data = {
        "generic": "sequence",
        "union": False,
        "idl_type": [1, 2],
        "ext_attrs": [],
    }

    with pytest.raises(RuntimeError, match="Sequence must have one element."):
        parsing._flatten_type(input_data)


def test__flatten_type_raises_ext_attrs_size() -> None:
    input_data = {
        "generic": "sequence",
        "union": False,
        "idl_type": [1],
        "ext_attrs": [{"name": "Size", "rhs": {"type": "float", "value": 4}}],
    }

    with pytest.raises(RuntimeError, match="Size attribute must be of type integer."):
        parsing._flatten_type(input_data)


def test__validate_and_parse_validation_has_errors() -> None:
    idl = """
typedef int foobar
    """
    with pytest.raises(RuntimeError, match="WebIDL validation errors:"):
        parsing._validate_and_parse(idl)


def test__flatten_members_raises_unsupported_type() -> None:
    with pytest.raises(RuntimeError, match="Unsupported WebIDL type 'foo'."):
        parsing._flatten_members([{"type": "foo"}])


def test__validate_and_parse_block_comments_added() -> None:
    idl = """
    /// This is a block comment for Foo
    dictionary Foo {
        int bar;
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["block_comment"] == "This is a block comment for Foo"


def test__validate_and_parse_block_comments_added_2() -> None:
    idl = """
    ///
    /// This is a block comment for Foo with pre and post comment indicators
    ///
    dictionary Foo {
        int bar;
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["block_comment"] == (
        "This is a block comment for Foo with pre and post comment indicators"
    )


def test__validate_and_parse_inline_comments_added() -> None:
    idl = """
    dictionary Foo {
        int bar; ///< This is an inline comment for bar
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["members"]["bar"]["inline_comment"] == "This is an inline comment for bar"


def test__validate_and_parse_mixed_comments_added() -> None:
    idl = """
    /// This is a block comment for Foo
    dictionary Foo {
        int bar; ///< This is an inline comment for bar
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["block_comment"] == "This is a block comment for Foo"
    assert parsed_idl["structs"]["Foo"]["members"]["bar"]["inline_comment"] == "This is an inline comment for bar"


def test__validate_and_parse_no_comments() -> None:
    idl = """
    dictionary Foo {
        int bar;
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert "block_comment" not in parsed_idl["structs"]["Foo"]


def test__validate_and_parse_invalid_comments() -> None:
    idl = """
    // This is a block comment for Foo
    dictionary Foo {
        int bar; // This is an inline comment for bar
    };
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert "block_comment" not in parsed_idl["structs"]["Foo"]


def test__get_comments_different_comment_styles() -> None:
    idl = """
/// This is a block comment for Foo
dictionary Foo {};

//! This is a block comment for Bar
dictionary Bar {};

/** This is a multi-line block comment for Baz
 *
 *  With multiple lines
 */
dictionary Baz {};

/*! This is a multi-line block comment for Qux
 *
 *  With multiple lines
 */
dictionary Qux {};

/*!
 This is a multi-line block comment for Quux
 */
dictionary Quux {};
"""
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["block_comment"] == "This is a block comment for Foo"
    assert parsed_idl["structs"]["Bar"]["block_comment"] == "This is a block comment for Bar"
    assert (
        parsed_idl["structs"]["Baz"]["block_comment"]
        == """This is a multi-line block comment for Baz

With multiple lines"""
    )
    assert (
        parsed_idl["structs"]["Qux"]["block_comment"]
        == """This is a multi-line block comment for Qux

With multiple lines"""
    )
    assert parsed_idl["structs"]["Quux"]["block_comment"] == "This is a multi-line block comment for Quux"


def test__validate_and_parse_type_def_with_comments() -> None:
    idl = """
/// This is a block comment for foobar
typedef int foobar; ///< This is a typedef for foobar
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["typedefs"]["foobar"]["block_comment"] == "This is a block comment for foobar"
    assert parsed_idl["typedefs"]["foobar"]["inline_comment"] == "This is a typedef for foobar"


def test__validate_and_parse_enum_with_comments() -> None:
    idl = """
/// This is a block comment for FooBar
enum FooBar {
    "foo", ///< This is the foo value
    "bar", ///< This is the bar value
    "baz", ///< This is the baz value
    /// This is a block comment for qux
    "qux"
};
    """
    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["enums"]["FooBar"]["block_comment"] == "This is a block comment for FooBar"
    assert parsed_idl["enums"]["FooBar"]["values"][0]["inline_comment"] == "This is the foo value"
    assert parsed_idl["enums"]["FooBar"]["values"][1]["inline_comment"] == "This is the bar value"
    assert parsed_idl["enums"]["FooBar"]["values"][2]["inline_comment"] == "This is the baz value"
    assert parsed_idl["enums"]["FooBar"]["values"][3]["block_comment"] == "This is a block comment for qux"


def test__type_check_impl_valid_union_type() -> None:
    type_data = {
        "type_name": ["int", "float"],
        "union": True,
        "vector": False,
        "map": False,
    }
    cpp_types = ["int", "float"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_union", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_union_type() -> None:
    type_data = {
        "type_name": ["int", "invalid_type"],
        "union": True,
        "vector": False,
        "map": False,
    }
    cpp_types = ["int"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'invalid_type' in union 'test_union' is not valid."):
        parsing._type_check_impl(type_data, "test_union", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_vector_type() -> None:
    type_data = {
        "type_name": "int",
        "union": False,
        "vector": True,
        "map": False,
    }
    cpp_types = ["int"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_vector", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_vector_type() -> None:
    type_data = {
        "type_name": "invalid_type",
        "union": False,
        "vector": True,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'invalid_type' in vector 'test_vector' is not valid."):
        parsing._type_check_impl(type_data, "test_vector", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_map_type() -> None:
    type_data = {
        "type_name": {"key": "string", "value": "int"},
        "union": False,
        "vector": False,
        "map": True,
    }
    cpp_types = ["int", "string"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_map", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_map_type() -> None:
    type_data = {
        "type_name": {"key": "string", "value": "invalid_type"},
        "union": False,
        "vector": False,
        "map": True,
    }
    cpp_types = ["string"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'invalid_type' in map 'test_map' is not valid."):
        parsing._type_check_impl(type_data, "test_map", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_typedef() -> None:
    type_data = {
        "type_name": "int",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types = ["int"]
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_typedef", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_typedef() -> None:
    type_data = {
        "type_name": "invalid_type",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'invalid_type' in 'test_typedef' is not valid."):
        parsing._type_check_impl(type_data, "test_typedef", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_struct_type() -> None:
    type_data = {
        "type_name": "Foo",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs = ["Foo"]
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_struct", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_struct_type() -> None:
    type_data = {
        "type_name": "Bar",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs = ["Foo"]
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'Bar' in 'test_struct' is not valid."):
        parsing._type_check_impl(type_data, "test_struct", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_enum_type() -> None:
    type_data = {
        "type_name": "FooBar",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations = ["FooBar"]
    structs: list[str] = []
    type_defs: list[str] = []

    parsing._type_check_impl(type_data, "test_enum", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_enum_type() -> None:
    type_data = {
        "type_name": "BarBaz",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations = ["FooBar"]
    structs: list[str] = []
    type_defs: list[str] = []

    with pytest.raises(RuntimeError, match="Member type 'BarBaz' in 'test_enum' is not valid."):
        parsing._type_check_impl(type_data, "test_enum", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_valid_type_def() -> None:
    type_data = {
        "type_name": "typedef_int",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs = ["typedef_int"]

    parsing._type_check_impl(type_data, "test_typedef", cpp_types, enumerations, structs, type_defs)


def test__type_check_impl_invalid_type_def() -> None:
    type_data = {
        "type_name": "typedef_float",
        "union": False,
        "vector": False,
        "map": False,
    }
    cpp_types: list[str] = []
    enumerations: list[str] = []
    structs: list[str] = []
    type_defs = ["typedef_int"]

    with pytest.raises(RuntimeError, match="Member type 'typedef_float' in 'test_typedef' is not valid."):
        parsing._type_check_impl(type_data, "test_typedef", cpp_types, enumerations, structs, type_defs)


def test__validate_and_parse_invalid_type() -> None:
    idl = """
typedef FOO foobar;
"""
    with pytest.raises(RuntimeError, match="Member type 'FOO' in 'foobar' is not valid."):
        parsing._validate_and_parse(idl)

    idl = """
typedef sequence<FOO> foobar;
"""
    with pytest.raises(RuntimeError, match="Member type 'FOO' in vector 'foobar' is not valid."):
        parsing._validate_and_parse(idl)

    idl = """
typedef record<ByteString, FOO> foobar;
"""
    with pytest.raises(RuntimeError, match="Member type 'FOO' in map 'foobar' is not valid."):
        parsing._validate_and_parse(idl)

    idl = """
dictionary Foo {
    FOO bar;
};
"""
    with pytest.raises(RuntimeError, match="Member type 'FOO' in 'Foo.bar' is not valid."):
        parsing._validate_and_parse(idl)

    idl = """
dictionary Foo {
    sequence<FOO> bar;
};
"""
    with pytest.raises(RuntimeError, match="Member type 'FOO' in vector 'Foo.bar' is not valid."):
        parsing._validate_and_parse(idl)


def test__add_comments_comment_for_undefined_type() -> None:
    idl = """
    /// Block comment for undefined
    typedef undefined foobar;

    typedef int baz; //< Inline comment for undefined
    """
    parsed_idl: ParsedIDL = {"typedefs": {"BAZ": {}}, "enums": {}, "structs": {}, "inheritance_data": {}}
    returned_idl = parsing._add_comments(idl, parsed_idl)

    assert returned_idl == parsed_idl


def test__add_comments_inline_comments_for_enum_def() -> None:
    idl = """
    enum Foo { ///< Inline comment for Foo
        "foo",
        "bar",
        "baz"
    };
    """

    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["enums"]["Foo"]["inline_comment"] == "Inline comment for Foo"


def test__add_comments_inline_comments_for_struct_def() -> None:
    idl = """
    dictionary Foo { ///< Inline comment for Foo
        int bar;
    };
    """

    parsed_idl = parsing._validate_and_parse(idl)
    assert parsed_idl["structs"]["Foo"]["inline_comment"] == "Inline comment for Foo"


def test__find_comments() -> None:
    idl = """
    /// This is a block comment for Foo
    dictionary Foo {
        int bar; ///< This is an inline comment for bar
        int baz; // This should be ignored
    };

    //!
    //! This is a block comment for Bar
    //!
    dictionary Bar {
    };
    """
    comment_data = parsing._find_comments(idl)

    assert comment_data["block_comments"]["dictionary Foo {"] == "/// This is a block comment for Foo"
    assert (
        comment_data["inline_comments"]["int bar;"]
        == "///< This is an inline comment for bar"
    )
    assert comment_data["block_comments"]["dictionary Bar {"] == "//!\n//! This is a block comment for Bar\n//!"
