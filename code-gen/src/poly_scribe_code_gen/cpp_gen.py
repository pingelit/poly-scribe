# SPDX-FileCopyrightText: 2024-present Pascal Palenda <pascal.palenda@akustik.rwth-aachen.de>
#
# SPDX-License-Identifier: MIT

import os
from pathlib import Path
from typing import Any, TypedDict

import jinja2


class AdditionalData(TypedDict):
    author_name: str
    author_email: str
    out_file: str
    year: str
    licence: str
    namespace: str


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

    package_dir = os.path.abspath(os.path.dirname(__file__))
    templates_dir = os.path.join(package_dir, "templates")

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(templates_dir),
        trim_blocks=True,
        lstrip_blocks=True,
    )

    j2_template = env.get_template("template.hpp.jinja")

    data = {**additional_data, **parsed_idl}

    res = j2_template.render(data)

    with open(out_file, "w") as f:
        f.write(res)
