import poly_scribe_code_gen.parse_idl as parsing


def test_parse_idl(mocker):
    mocker.patch("builtins.open", mocker.mock_open(read_data="dummy"))
    validate_mock = mocker.patch("poly_scribe_code_gen.parse_idl._validate_and_parse", return_value={})

    parsed_idl = parsing.parse_idl("dummy")

    assert parsed_idl == {}
    validate_mock.assert_called_once_with("dummy")


def test__validate_and_parse_empty_idl():
    idl = ""
    parsed_idl = parsing._validate_and_parse(idl)

    assert parsed_idl == {"typedefs": {}, "enums": {}, "structs": {}}


def test__validate_and_parse_typedef():
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
    assert type_def_data == "int"

    type_def_data = parsed_idl["typedefs"]["int_seq"]
    assert type_def_data["type_name"] == "int"
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is True
    assert type_def_data["union"] is False

    type_def_data = parsed_idl["typedefs"]["int_map"]
    assert type_def_data["type_name"] == {"key": "string", "value": "int"}
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is True
    assert type_def_data["vector"] is False
    assert type_def_data["union"] is False

    type_def_data = parsed_idl["typedefs"]["int_or_float"]
    assert type_def_data["type_name"] == ["int", "float"]
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] is None
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is False
    assert type_def_data["union"] is True

    type_def_data = parsed_idl["typedefs"]["int_seq_4"]
    assert type_def_data["type_name"] == "int"
    assert type_def_data["ext_attrs"] == []
    assert type_def_data["size"] == 4
    assert type_def_data["map"] is False
    assert type_def_data["vector"] is True
    assert type_def_data["union"] is False


def test__validate_and_parse_enum():
    idl = """
enum FooBar {
    "foo",
    "bar",
    "baz"
};
    """
    parsed_idl = parsing._validate_and_parse(idl)

    assert "FooBar" in parsed_idl["enums"]
    assert parsed_idl["enums"]["FooBar"] == ["foo", "bar", "baz"]


def test__validate_and_parse_struct():
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
    struct_data["inheritance"] is None
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
    struct_data["inheritance"] is None
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


def test__validate_and_parse_struct_inheritance():
    idl = """
dictionary Foo{
};
dictionary Bar : Foo {
};
dictionary Baz : Bar {
};
dictionary Qux: Foo {
};
    """

    parsed_idl = parsing._validate_and_parse(idl)

    assert "Foo" in parsed_idl["structs"]
    assert "Bar" in parsed_idl["structs"]
    assert "Baz" in parsed_idl["structs"]
    assert "Qux" in parsed_idl["structs"]

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


def test__validate_and_parse_struct_default_values_and_required():
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
