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
