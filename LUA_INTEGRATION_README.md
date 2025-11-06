# BarrysMod Lua Integration System

This document describes the complete Lua integration system implemented for BarrysMod, based on reverse engineering analysis of Garry's Mod server.dll.

## Overview

The Lua integration system provides a 1:1 implementation of Garry's Mod's server-side Lua binding architecture, allowing Lua scripts to control game entities, players, weapons, effects, and UI elements through exposed C++ functions.

## Architecture

### Core Components

1. **CLuaIntegration** - Main integration manager class
2. **CLuaUtility** - Parameter conversion and utility functions
3. **CGmodRunFunction** - Entity class for executing Lua functions
4. **LuaFunctionRegistration** - Function registration system
5. **Console Commands** - lua_openscript, lua_listbinds

### Key Features

- **200+ Function Binding Capacity** - Extensible registration system
- **Real-time Execution** - Lua functions called every game tick
- **Entity Integration** - CGmodRunFunction entity for Lua function execution
- **Parameter Conversion** - Automatic C++ ↔ Lua type conversion
- **Error Handling** - Comprehensive error reporting and recovery
- **Console Integration** - Commands for script loading and function listing

## Files Created

### Core Implementation
- `dlls/bmod_dll/lua_integration.h` - Main header with CLuaIntegration class
- `dlls/bmod_dll/lua_integration.cpp` - Core implementation
- `dlls/bmod_dll/lua_utility.cpp` - Utility functions for parameter handling
- `dlls/bmod_dll/gmod_runfunction.cpp` - CGmodRunFunction entity implementation
- `dlls/bmod_dll/lua_stubs.cpp` - Lua library stub implementations

### Headers
- `public/lua.h` - Core Lua header stub
- `public/lauxlib.h` - Lua auxiliary library header
- `public/lualib.h` - Lua standard library header

### Test Scripts
- `lua/test/basic_test.lua` - Example Lua script demonstrating the system

### Modified Files
- `dlls/gameinterface.cpp` - Added Lua system initialization and think calls

## Function Categories Implemented

### Player Functions
- `_PlayerGiveAmmo(playerid, amount, ammotype, playsounds)` - Give ammo to player
- `_PlayerSetSprint(playerid, enabled)` - Enable/disable sprint
- `_PlayerShowScoreboard(playerid)` - Show scoreboard to player

### Entity Functions
- `_EntRemove(entindex)` - Remove entity by index

### Effect Functions
- `_EffectSetRadius(radius)` - Set effect radius

### UI Functions
- `_GModTextStart(fontname)` - Initialize text display system

## Usage

### Console Commands

```
lua_openscript <filename>    # Load and execute Lua script
lua_listbinds               # List all registered C++ functions
```

### Example Usage

1. Load the test script:
   ```
   lua_openscript test/basic_test
   ```

2. List available functions:
   ```
   lua_listbinds
   ```

3. Create entities that execute Lua functions:
   ```
   ent_create gmod_runfunction
   ```

### Lua Script Example

```lua
-- Basic script structure
function DoLuaThinkFunctions()
    -- Called every game tick
end

function gamerulesThink()
    -- Game rules logic
end

-- Use exposed C++ functions
function GivePlayerAmmo()
    _PlayerGiveAmmo(1, 30, "pistol", true)
end
```

## Entity Usage

The `gmod_runfunction` entity executes Lua functions as entity think:

### Keyvalues
- `function` - Name of Lua function to execute
- `delay` - Delay between executions (default: 0.1)

### Inputs
- `RunFunction` - Execute specified function immediately

### Outputs
- `OnFinished` - Fired when function execution completes

## Integration Points

### Initialization
- **GameInit()** - Lua system initialized when server starts
- **GameShutdown()** - Lua system cleaned up when server stops

### Runtime
- **Main Game Loop** - `DoThinkFunctions()` called every tick after physics
- **Function Registration** - All C++ functions registered at startup
- **Script Loading** - Scripts loaded from `lua/` directory

## Development Notes

### Adding New Functions

1. Declare the function:
   ```cpp
   DECLARE_LUA_FUNCTION(MyNewFunction);
   ```

2. Implement the function:
   ```cpp
   int Lua_MyNewFunction(lua_State *L)
   {
       // Implementation here
       return 0; // Number of return values
   }
   ```

3. Register the function:
   ```cpp
   REGISTER_LUA_FUNCTION(MyNewFunction, "Description of function");
   ```

### Parameter Handling

Use CLuaUtility helper functions:
```cpp
int playerID = CLuaUtility::GetInt(L, 1);
bool enabled = CLuaUtility::GetBool(L, 2);
const char* text = CLuaUtility::GetString(L, 3);
```

### Return Values

Push return values to Lua stack:
```cpp
CLuaUtility::PushInt(L, result);
CLuaUtility::PushBool(L, success);
CLuaUtility::PushString(L, message);
return 3; // Number of return values
```

## Production Deployment

### Lua Library Integration

For production use, replace the stub implementations with real Lua 5.0.2:

1. Download Lua 5.0.2 source
2. Compile as static library
3. Replace header stubs with real headers
4. Link against lua library
5. Remove lua_stubs.cpp

### Performance Considerations

- Function calls occur every game tick - optimize heavily used functions
- Use entity-based execution for complex operations
- Cache frequently accessed data
- Monitor script execution time

## Reverse Engineering Insights

This implementation replicates the exact architecture discovered in Garry's Mod:

- **Function Registration Pattern** - Underscore prefix naming convention
- **Entity Think Integration** - CGmodRunFunction entity for script execution
- **Console Command System** - lua_openscript and lua_listbinds commands
- **Parameter Binding** - Direct C++ function exposure to Lua
- **Think Function Architecture** - DoLuaThinkFunctions called every tick

The system provides a complete foundation for expanding BarrysMod with Lua scripting capabilities matching Garry's Mod's functionality.