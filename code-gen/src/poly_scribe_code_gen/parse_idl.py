# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

"""Parse WebIDL files and extract relevant information.

This module provides functionality to parse WebIDL files, validate them,
and extract information about typedefs, enums, and structs.

It also handles polymorphism and adds comments to the parsed data.
"""

from __future__ import annotations

import re
from typing import TYPE_CHECKING, Any

from docstring_parser import Docstring
from docstring_parser import parse as parse_docstring
from pywebidl2 import parse, validate

from poly_scribe_code_gen._types import ParsedIDL, cpp_types

if TYPE_CHECKING:
    from pathlib import Path

type_transformer = {
    "boolean": "bool",
    "byte": "char",
    "ByteString": "string",
}
"""Mapping of WebIDL types to internal representations."""


def parse_idl(idl_file: Path) -> ParsedIDL:
    """Parse the given WebIDL file.

    This function reads a WebIDL file, validates its content, and extracts
    typedefs, enums, structs, and inheritance data. It also handles polymorphism
    and adds comments to the parsed data.
    It returns a dictionary containing the parsed IDL data.

    Args:
        idl_file: Path to the WebIDL file to parse.

    Returns:
        A dictionary containing parsed IDL data, including typedefs, enums, structs, and inheritance data.
    """

    with open(idl_file) as f:
        idl = f.read()

    return _validate_and_parse(idl)


def _validate_and_parse(idl: str) -> ParsedIDL:
    if not idl:
        return {"typedefs": {}, "enums": {}, "structs": {}, "inheritance_data": {}}

    errors = validate(idl)

    if errors:
        msg = "WebIDL validation errors:\n{}".format("\n".join(errors.__repr__()))
        raise RuntimeError(msg)

    parsed_idl_raw = parse(idl)

    typedefs, enums, dictionaries = _flatten(parsed_idl_raw)

    parsed_idl: ParsedIDL = {"typedefs": {}, "enums": {}, "structs": {}, "inheritance_data": {}}

    for name, definition in typedefs.items():
        parsed_idl["typedefs"][name] = {
            "type": _flatten_type(definition["idl_type"], parent_ext_attrs=definition["ext_attrs"])
        }

    for name, definition in enums.items():
        parsed_idl["enums"][name] = {"values": _flatten_enums(definition)}

    for name, definition in dictionaries.items():
        parsed_idl["structs"][name] = _flatten_dictionaries(definition)

    _type_check(parsed_idl, cpp_types)

    parsed_idl = _handle_polymorphism(parsed_idl)

    return _add_comments(idl, parsed_idl)


def _type_check(parsed_idl: ParsedIDL, types_cpp: list[str]) -> None:
    struct_names = list(parsed_idl["structs"].keys())
    enum_names = list(parsed_idl["enums"].keys())
    typedef_names = list(parsed_idl["typedefs"].keys())

    for typedef_name, typedef_data in parsed_idl["typedefs"].items():
        type_data = typedef_data["type"]
        _type_check_impl(type_data, typedef_name, types_cpp, enum_names, struct_names, typedef_names)

    for struct_name, struct_data in parsed_idl["structs"].items():
        for member_name, member_data in struct_data["members"].items():
            _type_check_impl(
                member_data["type"], f"{struct_name}.{member_name}", types_cpp, enum_names, struct_names, typedef_names
            )


def _type_check_impl(
    type_data: dict[str, Any],
    def_name: str,
    types_cpp: list[str],
    enumerations: list[str],
    structs: list[str],
    type_defs: list[str],
) -> None:
    def _check_type(type_name: str, context: str) -> None:
        if (
            type_name not in types_cpp
            and type_name not in enumerations
            and type_name not in structs
            and type_name not in type_defs
        ):
            msg = f"Member type '{type_name}' in {context} is not valid."
            raise RuntimeError(msg)

    if isinstance(type_data, str):
        _check_type(type_data, f"'{def_name}'")
        return

    if type_data["union"]:
        for contained_type in type_data["type_name"]:
            _check_type(contained_type, f"union '{def_name}'")
    elif type_data["vector"]:
        _check_type(type_data["type_name"], f"vector '{def_name}'")
    elif type_data["map"]:
        _check_type(type_data["type_name"]["value"], f"map '{def_name}'")
    else:
        _check_type(type_data["type_name"], f"'{def_name}'")


