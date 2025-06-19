# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

from pathlib import Path
from typing import Any

import black
import isort
import jinja2
from docstring_parser import DocstringStyle, compose

from poly_scribe_code_gen._types import AdditionalData, ParsedIDL


def generate_python_package(parsed_idl: ParsedIDL, additional_data: AdditionalData, out_dir: Path) -> None:
    if out_dir.is_file():
        msg = f"Output directory {out_dir} is not a directory"
        raise ValueError(msg)

    if additional_data["package"] is None:
        msg = "Package name is not set"
        raise ValueError(msg)

    out_dir.mkdir(parents=True, exist_ok=True)

    source_dir = out_dir / "src" / out_dir.name
    source_dir.mkdir(parents=True, exist_ok=True)

    generate_python(parsed_idl, additional_data, source_dir / "__init__.py")

    project_res = _render_pyproject_toml(additional_data)

    pyproject_toml = out_dir / "pyproject.toml"

    with open(pyproject_toml, "w") as f:
        f.write(project_res)


def generate_python(parsed_idl: ParsedIDL, additional_data: AdditionalData, out_file: Path) -> None:
    res = _render_template(parsed_idl, additional_data)

    out_file.parent.mkdir(parents=True, exist_ok=True)

    with open(out_file, "w") as f:
        f.write(res)

    black.format_file_in_place(out_file, write_back=black.WriteBack.YES, fast=True, mode=black.FileMode())

    isort.file(out_file)


def _render_template(parsed_idl: ParsedIDL, additional_data: AdditionalData) -> str:
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

    parsed_idl = _transform_comments(parsed_idl)

    data = {**additional_data, **parsed_idl}

    return j2_template.render(data)


def _render_pyproject_toml(additional_data: AdditionalData) -> str:
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

    j2_template = env.get_template("pyproject.jinja")

    return j2_template.render(additional_data)


def _transform_types(parsed_idl: ParsedIDL) -> ParsedIDL:
    for struct_name, struct_data in parsed_idl["structs"].items():
        for member_data in struct_data["members"].values():
            member_data["type"] = _transformer(member_data["type"], parsed_idl["inheritance_data"])
            if not member_data["required"]:
                member_data["type"] = f"Optional[{member_data['type']}]"

                if "str" in member_data["type"] and member_data["default"]:
                    member_data["default"] = f'"{member_data["default"]}"'

                if member_data["default"] is None:
                    member_data["default"] = "None"

        # Check if a member named "type" is already present in the struct and raise an error if so
        if any(member == "type" for member in struct_data["members"]):
            msg = f"Struct {struct_name} already has a member named 'type'"
            raise ValueError(msg)

        for derived_types in parsed_idl["inheritance_data"].values():
            if struct_name in derived_types and not any(member == "type" for member in struct_data["members"]):
                struct_data["members"]["type"] = {
                    "type": f'Literal["{struct_name}"]',
                    "default": f'"{struct_name}"',
                }

        if struct_name in parsed_idl["inheritance_data"] and not any(
            member == "type" for member in struct_data["members"]
        ):
            struct_data["members"]["type"] = {
                "type": f'Literal["{struct_name}"]',
                "default": f'"{struct_name}"',
            }

    for type_def in parsed_idl["typedefs"].values():
        type_def["type"] = _transformer(type_def["type"], parsed_idl["inheritance_data"])

    return parsed_idl


def _transformer(type_input: dict[str, Any], inheritance_data: dict[str, list[str]]) -> str:
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
            return (
                f"Annotated[List[{transformed_type}], Len("
                f"min_length={type_input['size']}, "
                f"max_length={type_input['size']})]"
            )

        return f"List[{transformed_type}]"
    if type_input["map"]:
        key_type = _transformer(type_input["type_name"]["key"], inheritance_data)
        # TODO: here we can check if the type changed??

        value_type = _transformer(type_input["type_name"]["value"], inheritance_data)
        return f"Dict[{key_type}, {value_type}]"

    msg = f"Unknown type: {type_input}"
    raise ValueError(msg)


def _get_polymorphic_type(type_input: str, inheritance_data: dict[str, list[str]]) -> str:
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


def _transform_comments(parsed_idl: ParsedIDL) -> ParsedIDL:
    for struct_data in parsed_idl["structs"].values():
        if "block_comment" in struct_data:
            struct_data["block_comment"] = compose(struct_data["block_comment"], style=DocstringStyle.GOOGLE)

        if "inline_comment" in struct_data:
            struct_data["block_comment"] = (
                struct_data.get("block_comment", "")
                + "\n\n"
                + compose(struct_data["inline_comment"], style=DocstringStyle.GOOGLE)
            )

        if "block_comment" in struct_data:
            struct_data["block_comment"] = struct_data["block_comment"].strip()

        for member_data in struct_data["members"].values():
            if "block_comment" in member_data:
                member_data["block_comment"] = compose(member_data["block_comment"], style=DocstringStyle.GOOGLE)

            if "inline_comment" in member_data:
                member_data["block_comment"] = (
                    member_data.get("block_comment", "")
                    + "\n\n"
                    + compose(member_data["inline_comment"], style=DocstringStyle.GOOGLE)
                )

            if "block_comment" in member_data:
                member_data["block_comment"] = member_data["block_comment"].strip()

    for type_def in parsed_idl["typedefs"].values():
        if "block_comment" in type_def:
            type_def["block_comment"] = compose(type_def["block_comment"], style=DocstringStyle.GOOGLE)

        if "inline_comment" in type_def:
            type_def["block_comment"] = (
                type_def.get("block_comment", "")
                + "\n\n"
                + compose(type_def["inline_comment"], style=DocstringStyle.GOOGLE)
            )

        if "block_comment" in type_def:
            type_def["block_comment"] = type_def["block_comment"].strip()

    for enum_data in parsed_idl["enums"].values():
        if "block_comment" in enum_data:
            enum_data["block_comment"] = compose(enum_data["block_comment"], style=DocstringStyle.GOOGLE)

        if "inline_comment" in enum_data:
            enum_data["block_comment"] = (
                enum_data.get("block_comment", "")
                + "\n\n"
                + compose(enum_data["inline_comment"], style=DocstringStyle.GOOGLE)
            )

        if "block_comment" in enum_data:
            enum_data["block_comment"] = enum_data["block_comment"].strip()

        for enum_value in enum_data["values"]:
            if "block_comment" in enum_value:
                enum_value["block_comment"] = compose(enum_value["block_comment"], style=DocstringStyle.GOOGLE)

            if "inline_comment" in enum_value:
                enum_value["block_comment"] = (
                    enum_value.get("block_comment", "")
                    + "\n\n"
                    + compose(enum_value["inline_comment"], style=DocstringStyle.GOOGLE)
                )

            if "block_comment" in enum_value:
                enum_value["block_comment"] = enum_value["block_comment"].strip()

    return parsed_idl
