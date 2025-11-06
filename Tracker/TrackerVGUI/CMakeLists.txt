cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERVGUI_SOURCE_FILES )
BEGIN_SRC( TRACKERVGUI_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/info_key.cpp"
			"${SRCDIR}/common/GameUI/Random.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/Tracker/common/IceKey.cpp"
			"${SRCDIR}/vgui2/controls/LabeledSlider.cpp"

			"${SRCDIR}/Tracker/TrackerUI/Buddy.cpp"
			"${SRCDIR}/Tracker/TrackerUI/BuddyButton.cpp"
			"${SRCDIR}/Tracker/TrackerUI/BuddySectionHeader.cpp"
			"${SRCDIR}/Tracker/TrackerUI/BuildNum.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogAbout.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogAuthRequest.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogChat.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogFindBuddy.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogLoginUser.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogOptions.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogRemoveUser.cpp"
			"${SRCDIR}/Tracker/TrackerUI/DialogUserInfo.cpp"
			"${SRCDIR}/Tracker/TrackerUI/OnlineStatus.cpp"
			"${SRCDIR}/Tracker/TrackerUI/OnlineStatusSelectComboBox.cpp"
			"${SRCDIR}/Tracker/TrackerUI/PanelValveLogo.cpp"
			"${SRCDIR}/Tracker/TrackerUI/ServerSession.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelBuddyList.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelConnectionIntro.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelConnectionTest.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser2.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser3.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelError.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddy.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyComplete.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyRequestAuth.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyResults.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelLoginUser.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelLoginUser2.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsChat.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsConnection.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsPersonal.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsSounds.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelSelectLoginOption.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelTrackerState.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelUserInfoDetails.cpp"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelUserInfoStatus.cpp"
			"${SRCDIR}/Tracker/TrackerUI/Tracker.cpp"
			"${SRCDIR}/Tracker/TrackerUI/TrackerDialog.cpp"
			"${SRCDIR}/Tracker/TrackerUI/TrackerDoc.cpp"
			"${SRCDIR}/Tracker/TrackerUI/TrackerRunGameEngine.cpp"
			"${SRCDIR}/Tracker/TrackerUI/TrackerUIVGuiModule.cpp"
			"${SRCDIR}/Tracker/TrackerUI/TrackerUser.cpp"

			"testfile.cpp"
		#}
	)
END_SRC( TRACKERVGUI_SOURCE_FILES "Source Files" )

set( TRACKERVGUI_HEADER_FILES )
BEGIN_SRC( TRACKERVGUI_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/info_key.h"
			#"${SRCDIR}/common/ITrackerUser.h"
			"${SRCDIR}/common/GameUI/Random.h"
			"${SRCDIR}/public/interface.h"

			"${SRCDIR}/Tracker/common/TrackerProtocol.h"
			"${SRCDIR}/Tracker/TrackerUI/Buddy.h"
			"${SRCDIR}/Tracker/TrackerUI/BuddyButton.h"
			"${SRCDIR}/Tracker/TrackerUI/BuddySectionHeader.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogAbout.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogAuthRequest.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogChat.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogFindBuddy.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogLoginUser.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogOptions.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogRemoveUser.h"
			"${SRCDIR}/Tracker/TrackerUI/DialogUserInfo.h"
			"${SRCDIR}/Tracker/TrackerUI/OnlineStatus.h"
			"${SRCDIR}/Tracker/TrackerUI/OnlineStatusSelectComboBox.h"
			"${SRCDIR}/Tracker/TrackerUI/PanelValveLogo.h"
			"${SRCDIR}/Tracker/TrackerUI/ServerSession.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelBuddyList.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelConnectionIntro.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelConnectionTest.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser2.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelCreateUser3.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelError.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddy.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyComplete.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyRequestAuth.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelFindBuddyResults.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelLoginUser.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelLoginUser2.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsChat.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsConnection.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsPersonal.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelOptionsSounds.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelSelectLoginOption.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelUserInfoDetails.h"
			"${SRCDIR}/Tracker/TrackerUI/SubPanelUserInfoStatus.h"
			#"${SRCDIR}/Tracker/TrackerUI/SystemBuddyButton.h"
			"${SRCDIR}/Tracker/TrackerUI/Tracker.h"
			"${SRCDIR}/Tracker/TrackerUI/TrackerDialog.h"
			"${SRCDIR}/Tracker/TrackerUI/TrackerDoc.h"
		#}
	)
END_SRC( TRACKERVGUI_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerVGUI
	EXECUTABLE WIN32
	PROPERTY_FOLDER "Tracker//Executables"
	INSTALL_DEST "${GAMEDIR}/platform/bin"
	INSTALL_OUTNAME "trackervgui"
	SOURCES ${TRACKERVGUI_SOURCE_FILES} ${TRACKERVGUI_HEADER_FILES}
)

target_include_directories(
	TrackerVGUI PRIVATE

	"${SRCDIR}/vgui2/vlocalize"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/vgui2/include"
	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/TrackerUI"
)

target_link_libraries(
	TrackerVGUI PRIVATE

	"$<${IS_WINDOWS}:odbc32>"
	"$<${IS_WINDOWS}:odbccp32>"
	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
	vgui_surfacelib
)
