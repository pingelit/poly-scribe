@echo off
REM Change directory to the script's directory
cd /d "%~dp0\..\..\code-gen"

hatch run code-gen -c %~dp0\plugin_data.hpp -p %~dp0\plugin_data.py -s %~dp0\plugin_schema.json PluginSystem -a %~dp0\additional.json %~dp0\plugin.webidl