# Pathfinder

Pathfinder is a C++ library for finding routes through EVE Online map data.

The project provides pathfinding and flood-fill helpers, along with Python test
coverage for the exposed functionality.

## Building

This project uses CMake.

```sh
cmake --preset windows
cmake --build --preset windows
```

Available presets and build options are defined in `CMakePresets.json` and
`CMakeLists.txt`.

## Testing

Tests are integrated with CTest.

```sh
ctest --preset windows
```
## 🤝 Contributing
Contribution follows the standard GIT PR model.

By submitting a pull request or otherwise contributing to this project, you agree to license your contribution under the MIT License, and you confirm that you have the right to do so.

## 📄 License and Legal Notices

© 2026 CCP Games

This software is provided by CCP Games and does not include or distribute any third-party libraries or frameworks.

This software is a C++ library for route finding over EVE Online map data.

Trademark Notice: CCP Games is a trademark of CCP ehf.

This project is licensed under the [MIT License](LICENSE.md). Nothing in the [MIT License](LICENSE.md) grants any rights to CCP Games' trademarks or game content.
