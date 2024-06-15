import json
import os

import pytest
from jsonschema import validate

from utils import gen_random_integration_test


@pytest.mark.parametrize("test_num", range(5))
def test_schema(test_num):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    json_data = data_struct.json()
    assert json_data is not None

    schema_file = os.getenv("SCHEMA_FILE")
    if schema_file is None:
        raise Exception("SCHEMA_FILE environment variable is not set")

    with open(schema_file) as f:
        schema = json.load(f)

    validate(instance=json.loads(json_data), schema=schema)
