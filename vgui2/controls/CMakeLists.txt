cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( VGUI_CONTROLS_SOURCE_FILES )
BEGIN_SRC( VGUI_CONTROLS_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/characterset.cpp"
			"${SRCDIR}/public/filesystem_helpers.cpp"
			"${SRCDIR}/public/keyvalues.cpp"
			"${SRCDIR}/public/mempool.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"$<${IS_LINUX}:${SRCDIR}/filesystem/filesystem_stdio/linux_support.cpp>"

			"AnimatingImagePanel.cpp"
			"AnimationController.cpp"
			"BuildGroup.cpp"
			"BuildModeDialog.cpp"
			"Button.cpp"
			"CheckButton.cpp"
			"ComboBox.cpp"
			"controls.cpp"
			"Divider.cpp"
			"EditablePanel.cpp"
			"FileOpenDialog.cpp"
			"FocusNavGroup.cpp"
			"Frame.cpp"
			"GraphPanel.cpp"
			"HTML.cpp"
			"Image.cpp"
			"ImageList.cpp"
			"ImagePanel.cpp"
			"Label.cpp"
			"LabelComboBox.cpp"
			"ListPanel.cpp"
			"ListViewPanel.cpp"
			"Menu.cpp"
			"MenuBar.cpp"
			"MenuButton.cpp"
			"MenuItem.cpp"
			"MessageBox.cpp"
			"Panel.cpp"
			"PanelListPanel.cpp"
			"ProgressBar.cpp"
			"ProgressBox.cpp"
			"PropertyDialog.cpp"
			"PropertyPage.cpp"
			"PropertySheet.cpp"
			"QueryBox.cpp"
			"RadioButton.cpp"
			"RichText.cpp"
			"ScrollBar.cpp"
			"ScrollBarSlider.cpp"
			"SectionedListPanel.cpp"
			"Slider.cpp"
			"TextEntry.cpp"
			"TextImage.cpp"
			"ToggleButton.cpp"
			"Tooltip.cpp"
			"TreeView.cpp"
			"URLLabel.cpp"
			"WizardPanel.cpp"
			"WizardSubPanel.cpp"
		#}
	)
END_SRC( VGUI_CONTROLS_SOURCE_FILES "Source Files" )

set( VGUI_CONTROLS_HEADER_FILES )
BEGIN_SRC( VGUI_CONTROLS_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/public/filesystem.h"
			"${SRCDIR}/public/filesystem_helpers.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/keyvalues.h"
			"${SRCDIR}/public/mempool.h"
			"${SRCDIR}/public/tier0/memdbgoff.h"
			"${SRCDIR}/public/tier0/memdbgon.h"
			"${SRCDIR}/public/utlmemory.h"
			"${SRCDIR}/public/utlrbtree.h"
			"${SRCDIR}/public/utlvector.h"

			"$<${IS_LINUX}:${SRCDIR}/filesystem/filesystem_stdio/linux_support.h>"

			"${SRCDIR}/public/vgui_controls/AnimatingImagePanel.h"
			"${SRCDIR}/public/vgui_controls/AnimationController.h"
			"${SRCDIR}/public/vgui_controls/BuildGroup.h"
			"${SRCDIR}/public/vgui_controls/BuildModeDialog.h"
			"${SRCDIR}/public/vgui_controls/Button.h"
			"${SRCDIR}/public/vgui_controls/CheckButton.h"
			"${SRCDIR}/public/vgui_controls/ComboBox.h"
			"${SRCDIR}/public/vgui_controls/Controls.h"
			"${SRCDIR}/public/vgui_controls/DialogManager.h"
			"${SRCDIR}/public/vgui_controls/Divider.h"
			"${SRCDIR}/public/vgui_controls/EditablePanel.h"
			"${SRCDIR}/public/vgui_controls/FileOpenDialog.h"
			"${SRCDIR}/public/vgui_controls/FocusNavGroup.h"
			"${SRCDIR}/public/vgui_controls/Frame.h"
			"${SRCDIR}/public/vgui_controls/GraphPanel.h"
			"${SRCDIR}/public/vgui_controls/HTML.h"
			"${SRCDIR}/public/vgui_controls/Image.h"
			"${SRCDIR}/public/vgui_controls/ImageList.h"
			"${SRCDIR}/public/vgui_controls/ImagePanel.h"
			"${SRCDIR}/public/vgui_controls/Label.h"
			"${SRCDIR}/public/vgui_controls/LabelComboBox.h"
			"${SRCDIR}/public/vgui_controls/LabeledSlider.h"
			"${SRCDIR}/public/vgui_controls/ListPanel.h"
			"${SRCDIR}/public/vgui_controls/ListViewPanel.h"
			"${SRCDIR}/public/vgui_controls/Menu.h"
			"${SRCDIR}/public/vgui_controls/MenuBar.h"
			"${SRCDIR}/public/vgui_controls/MenuButton.h"
			"${SRCDIR}/public/vgui_controls/MenuItem.h"
			"${SRCDIR}/public/vgui_controls/MessageBox.h"
			"${SRCDIR}/public/vgui_controls/Panel.h"
			"${SRCDIR}/public/vgui_controls/PanelAnimationVar.h"
			"${SRCDIR}/public/vgui_controls/PanelListPanel.h"
			"${SRCDIR}/public/vgui_controls/PHandle.h"
			"${SRCDIR}/public/vgui_controls/ProgressBar.h"
			"${SRCDIR}/public/vgui_controls/ProgressBox.h"
			"${SRCDIR}/public/vgui_controls/PropertyDialog.h"
			"${SRCDIR}/public/vgui_controls/PropertyPage.h"
			"${SRCDIR}/public/vgui_controls/PropertySheet.h"
			"${SRCDIR}/public/vgui_controls/QueryBox.h"
			"${SRCDIR}/public/vgui_controls/RadioButton.h"
			"${SRCDIR}/public/vgui_controls/RichText.h"
			"${SRCDIR}/public/vgui_controls/ScrollBar.h"
			"${SRCDIR}/public/vgui_controls/ScrollBarSlider.h"
			"${SRCDIR}/public/vgui_controls/SectionedListPanel.h"
			"${SRCDIR}/public/vgui_controls/Slider.h"
			"${SRCDIR}/public/vgui_controls/TextEntry.h"
			"${SRCDIR}/public/vgui_controls/TextImage.h"
			"${SRCDIR}/public/vgui_controls/ToggleButton.h"
			"${SRCDIR}/public/vgui_controls/Tooltip.h"
			"${SRCDIR}/public/vgui_controls/TreeView.h"
			"${SRCDIR}/public/vgui_controls/URLLabel.h"
			"${SRCDIR}/public/vgui_controls/WizardPanel.h"
			"${SRCDIR}/public/vgui_controls/WizardSubPanel.h"
		#}
	)
END_SRC( VGUI_CONTROLS_HEADER_FILES "Header Files" )

add_object(
	TARGET vgui_controls
	STATIC
	SOURCES ${VGUI_CONTROLS_SOURCE_FILES} ${VGUI_CONTROLS_HEADER_FILES}
)
