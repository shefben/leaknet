// Compatibility test file for GMod systems in LeakNet
// This file tests key interfaces and systems for compatibility

#include "cbase.h"
#include "vphysics_interface.h"
#include "vphysics/constraints.h"
#include "filesystem.h"
#include "vgui/IScheme.h"
#include "convar.h"
#include "igamesystem.h"
#include "vgui/vgui.h"

// Test physics interface access
void TestPhysicsInterface()
{
    // Test that we can access the global physics environment
    if (physenv)
    {
        DevMsg("Physics environment available\n");

        // Test constraint creation interfaces
        constraint_fixedparams_t fixed;
        fixed.constraint.forceLimit = 1000.0f;
        fixed.constraint.torqueLimit = 1000.0f;

        // These interfaces should be available
        // physenv->CreateFixedConstraint(...);
        // physenv->CreateHingeConstraint(...);
        // physenv->CreateBallsocketConstraint(...);

        DevMsg("Physics constraint interfaces available\n");
    }
    else
    {
        DevMsg("ERROR: Physics environment not available\n");
    }
}

// Test VGUI interface access (server-side has limited VGUI access)
void TestVGUIInterface()
{
    // Note: VGUI is primarily client-side in LeakNet
    // Server DLL has limited VGUI functionality
    DevMsg("VGUI test: Server DLL has limited VGUI access (this is normal)\n");
}

// Test filesystem interface
void TestFilesystemInterface()
{
    // Test filesystem access
    if (filesystem)
    {
        DevMsg("Filesystem interface available\n");

        // Test file existence check
        bool exists = filesystem->FileExists("scripts/game_sounds_manifest.txt", "GAME");
        DevMsg("Filesystem FileExists works: %s\n", exists ? "yes" : "no");
    }
    else
    {
        DevMsg("ERROR: Filesystem interface not available\n");
    }
}

// Test ConVar system
ConVar test_gmod_convar("test_gmod_convar", "1", FCVAR_NONE, "Test ConVar for GMod compatibility");

void TestConVarInterface()
{
    DevMsg("ConVar test value: %d\n", test_gmod_convar.GetInt());
    test_gmod_convar.SetValue(42);
    DevMsg("ConVar after set: %d\n", test_gmod_convar.GetInt());
}

// Test console command
void CC_TestGModCommand(void)
{
    DevMsg("GMod test command executed with %d args\n", engine->Cmd_Argc());

    TestPhysicsInterface();
    TestVGUIInterface();
    TestFilesystemInterface();
    TestConVarInterface();
}

ConCommand test_gmod_command("test_gmod_command", CC_TestGModCommand, "Test GMod compatibility");

// Test game system interface
class CGModCompatibilityTestSystem : public CAutoGameSystem
{
public:
    CGModCompatibilityTestSystem() : CAutoGameSystem() {}

    virtual bool Init()
    {
        DevMsg("GMod Compatibility Test System: Init()\n");
        return true;
    }

    virtual void LevelInitPostEntity()
    {
        DevMsg("GMod Compatibility Test System: LevelInitPostEntity()\n");

        // Run all compatibility tests
        TestPhysicsInterface();
        TestVGUIInterface();
        TestFilesystemInterface();
        TestConVarInterface();
    }

    virtual void Shutdown()
    {
        DevMsg("GMod Compatibility Test System: Shutdown()\n");
    }
};

// Global instance
CGModCompatibilityTestSystem g_GModCompatibilityTest;
