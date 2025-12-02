# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LeakNet is a refactored and enhanced version of the leaked Half-Life 2 beta source code from 2003. It's a complete game engine implementation that supports both client gameplay and dedicated server functionality, with modern build infrastructure and cross-platform compatibility.

## Build System & Commands

**IMPORTANT: Always use CMake for building. Never use MSBuild directly or pass MSBuild flags.**

### Windows Development

**Quick Setup:**
```bash
# For full game client (recommended for development)
./creategameprojects.bat
# Open LeakNet.sln in build/ folder, compile INSTALL target

# For all tools and utilities
./createeverything.bat
# Open LeakNet.sln in build/ folder, compile INSTALL target
```

**Command Line Building (ALWAYS use cmake):**
```bash
# Generate projects
./creategameprojects.bat  # or ./createeverything.bat

# Build all targets
cmake --build build --config Release

# Build specific targets
cmake --build build --config Release --target server_bmod
cmake --build build --config Release --target client_bmod
cmake --build build --config Release --target engine
cmake --build build --config Release --target studiorender

# Install
cmake --install build --config Release
```

**Never rebuild shaders unless necessary - they take a long time to compile.**

### Linux Development

**Dedicated Server Only:**
```bash
./creatededicatedprojects.sh
cd build_dedicated
make -j$(nproc --all) install
```

### Build Groups

The project uses CMake BUILD_GROUP configurations:

- **"game"** - Client game executable with all necessary components (default for development)
- **"dedicated"** - Linux dedicated server only (HLDS - Half-Life Dedicated Server)
- **"everything"** - All components including tools, utilities, and master server

### Output Location

All built binaries install to `leaknet-install/` directory (configurable via `-DGAMEDIR="path"`).

## Architecture Overview

### Core Module Structure

**Foundation Layer:**
- `tier0/` - Memory allocation, debugging, threading, platform abstraction
- `vstdlib/` - Standard library utilities and command-line parsing
- `public/` - Public header interfaces and shared utilities

**Engine Core:**
- `engine/` - Main game engine with client/server logic, networking, demo recording
- `filesystem/` - VPK file system support and file I/O
- `vphysics/` + `ivp/` - Physics simulation using Havok/IVP engine

**Rendering Pipeline:**
- `materialsystem/` - Material/shader system with DirectX 8/9 support
- `materialsystem/stdshaders/` - Standard shader effects
- `materialsystem/shaderdx8/` - DirectX 8 shader implementation
- `studiorender/` - Model rendering
- `vtf/` - VTF (Valve Texture Format) support

**Client-Side:**
- `cl_dll/` - Client game logic, player rendering, input handling
- `gameui/` - Game user interface system
- `vgui2/` - VGUI2 UI framework (controls, surfaces, game controls)
- `launcher/` + `launcher_main/` - Application launcher entry points

**Server-Side:**
- `dlls/` - Game server logic, entities, physics simulation
- `dedicated/` - Dedicated server executable
- `game_shared/` - Shared code between client and server

**Master Server System:**
- `Tracker/TrackerServer/` - Main master server
- `Tracker/ServerBrowser/` - Server browser functionality
- `master/` - Master server utilities

### Key Architectural Patterns

**Interface-Based Plugin System:**
- Uses versioned factory functions (`CreateInterfaceFn`) for loose coupling
- Modules export interfaces via DLLs/SOs allowing swappable implementations
- Core interfaces defined in `public/interface.h`

**Tiered Dependencies:**
```
Foundation (tier0, vstdlib)
    ↓
Engine Core (engine, filesystem, physics)
    ↓
Rendering (materialsystem, shaders)
    ↓
Game Implementation (client/server DLLs)
```

## Development Workflow

### Working with Graphics/Shaders

The material system supports multiple shader backends:
- `shaderdx8/` - DirectX 8 implementation (primary)
- `shaderempty/` - Stub implementation for dedicated servers
- `stdshaders/` - Standard shader effects

When modifying shaders, changes typically require rebuilding both the shader DLL and any dependent materials.

### Client vs Dedicated Server

The codebase uses conditional compilation for client/server differences:
- Client builds include GUI, rendering, and input systems
- Dedicated builds exclude graphics and use console-only interface
- Shared game logic lives in `game_shared/` and portions of `dlls/`

### Physics Integration

Physics simulation uses the IVP (Integrated Virtual Physics) engine with Havok solver:
- `ivp/ivp_physics/` - Core physics implementation
- `ivp/havana/havok/` - Havok physics solver integration
- `vphysics/` - Game engine physics interface

## File Formats & Assets

**Native Formats:**
- `.vpk` - Valve Package Format (game assets)
- `.vtf` - Valve Texture Format (textures)
- `.bsp` - Binary Space Partition (maps)
- `.mdl` - Model format

**Tools Available (in "everything" build):**
- `vtex` - Texture compilation
- `vbsp`, `vvis`, `vrad` - Map compilation tools
- `studiomdl` - Model compilation
- `bsppack`/`bspzip` - BSP archive utilities

## Common Development Tasks

### Building Specific Components

```bash
# Build only client game
cmake -S . -B build -A Win32 -DBUILD_GROUP="game"

# Build dedicated server (Linux)
cmake -S . -B build_dedicated -DCMAKE_C_FLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32" -DDEDICATED=ON

# Build everything including tools
cmake -S . -B build -A Win32 -DBUILD_GROUP="everything"
```

### Platform-Specific Notes

**Windows:**
- Requires Visual Studio with Win32 platform support
- Uses DirectX 8/9 SDK (included in `dx8sdk/`, `dx9sdk/`)
- 32-bit builds only (`-A Win32`)

**Linux:**
- Requires 32-bit development libraries
- Uses `-m32` compiler flags for 32-bit compatibility
- Dedicated server builds only

**Code Standards:**
- C++14 standard
- Uses CMake 3.20+ build system
- Position-independent code enabled for libraries

### Interface Development

When adding new interfaces:
1. Define interface in `public/` headers
2. Implement in appropriate module
3. Register with interface factory system
4. Version interfaces properly for compatibility

### Memory Management

The engine uses custom memory allocation:
- Foundation memory system in `tier0/`
- Debug memory tracking available in Debug builds
- Use engine allocation functions rather than standard malloc/free

## Master Server Integration

The project includes a complete master server implementation:
- `TrackerServer` - Main master server
- `TrackerSRV` - Server tracking service
- Server browser with filtering capabilities
- Database backends (partially implemented)

Master server functionality is included in "everything" builds but commented out portions suggest incomplete database integration.
- the 2007 engine source code directory for reference: F:\back-ups\betahl2_codebases\SourceEngine2007-master\SourceEngine2007-master\src_main
- clean leaknet codebase with v37 mdl support: F:\development\steam\emulator_bot\LeakNet
- the location of the game and engine directory is: C:\anon-hl2\  with bmod located within C:\anon-hl2\bmod