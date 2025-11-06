cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( TRACKERUI_SOURCE_FILES )
BEGIN_SRC( TRACKERUI_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/GameUI/Random.cpp"
			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/Tracker/common/IceKey.cpp"

			"Buddy.cpp"
			"BuddyButton.cpp"
			"BuddySectionHeader.cpp"
			"BuildNum.cpp"
			"DialogAbout.cpp"
			"DialogAuthRequest.cpp"
			"DialogChat.cpp"
			"DialogFindBuddy.cpp"
			"DialogLoginUser.cpp"
			"DialogOptions.cpp"
			"DialogRemoveUser.cpp"
			"DialogUserInfo.cpp"
			"OnlineStatus.cpp"
			"OnlineStatusSelectComboBox.cpp"
			"PanelValveLogo.cpp"
			"ServerSession.cpp"
			"SubPanelBuddyList.cpp"
			"SubPanelConnectionIntro.cpp"
			"SubPanelConnectionTest.cpp"
			"SubPanelCreateUser.cpp"
			"SubPanelCreateUser2.cpp"
			"SubPanelCreateUser3.cpp"
			"SubPanelError.cpp"
			"SubPanelFindBuddy.cpp"
			"SubPanelFindBuddyComplete.cpp"
			"SubPanelFindBuddyRequestAuth.cpp"
			"SubPanelFindBuddyResults.cpp"
			"SubPanelLoginUser.cpp"
			"SubPanelLoginUser2.cpp"
			"SubPanelOptionsChat.cpp"
			"SubPanelOptionsConnection.cpp"
			"SubPanelOptionsPersonal.cpp"
			"SubPanelOptionsSounds.cpp"
			"SubPanelSelectLoginOption.cpp"
			"SubPanelTrackerState.cpp"
			"SubPanelUserInfoDetails.cpp"
			"SubPanelUserInfoStatus.cpp"
			"Tracker.cpp"
			"TrackerDialog.cpp"
			"TrackerDoc.cpp"
			"TrackerRunGameEngine.cpp"
			"TrackerUIVGuiModule.cpp"
			"TrackerUser.cpp"
		#}
	)
END_SRC( TRACKERUI_SOURCE_FILES "Source Files" )

set( TRACKERUI_HEADER_FILES )
BEGIN_SRC( TRACKERUI_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/info_key.h"
			"${SRCDIR}/Tracker/Public/ITrackerUser.h"
			"${SRCDIR}/public/vstdlib/random.h"
			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/Tracker/common/TrackerProtocol.h"

			"Buddy.h"
			"BuddyButton.h"
			"BuddySectionHeader.h"
			"DialogAbout.h"
			"DialogAuthRequest.h"
			"DialogChat.h"
			"DialogFindBuddy.h"
			"DialogLoginUser.h"
			"DialogOptions.h"
			"DialogRemoveUser.h"
			"DialogUserInfo.h"
			"OnlineStatus.h"
			"OnlineStatusSelectComboBox.h"
			"PanelValveLogo.h"
			"ServerSession.h"
			"SubPanelBuddyList.h"
			"SubPanelConnectionIntro.h"
			"SubPanelConnectionTest.h"
			"SubPanelCreateUser.h"
			"SubPanelCreateUser2.h"
			"SubPanelCreateUser3.h"
			"SubPanelError.h"
			"SubPanelFindBuddy.h"
			"SubPanelFindBuddyComplete.h"
			"SubPanelFindBuddyRequestAuth.h"
			"SubPanelFindBuddyResults.h"
			"SubPanelLoginUser.h"
			"SubPanelLoginUser2.h"
			"SubPanelOptionsChat.h"
			"SubPanelOptionsConnection.h"
			"SubPanelOptionsPersonal.h"
			"SubPanelOptionsSounds.h"
			"SubPanelSelectLoginOption.h"
			"SubPanelUserInfoDetails.h"
			"SubPanelUserInfoStatus.h"
			#"SystemBuddyButton.h"
			"Tracker.h"
			"TrackerDialog.h"
			"TrackerDoc.h"
		#}
	)
END_SRC( TRACKERUI_HEADER_FILES "Header Files" )

add_object(
	TARGET TrackerUI
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/platform/friends"
	SOURCES ${TRACKERUI_SOURCE_FILES} ${TRACKERUI_HEADER_FILES}
)

target_include_directories(
	TrackerUI PRIVATE

	"${SRCDIR}/vgui2/include"
	"${SRCDIR}/vgui2/controls"
	"${SRCDIR}/Tracker/common"
	"${SRCDIR}/Tracker/Public"
)

target_compile_definitions(
	TrackerUI PRIVATE

	TRACKERUI_EXPORTS
)

target_link_libraries(
	TrackerUI PRIVATE

	vgui_controls
	vgui_surfacelib
)