def _add_comments(idl: str, parsed_idl: ParsedIDL) -> ParsedIDL:
    comment_data = _find_comments(idl)

    for struct_name, struct_data in parsed_idl["structs"].items():
        # check if struct_name is in the key of any of the block comments
        block_comment = next(
            (comment for key, comment in comment_data["block_comments"].items() if struct_name in key), None
        )
        if block_comment:
            struct_data["block_comment"] = _parse_comments(block_comment)

        inline_comment = next(
            (comment for key, comment in comment_data["inline_comments"].items() if struct_name in key), None
        )
        if inline_comment:
            struct_data["inline_comment"] = _parse_comments(inline_comment)

        for member_name, member_data in struct_data["members"].items():
            # check if member_name is in the key of any of the block comments
            block_comment = next(
                (comment for key, comment in comment_data["block_comments"].items() if member_name in key), None
            )
            if block_comment:
                member_data["block_comment"] = _parse_comments(block_comment)

            inline_comment = next(
                (comment for key, comment in comment_data["inline_comments"].items() if member_name in key), None
            )
            if inline_comment:
                member_data["inline_comment"] = _parse_comments(inline_comment)

    for enum_name, enum_data in parsed_idl["enums"].items():
        # check if enum_name is in the key of any of the block comments
        block_comment = next(
            (comment for key, comment in comment_data["block_comments"].items() if enum_name in key), None
        )
        if block_comment:
            enum_data["block_comment"] = _parse_comments(block_comment)

        inline_comment = next(
            (comment for key, comment in comment_data["inline_comments"].items() if enum_name in key), None
        )
        if inline_comment:
            enum_data["inline_comment"] = _parse_comments(inline_comment)

        for enum_value in enum_data["values"]:
            # check if enum_value is in the key of any of the block comments
            block_comment = next(
                (comment for key, comment in comment_data["block_comments"].items() if enum_value["name"] in key),
                None,
            )
            if block_comment:
                enum_value["block_comment"] = _parse_comments(block_comment)

            inline_comment = next(
                (comment for key, comment in comment_data["inline_comments"].items() if enum_value["name"] in key),
                None,
            )
            if inline_comment:
                enum_value["inline_comment"] = _parse_comments(inline_comment)

    for typedef_name, typedef_data in parsed_idl["typedefs"].items():
        # check if typedef_name is in the key of any of the block comments
        block_comment = next(
            (comment for key, comment in comment_data["block_comments"].items() if typedef_name in key), None
        )
        if block_comment:
            typedef_data["block_comment"] = _parse_comments(block_comment)

        inline_comment = next(
            (comment for key, comment in comment_data["inline_comments"].items() if typedef_name in key), None
        )
        if inline_comment:
            typedef_data["inline_comment"] = _parse_comments(inline_comment)

    return parsed_idl


def _parse_comments(comment: str) -> Docstring:
    # strip leading whitespace in each line of the comment.
    # also strip the leading comment characters.
    comment = re.sub(r"^\s*(?:[/*][/!\*<]*) ?", "", comment, flags=re.MULTILINE)
    # strip trailing whitespace and comment characters.
    comment = re.sub(r"\s*(?:\*|//[/!]*)\s*$", "", comment, flags=re.MULTILINE)

    return parse_docstring(comment.strip())


def _flatten_members(members: list[dict[str, Any]]) -> dict[str, Any]:
    output = {}
    for member in members:
        if member["type"] == "field":
            # Check if member["default"]["value"] is an empty dict
            if (
                member["default"]
                and isinstance(member["default"]["value"], dict)
                and member["default"]["value"] is not None
            ):
                default_value = "{}"
            elif member["default"] and member["default"]["value"] is not None:
                default_value = member["default"]["value"]
            else:
                default_value = None

            output[member["name"]] = {
                "type": _flatten_type(member["idl_type"], parent_ext_attrs=member["ext_attrs"]),  # type: ignore
                "required": bool(member["required"]),
                "default": default_value,
            }
        else:
            msg = f"Unsupported WebIDL type '{member['type']}'."
            raise RuntimeError(msg)

    return output


