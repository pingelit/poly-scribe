# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT
import argparse
from pathlib import Path

from poly_scribe_code_gen.__about__ import __version__
from poly_scribe_code_gen.cpp_gen import generate_cpp
from poly_scribe_code_gen.parse_idl import parse_idl


def poly_scribe_code_gen():
    parser = argparse.ArgumentParser(prog="poly-scribe-code-gen", description="Generate poly-scribe code from WebIDL.")
    parser.add_argument("-v", "--version", action="version", version=__version__)
    parser.add_argument("input", help="Input WebIDL file to generate code from", type=Path)
    parser.add_argument("-c", "--cpp", help="Generate C++ code", type=Path, metavar="out")

    args = parser.parse_args()

    parsed_idl = parse_idl(args.input)

    if args.cpp:
        generate_cpp(
            parsed_idl=parsed_idl, additional_data={"author_name": "test", "namespace": "foobar"}, out_file=args.cpp
        )
