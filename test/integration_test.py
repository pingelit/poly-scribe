import pytest
import integration_data

from utils import gen_random_integration_test


@pytest.mark.parametrize("test_num", range(5))
def test_integration_data(test_num):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    json_data = data_struct.model_dump_json()
    assert json_data is not None

    new_data_struct = integration_data.IntegrationTest.model_validate_json(json_data)

    assert data_struct == new_data_struct
