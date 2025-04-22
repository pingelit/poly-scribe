import json
import os
import subprocess

import pytest
from jsonschema import validate

from utils import gen_random_integration_test
import shutil
from pathlib import Path


@pytest.mark.parametrize("test_num", range(5))
def test_schema(test_num):
    data_struct = gen_random_integration_test()

    assert data_struct is not None

    json_data = data_struct.model_dump_json()
    assert json_data is not None

    schema_file = os.getenv("SCHEMA_FILE")
    if schema_file is None:
        raise Exception("SCHEMA_FILE environment variable is not set")

    with open(schema_file) as f:
        schema = json.load(f)

    validate(instance=json.loads(json_data), schema=schema)


def test_cpp():
    cpp_exe = os.getenv("CPP_EXE")
    if cpp_exe is None:
        raise Exception("CPP_EXE environment variable is not set")
    assert os.path.exists(cpp_exe)

    tmp_dir = os.getenv("TMP_DIR")
    if tmp_dir is None:
        raise Exception("TMP_DIR environment variable is not set")
    assert os.path.exists(tmp_dir)
    tmp_file = Path(tmp_dir) / "schema_test.json"

    subprocess.run([cpp_exe, tmp_file], check=True)

    schema_file = os.getenv("SCHEMA_FILE")
    if schema_file is None:
        raise Exception("SCHEMA_FILE environment variable is not set")

    with open(schema_file) as f:
        schema = json.load(f)

    with open(tmp_file) as f:
        instance = json.load(f)

    validate(instance=instance, schema=schema)


@pytest.mark.skipif(
    shutil.which("matlab") is None,
    reason="Matlab is not available",
)
@pytest.mark.skip(reason="Matlab code gen is not updated")
def test_matlab():
    matlab_exe = shutil.which("matlab")
    if matlab_exe is None:
        raise Exception("Matlab is not available")

    matlab_dir = os.getenv("MATLAB")
    if matlab_dir is None:
        raise Exception("MATLAB environment variable is not set")
    assert os.path.exists(matlab_dir)

    cpp_exe = os.getenv("CPP_EXE")
    if cpp_exe is None:
        raise Exception("CPP_EXE environment variable is not set")
    assert os.path.exists(cpp_exe)

    tmp_dir = os.getenv("TMP_DIR")
    if tmp_dir is None:
        raise Exception("TMP_DIR environment variable is not set")
    assert os.path.exists(tmp_dir)

    tmp_file = os.path.join(tmp_dir, "schema_test_matlab.json")
    tmp_cpp_file = os.path.join(tmp_dir, "schema_test.json")

    subprocess.run([cpp_exe, tmp_cpp_file])

    matlab_script = f"""
addpath('{matlab_dir}');

integration_test = IntegrationTest.load_from_file('{tmp_cpp_file}');

integration_test.store_to_file('{tmp_file}');
"""

    subprocess.run([matlab_exe, "-batch", matlab_script])

    schema_file = os.getenv("SCHEMA_FILE")
    if schema_file is None:
        raise Exception("SCHEMA_FILE environment variable is not set")

    with open(schema_file) as f:
        schema = json.load(f)

    with open(tmp_file) as f:
        instance = json.load(f)

    validate(instance=instance, schema=schema)
