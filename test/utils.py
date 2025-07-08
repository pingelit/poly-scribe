import random
import string

import integration_data


def random_string(length: int) -> str:
    return "".join(random.choice(string.ascii_letters) for _ in range(length))


def gen_random_base():
    obj = integration_data.Base(
        vec_3d=[random.random() for _ in range(3)],
        union_member=random.choice([random.random(), random.randint(0, 100), None]),
        str_vec=[random_string(5) for _ in range(random.choice([1, 2, 5]))],
    )
    return obj


def gen_random_derived_one():
    base_dict = gen_random_base().model_dump()
    base_dict.pop("type", None)
    obj = integration_data.DerivedOne(
        **base_dict,
        string_map={
            random_string(5): random_string(5) for _ in range(random.choice([1, 2, 5]))
        }
    )
    return obj


def gen_random_derived_two():
    base_dict = gen_random_base().model_dump()
    base_dict.pop("type", None)
    obj = integration_data.DerivedTwo(**base_dict)
    return obj


def gen_random_non_poly_derived():
    obj = integration_data.NonPolyDerived(value=random.randint(0, 100))
    return obj


def gen_random_integration_test():
    obj = integration_data.IntegrationTest(
        object_map={
            random_string(5): random.choice(
                [gen_random_derived_one, gen_random_derived_two]
            )()
            for _ in range(random.choice([1, 2, 5]))
        },
        object_vec=[
            random.choice([gen_random_derived_one, gen_random_derived_two])()
            for _ in range(random.choice([1, 2, 5]))
        ],
        object_array=[
            random.choice([gen_random_derived_one, gen_random_derived_two])()
            for _ in range(2)
        ],
        enum_value=random.choice(
            [integration_data.Enumeration.value1, integration_data.Enumeration.value2]
        ),
        non_poly_derived=gen_random_non_poly_derived(),
    )
    return obj


def compare_integration_data(
    lhs: integration_data.IntegrationTest, rhs: integration_data.IntegrationTest
):
    assert lhs.non_poly_derived == rhs.non_poly_derived
    assert lhs.enum_value == rhs.enum_value

    assert len(lhs.object_map) == len(rhs.object_map)
    for key in lhs.object_map:
        assert key in rhs.object_map
        compare_poly_structure(lhs.object_map[key], rhs.object_map[key])

    assert len(lhs.object_vec) == len(rhs.object_vec)
    for i in range(len(lhs.object_vec)):
        compare_poly_structure(lhs.object_vec[i], rhs.object_vec[i])

    assert len(lhs.object_array) == len(rhs.object_array)
    for i in range(len(lhs.object_array)):
        compare_poly_structure(lhs.object_array[i], rhs.object_array[i])


def compare_poly_structure(lhs: integration_data.Base, rhs: integration_data.Base):
    assert all(abs(a - b) < 1e-6 for a, b in zip(lhs.vec_3d, rhs.vec_3d))
    if isinstance(lhs.union_member, float) and isinstance(rhs.union_member, float):
        assert abs(lhs.union_member - rhs.union_member) < 1e-6
    else:
        assert lhs.union_member == rhs.union_member
    assert lhs.str_vec == rhs.str_vec

    if isinstance(lhs, integration_data.DerivedOne):
        assert lhs.string_map == rhs.string_map
    elif isinstance(lhs, integration_data.DerivedTwo):
        assert lhs.optional_value == rhs.optional_value
