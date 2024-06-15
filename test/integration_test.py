import random
import string

import pytest
import integration_data


def random_string(length: int) -> str:
    return "".join(random.choice(string.ascii_letters) for _ in range(length))


def gen_random_base():
    obj = integration_data.Base(
        vec_3d=[random.random() for _ in range(3)],
        union_member=random.choice([random.random(), random.randint(0, 100)]),
        str_vec=[random_string(5) for _ in range(random.choice([1, 2, 5]))],
    )
    return obj


def gen_random_derived_one():
    obj = integration_data.DerivedOne(
        **gen_random_base().dict(),
        string_map={
            random_string(5): random_string(5) for _ in range(random.choice([1, 2, 5]))
        }
    )
    return obj


def gen_random_derived_two():
    obj = integration_data.DerivedTwo(**gen_random_base().dict())
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


@pytest.mark.parametrize("test_num", range(5))
def test_integration_data(test_num):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    json_data = data_struct.json()
    assert json_data is not None

    new_data_struct = integration_data.IntegrationTest.parse_raw(json_data)

    assert data_struct == new_data_struct
