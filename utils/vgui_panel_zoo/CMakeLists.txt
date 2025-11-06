cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( PANEL_ZOO_SOURCE_FILES )
BEGIN_SRC( PANEL_ZOO_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/vgui2/controls/LabeledSlider.cpp"

			"AnimatingImagePanelDemo.cpp"
			"ButtonDemo.cpp"
			"ButtonDemo2.cpp"
			"CascadingMenu.cpp"
			"CControlCatalog.cpp"
			"CheckButtonDemo.cpp"
			"ComboBox2.cpp"
			"ComboBoxDemo.cpp"
			"DefaultColors.cpp"
			"DemoPage.cpp"
			"EditablePanel2Demo.cpp"
			"EditablePanelDemo.cpp"
			"FileOpenDialogDemo.cpp"
			"FrameDemo.cpp"
			"HTMLDemo.cpp"
			"HTMLDemo2.cpp"
			"ImageDemo.cpp"
			"ImagePanelDemo.cpp"
			"Label2Demo.cpp"
			"LabelDemo.cpp"
			"ListPanelDemo.cpp"
			"MenuBarDemo.cpp"
			"MenuDemo.cpp"
			"MenuDemo2.cpp"
			"MessageBoxDemo.cpp"
			"ProgressBarDemo.cpp"
			"QueryBoxDemo.cpp"
			"RadioButtonDemo.cpp"
			"SampleButtons.cpp"
			"SampleCheckButtons.cpp"
			"SampleDropDowns.cpp"
			"SampleEditFields.cpp"
			"SampleListCategories.cpp"
			"SampleListPanelBoth.cpp"
			"SampleListPanelColumns.cpp"
			"SampleMenus.cpp"
			"SampleRadioButtons.cpp"
			"SampleSliders.cpp"
			"SampleTabs.cpp"
			"ScrollBar2Demo.cpp"
			"ScrollBarDemo.cpp"
			"testfile.cpp"
			"TextEntryDemo.cpp"
			"TextEntryDemo2.cpp"
			"TextEntryDemo3.cpp"
			"TextEntryDemo4.cpp"
			"TextEntryDemo5.cpp"
			"TextImageDemo.cpp"
			"ToggleButtonDemo.cpp"
			"TooltipDemo.cpp"
			"WizardPanelDemo.cpp"
		#}
	)
END_SRC( PANEL_ZOO_SOURCE_FILES "Source Files" )

set( PANEL_ZOO_HEADER_FILES )
BEGIN_SRC( PANEL_ZOO_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/interface.h"

			"CControlCatalog.h"
			"DemoPage.h"
			"MenuDemo.h"
			"SampleListPanelBoth.h"
		#}
	)
END_SRC( PANEL_ZOO_HEADER_FILES "Header Files" )

add_object(
	TARGET panel_zoo
	EXECUTABLE WIN32
	INSTALL_DEST "${GAMEDIR}/platform/bin"
	PROPERTY_FOLDER "Utils//Executables"
	SOURCES ${PANEL_ZOO_SOURCE_FILES} ${PANEL_ZOO_HEADER_FILES}
)

target_include_directories(
	panel_zoo PRIVATE

	"${SRCDIR}/vgui2/vlocalize"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/vgui2/include"
)

target_compile_definitions(
	panel_zoo PRIVATE
)

target_link_libraries(
	panel_zoo PRIVATE

	vgui_controls
	vgui_surfacelib
)