def _flatten_type(
    input_type: dict[str, Any], *, parent_ext_attrs: list[dict[str, Any]] | None = None
) -> dict[str, Any]:
    if parent_ext_attrs is None:
        parent_ext_attrs = []
    output = {}
    size = None
    if not input_type["generic"] and not input_type["union"]:
        output = type_transformer.get(input_type["idl_type"], input_type["idl_type"])
    elif not input_type["generic"] and input_type["union"]:
        output = {
            "type_name": [_flatten_type(x) for x in input_type["idl_type"]],
            "vector": False,
            "union": True,
            "map": False,
            "ext_attrs": input_type["ext_attrs"],
            "size": size,
        }
    elif input_type["generic"] and not input_type["union"]:
        if input_type["generic"] == "ObservableArray" or input_type["generic"] == "sequence":
            ext_attrs = parent_ext_attrs + input_type["ext_attrs"]
            if any(attr["name"] == "Size" for attr in ext_attrs):
                size_ext_attr = next(attr for attr in ext_attrs if attr["name"] == "Size")
                if size_ext_attr["rhs"]["type"] != "integer":
                    msg = "Size attribute must be of type integer."
                    raise RuntimeError(msg)
                size = int(size_ext_attr["rhs"]["value"])

                input_type["ext_attrs"] = [attr for attr in ext_attrs if attr["name"] != "Size"]

            if len(input_type["idl_type"]) != 1:
                msg = "Sequence must have one element."
                raise RuntimeError(msg)

            output = {
                "type_name": _flatten_type(input_type["idl_type"][0]),
                "vector": True,
                "union": False,
                "map": False,
                "ext_attrs": input_type["ext_attrs"],
                "size": size,
            }
        if input_type["generic"] == "record":
            if len(input_type["idl_type"]) != 2:  # noqa: PLR2004
                msg = "Record must have two elements."
                raise RuntimeError(msg)

            output = {
                "type_name": {
                    "key": _flatten_type(input_type["idl_type"][0]),
                    "value": _flatten_type(input_type["idl_type"][1]),
                },
                "vector": False,
                "union": False,
                "map": True,
                "ext_attrs": input_type["ext_attrs"],
                "size": size,
            }
    else:
        msg = "Unrecognised WebIDL type structure."
        raise RuntimeError(msg)

    return output


def _handle_polymorphism(input_idl: ParsedIDL) -> ParsedIDL:
    def replace_type(type_data: dict[str, Any] | str, derived_type: str, base: str) -> dict[str, Any] | str:
        if isinstance(type_data, str):
            return base if type_data == derived_type else type_data

        if type_data.get("type_name") == derived_type:
            type_data["type_name"] = base
        elif isinstance(type_data.get("type_name"), list):
            type_data["type_name"] = [replace_type(t, derived_type, base) for t in type_data["type_name"]]
        elif isinstance(type_data.get("type_name"), dict):
            type_data["type_name"]["key"] = replace_type(type_data["type_name"]["key"], derived_type, base)
            type_data["type_name"]["value"] = replace_type(type_data["type_name"]["value"], derived_type, base)

        return type_data

    structures = input_idl["structs"]

    inheritance_data: dict[str, list[str]] = {}

    for name, data in structures.items():
        if inherits_from := data["inheritance"]:
            if inherits_from not in inheritance_data:
                inheritance_data[inherits_from] = []

            inheritance_data[inherits_from].append(name)

    input_idl["inheritance_data"] = inheritance_data

    for base, derived in inheritance_data.items():
        for derived_type in derived:
            for struct in structures.values():
                for member in struct["members"].values():
                    member["type"] = replace_type(member["type"], derived_type, base)

            for typedef in input_idl["typedefs"].values():
                typedef["type"] = replace_type(typedef["type"], derived_type, base)

    return input_idl


