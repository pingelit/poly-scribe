typedef [Size=3] sequence<double> Vector; // (1)

enum Enumeration { // (2)
    "value1",
    "value2"
};

dictionary PluginBase // (3)
{
    required string name; // (4)
    required string description;
};

dictionary PluginA : PluginBase
{
    int paramA = 42; // (5)
    Vector paramVector; // (6)
};

dictionary PluginB : PluginBase
{
    float paramB;
    Enumeration paramEnum;
};

dictionary PluginSystem
{
    record<ByteString, PluginBase> plugin_map; // (7)
};