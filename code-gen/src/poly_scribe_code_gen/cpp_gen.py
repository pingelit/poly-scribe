# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT
from __future__ import annotations

from pathlib import Path
from typing import TYPE_CHECKING, Any

import jinja2

if TYPE_CHECKING:
    from docstring_parser import Docstring

    from poly_scribe_code_gen._types import AdditionalData, ParsedIDL


def generate_cpp(parsed_idl: ParsedIDL, additional_data: AdditionalData, out_file: Path) -> None:
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

    res = _render_template(parsed_idl, additional_data)

    out_file.parent.mkdir(parents=True, exist_ok=True)

    with open(out_file, "w") as f:
        f.write(res)


def _render_template(parsed_idl: ParsedIDL, additional_data: AdditionalData) -> str:
    if not additional_data.get("package"):
        msg = "Missing package name in additional data"
        raise ValueError(msg)

    package_dir = Path(__file__).resolve().parent
    templates_dir = package_dir / "templates"

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        trim_blocks=True,
        lstrip_blocks=True,
        autoescape=jinja2.select_autoescape(
            disabled_extensions=("jinja",),
            default_for_string=True,
            default=False,
        ),
    )

    j2_template = env.get_template("reflect.jinja")

    parsed_idl = _transform_types(parsed_idl)
    parsed_idl = _flatten_struct_inheritance(parsed_idl)
    parsed_idl = _handle_rfl_tagged_union(parsed_idl)
    parsed_idl = _transform_comments(parsed_idl)

    data = {**additional_data, **parsed_idl}

    return j2_template.render(data)


def _transform_types(parsed_idl: ParsedIDL) -> ParsedIDL:
    for struct_data in parsed_idl["structs"].values():
        for member_data in struct_data["members"].values():
            member_data["type"] = _transformer(member_data["type"], parsed_idl["inheritance_data"])
            if not member_data["required"]:
                member_data["type"] = f"std::optional<{member_data['type']}>"

            if "std::string" in member_data["type"] and member_data["default"]:
                member_data["default"] = f'"{member_data["default"]}"'

    for type_def_data in parsed_idl["typedefs"].values():
        type_def_data["type"] = _transformer(type_def_data["type"], parsed_idl["inheritance_data"])

    return parsed_idl


def _transformer(type_input: dict[str, Any], inheritance_data: None | dict[str, list[str]] = None) -> str:
    if isinstance(type_input, str):
        conversion = {"string": "std::string", "ByteString": "std::string"}

        if inheritance_data and type_input in inheritance_data:
            return f"{type_input}_t"

        return conversion.get(type_input, type_input)

    if type_input["vector"]:
        transformed_type = _transformer(type_input["type_name"], inheritance_data)

        if type_input["size"] is not None:
            return f"std::array<{transformed_type}, {type_input['size']}>"

        return f"std::vector<{transformed_type}>"
    if type_input["map"]:
        key_type = _transformer(type_input["type_name"]["key"], inheritance_data)
        value_type = _transformer(type_input["type_name"]["value"], inheritance_data)
        return f"std::unordered_map<{key_type}, {value_type}>"
    if type_input["union"]:
        contained_types = [_transformer(contained, inheritance_data) for contained in type_input["type_name"]]
        transformed_type = ",".join(contained_types)
        return f"std::variant<{transformed_type}>"

    msg = f"Unknown type: {type_input}"
    raise ValueError(msg)


def _flatten_struct_inheritance(parsed_idl: ParsedIDL) -> ParsedIDL:
    inheritance_data = parsed_idl["inheritance_data"]

    sorted_inheritance_data = _sort_inheritance_data(inheritance_data)

    for base_type, derived_types in sorted_inheritance_data:
        base_struct = parsed_idl["structs"][base_type]
        for derived_type in derived_types:
            derived_struct = parsed_idl["structs"][derived_type]

            derived_struct["members"].update(base_struct["members"])

    return parsed_idl


