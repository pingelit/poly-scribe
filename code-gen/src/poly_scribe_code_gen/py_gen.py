# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
from pprint import pprint
from typing import Any

import jinja2
from cpp_gen import AdditionalData


def generate_python(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_file: Path):
    parsed_idl = _transform_types(parsed_idl)

    pprint(parsed_idl)

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


def _transform_types(parsed_idl):
    def _polymorphic_transformer(type_name):
        if type_name in parsed_idl["inheritance_data"]:
            derived = parsed_idl["inheritance_data"][type_name]

            for derived_type in derived:
                if derived_type in parsed_idl["inheritance_data"]:
                    derived.extend(parsed_idl["inheritance_data"][derived_type])

            derived_list = ", ".join(derived)
            return f"Union[{derived_list}, {type_name}]"

        for base_type, derived_types in parsed_idl["inheritance_data"].items():
            if type_name in derived_types and len(derived_types) > 1:
                derived = parsed_idl["inheritance_data"][base_type]

                for derived_type in derived:
                    if derived_type in parsed_idl["inheritance_data"]:
                        derived.extend(parsed_idl["inheritance_data"][derived_type])

                derived_list = ", ".join(derived)
                return f"Union[{derived_list}, {type_name}]"

        return type_name

    def _transformer(type_input):
        if type_input["union"]:
            contained_types = []
            for contained in type_input["type_name"]:
                contained_types.append(_transformer(contained))
            transformed_type = ",".join(contained_types)
            return f"Union[{transformed_type}]", None
        if type_input["vector"]:
            transformed_type = _transformer(type_input["type_name"][0])
            for attr in type_input["ext_attrs"]:
                if attr["name"] == "Size" and attr["rhs"]["type"] == "integer":
                    size = attr["rhs"]["value"]
                    return f"Annotated[List[{transformed_type[0]}], Len(min_length={size}, max_length={size})]", None
                    return f"std::array<{transformed_type}, {size}>"
            return f"List[{transformed_type[0]}]", None
        if type_input["map"]:
            transformed_key_type = _transformer(type_input["type_name"][0])[0]
            transformed_value_type = _transformer(type_input["type_name"][1])[0]
            return f"Dict[{transformed_key_type}, {transformed_value_type}]", None
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
            return (
                conversion[type_input["type_name"]]
                if type_input["type_name"] in conversion
                else _polymorphic_transformer(type_input["type_name"]),
                None,
            )

    for struct in parsed_idl["structs"]:
        for member in struct["members"]:
            member["type"] = _transformer(member["type"])[0]

    for type_def in parsed_idl["type_defs"]:
        type_def["type"] = _transformer(type_def["type"])[0]

    return parsed_idl
