# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Switch from `cereal` to `reflectcpp` (#36)
- Rework the python code generation and add unit tests (#37)
- Rework the integration tests (#39)
- Add support for `yaml` and `cbor` serialization (#40)
- Add python code generation type checks (#41)
- Test that that default strings are quoted in the generated code (#42)
- Test that optional vectors and maps are handled correctly in the generated code (#43)
- Generate full python projects with `pyproject.toml` (#44)
- Docstring rendering (#45)
- Add documentation site (#46)
- Improve the CMake functionality (#49)
- Add CMake install commands (#51)
- Refine optional values in C++ code generation (#53)
- Handle multiple inheritance in C++ code generation (#54)
- Handle user defined default values (#55)
- Refine optional values in python code generation (#56)
- Refine python and json schema generation (#57)
- Final todos for v1.0.0 release (#58)

### Changed

- Fixed wrongly associated docstrings (#48)
- Fix `false` as valid default value (#52)
- Minor improvements to python, pkg name, docstrings (#59)
- C++ defaulted values must be optional (#60)

## `cereal-impl`

- Previous implementation idea using `cereal`.
  For more information, please see the PRs on the git repository.
