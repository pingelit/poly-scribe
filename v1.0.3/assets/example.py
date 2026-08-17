import plugin_data as pd

pd_system = pd.PluginSystem(
    plugin_map={
        "plugin1": pd.PluginA(
            name="Plugin A",
            description="This is Plugin A",
            paramVector=[1.0, 2.0, 3.0],
        ),
        "plugin2": pd.PluginB(
            name="Plugin B",
            description="This is Plugin B",
            paramB=3.14,
            paramEnum=pd.Enumeration.value1,
        ),
    }
)

pd.save("example.json", pd_system)
