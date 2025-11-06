cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

# @TODO(SanyaSho): change name to mxtoolkitwin32

set( MXTK_SOURCE_FILES )
BEGIN_SRC( MXTK_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/utils/mxtk/src/common/mxBmp.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/common/mxpath.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/common/mxPcx.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/common/mxstring.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/common/mxTga.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mx.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxButton.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxCheckBox.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxChoice.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxChooseColor.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxFileDialog.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxGlWindow.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxGroupBox.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxLabel.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxLineEdit.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxListBox.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxlistview.cpp"
			"${SRCDIR}/utils/mxtk/src/win32/mxMatSysWindow.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxMenu.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxMenuBar.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxMessageBox.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxPopupMenu.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxProgressBar.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxRadioButton.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxScrollbar.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxSlider.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxTab.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxToolTip.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxTreeview.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxWidget.cpp" # Debug: BrowseInformation TRUE
			"${SRCDIR}/utils/mxtk/src/win32/mxWindow.cpp" # Debug: BrowseInformation TRUE
		#}
	)
END_SRC( MXTK_SOURCE_FILES "Source Files" )

set( MXTK_HEADER_FILES )
BEGIN_SRC( MXTK_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/utils/mxtk/include/mx/gl.h"
			"${SRCDIR}/utils/mxtk/include/mx/mx.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxBmp.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxCheckBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChoice.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxChooseColor.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxEvent.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxFileDialog.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGlWindow.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxGroupBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxImage.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxInit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLabel.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLineEdit.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxLinkedList.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxListBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxlistview.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMatSysWindow.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMenuBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxMessageBox.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxpath.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxPcx.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxPopupMenu.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxProgressBar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxRadioButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxScrollbar.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxSlider.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxstring.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTab.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTga.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToggleButton.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxToolTip.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxTreeView.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWidget.h"
			"${SRCDIR}/utils/mxtk/include/mx/mxWindow.h"
		#}
	)
END_SRC( MXTK_HEADER_FILES "Header Files" )

add_object(
	TARGET mxtk
	STATIC
	PROPERTY_FOLDER "Utils//Static Libs"
	SOURCES ${MXTK_SOURCE_FILES} ${MXTK_HEADER_FILES}
)

target_include_directories(
	mxtk PRIVATE

	"${SRCDIR}/utils/mxtk/include"
	"${SRCDIR}/common/MaterialSystem"
)
