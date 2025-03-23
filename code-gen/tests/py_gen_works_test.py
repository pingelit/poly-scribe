import types

from poly_scribe_code_gen import py_gen
from poly_scribe_code_gen.parse_idl import _validate_and_parse


# based on
# https://stackoverflow.com/questions/55905240/python-dynamically-import-modules-code-from-string-with-importlib
def import_code(code: str, name: str) -> types.ModuleType:
    # create blank module
    module = types.ModuleType(name)
    # populate the module with code
    exec(code, module.__dict__)  # noqa: S102
    return module


def test_python_gen_polymorphic_structs_work() -> None:
    idl = """
dictionary Base {
    (int or float) union_member;
    required sequence<string> str_vec;
};

dictionary DerivedOne : Base {
    required record<ByteString, string> string_map;
};

dictionary DerivedTwo : Base {
    float optional_value = 3.141;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "foo"})

    module = import_code(result, "foobar")

    base = module.Base(str_vec=["a", "b"])
    derived_one = module.DerivedOne(string_map={"a": "b"}, str_vec=["a", "b"])
    derived_two = module.DerivedTwo(str_vec=["a", "b"])
    derived_two_1 = module.DerivedTwo(str_vec=["a", "b"], union_member=3.141)

    assert base
    assert derived_one
    assert derived_two
    assert derived_two_1


def test_python_gen_required_opt_list_works() -> None:
    idl = """
dictionary Foo {
    required sequence<int> required_list;
    sequence<int> optional_list;
    required record<ByteString, string> required_map;
    record<ByteString, string> optional_map;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "foo"})

    module = import_code(result, "foobar")

    foo = module.Foo(required_list=[1, 2, 3], required_map={})

    assert foo
    assert foo.required_list == [1, 2, 3]
    assert foo.optional_list is None
    assert foo.required_map == {}
    assert foo.optional_map is None


def test_python_gen_union_works() -> None:
    idl = """
dictionary Base {
    (int or float) union_member;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "foo"})

    module = import_code(result, "foobar")

    base = module.Base(union_member=1)
    base_1 = module.Base(union_member=1.1)

    assert base
    assert base_1

    assert isinstance(base.union_member, int)
    assert isinstance(base_1.union_member, float)


def test_python_gen_union_polymorphic_works() -> None:
    idl = """
dictionary Base {
    int member;
};

dictionary DerivedOne : Base {
};

dictionary DerivedTwo : Base {
};

dictionary Container {
    (string or Base) union_test;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "foo"})

    module = import_code(result, "foobar")

    container = module.Container(union_test="test")
    container_1 = module.Container(union_test=module.DerivedOne())
    container_2 = module.Container()

    assert container
    assert container_1
    assert container_2

    assert isinstance(container.union_test, str)
    assert isinstance(container_1.union_test, module.DerivedOne)
    assert container_2.union_test is None


def test_python_gen_deserialize_works() -> None:
    idl = """
dictionary Base {
    int member;
};

dictionary DerivedOne : Base {
};

dictionary DerivedTwo : Base {
};

dictionary Container {
    required sequence<Base> object_vec;
    required Base poly;
    required (string or Base) union_test_1;
};
"""
    parsed_idl = _validate_and_parse(idl)

    result = py_gen._render_template(parsed_idl, {"package": "foo"})

    module = import_code(result, "foobar")

    json_input = """
{
    "object_vec": [
        {
            "type": "DerivedTwo",
            "member": 1
        },
        {
            "type": "DerivedOne",
            "member": 2
        }
    ],
    "poly": {
        "type": "Base",
        "member": 3
    },
    "union_test_1": "test"
}
"""

    container = module.Container.model_validate_json(json_input)

    assert container
    assert len(container.object_vec) == 2
    assert container.object_vec[0].member == 1
    assert isinstance(container.object_vec[0], module.DerivedTwo)
    assert isinstance(container.object_vec[1], module.DerivedOne)
    assert container.object_vec[1].member == 2
    assert container.poly.member == 3
    assert isinstance(container.poly, module.Base)
    assert isinstance(container.union_test_1, str)
    assert container.union_test_1 == "test"
