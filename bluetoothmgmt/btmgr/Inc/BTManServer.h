// Copyright (c) 1999-2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @internalAll
*/

#ifndef BTManServer_H
#define BTManServer_H

#include <bt_subscribe.h>
#include "BTRegistryDB.h"
#include "btmanclientserver.h"
#include "btmanserverburmgr.h"

#if KMaxBluetoothNameLen > KMaxFriendlyNameLen
#define KLongestName KMaxBluetoothNameLen
#else
#define KLongestName KMaxFriendlyNameLen
#endif

class CBTSecMan;
class CBTRegistryHelper;
class CBTManSession;
class CBTManSubSession;

/**
BTMan Server Shutdown Timer.
Closes the BTMan server after a preset delay. Activated before the first client connects
and after the last client disconnects.
**/
NONSHARABLE_CLASS(CBTManServerShutdown) : public CTimer
	{
	enum {KBTManServerShutdownDelay=5000000};	// 5s
public:
	inline CBTManServerShutdown();
	inline void ConstructL();
	inline void Start();
private:
	void RunL();
	};

void PanicServer(TInt aPanic);
void PanicClient(const RMessage2& aMessage, TInt aPanic, CBTManSession* aSession);

//**********************************
//MBTManHelper
//**********************************
/**
Helper interface class.
Defines interface to allow CBTManMessage to use SecMan helper objects.
**/
class MBTManHelper
	{
public:
	virtual void Delete() = 0;
	virtual const RMessage2& Message() = 0;
	};

//**********************************
//CBTManMessage
//**********************************
/**
RMessage2 container class.
Ties together client requests and helper objects dealing with them.
Messages must be completed via this class.
**/
NONSHARABLE_CLASS(CBTManMessage) : public CBase
	{
public:
	static CBTManMessage* NewL(const RMessage2& aMessage);
	~CBTManMessage();
	void SetHelper(MBTManHelper* aHelper);
	void RemoveHelper();
	void Complete(TInt aReason);
	const RMessage2& Message();
	const TAny* CancelPtr();
	MBTManHelper* Helper();
	TBool operator==(CBTManMessage& aMessage) const;
private:
	CBTManMessage(const RMessage2& aMessage);
	void ConstructL();
private:
	RMessage2	iMessage;	//<The message to be completed
	const TAny* iCancelPtr;	//<Address of the TRequestStatus of the client-side active object - used for cancelling purposes
	MBTManHelper* iHelper;	//<The helper associated with the message
	};


//**********************************
//CBTManServer
//**********************************
/**
The BTMan Server.
**/
NONSHARABLE_CLASS(CBTManServer) : public CPolicyServer, public MBTBURNotify
	{
public:
	//construct / destruct
	static CServer2* NewLC();
	~CBTManServer();

	// From MBTBURNotify
	virtual void BUROperationStarted();
	virtual void BUROperationStopped();

	void AddSession();
	void DropSession();

	inline TInt MaxSessionCount() const;
	inline TInt SessionCount() const;
	
	//return an object container. Will produce containers with unique ids within the server
	CObjectCon* NewContainerL();
	void DeleteContainer(CObjectCon* aCon);

	inline CBTRegistry& Registry() const;

	void Publish(TUint aKey, TInt aValue); // only publish ints for now, ignore error
	void NotifyViewChange(CBTManSubSession& aSubSessionViewOwner, const TDesC& aViewDescriptor);
	void NotifyViewChange(const TDesC& aViewDescriptor);

private:
	CBTManServer();
	void ConstructL();
	//open/close a session
	CSession2* NewSessionL(const TVersion& aVersion,const RMessage2& aMessage) const;
	void TryToStartShutdownTimer();

private:
	TInt					iMaxSessionCount;
	TInt					iSessionCount;	//<The number of sessions
	CBTManServerShutdown	iShutdown;//<A timer used to shut the server down after all clients have exited.
	CObjectConIx*			iContainerIndex;//<The server has an onject container index which creates an object container for each session
	CBTRegistry*			iRegistry;
	RProperty				iProperty;	// so that subsessions can publish change info
	CBTManServerBURMgr*		iBURManager;	// Manage backup and restore events generated by the Secure Backup Engine

	TBool			iBUROperationStarted;	// Whether or not a backup or restore operation is in progress (meaning that this server should not shutdown)
	};

