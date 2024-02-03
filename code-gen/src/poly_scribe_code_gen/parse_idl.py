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

    for definition in parsed_idl["definitions"]:
        if definition["type"] == "interface":
            if definition["name"] in block_comments_dict:
                definition["block_comment"] = block_comments_dict[definition["name"]]

            if definition["name"] in inline_comments_dict:
                definition["inline_comment"] = inline_comments_dict[definition["name"]]

            for member in definition["members"]:
                if member["type"] == "attribute" and member["name"] in block_comments_dict:
                    member["block_comment"] = block_comments_dict[member["name"]]

                if member["type"] == "attribute" and member["name"] in inline_comments_dict:
                    member["inline_comment"] = inline_comments_dict[member["name"]]

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
    for definition in parsed_idl["definitions"]:
        if definition["type"] == "enum":
            enumerations.append(definition["name"])
        if definition["type"] == "interface":
            structs.append(definition["name"])

    for definition in parsed_idl["definitions"]:
        if definition["type"] == "interface":
            def_name = definition["name"]
            if definition["inheritance"] and definition["inheritance"] not in structs:
                base_name = definition["inheritance"]
                print(f"Base of '{def_name}' cannot be found, base name is '{base_name}'.")

            for member in definition["members"]:
                if member["type"] == "attribute":
                    attribute_type = member["idlType"]
                    _recursive_type_check(attribute_type, def_name, cpp_types, enumerations, structs)

    return parsed_idl


def _recursive_type_check(input_data, def_name, cpp_types, enumerations, structs):
    if not input_data["generic"] and not input_data["union"]:
        type_data = input_data["idlType"]
        if type_data not in cpp_types and type_data not in enumerations and type_data not in structs:
            print(f"Member type '{type_data}' in interface '{def_name}' is not valid.")
    elif not input_data["generic"] and input_data["union"]:
        for type_data in input_data["idlType"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs)
    elif input_data["generic"] and not input_data["union"]:
        for type_data in input_data["idlType"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs)
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

    block_comment_pattern = r"((?:(?:///|/\*\*|/\*\!|//\!).*?\n.*)+).*(?:attribute\s+\w+|interface)\s+(\w+)"
    for m in re.finditer(block_comment_pattern, idl):
        block_comments_dict[m.group(2)] = re.sub(r"(?:[ \t]+)(///|/\*\*|/\*\!|//\!)", r"\1", m.group(1)).strip()

    inline_comment_pattern = r"attribute.*?(\w+);\s*((?:///<|/\*\!<|/\*\*<|//\!<).*?)\n"
    for m in re.finditer(inline_comment_pattern, idl):
        inline_comments_dict[m.group(1)] = m.group(2)

    return block_comments_dict, inline_comments_dict
