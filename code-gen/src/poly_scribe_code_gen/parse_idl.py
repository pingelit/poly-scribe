# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import json
import re
from pathlib import Path
from typing import Any

from javascript import require

from poly_scribe_code_gen.types import cpp_types


def parse_idl(idl_file: Path) -> dict[str, Any]:
    """Parse the given WebIDL file.

    Parameters
    ----------
    idl_file : Path
        File path to the WebIDL file
    """

    with open(idl_file) as f:
        idl = f.read()

    parsed_idl = _validate_and_parse(idl)

    block_comments_dict, inline_comments_dict = _get_comments(idl)

    for definition in parsed_idl["structs"]:
        if definition["name"] in block_comments_dict:
            definition["block_comment"] = block_comments_dict[definition["name"]]

        if definition["name"] in inline_comments_dict:
            definition["inline_comment"] = inline_comments_dict[definition["name"]]

        for member in definition["members"]:
            if member["name"] in block_comments_dict:
                member["block_comment"] = block_comments_dict[member["name"]]

            if member["name"] in inline_comments_dict:
                member["inline_comment"] = inline_comments_dict[member["name"]]

    for definition in parsed_idl["enums"]:
        pass  # todo

    return parsed_idl


def _validate_and_parse(idl: str) -> dict[str, Any]:
    webidl2 = require("webidl2")

    tree = webidl2.parse(idl)
    validation = webidl2.validate(tree)

    for error in validation:
        print(error.message, end="\n-----\n")

    parsed_idl = {"definitions": tree.valueOf()}

    # print(json.dumps(parsed_idl, indent=4))
    enumerations = []
    structs = []
    type_defs = []
    for definition in parsed_idl["definitions"]:
        if definition["type"] == "enum":
            enumerations.append(definition["name"])
        if definition["type"] == "dictionary":
            structs.append(definition["name"])
        if definition["type"] == "typedef":
            type_defs.append(definition["name"])

    for definition in parsed_idl["definitions"]:
        if definition["type"] == "dictionary":
            def_name = definition["name"]
            if definition["inheritance"] and definition["inheritance"] not in structs:
                base_name = definition["inheritance"]
                print(f"Base of '{def_name}' cannot be found, base name is '{base_name}'.")

            for member in definition["members"]:
                if member["type"] == "field":
                    attribute_type = member["idlType"]
                    _recursive_type_check(attribute_type, def_name, cpp_types, enumerations, structs, type_defs)

    parsed_idl = _flatten(parsed_idl)

    parsed_idl = _handle_polymorphism(parsed_idl)

    return parsed_idl


def _recursive_type_check(input_data, def_name, cpp_types, enumerations, structs, type_defs):
    if not input_data["generic"] and not input_data["union"]:
        type_data = input_data["idlType"]
        if (
            type_data not in cpp_types
            and type_data not in enumerations
            and type_data not in structs
            and type_data not in type_defs
        ):
            print(f"Member type '{type_data}' in interface '{def_name}' is not valid.")
    elif not input_data["generic"] and input_data["union"]:
        for type_data in input_data["idlType"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs, type_defs)
    elif input_data["generic"] and not input_data["union"]:
        for type_data in input_data["idlType"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs, type_defs)
    else:
        raise RuntimeError("Unrecognised WebIDL type structure.")


def _get_comments(idl: str) -> tuple[dict[str, Any], dict[str, Any]]:
    """Extract comment lines from the given WebIDL string.

    Parameters
    ----------
    idl : str
        String containing the WebIDL.
    """
    block_comments_dict = {}
    inline_comments_dict = {}

    block_comment_pattern = (
        r"((?:(?:///|/\*\*|/\*\!|//\!).*?\n.*)+).*(?:readonly|\[.*\])?\n?.*(?:attribute\s+\w+|interface|enum)\s+(\w+)"
    )
    for m in re.finditer(block_comment_pattern, idl):
        block_comments_dict[m.group(2)] = re.sub(r"(?:[ \t]+)(///|/\*\*|/\*\!|//\!)", r"\1", m.group(1)).strip()

    inline_comment_pattern = r"attribute.*?(\w+);\s*((?:///<|/\*\!<|/\*\*<|//\!<).*?)\n"
    for m in re.finditer(inline_comment_pattern, idl):
        inline_comments_dict[m.group(1)] = m.group(2)

    return block_comments_dict, inline_comments_dict


def _flatten(parsed_idl):
    output = {"structs": [], "enums": [], "type_defs": []}
    for definition in parsed_idl["definitions"]:
        if definition["type"] == "dictionary":
            output["structs"].append(
                {
                    "name": definition["name"],
                    "inheritance": definition["inheritance"],
                    "members": _flatten_members(definition["members"]),
                    "extAttrs": definition["extAttrs"],
                }
            )
        if definition["type"] == "enum":
            output["enums"].append(
                {
                    "name": definition["name"],
                    "values": [val["value"] for val in definition["values"]],
                    "extAttrs": definition["extAttrs"],
                }
            )
        if definition["type"] == "typedef":
            output["type_defs"].append(
                {
                    "name": definition["name"],
                    "type": _flatten_type(definition["idlType"]),
                    "extAttrs": definition["extAttrs"],
                }
            )
    return output


def _flatten_members(members):
    output = []
    for member in members:
        if member["type"] == "field":
            output.append(
                {"name": member["name"], "extAttrs": member["extAttrs"], "type": _flatten_type(member["idlType"])}
            )

    return output


def _flatten_type(input_type):
    output = {}
    if not input_type["generic"] and not input_type["union"]:
        output = {
            "type_name": input_type["idlType"],
            "vector": False,
            "union": False,
            "map": False,
            "extAttrs": input_type["extAttrs"],
        }
    elif not input_type["generic"] and input_type["union"]:
        output = {
            "type_name": [_flatten_type(x) for x in input_type["idlType"]],
            "vector": False,
            "union": True,
            "map": False,
            "extAttrs": input_type["extAttrs"],
        }
    elif input_type["generic"] and not input_type["union"]:
        if input_type["generic"] == "ObservableArray" or input_type["generic"] == "sequence":
            output = {
                "type_name": [_flatten_type(x) for x in input_type["idlType"]],
                "vector": True,
                "union": False,
                "map": False,
                "extAttrs": input_type["extAttrs"],
            }
        if input_type["generic"] == "record":
            output = {
                "type_name": [_flatten_type(x) for x in input_type["idlType"]],
                "vector": False,
                "union": False,
                "map": True,
                "extAttrs": input_type["extAttrs"],
            }
    else:
        raise RuntimeError("Unrecognised WebIDL type structure.")

    return output


def _handle_polymorphism(input_idl):
    structures = input_idl["structs"]

    inheritance_data = {}

    for struct in structures:
        current_name = struct["name"]
        if inherits_from := struct["inheritance"]:
            if inherits_from not in inheritance_data:
                inheritance_data[inherits_from] = []

            inheritance_data[inherits_from].append(current_name)

    for struct in structures:
        struct["polymorphic_base"] = False
        struct["polymorphic"] = False

        if struct["name"] in inheritance_data and len(inheritance_data[struct["name"]]) > 1:
            struct["polymorphic_base"] = True

        if struct["inheritance"] in inheritance_data and len(inheritance_data[struct["inheritance"]]) > 1:
            struct["polymorphic"] = True

    # todo: handle multiple levels of inheritance?

    input_idl["inheritance_data"] = inheritance_data
    return input_idl
