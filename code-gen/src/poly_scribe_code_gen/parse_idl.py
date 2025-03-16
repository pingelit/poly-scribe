# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import re
from pathlib import Path
from typing import Any

from pywebidl2 import parse, validate

from poly_scribe_code_gen._types import cpp_types

type_transformer = {
    "boolean": "bool",
    "byte": "char",
    "ByteString": "string",
}


def parse_idl(idl_file: Path) -> dict[str, Any]:
    """Parse the given WebIDL file.

    Parameters
    ----------
    idl_file : Path
        File path to the WebIDL file
    """

    with open(idl_file) as f:
        idl = f.read()

    return _validate_and_parse(idl)


def _validate_and_parse(idl: str) -> dict[str, Any]:
    if not idl:
        return {"typedefs": {}, "enums": {}, "structs": {}}

    errors = validate(idl)

    if errors:
        msg = "WebIDL validation errors:\n{}".format("\n".join(errors.__repr__()))
        raise RuntimeError(msg)

    parsed_idl = parse(idl)

    typedefs, enums, dictionaries = _flatten(parsed_idl)

    parsed_idl = {}

    parsed_idl["typedefs"] = {}
    for name, definition in typedefs.items():
        parsed_idl["typedefs"][name] = {
            "type": _flatten_type(definition["idl_type"], parent_ext_attrs=definition["ext_attrs"])
        }

    parsed_idl["enums"] = {}
    for name, definition in enums.items():
        parsed_idl["enums"][name] = {"values": _flatten_enums(definition)}

    parsed_idl["structs"] = {}
    for name, definition in dictionaries.items():
        parsed_idl["structs"][name] = _flatten_dictionaries(definition)

    _type_check(parsed_idl, cpp_types)

    parsed_idl = _handle_polymorphism(parsed_idl)

    return _add_comments(idl, parsed_idl)


def _type_check(parsed_idl, types_cpp) -> None:
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


def _type_check_impl(type_data, def_name, types_cpp, enumerations, structs, type_defs) -> None:
    def _check_type(type_name, context):
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


def _add_comments(idl: str, parsed_idl: dict[str, Any]) -> dict[str, Any]:
    def strip_comments(comment: str) -> str:
        # strip leading whitespace in each line of the comment.
        # also strip the leading comment characters.
        comment = re.sub(r"^\s*(?:[/*][/!\*<]*)[ \t]*", "", comment, flags=re.MULTILINE)
        # strip trailing whitespace and comment characters.
        comment = re.sub(r"\s*(?:\*|//[/!]*)\s*$", "", comment, flags=re.MULTILINE)

        return comment.strip()

    pattern = re.compile(
        r"^\s*((?:///|//!)\s*[^\n]*(?:\n\s*(?:///|//!)\s*[^\n]*)*|/\*\*[\s\S]*?\*/|/\*![\s\S]*?\*/)\s*\n\s*(\S.*)",
        re.MULTILINE,
    )
    capture = pattern.findall(idl)

    for comment, definition in capture:
        comment = strip_comments(comment)  # noqa: PLW2901

        for struct_name, struct_data in parsed_idl["structs"].items():
            if struct_name in definition:
                struct_data["block_comment"] = comment

        for enum_name, enum_data in parsed_idl["enums"].items():
            if enum_name in definition:
                enum_data["block_comment"] = comment

            for enum_value in enum_data["values"]:
                if enum_value["name"] in definition:
                    enum_value["block_comment"] = comment

        for typedef_name, typedef_data in parsed_idl["typedefs"].items():
            if typedef_name in definition:
                typedef_data["block_comment"] = comment

    inline_comment_pattern = re.compile(r"^\s*(.*?)\s*(?:///\s*<|//!\s*<|/\*\s*<|/\**\s*<)\s*(.*)$", re.MULTILINE)

    inline_capture = inline_comment_pattern.findall(idl)

    for definition, comment in inline_capture:
        comment = strip_comments(comment)  # noqa: PLW2901

        for struct_name, struct_data in parsed_idl["structs"].items():
            if struct_name in definition:
                struct_data["inline_comment"] = comment

            for member_name, member_data in struct_data["members"].items():
                if member_name in definition:
                    member_data["inline_comment"] = comment

        for enum_name, enum_data in parsed_idl["enums"].items():
            if enum_name in definition:
                enum_data["inline_comment"] = comment

            for enum_value_data in enum_data["values"]:
                if enum_value_data["name"] in definition:
                    enum_value_data["inline_comment"] = comment

        for typedef_name, typedef_data in parsed_idl["typedefs"].items():
            if typedef_name in definition:
                typedef_data["inline_comment"] = comment

    return parsed_idl


def _flatten_members(members):
    output = {}
    for member in members:
        if member["type"] == "field":
            output[member["name"]] = {
                "type": _flatten_type(member["idl_type"], parent_ext_attrs=member["ext_attrs"]),
                "required": bool(member["required"]),
                "default": member["default"]["value"] if member["default"] and member["default"]["value"] else None,
            }
        else:
            msg = f"Unsupported WebIDL type '{member['type']}'."
            raise RuntimeError(msg)

    return output


def _flatten_type(input_type, *, parent_ext_attrs=None):
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


def _handle_polymorphism(input_idl):
    def replace_type(type_data, derived_type, base):
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

    inheritance_data = {}

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


def _flatten(input_idl):
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


def _flatten_enums(definition):
    enum_values = []
    for val in definition["values"]:
        if val["type"] == "enum-value":
            enum_values.append({"name": val["value"]})
        else:
            msg = f"Unsupported WebIDL type '{val['type']}' in enum."
            raise RuntimeError(msg)

    return enum_values


def _flatten_dictionaries(definition):
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
