import pytest
import integration_data
import os
import subprocess

from pathlib import Path
from utils import gen_random_integration_test
from pydantic_yaml import parse_yaml_raw_as, to_yaml_str


@pytest.mark.parametrize("test_num", range(5))
def test_integration_data(test_num):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    json_data = data_struct.model_dump_json()
    assert json_data is not None

    new_data_struct = integration_data.IntegrationTest.model_validate_json(json_data)

    assert data_struct == new_data_struct


# @pytest.mark.parametrize("test_num", range(5))
@pytest.mark.parametrize("input_format", ["json", "yaml"])
@pytest.mark.parametrize("output_format", ["json", "yaml"])
def test_integration_data_round_trip(input_format, output_format):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    cpp_exe = os.getenv("CPP_EXE")
    if cpp_exe is None:
        raise Exception("CPP_EXE environment variable is not set")
    assert os.path.exists(cpp_exe)

    tmp_dir = os.getenv("TMP_DIR")
    if tmp_dir is None:
        raise Exception("TMP_DIR environment variable is not set")
    assert os.path.exists(tmp_dir)
    py_out = Path(tmp_dir).absolute() / f"integration_py_out.{output_format}"
    cpp_out = Path(tmp_dir).absolute() / f"integration_cpp_out.{input_format}"

    with open(py_out, "w") as f:
        if output_format == "json":
            f.write(data_struct.model_dump_json(indent=2))
        elif output_format == "yaml":
            f.write(to_yaml_str(data_struct))
        else:
            raise Exception(f"Invalid output format: {output_format}")

    subprocess.run([cpp_exe, cpp_out, py_out], check=True)

    with open(cpp_out) as f:
        if input_format == "json":
            new_data = integration_data.IntegrationTest.model_validate_json(f.read())
        elif input_format == "yaml":
            new_data = parse_yaml_raw_as(integration_data.IntegrationTest, f.read())
        else:
            raise Exception(f"Invalid input format: {input_format}")

    assert data_struct == new_data
