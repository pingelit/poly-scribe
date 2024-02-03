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
        if definition["type"] == "interface" and definition["name"] in block_comments_dict:
            definition["block_comment"] = block_comments_dict[definition["name"]]

        if definition["type"] == "interface" and definition["name"] in inline_comments_dict:
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
            # todo check if base exists if given

            for member in definition["members"]:
                if member["type"] == "attribute":
                    attribute_type = member["idl_type"]["idl_type"]
                    if (
                        attribute_type not in cpp_types
                        and attribute_type not in enumerations
                        and attribute_type not in structs
                    ):
                        def_name = definition["name"]
                        print(f"Member type '{attribute_type}' in interface '{def_name}' is not valid.")
                        # print(json.dumps(member, indent=4))

    return parsed_idl


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
