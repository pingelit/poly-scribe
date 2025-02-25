# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import black
import isort
import jinja2

from poly_scribe_code_gen._types import AdditionalData


def generate_python(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_file: Path):
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

    j2_template = env.get_template("python.jinja")

    data = {**additional_data, **parsed_idl}

    res = j2_template.render(data)

    with open(out_file, "w") as f:
        f.write(res)

    black.format_file_in_place(out_file, write_back=black.WriteBack.YES, fast=True, mode=black.FileMode())

    isort.file(out_file)


def _transform_types(parsed_idl):
    @dataclass
    class ExtraData:
        polymorphic: bool = False

    def _polymorphic_transformer(type_name):
        if type_name in parsed_idl["inheritance_data"]:
            derived = parsed_idl["inheritance_data"][type_name]

            for derived_type in derived:
                if derived_type in parsed_idl["inheritance_data"]:
                    derived.extend(parsed_idl["inheritance_data"][derived_type])

            derived_list = ", ".join(derived)
            return f"Union[{derived_list}, {type_name}]", ExtraData(polymorphic=True)

        for base_type, derived_types in parsed_idl["inheritance_data"].items():
            if type_name in derived_types and len(derived_types) > 1:
                derived = parsed_idl["inheritance_data"][base_type]

                for derived_type in derived:
                    if derived_type in parsed_idl["inheritance_data"]:
                        derived.extend(parsed_idl["inheritance_data"][derived_type])

                derived_list = ", ".join(derived)
                return f"Union[{derived_list}, {type_name}]", ExtraData(polymorphic=True)

        return type_name, ExtraData

    def _transformer(type_input):
        if type_input["union"]:
            contained_types = []
            for contained in type_input["type_name"]:
                transformed_type, extra_data = _transformer(contained)
                if extra_data.polymorphic:
                    msg = "Unions with polymorphic types are not supported"
                    raise ValueError(msg)
                contained_types.append(transformed_type)
            transformed_type = ",".join(contained_types)
            return f"Union[{transformed_type}]", ExtraData()
        if type_input["vector"]:
            transformed_type, extra_data = _transformer(type_input["type_name"][0])
            for attr in type_input["ext_attrs"]:
                if attr["name"] == "Size" and attr["rhs"]["type"] == "integer":
                    size = attr["rhs"]["value"]
                    return f"Annotated[List[{transformed_type}], Len(min_length={size}, max_length={size})]", extra_data
            return f"List[{transformed_type}]", extra_data
        if type_input["map"]:
            transformed_key_type, extra_data_key = _transformer(type_input["type_name"][0])
            if extra_data_key.polymorphic:
                msg = "Maps with polymorphic keys are not supported"
                raise ValueError(msg)

            transformed_value_type, extra_data_value = _transformer(type_input["type_name"][1])
            return f"Dict[{transformed_key_type}, {transformed_value_type}]", extra_data_value
        else:
            conversion = {
                "string": "str",
                "ByteString": "str",
                "bool": "bool",
                "float": "float",
                "double": "float",
                "long double": "float",
                "char": "int",
                "unsigned char": "int",
                "short": "int",
                "unsigned short": "int",
                "int": "int",
                "unsigned int": "int",
                "long": "int",
                "unsigned long": "int",
                "long long": "int",
                "unsigned long long": "int",
            }
            if type_input["type_name"] in conversion:
                return conversion[type_input["type_name"]], ExtraData()
            else:
                return _polymorphic_transformer(type_input["type_name"])

    for struct in parsed_idl["structs"]:
        for member in struct["members"]:
            transformed_type, extra_data = _transformer(member["type"])
            member["type"] = transformed_type
            if extra_data.polymorphic:
                member["type"] = f"Annotated[{member['type']}, Field(discriminator=\"type\")]"

        for _, derived_types in parsed_idl["inheritance_data"].items():
            if struct["name"] in derived_types:
                # check if there is no member in struct is already named "type"
                if not any(member["name"] == "type" for member in struct["members"]):
                    struct_name = struct["name"]
                    struct["members"].append(
                        {
                            "name": "type",
                            "type": f'Literal["{struct_name}"]',
                            "extra_data": ExtraData(),
                            "default": f"{struct_name}",
                        }
                    )

    for type_def in parsed_idl["type_defs"]:
        type_def["type"] = _transformer(type_def["type"])[0]

    return parsed_idl
