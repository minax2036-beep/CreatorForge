# CreatorForge Architecture

## Principles

1. Keep the user interface responsive.
2. Separate editing data from rendering and export operations.
3. Keep media engines independent behind stable interfaces.
4. Make project data portable and versioned.
5. Prefer non-destructive editing: store operations and parameters rather than overwriting source assets.

## Planned modules

- **Core** — project lifecycle, assets, commands, undo/redo, settings and plugin discovery.
- **GUI** — Qt-based workspace, docks, timeline views, preview and inspectors.
- **Video** — media probing, decoding, timeline composition and export.
- **Animation** — frame sequences, drawing layers, onion skin and keyframes.
- **Image** — layers, masks, transforms, blend modes and compositing.
- **FFmpeg** — conversion, compression, extraction and background job queue.
- **Audio** — waveform display, playback, mixing and audio effects.
- **Rendering** — preview renderer, render graph, caching and GPU integration.

## Current implementation

Version 0.1.0 contains the Qt application shell, four workspace tabs, dockable media/properties/timeline panels, and a temporary media import list. The editing engines and persistent project management are not implemented yet.

## Dependency policy

Use Qt 6 for the desktop application. Integrate FFmpeg through its libraries when the media engine is implemented. Keep OpenCV optional and use it only where it provides clear value. Rendering backend decisions should be isolated so the application can evolve beyond the initial OpenGL implementation.