inline TInt CBTManServer::MaxSessionCount() const {return iMaxSessionCount;}
inline TInt CBTManServer::SessionCount() const {return iSessionCount;}
inline CBTRegistry& CBTManServer::Registry() const {return *iRegistry;}

//
//**********************************
//CBTManSession
//**********************************
/**
BTMan Server Session.
General purpose session used to create functionally specific subsessions.
**/
NONSHARABLE_CLASS(CBTManSession) : public CSession2
	{
private:
	enum TSubSessionType{EHostResolver, ERegistry, ELocalDevice, ECommPortSettings,ESecuritySettingsB};
public:
	CBTManSession(CBTRegistry& aRegistry, const RMessage2& aMessage);
	void CreateL();
	void ConstructL();
	~CBTManSession();

	//Create/delete subsession
	void NewSubSessionL(TSubSessionType aType, const RMessage2 &aMessage);
	void CloseSubSession(const RMessage2 &aMessage);
	void DeleteSubsession(TInt aHandle, const RMessage2 &aMessage); //can't fail - can panic client
	CBTManSubSession* SubSessionFromHandle(TInt aHandle);
	CBTManServer& Server();

	void ServiceL(const RMessage2 &aMessage);
	TInt HandleError(TInt aError, const RMessage2 &aMessage);
	TBool DispatchSessMessageL(const RMessage2 &aMessage);
	void DispatchSubSessMessageL(const RMessage2 &aMessage);

	void CompleteMessage(const RMessage2& aMessage, TInt aReason);
	void CompleteMessage(MBTManHelper* aHelper, TInt aReason);
	void CreateBTManMessageL(const RMessage2& aMessage);
	CBTManMessage* FindMessage(const RMessage2& aMessage);
	CBTManMessage* FindMessage(MBTManHelper* aHelper);
	void DeleteMessage(CBTManMessage* aMessage);

	void CancelRequest(const RMessage2& aMessage);

	TBool SubSessionHasOverlappingView(CBTManSubSession& aSubSessionViewOwner, const TDesC& aViewDescriptor);
	TBool SubSessionHasOverlappingView(const TDesC& aViewDescriptor);

private:
	void DoCompleteMessage(CBTManMessage& aMessage, TInt aReason);
	void CompleteOutstandingMessages();
	void ClientRegChangeNotification(TAny* aPtr1);

private:
	CObjectCon*					iContainer;		//<object container for this session
	CObjectIx*					iSubSessions;	//<object index which stores subsessions
	TInt						iResourceCount;		//<total number of resources allocated
	CBTRegistry&				iRegistry;

	CArrayPtr<CBTManMessage>*	iMessageArray;	//array of outstanding messages
	RMessage2 iCurrentMessage;
	TBool	  					iClientPanic;   //Flag to indicate if ClientPanic'd when session closed

	};


//**********************************
// CBTManSubSession
//**********************************
/**
BTMan Subsession base class.
**/
NONSHARABLE_CLASS(CBTManSubSession) : public CObject
	{
public:
	virtual void Cleanup(TInt aError)=0;
	virtual TBool IsOverlappingView(const TDesC& aViewDescriptor);
	virtual void SetViewChangeNotificationMessage(const RMessage2& aMessage);

protected:
	CBTManSubSession(CBTManSession& aSession, CBTRegistry& aRegistry);
	void NotifyChange(TUint aTableChanged);
	void NotifyChange(TUint aTableChanged, CBTManSubSession& aSubSessionViewOwner, const TDesC& aViewDescriptor);

protected:
	CBTManSession&	iSession;	// session owning us
	CBTRegistry&	iRegistry;	// the class that shields us from DBMS

	RMessage2       iViewChangeNotificationMessage; // not all subsession types may use this
	};

