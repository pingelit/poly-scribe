# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import re
from pathlib import Path

from pywebidl2 import parse


def parse_idl(idl_file: Path):
    """Parse the given WebIDL file.

    Parameters
    ----------
    idl_file : Path
        File path to the WebIDL file
    """

    with open(idl_file) as f:
        idl = f.read()

    parsed_idl = parse(idl)

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


def _get_comments(idl: str):
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