def _flatten(input_idl: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    typedefs = {}
    enums = {}
    dictionaries = {}

    for definition in input_idl["definitions"]:
        if definition["type"] == "dictionary":
            dictionaries[definition["name"]] = definition
        elif definition["type"] == "enum":
            enums[definition["name"]] = definition
        elif definition["type"] == "typedef":
            typedefs[definition["name"]] = definition
        else:
            definition_type = definition["type"]
            msg = f"Unsupported WebIDL type '{definition_type}'."
            raise RuntimeError(msg)

    return typedefs, enums, dictionaries


def _flatten_enums(definition: dict[str, Any]) -> list[dict[str, Any]]:
    enum_values = []
    for val in definition["values"]:
        if val["type"] == "enum-value":
            enum_values.append({"name": val["value"]})
        else:
            msg = f"Unsupported WebIDL type '{val['type']}' in enum."
            raise RuntimeError(msg)

    return enum_values


def _flatten_dictionaries(definition: dict[str, Any]) -> dict[str, Any]:
    dictionary_definition = {}
    dictionary_definition["members"] = _flatten_members(definition["members"])

    dictionary_definition["inheritance"] = definition["inheritance"]

    if definition["partial"]:
        msg = "Partial dictionaries are not supported."
        raise RuntimeError(msg)

    if definition["ext_attrs"]:
        msg = "Dictionary ext_attrs are not supported."
        raise RuntimeError(msg)

    return dictionary_definition


def _find_comments(idl: str) -> dict[str, dict[tuple[str, ...], str]]:
    block_comment_indicators = ["///", "//!"]
    multi_line_block_comment_indicators = ["/**", "/*!"]
    multi_line_block_comment_end_indicators = ["*/"]
    inline_comment_indicators = ["///<", "//!<", "/**<", "/*!<"]

    typedef_pattern = (
        r"""\btypedef\s+(?:\[\s*[^\]]*\s*\]\s*)?(?:[\w]+(?:<[^<>]*?(?:<[^<>]*?>)?[^<>]*?>)?)\s+([a-zA-Z_]\w*)\s*;"""
    )
    dictionary_pattern = r"""\bdictionary\s+([a-zA-Z_]\w*)(?:\s*:\s*[a-zA-Z_]\w*)?\s*"""
    member_pattern = r"""(?:\[\s*[^\]]*\s*\]\s*)?(?:required\s+)?(?:[\w]+(?:<[^<>]*?(?:<[^<>]*?>)?[^<>]*?>)?)\s+([a-zA-Z_]\w*)\s*(?:=|;)"""
    enum_pattern = r"""\benum\s+([a-zA-Z_]\w*)"""
    enum_value_pattern = r"""\"([^"]+)\""""

    combined_pattern = f"{typedef_pattern}|{dictionary_pattern}|{member_pattern}|{enum_pattern}|{enum_value_pattern}"

    identifier_regex = re.compile(combined_pattern)

    block_comment_data = {}
    inline_comment_data = {}
    tmp_block_comment = ""
    in_block_comment = False
    in_multi_line_block_comment = False
    multi_line_block_comment_end = False
    for idl_line in idl.splitlines():
        idl_line_strip = idl_line.strip()
        if any(indicator in idl_line for indicator in inline_comment_indicators):
            split_line = idl_line.split(
                next(indicator for indicator in inline_comment_indicators if indicator in idl_line), 1
            )
            split_line[1] = idl_line[len(split_line[0]) :].strip()

            key = identifier_regex.findall(split_line[0].strip())
            key_flat = tuple(item for sublist in key for item in sublist if item)
            inline_comment_data[tuple(key_flat)] = split_line[1].strip()

        if any(idl_line_strip.startswith(indicator) for indicator in block_comment_indicators):
            tmp_block_comment += idl_line_strip + "\n"
            in_block_comment = True
        elif any(idl_line_strip.startswith(indicator) for indicator in multi_line_block_comment_indicators):
            tmp_block_comment += idl_line_strip + "\n"
            in_multi_line_block_comment = True
        elif (
            any(idl_line_strip.startswith(indicator) for indicator in multi_line_block_comment_end_indicators)
            and in_multi_line_block_comment
        ):
            tmp_block_comment += idl_line_strip + "\n"
            multi_line_block_comment_end = True
            in_multi_line_block_comment = False
        elif in_multi_line_block_comment:
            tmp_block_comment += idl_line_strip + "\n"
        elif in_block_comment or multi_line_block_comment_end:
            key = identifier_regex.findall(idl_line_strip)
            key_flat = tuple(item for sublist in key for item in sublist if item)
            block_comment_data[tuple(key_flat)] = tmp_block_comment.strip()
            # reset the block comment data
            in_block_comment = False
            in_multi_line_block_comment = False
            multi_line_block_comment_end = False
            tmp_block_comment = ""
        else:
            # this line is not a comment, so we can ignore it.
            pass

    return {"block_comments": block_comment_data, "inline_comments": inline_comment_data}
