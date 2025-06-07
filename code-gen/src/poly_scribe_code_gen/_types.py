import sys
from typing import Any, Optional, TypedDict

if sys.version_info < (3, 11):
    from typing_extensions import NotRequired
else:
    from typing import NotRequired

integer_types = [
    "bool",
    "char",
    "unsigned char",
    "short",
    "unsigned short",
    "int",
    "unsigned int",
    "long",
    "unsigned long",
    "long long",
    "unsigned long long",
]
"""Integer types for poly-scribe."""

floating_point_types = ["float", "double", "long double"]
"""Floating point types for poly-scribe."""

std_types = ["ByteString", "string"]
"""String types for poly-scribe."""

cpp_types = integer_types + floating_point_types + std_types
"""All types for poly-scribe.

These are espcially useful for the C++ code generation.
"""


class AdditionalData(TypedDict):
    """Additional data for the code generation.

    This is used to pass additional data to the code generation process.
    """
    author_name: NotRequired[str]
    """Author name for the generated code."""
    author_email: NotRequired[str]
    """Author email for the generated code."""
    out_file: NotRequired[Optional[str]]
    """Output file for the generated code."""
    year: NotRequired[str]
    """Year for the generated code for copyright."""
    licence: NotRequired[str]
    """Licence for the generated code."""
    package: str
    """Package name for the generated code."""


class ParsedIDL(TypedDict):
    """Parsed IDL data.

    """
    typedefs: dict[str, dict[str, Any]]
    """Typedefs in the IDL."""
    enums: dict[str, dict[str, Any]]
    """Enums in the IDL."""
    structs: dict[str, dict[str, Any]]
    """Structs in the IDL."""
    inheritance_data: dict[str, list[str]]
    """Inheritance data of the contained structs."""
