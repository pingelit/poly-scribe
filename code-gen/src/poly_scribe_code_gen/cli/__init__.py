# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT
import argparse
import copy
import datetime
import json
from pathlib import Path
from typing import TYPE_CHECKING

from poly_scribe_code_gen.__about__ import __version__
from poly_scribe_code_gen.cpp_gen import generate_cpp

# from poly_scribe_code_gen.matlab_gen import generate_matlab
from poly_scribe_code_gen.parse_idl import parse_idl
from poly_scribe_code_gen.py_gen import generate_python, generate_python_package

if TYPE_CHECKING:
    from poly_scribe_code_gen._types import AdditionalData


def poly_scribe_code_gen() -> int:
    parser = argparse.ArgumentParser(prog="poly-scribe-code-gen", description="Generate poly-scribe code from WebIDL.")
    parser.add_argument("-v", "--version", action="version", version=__version__)
    parser.add_argument("input", help="Input WebIDL file to generate code from", type=Path)
    parser.add_argument("-c", "--cpp", help="Generate C++ code", type=Path, metavar="out")
    parser.add_argument("-p", "--py", help="Generate Python code", type=Path, metavar="out")
    parser.add_argument("-pp", "--py-package", help="Generate Python package", type=Path, metavar="out")
    parser.add_argument(
        "-s",
        "--schema",
        help="Generate JSON schema for the given IDL class",
        type=str,
        metavar=("out", "class"),
        nargs=2,
    )
    # parser.add_argument("-m", "--matlab", help="Generate Matlab code", type=Path, metavar="out")
    parser.add_argument(
        "-a", "--additional-data", help="Additional data for the generation", type=Path, metavar="data", required=True
    )

    args = parser.parse_args()

    parsed_idl = parse_idl(args.input)

    with open(args.additional_data) as f:
        additional_data: AdditionalData = json.load(f)

    additional_data["year"] = str(datetime.datetime.now(tz=datetime.timezone.utc).date().year)
    additional_data["out_file"] = args.cpp.name if args.cpp else None

    if args.cpp:
        cpp_idl_copy = copy.deepcopy(parsed_idl)
        generate_cpp(parsed_idl=cpp_idl_copy, additional_data=additional_data, out_file=args.cpp)

    # if args.matlab:
    #     matlab_idl_copy = copy.deepcopy(parsed_idl)
    #     generate_matlab(parsed_idl=matlab_idl_copy, additional_data=additional_data, out_path=args.matlab)

    if args.py:
        python_idl_copy = copy.deepcopy(parsed_idl)
        generate_python(parsed_idl=python_idl_copy, additional_data=additional_data, out_file=args.py)

    if args.py_package:
        python_idl_copy = copy.deepcopy(parsed_idl)
        generate_python_package(parsed_idl=python_idl_copy, additional_data=additional_data, out_dir=args.py_package)

    if args.schema and not args.py:
        msg = "Schema can only be generated with python"
        raise RuntimeError(msg)

    if args.schema:
        import importlib.util
        import inspect
        import sys

        module_name = "idl_module"
        spec = importlib.util.spec_from_file_location(module_name, args.py)
        if spec is None:
            msg = f"Failed to load python file '{args.py}'"
            raise RuntimeError(msg)
        if spec.loader is None:
            msg = f"Failed to load python file '{args.py}'"
            raise RuntimeError(msg)
        idl_module = importlib.util.module_from_spec(spec)
        sys.modules[module_name] = idl_module
        spec.loader.exec_module(idl_module)

        out_file = Path(args.schema[0])
        requested_model = args.schema[1]

        class_members = inspect.getmembers(sys.modules[module_name], inspect.isclass)

        class_members_flatten = [m[0] for m in class_members]

        if requested_model not in class_members_flatten:
            msg = f"Data model '{requested_model}' not found in given IDL."
            raise RuntimeError(msg)

        schema_data = getattr(idl_module, requested_model).schema_json(indent=2)

        with open(out_file, "w") as f:
            f.write(schema_data)

    return 0
