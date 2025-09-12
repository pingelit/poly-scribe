from pathlib import Path
from typing import (Annotated, Any, Dict, List, Literal, Optional, Tuple, Type,
                    TypeVar, Union)

import cbor2
from annotated_types import Len
from pydantic import BaseModel, Field
from pydantic_yaml import parse_yaml_file_as, to_yaml_file
from strenum import StrEnum

T = TypeVar("T", bound=BaseModel)


Vector = Annotated[List[float], Len(min_length=3, max_length=3)]


class Enumeration(StrEnum):
    value1 = "value1"
    value2 = "value2"


class PluginBase(BaseModel):
    name: str
    description: str
    type: Literal["PluginBase"] = "PluginBase"


class PluginA(PluginBase):
    paramA: Optional[int] = 42
    paramVector: Optional[Vector] = None
    type: Literal["PluginA"] = "PluginA"


class PluginB(PluginBase):
    paramB: Optional[float] = None
    paramEnum: Optional[Enumeration] = None
    type: Literal["PluginB"] = "PluginB"


class PluginSystem(BaseModel):
    plugin_map: Optional[
        Dict[
            str,
            Annotated[Union[PluginA, PluginB, PluginBase], Field(discriminator="type")],
        ]
    ] = None


def load(model_type: Type[T], file: Union[Path, str]) -> T:
    if isinstance(file, str):
        file = Path(file).resolve()
    elif isinstance(file, Path):
        file = file.resolve()
    else:
        msg = f"Expected Path or str, but got {file!r}"
        raise TypeError(msg)

    if not file.exists():
        msg = f"File {file} does not exist"
        raise FileNotFoundError(msg)

    if file.suffix == ".yaml":
        return parse_yaml_file_as(model_type, file)
    elif file.suffix == ".json":
        json_string = file.read_text()
        return model_type.model_validate_json(json_string)
    elif file.suffix == ".cbor":
        with file.open("rb") as f:
            data = cbor2.load(f)
        return model_type.model_validate(data)
    else:
        raise ValueError(f"Unsupported file extension {file.suffix}")


def save(file: Union[Path, str], model: Union[BaseModel]):
    if isinstance(file, str):  # local path to file
        file = Path(file).resolve()
    elif isinstance(file, Path):
        file = file.resolve()
    else:
        raise TypeError(f"Expected Path, str, or stream, but got {file!r}")

    if file.suffix == ".yaml":
        to_yaml_file(file, model)
    elif file.suffix == ".json":
        json_string = model.model_dump_json(indent=4)
        file.write_text(json_string)
    elif file.suffix == ".cbor":
        with file.open("wb") as f:
            cbor2.dump(model.model_dump(), f)
    else:
        raise ValueError(f"Unsupported file extension {file.suffix}")