def _sort_inheritance_data(inheritance_data: dict[str, list[str]]) -> list[tuple[str, list[str]]]:
    sorted_inheritance_data: list[tuple[str, list[str]]] = []

    for base_type, derived_types in inheritance_data.items():
        inserted = False
        for i, (sorted_base_type, _) in enumerate(sorted_inheritance_data):
            if sorted_base_type in derived_types:
                sorted_inheritance_data.insert(i, (base_type, derived_types))
                inserted = True
                break
        if not inserted:
            sorted_inheritance_data.append((base_type, derived_types))

    return sorted_inheritance_data


def _handle_rfl_tagged_union(parsed_idl: ParsedIDL) -> ParsedIDL:
    new_inheritance_data = {}
    for key in parsed_idl["inheritance_data"]:
        new_inheritance_data[f"{key}_t"] = parsed_idl["inheritance_data"][key]
        new_inheritance_data[f"{key}_t"].insert(0, key)
    parsed_idl["inheritance_data"] = new_inheritance_data
    return parsed_idl


def _render_doxystring(doc_string: Docstring) -> str:
    doxy_string = "///\n"

    if doc_string.short_description:
        doxy_string += "/// \\brief " + doc_string.short_description + "\n"

    if doc_string.long_description:
        doxy_string += "///\n/// " + doc_string.long_description.replace(r"\n", r"\n/// ") + "\n"

    if doc_string.params:
        for param in doc_string.params:
            if not param.description:
                doxy_string += f"/// \\param {param.arg_name}\n"
            else:
                doxy_string += f"/// \\param {param.arg_name} {param.description.replace(r"\n", r"\n/// ")}" + "\n"

    if doc_string.returns:
        if not doc_string.returns.description:
            doxy_string += "/// \\return None\n"
        else:
            doxy_string += f"/// \\return {doc_string.returns.description.replace(r"\n", r"\n/// ")}\n"

    if doc_string.raises:
        for exception in doc_string.raises:
            if not exception.description:
                doxy_string += f"/// \\throws {exception.type_name}\n"
            else:
                doxy_string += f"/// \\throws {exception.type_name} {exception.description.replace(r"\n", r"\n/// ")}\n"

    doxy_string += "///\n"

    return doxy_string


def _transform_comments(parsed_idl: ParsedIDL) -> ParsedIDL:
    for struct_data in parsed_idl["structs"].values():
        if "block_comment" in struct_data:
            struct_data["block_comment"] = _render_doxystring(struct_data["block_comment"])

        if "inline_comment" in struct_data:
            struct_data["block_comment"] = struct_data.get("block_comment", "") + _render_doxystring(
                struct_data["inline_comment"]
            )

        for member_data in struct_data["members"].values():
            if "block_comment" in member_data:
                member_data["block_comment"] = _render_doxystring(member_data["block_comment"])

            if "inline_comment" in member_data:
                member_data["block_comment"] = member_data.get("block_comment", "") + _render_doxystring(
                    member_data["inline_comment"]
                )

    for type_def in parsed_idl["typedefs"].values():
        if "block_comment" in type_def:
            type_def["block_comment"] = _render_doxystring(type_def["block_comment"])

        if "inline_comment" in type_def:
            type_def["block_comment"] = type_def.get("block_comment", "") + _render_doxystring(
                type_def["inline_comment"]
            )

    for enum_data in parsed_idl["enums"].values():
        if "block_comment" in enum_data:
            enum_data["block_comment"] = _render_doxystring(enum_data["block_comment"])

        if "inline_comment" in enum_data:
            enum_data["block_comment"] = enum_data.get("block_comment", "") + _render_doxystring(
                enum_data["inline_comment"]
            )

        for enum_value in enum_data["values"]:
            if "block_comment" in enum_value:
                enum_value["block_comment"] = _render_doxystring(enum_value["block_comment"])

            if "inline_comment" in enum_value:
                enum_value["block_comment"] = enum_value.get("block_comment", "") + _render_doxystring(
                    enum_value["inline_comment"]
                )

    return parsed_idl
