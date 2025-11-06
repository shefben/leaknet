cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

set( ADMINSERVER_SOURCE_FILES )
BEGIN_SRC( ADMINSERVER_SOURCE_FILES "Source Files" )
	SRC_GRP(
		SOURCES
		#{
			"AdminServer.cpp"
			"ConfigPanel.cpp"
			"serverpage.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Utils"
		SOURCES
		#{
			#"../../common/Info.cpp"
			"../../public/interface.cpp"
			"../common/msgbuffer.cpp"
			"../TrackerNET/NetAddress.cpp"
			"../common/netapi.cpp"
			"../common/Socket.cpp"
			#"../../shared/TokenLine.cpp"
			"../common/util.cpp"
			"../../public/utlbuffer.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Generic Dialogs"
		SOURCES
		#{
			"DialogAddBan.cpp"
			"DialogAddServer.cpp"
			"DialogCvarChange.cpp"
			"DialogGameInfo.cpp"
			"DialogKickPlayer.cpp"
			"DialogServerPassword.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Favorites"
		SOURCES
		#{
			"BaseGamesPage.cpp"
			"FavoriteGames.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Manage"
		SOURCES
		#{
			"GamePanelInfo.cpp"
			"HelpText.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Manage//Menus"
		SOURCES
		#{
			"BanContextMenu.cpp"
			"RulesContextMenu.cpp"
			"ServerContextMenu.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Manage//Msg Handlers"
		SOURCES
		#{
			"logmsghandler.cpp"
			"MasterMsgHandler.cpp"
			"PlayerContextMenu.cpp"
			"playermsghandler.cpp"
			"rconmsghandler.cpp"
			"rulesinfomsghandler.cpp"
			"serverinfomsghandler.cpp"
			"serverpingmsghandler.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Manage//Panels"
		SOURCES
		#{
			"BanPanel.cpp"
			"ChatPanel.cpp"
			"GraphPanel.cpp"
			"MOTDPanel.cpp"
			"PlayerPanel.cpp"
			"RawLogPanel.cpp"
			"ServerConfigPanel.cpp"
		#}
	)

	SRC_GRP(
		SUBGROUP "Manage//Queries"
		SOURCES
		#{
			"banlist.cpp"
			"cmdlist.cpp"
			"mapslist.cpp"
			"playerlist.cpp"
			"PlayerListCompare.cpp"
			"rcon.cpp"
			"rulesinfo.cpp"
			"serverinfo.cpp"
			"ServerList.cpp"
			"ServerListCompare.cpp"
			"serverping.cpp"
		#}
	)
END_SRC( ADMINSERVER_SOURCE_FILES "Source Files" )

set( ADMINSERVER_HEADER_FILES )
BEGIN_SRC( ADMINSERVER_HEADER_FILES "Header Files" )
	SRC_GRP(
		SOURCES
		#{
			#"../../common/AdminServer/IAdminServer.h"

			#"../../common/info.h"
			"../../common/ServerBrowser/IServerBrowser.h"
			"../../public/interface.h"
			"../../public/utlbuffer.h"
			"../../public/utllinkedlist.h"
			#"../../shared/TokenLine.h"
			"../common/IGameList.h"
			"../common/inetapi.h"
			"../common/IServerRefreshResponse.h"
			"../common/MasterMsgHandler.h"
			"../common/msgbuffer.h"
			"../common/server.h"
			"../common/Socket.h"
			"../common/util.h"

			"AdminServer.h"
			"ban.h"
			"BanContextMenu.h"
			"banlist.h"
			"BanPanel.h"
			"BaseGamesPage.h"
			"ChatContextMenu.h"
			"ChatPanel.h"
			"cmdlist.h"
			"ConfigPanel.h"
			"DialogAddBan.h"
			"DialogAddServer.h"
			"DialogCvarChange.h"
			"DialogGameInfo.h"
			"DialogKickPlayer.h"
			"DialogServerPassword.h"
			"FavoriteGames.h"
			"GamePanelInfo.h"
			"GraphPanel.h"
			"HelpText.h"
			"Iresponse.h"
			"logmsghandler.h"
			"maps.h"
			"mapslist.h"
			"MOTDPanel.h"
			"Player.h"
			"PlayerContextMenu.h"
			"playerlist.h"
			"PlayerListCompare.h"
			"playermsghandler.h"
			"PlayerPanel.h"
			"point.h"
			"RawLogPanel.h"
			"rcon.h"
			"rconmsghandler.h"
			"RulesContextMenu.h"
			"rulesinfo.h"
			"rulesinfomsghandler.h"
			"ServerConfigPanel.h"
			"ServerContextMenu.h"
			"serverinfo.h"
			"serverinfomsghandler.h"
			"ServerList.h"
			"ServerListCompare.h"
			"serverpage.h"
			"serverping.h"
			"serverpingmsghandler.h"
		#}
	)
END_SRC( ADMINSERVER_HEADER_FILES "Header Files" )

add_object(
	TARGET AdminServer
	MODULE
	PROPERTY_FOLDER "Tracker//Shared Libs"
	INSTALL_DEST "${GAMEDIR}/platform/admin"
	SOURCES ${ADMINSERVER_SOURCE_FILES} ${ADMINSERVER_HEADER_FILES}
)

target_include_directories(
	AdminServer PRIVATE

	"${SRCDIR}/Tracker/Public"
	"${SRCDIR}/Tracker/common"

	"${SRCDIR}/public/vgui_controls"
	"${SRCDIR}/public/vgui"
)

target_compile_definitions(
	AdminServer PRIVATE

	ADMINSERVER_EXPORTS
)

target_link_libraries(
	AdminServer PRIVATE

	"$<${IS_WINDOWS}:wsock32>"

	vgui_controls
)