//**********************************
// CBTRegistrySubSession
//**********************************
/**
Registry Subsession.
Represents (server-side) remote Bluetooth devices
**/
NONSHARABLE_CLASS(CBTRegistrySubSession): public CBTManSubSession
	{
public:
	static CBTRegistrySubSession* NewL(CBTManSession& aSession, CBTRegistry& aRegistry);
	~CBTRegistrySubSession();

	void PreLoadL(const RMessage2& aMessage);
	void GetDeviceL(const TBTNamelessDevice& aDevice, const RMessage2& aMessage);
	void AddDeviceL(const CBTDevice& aDetails, const RMessage2& aMessage);
	void ModifyL(const CBTDevice& aDetails, const RMessage2& aMessage);
	void ModifyL(const TBTNamelessDevice& aNewDetails, const RMessage2& aMessage);
	void ModifyNameL(const TBTDevAddr& aAddress, const RMessage2& aMessage);
	void OpenViewL(const TBTRegistrySearch& aSearch, const RMessage2& aMessage);
	void UnpairL(const TBTDevAddr& aAddress, const RMessage2& aMessage);
	void UnpairViewL(const RMessage2& aMessage);
	void RetrieveL(const RMessage2& aMessage);
	void DeleteViewL(const RMessage2& aMessage);
	void CloseView(const RMessage2& aMessage);
	void Cleanup(TInt aError);

private:
	void ConstructL();
	CBTRegistrySubSession(CBTManSession& aSession, CBTRegistry& aRegistry);
	void DoCloseView();
	void NotifyChange(CBTManSubSession& aSubSessionViewOwner, const TDesC& aViewDescriptor);

	virtual TBool IsOverlappingView(const TDesC& aViewDescriptor);
	virtual void SetViewChangeNotificationMessage(const RMessage2& aMessage);

private:
	RDbView*		iDBView;
	TInt			iViewCount;
	TDbBookmark		iBookmark;	// we look after our bookmark for registry to zip back to
	CBufFlat*		iResultBuffer; // for communicating back to client
	TInt			iNumDevicesInBuffer;

	// Each Registry subsession keeps one of these to then use for view spoil notifications.
	// When the server sees a subsession perform a writeable view operation it will give the changed 
	// subsession's precompiled query to all other subsessions, each of which will be asked to run 
	// the precompiled query against its own rowset row by row.
	// Any Match will be reported back and tells the server that a spoil has occurred
	
	HBufC* iUnderlyingSearch;

	};



//**********************************
// CBTLocalDeviceSubSession
//**********************************
/**
Registry Subsession.
Represents (server-side) local device details
**/
NONSHARABLE_CLASS(CBTLocalDeviceSubSession): public CBTManSubSession
	{
public:
	static CBTLocalDeviceSubSession* NewL(CBTManSession& aSession, CBTRegistry& aRegistry);
	~CBTLocalDeviceSubSession();

	void GetL(const RMessage2& aMessage);
	void UpdateL(const TBTLocalDevice& aLocalDevice, const RMessage2& aMessage);
	void Cleanup(TInt aError);
private:
	CBTLocalDeviceSubSession(CBTManSession& aSession, CBTRegistry& aRegistry);
	void ConstructL();
	};

//**********************************
// CBTCommPortSettingsSubSession
//**********************************
/**
Registry Subsession.
Represents (server-side) settings for virtual serial ports
**/
NONSHARABLE_CLASS(CBTCommPortSettingsSubSession): public CBTManSubSession
	{
public:
	static CBTCommPortSettingsSubSession* NewL(CBTManSession& aSession, CBTRegistry& aRegistry);
	~CBTCommPortSettingsSubSession();

	void GetL(const TBTCommPortSettings& aLocalDevice, const RMessage2& aMessage);
	void UpdateL(const TBTCommPortSettings& aLocalDevice, const RMessage2& aMessage);
	void DeleteL(const TBTCommPortSettings& aLocalDevice, const RMessage2& aMessage);
	void Cleanup(TInt aError);
private:
	CBTCommPortSettingsSubSession(CBTManSession& aSession, CBTRegistry& aRegistry);
	void ConstructL();	
	};


/*
* Panic definitions
*/
_LIT(KBTManPanic,"BTManServer");
// reasons for server panic
enum TBTManServerPanic
	{
	EBTManBadRequest,
	EBTManBadDescriptor,
	EBTManBadSubSessionHandle,
	EBTManBadSubSessionRemove,
	EBTManBadSubSessionType,
	EBTManBadHelper,
	EBTManBadBTManMessage,
	EBTManBadServiceArray,
	EBTManBadNotifierArray,
	EBTManBadHCIRequestRequest,
	EBTManTooManyDevicesInView,
	EBTManUnknownSeverity,
	EBTManClientShouldBeBusy,
	EBTManBadState,
	EBTManUnexpectedDbError,
	};



#endif

