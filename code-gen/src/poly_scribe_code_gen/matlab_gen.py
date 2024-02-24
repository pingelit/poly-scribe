# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
from pprint import pprint
from typing import Any, TypedDict

from poly_scribe_code_gen.cpp_gen import AdditionalData

import jinja2


def generate_matlab(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_path: Path):
    if not out_path.is_dir():
        raise ValueError("The output path must be a directory")

    pprint(parsed_idl)

    parsed_idl = _transform_types(parsed_idl)

    package_dir = os.path.abspath(os.path.dirname(__file__))
    templates_dir = os.path.join(package_dir, "templates")

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        trim_blocks=True,
        lstrip_blocks=True,
        autoescape=jinja2.select_autoescape(
            disabled_extensions=("hpp.jinja",),
            default_for_string=True,
            default=False,
        ),
    )

    j2_template = env.get_template("matlab.jinja")

    for struct in parsed_idl["structs"]:
        data = {**additional_data, **struct}

        res = j2_template.render(data)

        name = struct["name"]
        with open(out_path / f"{name}.m", "w") as f:
            f.write(res)


def _transform_types(parsed_idl):
    conversion = {
        "string": "char",
        "ByteString": "char",
        "bool": "logical",
        "float": "single",
        "char": "int8",
        "unsigned char": "uint8",
        "short": "int16",
        "unsigned short": "uint16",
        "int": "int32",
        "unsigned int": "uint32",
        "long": "int64",
        "unsigned long": "uint64",
        "long long": "int64",
        "unsigned long long": "uint64",
        "long double": "double",
    }

    def pod_transformer(type_input):
        return (
            type_input["type_name"]
            if type_input["type_name"] not in conversion
            else conversion[type_input["type_name"]]
        )

    def union_transformer(type_input):
        contained_types = []
        for contained in type_input["type_name"]:
            contained_types.append(pod_transformer(contained))
        return contained_types

    def type_transformer(type_input):
        if type_input["map"] or type_input["vector"]:
            msg = "Type cannot be a map or a vector"
            raise ValueError(msg)

        if type_input["union"]:
            return union_transformer(type_input)
        else:
            return [pod_transformer(type_input)]

    def vector_transformer(type_input):
        if not type_input["vector"]:
            msg = "The type is not a vector"
            raise ValueError(msg)

        transformed_type, size = vector_transformer(type_input["type_name"][0])
        for attr in type_input["ext_attrs"]:
            if attr["name"] == "Size" and attr["rhs"]["type"] == "integer":
                size = attr["rhs"]["value"]
                return f"std::array<{transformed_type}, {size}>"
        return f"std::vector<{transformed_type}>"

    def _matlab_transformer(type_input):
        if type_input["map"]:
            return [], None
        elif type_input["vector"]:
            transformed_type = _matlab_transformer(type_input["type_name"][0])
            return transformed_type[0], None

        return type_transformer(type_input), None

    for struct in parsed_idl["structs"]:
        for member in struct["members"]:
            foo = _matlab_transformer(member["type"])
            if not member["default"]:
                if len(foo[0]) >= 1:
                    member["default"] = "''" if foo[0][0] == "char" else "[]"
                else:
                    member["default"] = "[]"
            member["validation"] = {
                "must_be": ", ".join(f'"{t}"' for t in foo[0]),
            }

    for type_def in parsed_idl["type_defs"]:
        type_def["type"] = _matlab_transformer(type_def["type"])

    return parsed_idl
