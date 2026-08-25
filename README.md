# FreeEffect

A free and open-source alternative to Adobe After Effects, built with C++ and Qt.

## Features

- Professional compositing and motion graphics
- Timeline with keyframe animation
- Layer-based composition
- Video, image, and audio import
- MP4/H.264 export via Render Queue
- After Effects-compatible interface and workflows
- Undo/Redo support for all operations
- Project save/load with `.feproj` format

## Building

### Prerequisites

- CMake 3.20+
- Qt 6
- OpenGL
- FFmpeg (libavcodec, libavformat, libavutil, libswscale)
- nlohmann/json

### Build Commands

```bash
# Clone the repository
git clone https://github.com/yourusername/FreeEffect.git
cd FreeEffect

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
cd build && ctest
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this project.
