# poly-scribe-code-gen

Generate code for serializing data structures from WebIDL definitions.

## Documentation

### Fixed size arrays

In order to get a fixed-size array or vector, the extra argument `Size` can be used.
For example, an array with 4 `double` can be defined as:

```
[Size=4] sequence<double> four_elements;
```

### Polymorphic default values

Default values for polymorphic types are tricky.
The syntax is as follows:

```
[Default=DerivedType] BaseType default_poly = {};
```

However for this to work correctly, all members of the DerivedType must either have a default value or be optional/nullable.
