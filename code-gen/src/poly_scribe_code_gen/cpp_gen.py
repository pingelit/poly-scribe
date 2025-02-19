# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

from pathlib import Path
from typing import Any

from poly_scribe_code_gen._types import AdditionalData

import jinja2


def generate_cpp(parsed_idl: dict[str, Any], additional_data: AdditionalData, out_file: Path):
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


def _render_template(parsed_idl, additional_data):

    if not additional_data.get("package"):
        raise ValueError("Missing package name in additional data")

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

    data = {**additional_data, **parsed_idl}

    rendered_result = j2_template.render(data)

    return rendered_result


def _transform_types(parsed_idl):
    # def _polymorphic_transformer(type_name):
    #     if type_name in parsed_idl["inheritance_data"]:
    #         return f"std::shared_ptr<{type_name}>"

    #     for base_type, derived_types in parsed_idl["inheritance_data"].items():
    #         if type_name in derived_types and len(derived_types) > 1:
    #             return f"std::shared_ptr<{base_type}>"

    #     return type_name

    for struct_name, struct_data in parsed_idl["structs"].items():
        for member_name, member_data in struct_data["members"].items():
            map_or_vector = isinstance(member_data["type"], dict) and (
                member_data["type"]["map"] or (member_data["type"]["vector"] and not member_data["type"]["size"])
            )
            member_data["type"] = _transformer(member_data["type"], parsed_idl["inheritance_data"])
            if not member_data["required"] and not map_or_vector:
                member_data["type"] = f"std::optional<{member_data['type']}>"

    for type_def_data in parsed_idl["typedefs"].values():
        type_def_data["type"] = _transformer(type_def_data["type"], parsed_idl["inheritance_data"])

    return parsed_idl


def _transformer(type_input, inheritance_data=None):
    if isinstance(type_input, str):
        conversion = {"string": "std::string", "ByteString": "std::string"}

        if inheritance_data and type_input in inheritance_data and len(inheritance_data[type_input]) > 1:
            return f"{type_input}_t"

        return conversion.get(type_input, type_input)
        return (
            conversion[type_input]
            if type_input in conversion
            else None  # _polymorphic_transformer(type_input["type_name"])
        )

    if type_input["vector"]:
        transformed_type = _transformer(type_input["type_name"], inheritance_data)

        if type_input["size"] is not None:
            return f"std::array<{transformed_type}, {type_input['size']}>"

        return f"std::vector<{transformed_type}>"
        pass
    elif type_input["map"]:
        key_type = _transformer(type_input["type_name"]["key"], inheritance_data)
        value_type = _transformer(type_input["type_name"]["value"], inheritance_data)
        return f"std::unordered_map<{key_type}, {value_type}>"
    elif type_input["union"]:
        contained_types = []
        for contained in type_input["type_name"]:
            contained_types.append(_transformer(contained, inheritance_data))
        transformed_type = ",".join(contained_types)
        return f"std::variant<{transformed_type}>"
    else:
        raise ValueError(f"Unknown type: {type_input}")


def _flatten_struct_inheritance(parsed_idl):
    inheritance_data = parsed_idl["inheritance_data"]

    sorted_inheritance_data = _sort_inheritance_data(inheritance_data)

    for base_type, derived_types in sorted_inheritance_data:
        base_struct = parsed_idl["structs"][base_type]
        for derived_type in derived_types:
            derived_struct = parsed_idl["structs"][derived_type]

            derived_struct["members"].update(base_struct["members"])

    return parsed_idl


def _sort_inheritance_data(inheritance_data) -> list[tuple[str, list[str]]]:
    sorted_inheritance_data = []

    for base_type, derived_types in inheritance_data.items():
        inserted = False
        for i, (sorted_base_type, sorted_derived_types) in enumerate(sorted_inheritance_data):
            if sorted_base_type in derived_types:
                sorted_inheritance_data.insert(i, (base_type, derived_types))
                inserted = True
                break
        if not inserted:
            sorted_inheritance_data.append((base_type, derived_types))

    return sorted_inheritance_data


def _handle_rfl_tagged_union(parsed_idl):
    new_inheritance_data = {}
    for key in parsed_idl["inheritance_data"]:
        new_inheritance_data[f"{key}_t"] = parsed_idl["inheritance_data"][key]
    parsed_idl["inheritance_data"] = new_inheritance_data
    return parsed_idl
