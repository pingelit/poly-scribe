# Additional features

## Default values

Sometimes, you may want to provide default values for user defined types.
This can be achieved by setting the default value in the IDL to `{}`, like in this example:

``` webidl
dictionary SubConfig
{
    int answer = 42;
};

dictionary Config
{
    SubConfig sub = {};
};
```

This will create a new instance of `SubConfig` in `Config`.
Note, that this is only really possible if `SubConfig` has no required fields.

As an extension to this, this approach can also be used with polymorphic types.
Here, there might not be a single concrete type to instantiate, but rather a family of types that share a common base.
In this case you can define the desired default type in the IDL via the extra attribute `Default` like so:

``` webidl
dictionary Base
{};

dictionary A : Base
{};

dictionary B : Base
{};

dictionary Container
{
    [Default=A] Base sub = {};
}
```
