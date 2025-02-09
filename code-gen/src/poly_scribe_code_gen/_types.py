from typing import Optional, TypedDict

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
    author_name: str
    author_email: str
    out_file: Optional[str]
    year: str
    licence: str
    namespace: str
