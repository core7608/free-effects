# Contributing to FreeEffect

Thank you for your interest in contributing to FreeEffect! This document provides guidelines and information about contributing to this project.

## Getting Started

1. Fork the repository on GitHub
2. Clone your fork locally
3. Create a feature branch (`git checkout -b feature/amazing-feature`)
4. Make your changes
5. Commit your changes (`git commit -m 'Add some amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Development Setup

### Prerequisites

- CMake 3.20+
- Qt 6
- OpenGL
- FFmpeg development libraries
- nlohmann/json

### Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Running Tests

```bash
cd build && ctest --output-on-failure
```

## Code Style

- Follow the existing code style in the project
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and reasonably sized

## Commit Messages

- Use clear, descriptive commit messages
- Start with a type: `feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `test:`, `chore:`
- Example: `feat: add keyframe interpolation support`

## Pull Request Process

1. Update the README.md if needed
2. Ensure all tests pass
3. Follow the existing code style
4. Keep pull requests focused on a single change

## Reporting Issues

- Use the GitHub issue tracker
- Include steps to reproduce the issue
- Include your operating system and Qt version

## License

By contributing, you agree that your contributions will be licensed under the GPLv3 License.
