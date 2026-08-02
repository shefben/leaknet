## Modified hl2 beta leaknet engine with barrysmod!

The engine patch for the leaked Half-Life 2 beta sources from 2003.  It is a modified leaknet engine which supports retail and beta model and map formats along with retail material formats as well.  It includes a reverse engineered garrysmod 9.0.4b reimplemented into the beta leaknet engine with full lua and mod support.  The goal is to have a fully working garrysmod in the beta engine which supports ALL garrysmod 9.0.4b lua scripts and mods.

 
### Features
- Full Beta and Retail v37-v48 MDL support
- Beta and Retail BSP support
- Beta and Retail Material support
- Partial Retail Shader support
- Various bug/security fixes
- Linux dedicated server support
- CMake build system
- Working MasterServer functionality
- Replaced miles with minimp3 (for mp3 only, miles is still used as voice codec)
- Full Gmod 9.0.4b lua script and mod support.

### Compiling
#### Windows
- Choose what you need: `createeverything.bat` - for building all available projects (everything.sln) or `creategameprojects.bat` - if you need to build basic game without utils and tools.
- To build windows client run:
```
./createeverything.bat or ./creategameprojects.bat
open the `LeakNet.sln` inside the `build` folder
compile target `INSTALL`
```
you also can use cmake from command line if you want:
```
./createeverything.bat or ./creategameprojects.bat
cmake --build build --config "Release" or "Debug"
cmake --install build --config "Release" or "Debug"
```
#### Linux
- To build dedicated server run:
```
./creatededicated.sh
cd build_dedicated
make -j$(nproc --all) install
```

- All installed libs will be stored in `(srcrepo)/leaknet-install` (can be changed through command-line argument -DGAMEDIR="dir").
