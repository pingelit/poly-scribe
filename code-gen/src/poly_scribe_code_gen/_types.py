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

floating_point_types = ["float", "double", "long double"]

std_types = ["ByteString", "string"]

cpp_types = integer_types + floating_point_types + std_types


class AdditionalData(TypedDict):
    author_name: NotRequired[str]
    author_email: NotRequired[str]
    out_file: NotRequired[Optional[str]]
    year: NotRequired[str]
    licence: NotRequired[str]
    package: str


class ParsedIDL(TypedDict):
    typedefs: dict[str, dict[str, Any]]
    enums: dict[str, dict[str, Any]]
    structs: dict[str, dict[str, Any]]
    inheritance_data: dict[str, list[str]]
