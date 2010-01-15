// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// tavsrcmtupdater.h
//

#ifndef TAVSRCMTUPDATER_H
#define TAVSRCMTUPDATER_H

#include <remconmediainformationtargetobserver.h>
#include <playerinformationtargetobserver.h>
#include <remcondatabaseunawarenowplayingtargetobserver.h>
#include <remcondatabaseunawarenowplayingbrowseobserver.h>
#include <remcondatabaseunawaremedialibrarybrowseobserver.h>
#include <remcon/avrcpspec.h>
#include "mediainformation.h"

_LIT(KAVRCPSettingsResourceFilename, "z:\\bluetooth\\avrcp\\avrcp-settings.rsc");
const TInt KDefaultLength = 120000;
const TInt KPositionIncrement = 5000;

class CStopBrowseWatcher
	: public CActive
	{
public:
	static CStopBrowseWatcher* NewL();
	~CStopBrowseWatcher();
	
	void Start();
	void Complete();
	
private:
	CStopBrowseWatcher();
	void ConstructL();
	
private: // from CActive
	void RunL();
	void DoCancel();

private:
	RThread iThread;
	};

class CActiveCallBackConsole;
class CRemConDatabaseUnawareMediaBrowseTarget;
class MRemConDatabaseUnawareMediaLibraryBrowse;
class MRemConDatabaseUnawareNowPlayingBrowse;
class CRemConDatabaseUnawareNowPlayingTarget;
class CTavsrcMtUpdater : public CBase, public MActiveConsoleNotify, 
	public MPlayerApplicationSettingsNotify, 
	public MRemConDatabaseUnawareMediaLibraryBrowseObserver,
	public MRemConDatabaseUnawareNowPlayingBrowseObserver,
	public MRemConDatabaseUnawareNowPlayingTargetObserver
	{
public:
	static CTavsrcMtUpdater* NewL(CRemConInterfaceSelector& aIfSel, TUint aRemConInterfaces);
	~CTavsrcMtUpdater();
	
private:
	CTavsrcMtUpdater();
	void ConstructL(CRemConInterfaceSelector& aIfSel, TUint aRemConInterfaces);
	
	void KeyPressed(TChar aKey);
	void MtMenu();
	
	void ReadAVRCPSettingsFile(const TDesC& aResourceFileName);
	
	void AddEvent(TRegisterNotificationEvent aEvent);
	void AddCompanyId(TInt aCompanyId);
	void TrackReachedEnd();
	void TrackReachedStart();
	void PlackbackStatusChanged(MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus);
	void TrackChanged(TUint64 aIndex, TUint32 aLengthInMilliseconds);
	void SetBatteryStatus(MPlayerEventsObserver::TTargetBatteryStatus aStatus);
	void SetPlaybackPosition(TUint32 aPosition);
	void SetAsActivePlayer();
	
	MPlayerEventsObserver::TPlaybackStatus NextPlaybackStatus();
	MPlayerEventsObserver::TTargetBatteryStatus NextBatteryStatus();

private:
	// from MPlayerApplicationSettingsNotify
	void MpasnSetPlayerApplicationValueL(const RArray<TInt>& aAttributeID, const RArray<TInt>& aAttributeValue);
	
	// from MRemConDatabaseUnawareNowPlayingTargetObserver
	void MrcdunptoPlayItem(const TRemConItemUid& aItem, TRemConFolderScope aScope);
	void MrcdunptoAddToNowPlaying(const TRemConItemUid& aItem, TRemConFolderScope aScope);
	
	// from MRemConDatabaseUnawareMediaLibraryBrowseObserver
	void MrcdumlboGetFolderListing(TRemConFolderScope aScope, TUint aStartItem, TUint aEndItem);
	void MrcdumlboFolderUp();
	void MrcdumlboFolderDown(const TRemConItemUid& aFolder);
	void MrcdumlboGetPath(RPointerArray<HBufC8>& aPath);
	void MrcdumlboSearch(const TDesC8& aSearch);
	TInt MrcdumlboGetItem(TRemConFolderScope aScope,
				const TRemConItemUid& aItemId, 
				TMediaAttributeIter& aIter);
	
	// from MRemConDatabaseUnawareNowPlayingBrowseObserver
	void MrcdunpboGetFolderListing(TUint aStartItem, TUint aEndItem);
	TInt MrcdunpboGetItem(const TRemConItemUid& aItemId, 
			TMediaAttributeIter& aIter);
	
	void DisplayCurrentState();
	void SetAttributeL(REAResponse& aAttribute, TRemConItemUid& aUid);
	
private: // Utility functions
	TInt PrepareItemDataL(TMediaAttributeIter& aIter,
			const TRemConItemUid& aItemId,
			HBufC8* &aItemName,
			RArray<TMediaElementAttribute>& aItemAttributes);
	
private: // second thread functions
	static TInt MediaBrowseThread(TAny* aPtr);
	void BrowseSetupL(CRemConInterfaceSelector& aIfSel);
	void BrowseCleanup();
	
	static TInt BrowseKeyPressed(TAny* aPtr, TChar aKey);
	void BrowseKeyPressedL(TChar aKey);
	void BrowseMenu();
	
private:
	CPlayerInfoTarget* iPlayerInformation;
	MPlayerCapabilitiesObserver* iPlayerCapabilitiesObserver;
	MPlayerApplicationSettingsObserver* iPlayerApplicationSettingsObserver;
	MPlayerEventsObserver* iPlayerEventsObserver;
	
	CRemConDatabaseUnawareMediaBrowseTarget*		iMediaBrowse;
	MRemConDatabaseUnawareMediaLibraryBrowse*		iMediaBrowseInterface;
	MRemConDatabaseUnawareNowPlayingBrowse*			iNowPlayingBrowseInterface;
	CRemConDatabaseUnawareNowPlayingTarget*			iNowPlaying;
	
	CActiveConsole* iMtUpdaterConsole;
	CActiveCallBackConsole*	iMtBrowseConsole;
	
	TInt iIndex;
	MPlayerEventsObserver::TPlaybackStatus iPlaybackStatus;
	MPlayerEventsObserver::TTargetBatteryStatus iBatteryStatus;
	TUint32 iPosition;
	TUint32 iLength;
	
	
	RArray<TInt> iNotificationEvents;
	RArray<TInt> iCompanyIds;
	
	TInt iFolderDepth;
	
	TUint iUidCounter;
	
	TBool iMediaBrowseThreadOpen;
	RThread iMediaBrowseThread;
	CStopBrowseWatcher* iMtBrowseStopper;
	};

#endif //TAVSRCMTUPDATER_H
