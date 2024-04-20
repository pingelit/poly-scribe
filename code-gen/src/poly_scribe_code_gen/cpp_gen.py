# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
from typing import Any, TypedDict

import jinja2


class AdditionalData(TypedDict):
    author_name: str
    author_email: str
    out_file: str
    year: str
    licence: str
    namespace: str


def generate_cpp(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_file: Path):
    """Generate a C++ header for a poly-scribe data structure.

    Parameters
    ----------
    parsed_idl : dict[str, Any]
        The IDL data structure
    additional_data : dict[str, Any]
        Additional data to be used in the rendering
    out_file : Path
        Output file
    """

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

    j2_template = env.get_template("template.hpp.jinja")

    data = {**additional_data, **parsed_idl}

    res = j2_template.render(data)

    out_file.parent.mkdir(parents=True, exist_ok=True)

    with open(out_file, "w") as f:
        f.write(res)


def _transform_types(parsed_idl):
    def _polymorphic_transformer(type_name):
        if type_name in parsed_idl["inheritance_data"]:
            return f"std::shared_ptr<{type_name}>"

        for base_type, derived_types in parsed_idl["inheritance_data"].items():
            if type_name in derived_types and len(derived_types) > 1:
                return f"std::shared_ptr<{base_type}>"

        return type_name

    def _transformer(type_input):
        if not type_input["union"] and not type_input["vector"] and not type_input["map"]:
            conversion = {"string": "std::string", "ByteString": "std::string"}
            return (
                conversion[type_input["type_name"]]
                if type_input["type_name"] in conversion
                else _polymorphic_transformer(type_input["type_name"])
            )
        if not type_input["union"] and type_input["vector"] and not type_input["map"]:
            transformed_type = _transformer(type_input["type_name"][0])
            for attr in type_input["ext_attrs"]:
                if attr["name"] == "Size" and attr["rhs"]["type"] == "integer":
                    size = attr["rhs"]["value"]
                    return f"std::array<{transformed_type}, {size}>"
            return f"std::vector<{transformed_type}>"
        if not type_input["union"] and not type_input["vector"] and type_input["map"]:
            transformed_key_type = _transformer(type_input["type_name"][0])
            transformed_value_type = _transformer(type_input["type_name"][1])
            return f"std::unordered_map<{transformed_key_type}, {transformed_value_type}>"
        if type_input["union"] and not type_input["vector"] and not type_input["map"]:
            contained_types = []
            for contained in type_input["type_name"]:
                contained_types.append(_transformer(contained))
            transformed_type = ",".join(contained_types)
            return f"std::variant<{transformed_type}>"

    for struct in parsed_idl["structs"]:
        for member in struct["members"]:
            member["type"] = _transformer(member["type"])

    for type_def in parsed_idl["type_defs"]:
        type_def["type"] = _transformer(type_def["type"])

    return parsed_idl
