-- Basic Lua Test Script for BarrysMod
-- This script demonstrates the Lua integration system

print("BarrysMod Lua Integration Test Started!")

-- Test function that can be called from C++
function DoLuaThinkFunctions()
    -- This function gets called every game tick
    -- Add your game logic here
end

function gamerulesThink()
    -- Game rules thinking function
    -- Called every game tick
end

function TestPlayerFunction()
    print("TestPlayerFunction called from Lua!")

    -- Example: Give ammo to player 1
    -- _PlayerGiveAmmo(1, 30, "pistol", true)

    return true
end

function TestEntityFunction()
    print("TestEntityFunction called from Lua!")

    -- Example: Remove entity with index 100
    -- _EntRemove(100)

    return true
end

function TestUIFunction()
    print("TestUIFunction called from Lua!")

    -- Example: Initialize text system
    -- _GModTextStart("Arial")

    return true
end

print("BarrysMod Lua Test Script Loaded Successfully!")
print("Available test functions:")
print("  TestPlayerFunction()")
print("  TestEntityFunction()")
print("  TestUIFunction()")
print("Use 'lua_openscript test/basic_test' to load this script")
print("Use 'lua_listbinds' to see available C++ functions")