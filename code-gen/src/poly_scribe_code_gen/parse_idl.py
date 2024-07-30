# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import re
from pathlib import Path
from typing import Any

from pywebidl2 import parse, validate

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
        if definition["name"] in block_comments_dict:
            definition["block_comment"] = block_comments_dict[definition["name"]]

        if definition["name"] in inline_comments_dict:
            definition["inline_comment"] = inline_comments_dict[definition["name"]]

    return parsed_idl


def _validate_and_parse(idl: str) -> dict[str, Any]:
    errors = validate(idl)

    if errors:
        print(errors)

    parsed_idl = parse(idl)

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
                    attribute_type = member["idl_type"]
                    _recursive_type_check(attribute_type, def_name, cpp_types, enumerations, structs, type_defs)

    parsed_idl = _flatten(parsed_idl)

    parsed_idl = _handle_polymorphism(parsed_idl)

    parsed_idl = _sort_structs(parsed_idl)

    return parsed_idl


def _recursive_type_check(input_data, def_name, cpp_types, enumerations, structs, type_defs):
    if not input_data["generic"] and not input_data["union"]:
        type_data = input_data["idl_type"]
        if (
            type_data not in cpp_types
            and type_data not in enumerations
            and type_data not in structs
            and type_data not in type_defs
        ):
            print(f"Member type '{type_data}' in interface '{def_name}' is not valid.")
    elif not input_data["generic"] and input_data["union"]:
        for type_data in input_data["idl_type"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs, type_defs)
    elif input_data["generic"] and not input_data["union"]:
        for type_data in input_data["idl_type"]:
            _recursive_type_check(type_data, def_name, cpp_types, enumerations, structs, type_defs)
    else:
        msg = "Unrecognised WebIDL type structure."
        raise RuntimeError(msg)


def _get_comments(idl: str) -> tuple[dict[str, Any], dict[str, Any]]:
    """Extract comment lines from the given WebIDL string.

    Parameters
    ----------
    idl : str
        String containing the WebIDL.
    """
    block_comments_dict = {}
    inline_comments_dict = {}

    block_comment_pattern = r"((?:^[^\S\n]*(?:///|/\*\*|/\*\!|//\!).*\n[^\S\n]*)+)\S+[^\S\n](\w+)(?=\s*[:;\n])"
    for m in re.finditer(block_comment_pattern, idl, re.MULTILINE):
        block_comments_dict[m.group(2)] = re.sub(r"(?:[ \t]+)(///|/\*\*|/\*\!|//\!)", r"\1", m.group(1)).strip()

    inline_comment_pattern = r"(?:attribute)?.*?(\w+)(?:\s*=.*)?;\s*((?:///<|/\*\!<|/\*\*<|//\!<).*?)\n"
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
                    "ext_attrs": definition["ext_attrs"],
                }
            )
        if definition["type"] == "enum":
            output["enums"].append(
                {
                    "name": definition["name"],
                    "vals": [val["value"] for val in definition["values"]],
                    "ext_attrs": definition["ext_attrs"],
                }
            )
        if definition["type"] == "typedef":
            output["type_defs"].append(
                {
                    "name": definition["name"],
                    "type": _flatten_type(definition["idl_type"]),
                    "ext_attrs": definition["ext_attrs"],
                }
            )
    return output


def _flatten_members(members):
    output = []
    for member in members:
        if member["type"] == "field":
            output.append(
                {
                    "name": member["name"],
                    "ext_attrs": member["ext_attrs"],
                    "type": _flatten_type(member["idl_type"]),
                    "required": True if member["required"] == "true" else False,
                    "default": member["default"]["value"] if member["default"] and member["default"]["value"] else None,
                }
            )
            output[-1]["type"]["ext_attrs"] = output[-1]["type"]["ext_attrs"] + member["ext_attrs"]

    return output


def _flatten_type(input_type):
    output = {}
    if not input_type["generic"] and not input_type["union"]:
        output = {
            "type_name": input_type["idl_type"],
            "vector": False,
            "union": False,
            "map": False,
            "ext_attrs": input_type["ext_attrs"],
        }
    elif not input_type["generic"] and input_type["union"]:
        output = {
            "type_name": [_flatten_type(x) for x in input_type["idl_type"]],
            "vector": False,
            "union": True,
            "map": False,
            "ext_attrs": input_type["ext_attrs"],
        }
    elif input_type["generic"] and not input_type["union"]:
        if input_type["generic"] == "ObservableArray" or input_type["generic"] == "sequence":
            output = {
                "type_name": [_flatten_type(x) for x in input_type["idl_type"]],
                "vector": True,
                "union": False,
                "map": False,
                "ext_attrs": input_type["ext_attrs"],
            }
        if input_type["generic"] == "record":
            output = {
                "type_name": [_flatten_type(x) for x in input_type["idl_type"]],
                "vector": False,
                "union": False,
                "map": True,
                "ext_attrs": input_type["ext_attrs"],
            }
    else:
        msg = "Unrecognised WebIDL type structure."
        raise RuntimeError(msg)

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


def _sort_structs(input_idl):
    struct_names = [s["name"] for s in input_idl["structs"]]

    def _custom_type_search(type_input):
        if type_input["union"]:
            contained_types = []
            for contained in type_input["type_name"]:
                custom_type = _custom_type_search(contained)
                contained_types.extend(custom_type)
            return list(set(contained_types))
        if type_input["vector"]:
            return _custom_type_search(type_input["type_name"][0])
        if type_input["map"]:
            custom_type_key = _custom_type_search(type_input["type_name"][0])
            custom_type_value = _custom_type_search(type_input["type_name"][1])
            return list(set(custom_type_key + custom_type_value))
        else:
            return [type_input["type_name"]] if type_input["type_name"] in struct_names else []

    usage_data = {}
    for struct in input_idl["structs"]:
        uses = []
        for member in struct["members"]:
            uses.extend(_custom_type_search(member["type"]))

        if uses:
            usage_data[struct["name"]] = uses

    input_idl["usage_data"] = usage_data

    inheritance_data = input_idl["inheritance_data"]
    usage_data = input_idl["usage_data"]

    ordered_structs = []

    for base, uses in usage_data.items():
        if base in ordered_structs:
            for use in uses:
                if use in ordered_structs:
                    raise NotImplementedError
                ordered_structs.insert(ordered_structs.index(base), use)
        else:
            for use in uses:
                if use not in ordered_structs:
                    ordered_structs.append(use)

            ordered_structs.append(base)

    for base, derived in inheritance_data.items():
        if base not in ordered_structs and not all(d in ordered_structs for d in derived):
            ordered_structs.append(base)
            ordered_structs.extend(derived)
        if base not in ordered_structs and any(d in ordered_structs for d in derived):
            d_idx = min(ordered_structs.index(d) for d in derived if d in ordered_structs)

            ordered_structs.insert(d_idx, base)

            for d in derived:
                if d not in ordered_structs:
                    ordered_structs.insert(d_idx + 1, d)
        else:
            base_idx = ordered_structs.index(base)

            for d in derived:
                if d in ordered_structs:
                    d_idx = ordered_structs.index(d)
                    if d_idx < base_idx:
                        ordered_structs[d_idx], ordered_structs[base_idx] = (
                            ordered_structs[base_idx],
                            ordered_structs[d_idx],
                        )  # This might very well be wrong!
                else:
                    ordered_structs.insert(base_idx + 1, d)

    def _sort_like_list(obj):
        try:
            return ordered_structs.index(obj["name"])
        except ValueError:
            return len(ordered_structs)

    input_idl["structs"] = sorted(input_idl["structs"], key=_sort_like_list)

    return input_idl
