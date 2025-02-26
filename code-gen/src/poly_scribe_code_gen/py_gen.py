# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import black
import isort
import jinja2

from poly_scribe_code_gen._types import AdditionalData


def generate_python(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_file: Path):
    res = _render_template(parsed_idl, additional_data)

    out_file.parent.mkdir(parents=True, exist_ok=True)

    with open(out_file, "w") as f:
        f.write(res)

    black.format_file_in_place(out_file, write_back=black.WriteBack.YES, fast=True, mode=black.FileMode())

    isort.file(out_file)


def _render_template(parsed_idl, additional_data):
    package_dir = Path(__file__).resolve().parent
    templates_dir = package_dir / "templates"

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

    parsed_idl = _transform_types(parsed_idl)

    data = {**additional_data, **parsed_idl}

    return j2_template.render(data)


def _transform_types(parsed_idl):
    for struct_name, struct_data in parsed_idl["structs"].items():
        for member_data in struct_data["members"].values():
            member_data["type"] = _transformer(member_data["type"], parsed_idl["inheritance_data"])
            if not member_data["required"]:
                member_data["type"] = f"Optional[{member_data['type']}]"

                if member_data["default"] is None:
                    member_data["default"] = "None"

        for derived_types in parsed_idl["inheritance_data"].values():
            if struct_name in derived_types:
                # check if there is no member in struct is already named "type"
                if not any(member == "type" for member in struct_data["members"]):
                    struct_data["members"]["type"] = {
                        "type": f'Literal["{struct_name}"]',
                        "default": f'"{struct_name}"',
                    }
                else:
                    msg = f"Struct {struct_data['name']} already has a member named 'type'"
                    raise ValueError(msg)

        if struct_name in parsed_idl["inheritance_data"]:
            if not any(member == "type" for member in struct_data["members"]):
                struct_data["members"]["type"] = {
                    "type": f'Literal["{struct_name}"]',
                    "default": f'"{struct_name}"',
                }
            else:
                msg = f"Struct {struct_data['name']} already has a member named 'type'"
                raise ValueError(msg)

    for type_def in parsed_idl["typedefs"].values():
        type_def["type"] = _transformer(type_def["type"], parsed_idl["inheritance_data"])

    return parsed_idl


@dataclass
class ExtraData:
    polymorphic: bool = False


def _polymorphic_transformer(type_name, parsed_idl):
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


def _transformer(type_input, inheritance_data):
    if isinstance(type_input, str):
        type_input = _get_polymorphic_type(type_input, inheritance_data)

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

        return conversion.get(type_input, type_input)

    if type_input["union"]:
        contained_types = [_transformer(contained, inheritance_data) for contained in type_input["type_name"]]
        transformed_type = ",".join(contained_types)
        return f"Union[{transformed_type}]"
    if type_input["vector"]:
        transformed_type = _transformer(type_input["type_name"], inheritance_data)

        if type_input["size"] is not None:
            return f"Annotated[List[{transformed_type}], Len(min_length={type_input['size']}, max_length={type_input['size']})]"

        return f"List[{transformed_type}]"
    if type_input["map"]:
        key_type = _transformer(type_input["type_name"]["key"], inheritance_data)
        # TODO: here we can check if the type changed??

        value_type = _transformer(type_input["type_name"]["value"], inheritance_data)
        return f"Dict[{key_type}, {value_type}]"

    msg = f"Unknown type: {type_input}"
    raise ValueError(msg)


def _get_polymorphic_type(type_input: str, inheritance_data) -> str:
    union_content = []

    if type_input in inheritance_data:
        union_content.extend(inheritance_data[type_input])
        union_content.append(type_input)

    for base_type, derived_types in inheritance_data.items():
        if type_input in derived_types:
            union_content.extend(derived_types)
            union_content.append(base_type)

    if union_content:
        return f"Annotated[Union[{', '.join(union_content)}],Field(discriminator=\"type\")]"

    return type_input
