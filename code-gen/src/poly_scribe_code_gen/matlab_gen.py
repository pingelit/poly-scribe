# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
from typing import Any

import jinja2

from poly_scribe_code_gen.cpp_gen import AdditionalData


def generate_matlab(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_path: Path):
    out_path.mkdir(parents=True, exist_ok=True)

    if not out_path.is_dir():
        msg = "The output path must be a directory"
        raise ValueError(msg)

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
        if struct["name"] in parsed_idl["inheritance_data"]:
            additional_data = {**additional_data, "sub_classes": parsed_idl["inheritance_data"][struct["name"]]}

        data = {**additional_data, **struct}

        res = j2_template.render(data)

        name = struct["name"]
        with open(out_path / f"{name}.m", "w") as f:
            f.write(res)

    j2_template = env.get_template("matlab_enum.jinja")

    for enum in parsed_idl["enums"]:
        data = {**additional_data, **enum}

        res = j2_template.render(data)

        name = enum["name"]
        with open(out_path / f"{name}.m", "w") as f:
            f.write(res)


def _transform_types(parsed_idl):
    # https://www.mathworks.com/help/matlab/matlab_oop/example-representing-structured-data.html
    # https://www.mathworks.com/help/matlab/matlab_prog/fundamental-matlab-classes.html
    # https://www.mathworks.com/help/matlab/matlab_oop/initialize-property-values.html
    # https://www.mathworks.com/help/matlab/matlab_oop/validate-property-values.html
    # https://www.mathworks.com/help/matlab/matlab_oop/property-size-and-class-validation.html
    conversion = {
        "string": "string",
        "ByteString": "string",
        "bool": "logical",
        "float": "single",
        "double": "double",
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
        if type_input["type_name"] in conversion:
            return conversion[type_input["type_name"]]
        elif next((item for item in parsed_idl["type_defs"] if item["name"] == type_input["type_name"]), None):
            # todo: handle nested typedefs at least in vectors
            msg = "Nested type cannot be a typedef"
            raise ValueError(msg)
        else:
            return type_input["type_name"]

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
        if type_input["map"]:
            msg = "Type cannot be a map"
            raise ValueError(msg)
        elif type_input["union"] or not type_input["vector"]:
            return type_transformer(type_input), []

        transformed_type, size = vector_transformer(type_input["type_name"][0])

        current_size = ":"
        for attr in type_input["ext_attrs"]:
            if attr["name"] == "Size" and attr["rhs"]["type"] == "integer":
                vec_size = attr["rhs"]["value"]
                current_size = f"{vec_size}"

        return transformed_type, [current_size, *size]

    def map_transformer(type_input):
        if type_input["type_name"][0]["type_name"] not in ["string", "ByteString"]:
            msg = "Map key must be a string"
            raise ValueError(msg)

        transformed_value_type = type_transformer(type_input["type_name"][1])
        return transformed_value_type, [], "struct"

    def _matlab_transformer(type_input):
        if type_input["map"]:
            return map_transformer(type_input)
        elif type_input["vector"]:
            transformed_type, size = vector_transformer(type_input)
            return transformed_type, size

        if definition := next(
            (item for item in parsed_idl["type_defs"] if item["name"] == type_input["type_name"]), None
        ):
            return _matlab_transformer(definition["type"])

        return type_transformer(type_input), []

    for struct in parsed_idl["structs"]:
        for member in struct["members"]:
            foo = _matlab_transformer(member["type"])

            if len(foo[1]) == 0:
                variable_shape = "(1,1)"
            elif len(foo[1]) == 1:
                variable_shape = f"(1,{foo[1][0]})"
            else:
                variable_shape = f"({','.join(foo[1])})"

            non_pod = False
            if next((item for item in parsed_idl["structs"] if item["name"] in foo[0]), None):
                if len(foo[0]) > 1:
                    msg = "Nested structs are not supported"
                    raise ValueError(msg)

                struct["has_non_pod"] = True
                non_pod = True

            if not member["default"] and non_pod and not len(foo) == 3:
                if ":" in variable_shape:
                    member["default"] = "cell(1)"
                else:
                    member["default"] = f"cell{variable_shape}"

            if isinstance(member["default"], dict):
                member["default"] = member["default"]["value"]

            member["validation"] = {
                "must_be": ", ".join(f'"{t}"' for t in foo[0]),
                "size": variable_shape,
                "non_pod": non_pod,
                "type": foo[0][0] if len(foo[0]) == 1 else None,
            }

            if len(foo) == 3:  # noqa: PLR2004
                struct["has_map"] = True

    return parsed_idl
