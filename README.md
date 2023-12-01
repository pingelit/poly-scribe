# poly-scribe

`poly-scribe` is a convenience wrapper around the [`cereal`](https://github.com/USCiLab/cereal) serialization library for saving or "scribing" data objects.
This includes polymorphic objects as well as data validation.
`cereal` can already serialize polymorphic objects, however, this library pursues a less sophisticated approach to scribing data objects.
Polymorphic data objects are serialized with a simple type tag which is used to serialize the correct type.
At the time of loading the data is then validated.
These features make this library a good fit for configurations and scribing data objects.
