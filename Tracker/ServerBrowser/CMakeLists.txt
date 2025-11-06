cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( SERVERBROWSER_SOURCE_FILES )
BEGIN_SRC( SERVERBROWSER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/GameUI/Random.cpp"
			"${SRCDIR}/common/info_key.cpp"

			"${SRCDIR}/public/interface.cpp"
			"${SRCDIR}/public/tier0/memoverride.cpp"
			"${SRCDIR}/public/utlbuffer.cpp"
			"${SRCDIR}/public/utlsymbol.cpp"

			"${SRCDIR}/Tracker/common/msgbuffer.cpp"
			"${SRCDIR}/Tracker/common/netapi.cpp"
			"${SRCDIR}/Tracker/common/Socket.cpp"
			"${SRCDIR}/Tracker/common/util.cpp"

			"BaseGamesPage.cpp"
			"DialogAddServer.cpp"
			"DialogGameInfo.cpp"
			"DialogServerPassword.cpp"
			"FavoriteGames.cpp"
			"FriendsGames.cpp"
			"InternetGames.cpp"
			"LanBroadcastMsgHandler.cpp"
			"LanGames.cpp"
			"MasterMsgHandler.cpp"
			"ModList.cpp"
			"ServerBrowser.cpp"
			"ServerBrowserDialog.cpp"
			"ServerContextMenu.cpp"
			"ServerList.cpp"
			"ServerListCompare.cpp"
			"ServerMsgHandlerDetails.cpp"
			"SpectateGames.cpp"
		#}
	)
END_SRC( SERVERBROWSER_SOURCE_FILES "Source Files" )

set( SERVERBROWSER_HEADER_FILES )
BEGIN_SRC( SERVERBROWSER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			"${SRCDIR}/common/info_key.h"
			"${SRCDIR}/common/ServerBrowser/IServerBrowser.h"

			"${SRCDIR}/public/interface.h"
			"${SRCDIR}/public/utlbuffer.h"
			"${SRCDIR}/public/utllinkedlist.h"

			"${SRCDIR}/Tracker/common/IGameList.h"
			"${SRCDIR}/Tracker/common/inetapi.h"
			"${SRCDIR}/Tracker/common/IServerRefreshResponse.h"
			"${SRCDIR}/Tracker/common/MasterMsgHandler.h"
			"${SRCDIR}/Tracker/common/msgbuffer.h"
			"${SRCDIR}/Tracker/common/server.h"
			"${SRCDIR}/Tracker/common/Socket.h"
			"${SRCDIR}/Tracker/common/util.h"

			"BaseGamesPage.h"
			"DialogAddServer.h"
			"DialogGameInfo.h"
			"DialogServerPassword.h"
			"FavoriteGames.h"
			"FriendsGames.h"
			"InternetGames.h"
			"LanBroadcastMsgHandler.h"
			"LanGames.h"
			"ModList.h"
			"ServerBrowser.h"
			"ServerBrowserDialog.h"
			"ServerContextMenu.h"
			"ServerList.h"
			"ServerListCompare.h"
			"ServerMsgHandlerDetails.h"
			"SpectateGames.h"
		#}
	)
END_SRC( SERVERBROWSER_HEADER_FILES "Header Files" )

add_object(
	TARGET ServerBrowser
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/platform/servers"
	SOURCES ${SERVERBROWSER_SOURCE_FILES} ${SERVERBROWSER_HEADER_FILES}
)

target_include_directories(
	ServerBrowser PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"
)

target_compile_definitions(
	ServerBrowser PRIVATE

	SERVERBROWSER_EXPORTS
)

target_link_libraries(
	ServerBrowser PRIVATE

	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
)

