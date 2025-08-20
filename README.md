# poly-scribe

[![ci](https://github.com/pingelit/poly-scribe/actions/workflows/ci.yml/badge.svg)](https://github.com/pingelit/poly-scribe/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/pingelit/poly-scribe/graph/badge.svg)](https://codecov.io/gh/pingelit/poly-scribe)
[![CodeQL](https://github.com/pingelit/poly-scribe/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/pingelit/poly-scribe/actions/workflows/codeql-analysis.yml)

## About poly-scribe

`poly-scribe` is a software environment designed to facilitate easy (polymorphic) data structure serialization, deserialization and exchange between different programming languages.
The intend is to provide a type-safe and similar interface across multiple languages, allowing developers to work with data structures without worrying about the underlying serialization format.
This is achieved by generating code based on a common interface definition.
For this purpose, `poly-scribe` uses WebIDL (Web Interface Definition Language) to define the data structures.
The generated code can then be used in various programming languages such as C++ or Python.
As such, a simple call to a `load` and `save` function will serialize and deserialize the data structure to and from a file respectively.

To simplify the process of generating code, `poly-scribe` provides a python package called `poly_scribe_code_gen` as well as a CMake function to generate code for a C++ project.

For more information on how to write WebIDL definitions and what the generated code looks like, please refer to the [Quick Start Guide](https://pingelit.github.io/poly-scribe/latest/quick_start/).
For more detailed information on the code generation process, please refer to the [Code Generation API](https://pingelit.github.io/poly-scribe/latest/reference/).
