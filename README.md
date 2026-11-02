# CreatorForge

**CreatorForge** is an ambitious Linux desktop creative suite written in C++20.

The long-term goal is to bring professional video editing, 2D animation, image compositing, and FFmpeg-powered media processing into one unified application.

## Project status

**Early development — v0.1.0 foundation**

The current milestone focuses on the application shell and project structure. Editing engines are planned and are not yet implemented.

## Planned features

- Unified desktop workspace
- Video editing with a multi-track timeline
- 2D frame animation and keyframes
- Layer-based image editing and compositing
- FFmpeg-based media conversion and export
- Project files, autosave, undo/redo, and asset management
- Extensible architecture

## Technology

- C++20
- Qt 6 Widgets
- CMake
- FFmpeg (planned)
- OpenGL rendering (planned)
- OpenCV (optional, planned)

## Build on Linux

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev
```

Build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
```

Run:

```bash
./build/creatorforge
```

## License

CreatorForge is distributed under the MIT License. See [LICENSE](LICENSE).

Third-party libraries retain their own licenses.
