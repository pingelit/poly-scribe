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
