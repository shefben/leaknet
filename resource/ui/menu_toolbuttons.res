// GMod 9 build/spawn menu layout and dynamic-control templates.
// The geometry matches the original client.dll. CClientSpawnDialog reads the
// BuildMenuLayout block, and CToolButtonsPanel applies the three templates to
// the controls created from settings/menu_main/*.txt.

"Resource/UI/menu_toolbuttons.res"
{
	"BuildMenuLayout"
	{
		"dialog_wide"			"790"
		"panel_top"			"10"
		"panel_bottom"			"10"
		"prop_x"				"10"
		"prop_wide"			"238"
		"tool_x"				"270"
		"tool_wide"			"510"
		"context_x"			"545"
		"context_wide"			"235"
		"context_bottom"		"15"

		// The tool settings ("advanced") box is a full-height column of its
		// own.  While it is up the tool list is clipped to
		// context_x - column_gap so its buttons reflow next to the box instead
		// of ending up hidden behind it.
		"column_gap"			"8"
	}

	"ToolButtonsPanel"
	{
		"wide"					"510"
		"tall"					"530"
		"visible"				"1"
		"enabled"				"1"
		"bgcolor_override"		"50 50 50 200"
	}

	"BuildMenuLabelTemplate"
	{
		"wide"					"502"
		"tall"					"17"
		"visible"				"1"
		"enabled"				"1"
		"textAlignment"			"west"
		"font"					"SpawnMenuButton"
		"fgcolor_override"		"230 230 170 255"
		"bgcolor_override"		"0 0 0 0"
	}

	"BuildMenuButtonTemplate"
	{
		"wide"					"250"
		"tall"					"16"
		"visible"				"1"
		"enabled"				"1"
		"textAlignment"			"west"
		"font"					"SpawnMenuButton"
		"dulltext"				"0"
		"brighttext"			"0"
		"default"				"0"
	}

	"BuildMenuDoubleButtonTemplate"
	{
		"wide"					"502"
		"tall"					"34"
		"visible"				"1"
		"enabled"				"1"
		"textAlignment"			"west"
		"font"					"SpawnMenuButton"
		"dulltext"				"0"
		"brighttext"			"0"
		"default"				"0"
	}
}
