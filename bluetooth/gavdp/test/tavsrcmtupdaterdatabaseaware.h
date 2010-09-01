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
#include <remcondatabaseawarenowplayingtargetobserver.h>
#include <remcondatabaseawarenowplayingbrowseobserver.h>
#include <remcondatabaseawaremedialibrarybrowseobserver.h>
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
class CRemConDatabaseAwareNowPlayingTarget;
class CRemConDatabaseAwareMediaBrowseTarget;
class MRemConDatabaseAwareMediaLibraryBrowse;
class MRemConDatabaseAwareNowPlayingBrowse;
class CTavsrcMtUpdater : public CBase, public MActiveConsoleNotify, 
	public MPlayerApplicationSettingsNotify, 
	public MRemConDatabaseAwareMediaLibraryBrowseObserver,
	public MRemConDatabaseAwareNowPlayingBrowseObserver,
	public MRemConDatabaseAwareNowPlayingTargetObserver
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
	
	// from MRemConDatabaseAwareNowPlayingTargetObserver
	void MrcdanptoPlayItem(const TRemConItemUid& aItem, TRemConFolderScope aScope, TUint16 aUidCounter);
	void MrcdanptoAddToNowPlaying(const TRemConItemUid& aItem, TRemConFolderScope aScope, TUint16 aUidCounter);
	
	// from MRemConDatabaseAwareMediaLibraryBrowseTargetObserver
	void MrcdamlboGetFolderListing(TRemConFolderScope aScope, TUint aStartItem, TUint aEndItem);
	void MrcdamlboFolderUp(TUint16 aMediaLibraryStateCookie);
	void MrcdamlboFolderDown(const TRemConItemUid& aFolder, TUint16 aMediaLibraryStateCookie);
	void MrcdamlboGetPath(RPointerArray<HBufC8>& aPath);
	void MrcdamlboSearch(const TDesC8& aSearch);
	TInt MrcdamlboGetItem(TRemConFolderScope aScope,
				const TRemConItemUid& aItemId, 
				TMediaAttributeIter& aIter, 
				TUint16 aMediaLibraryStateCookie);
	
	// from MRemConDatabaseAwareNowPlayingBrowseObserver
	void MrcdanpboGetFolderListing(TUint aStartItem, TUint aEndItem);
	TInt MrcdanpboGetItem(const TRemConItemUid& aItemId, 
				TMediaAttributeIter& aIter, 
				TUint16 aMediaLibraryStateCookie);
	
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
	
	CRemConDatabaseAwareMediaBrowseTarget*			iMediaBrowse;
	MRemConDatabaseAwareMediaLibraryBrowse*			iMediaBrowseInterface;
	MRemConDatabaseAwareNowPlayingBrowse*			iNowPlayingBrowseInterface;
	CRemConDatabaseAwareNowPlayingTarget*			iNowPlaying;
	
	CActiveConsole*	iMtUpdaterConsole;
	
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
