//=========== (C) Copyright 2000 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//=============================================================================

#ifndef TRACKERUSER_H
#define TRACKERUSER_H
#pragma once

//-----------------------------------------------------------------------------
// Purpose: ITrackerUser interface
//-----------------------------------------------------------------------------
class ITrackerUser
{
public:
	virtual bool IsValid();

	// returns the tracker userID of the current user
	virtual int GetTrackerID();

	// returns information about a user
	virtual const char* GetUserName(int userID);
	virtual const char* GetFirstName(int userID);
	virtual const char* GetLastName(int userID);
	virtual const char* GetEmail(int userID);

	// returns true if friendID is a friend of the current user 
	// ie. the current is authorized to see when the friend is online
	virtual bool IsFriend(int friendID);

	// requests authorization from a user
	virtual void RequestAuthorizationFromUser(int potentialFriendID);

	// returns the status of the user, > 0 is online, 4 is ingame
	virtual int GetUserStatus(int friendID);

	// gets the IP address of the server the user is on, returns false if couldn't get
	virtual bool GetUserGameAddress(int friendID, int* ip, int* port);

	// returns the number of friends, so we can iterate
	virtual int GetNumberOfFriends();

	// returns the userID of a friend - friendIndex is valid in the range [0, GetNumberOfFriends)
	virtual int GetFriendTrackerID(int friendIndex);

	// sets whether or not the user can receive messages at this time
	// messages will be queued until this is set to true
	virtual void SetCanReceiveMessages(bool state);
};

#define TRACKERUSER_INTERFACE_VERSION "TrackerUser001"

#endif // TRACKERUSER_H
