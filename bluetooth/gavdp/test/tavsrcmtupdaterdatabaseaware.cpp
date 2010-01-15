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
//

#include <e32property.h>
#include <f32file.h>
#include <e32debug.h>
#include <barsc2.h>
#include <remcondatabaseawarenowplayingtarget.h>
#include <remcondatabaseawaremediabrowsetarget.h>
#include <remcondatabaseawaremedialibrarybrowse.h>
#include <remcondatabaseawarenowplayingbrowse.h>
#include <remconmediaerror.h>
#include <reftsp/reftspactiveplayerobserver.h>

#include "browsingframe.h"

#include "tavsrc.h"
#include "tavsrcmtupdaterdatabaseaware.h"
#include "activecallbackconsole.h"

#ifdef __WINS__
GLDEF_D TSize gUpdaterConsole(75,30);
GLDEF_D TSize gBrowseConsole(75,30);
#else
GLDEF_D TSize gUpdaterConsole(KConsFullScreen,KConsFullScreen);
GLDEF_D TSize gBrowseConsole(KConsFullScreen,KConsFullScreen);
#endif


#define BROWSE_CONSOLE iMtBrowseConsole

const TRemConItemUid KUidMarker = 0xc0ffeeee00000000ull;
const TRemConItemUid KFolderMarker = 0x0000000100000000ull;
const TInt KNumberFolderItemsInFolder = 1;
const TInt KNumberMediaItemsInFolder = 1502;
const TInt KNumberItemsInFolder = KNumberMediaItemsInFolder + KNumberFolderItemsInFolder; 
const TInt KMaxFolderDepth = 10;

_LIT8(KFolderName, "Folder%08x");

CTavsrcMtUpdater* CTavsrcMtUpdater::NewL(CRemConInterfaceSelector& aIfSel, TUint aRemConInterfaces)
	{
	CTavsrcMtUpdater* mt = new(ELeave)CTavsrcMtUpdater();
	CleanupStack::PushL(mt);
	mt->ConstructL(aIfSel, aRemConInterfaces);
	CleanupStack::Pop();
	return mt;
	}

struct TMediaBrowseThreadParams
	{
	CRemConInterfaceSelector*	iIfSel;
	CTavsrcMtUpdater*			iUpdater;
	};

TInt CTavsrcMtUpdater::MediaBrowseThread(TAny* aPtr)
	{
	TMediaBrowseThreadParams* params = reinterpret_cast<TMediaBrowseThreadParams*>(aPtr);
	
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	CActiveScheduler* activescheduler = new CActiveScheduler;
	CActiveScheduler::Install(activescheduler);
	
	TInt err = KErrNoMemory;
	if(cleanupStack && activescheduler)
		{
		CTavsrcMtUpdater* self = params->iUpdater;
		TRAP(err, self->BrowseSetupL(*(params->iIfSel)))
		RThread().Rendezvous(err);
		if(err == KErrNone)
			{
			CActiveScheduler::Start();
			}
		self->BrowseCleanup();
		}

	delete activescheduler;
	delete cleanupStack;

	return err;
	}

void CTavsrcMtUpdater::BrowseSetupL(CRemConInterfaceSelector& aIfSel)
	{
	iMtBrowseConsole = CActiveCallBackConsole::NewL(BrowseKeyPressed, this, _L("MtBrowse"),gBrowseConsole);
	BrowseMenu();
	
	iMtBrowseStopper = CStopBrowseWatcher::NewL();
	iMediaBrowse = CRemConDatabaseAwareMediaBrowseTarget::NewL(aIfSel, 
			*this, *this, EFalse, iMediaBrowseInterface, 
			iNowPlayingBrowseInterface,
			iUidCounter);
	iMtBrowseStopper->Start();
	}

void CTavsrcMtUpdater::BrowseCleanup()
	{
	delete iMtBrowseStopper; iMtBrowseStopper = NULL;
	delete iMtBrowseConsole; iMtBrowseConsole = NULL;
	}


void CTavsrcMtUpdater::ConstructL(CRemConInterfaceSelector& aIfSel, TUint aRemConInterfaces)
	{ 
	if (aRemConInterfaces & EPlayerInformation)
		{
		iPlayerInformation = CPlayerInfoTarget::NewL(aIfSel,
											iPlayerCapabilitiesObserver,
											iPlayerApplicationSettingsObserver,
											iPlayerEventsObserver,
											*this);
		}
	
	if (aRemConInterfaces & ENowPlaying)
		{
		iNowPlaying = CRemConDatabaseAwareNowPlayingTarget::NewL(aIfSel, *this);
		}
	
	if (aRemConInterfaces & EMediaBrowse)
		{
		// Create media browse thread (and wait until it is running)...
		TMediaBrowseThreadParams params;
		params.iIfSel = &aIfSel;
		params.iUpdater = this;
		TRequestStatus status;
		User::LeaveIfError(iMediaBrowseThread.Create(KNullDesC, CTavsrcMtUpdater::MediaBrowseThread, KDefaultStackSize, NULL, &params));
		iMediaBrowseThread.Rendezvous(status);
		iMediaBrowseThread.Resume();
		User::WaitForRequest(status);
		User::LeaveIfError(status.Int());
		iMediaBrowseThreadOpen = ETrue;
		}
	
	iMtUpdaterConsole = CActiveConsole::NewL(*this,_L("MtUpdater"),gUpdaterConsole);
	
			
	// configure the PlayerInformation from resource file	
	ReadAVRCPSettingsFile(KAVRCPSettingsResourceFilename);
	
	// We don't need to call AddEvent() for PlaybackStatusChanged or TrackChanged
	// since these are mandatory events which must be supported. A KErrAlreadyExists
	// error is returned if these are called from here.
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationPlaybackStatusChanged);
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationTrackChanged);
	
	// configure the interface with the events supported by tavsrc
	// This cannot fail as we constructed the iNotificationEvents array to have a granularity sufficient to 
	// contain all events without having to grow
	User::LeaveIfError( iPlayerCapabilitiesObserver->AddEvent(ERegisterNotificationTrackReachedEnd));
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationTrackReachedEnd);
	
	User::LeaveIfError( iPlayerCapabilitiesObserver->AddEvent(ERegisterNotificationTrackReachedStart));
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationTrackReachedStart);
	
	User::LeaveIfError( iPlayerCapabilitiesObserver->AddEvent(ERegisterNotificationPlaybackPosChanged));
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationPlaybackPosChanged);
	
	User::LeaveIfError( iPlayerCapabilitiesObserver->AddEvent(ERegisterNotificationBatteryStatusChanged));
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationBatteryStatusChanged);
	
	User::LeaveIfError( iPlayerCapabilitiesObserver->AddEvent(ERegisterNotificationPlayerApplicationSettingChanged));
	(void)iNotificationEvents.InsertInOrder(ERegisterNotificationPlayerApplicationSettingChanged);
			
	// save a Company ID
	iPlayerCapabilitiesObserver->AddCompanyId(0x4321);
	iCompanyIds.InsertInOrderL(0x4321);

	// set the player events to some test values 
	iPlayerEventsObserver->TrackReachedEnd();
	 
	// playing 12, which is 2:30 long with a low battery
	iPlayerEventsObserver->PlaybackStatusChanged(MPlayerEventsObserver::EStopped);
	iPlaybackStatus = MPlayerEventsObserver::EStopped;
	
	iPlayerEventsObserver->TrackChanged(12, 150*1000);
	iLength = 150*1000;
	
	iPlayerEventsObserver->SetBatteryStatus(MPlayerEventsObserver::ECritical);
	iBatteryStatus = MPlayerEventsObserver::ECritical;

	// current position is 1 minute 10 secs though track 12
	iPlayerEventsObserver->SetPlaybackPosition(70000);	
	iPosition = 70000;
	
	_LIT_SECURITY_POLICY_PASS(KPassPolicy);
	TInt err = RProperty::Define(TUid::Uid(KRefTspProperty), KRefTspActivePlayer, RProperty::EInt, KPassPolicy, KPassPolicy);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		User::Leave(err);
		}

	MtMenu();
	}
	
CTavsrcMtUpdater::CTavsrcMtUpdater()
	: iNotificationEvents(ERegisterNotificationReservedLast),
	  iUidCounter(1)
	{
	}
	
CTavsrcMtUpdater::~CTavsrcMtUpdater()
	{
	delete iMtUpdaterConsole;
	iNotificationEvents.Close();
	iCompanyIds.Close();
	if(iMediaBrowseThreadOpen)
		{
		TRequestStatus status;
		iMediaBrowseThread.Logon(status);
		iMtBrowseStopper->Complete();
		User::WaitForRequest(status);
		// the browse thread should not be running now.
		}
	iMediaBrowseThread.Close();
	}
	
void CTavsrcMtUpdater::ReadAVRCPSettingsFile(const TDesC& aResourceFileName)
/**
 - Creates the resource reader.
 - Reads the default policy evaluator and dialog creator UIDs.
 - Reads the number of policies in the resource file.
 
@param aRFs					The file server session used by the resource parser.
@param aResourceFileName	The name of the AVRCP settings file to read.
*/
	{
	RFs rfs;
	rfs.Connect();
	RFile r;
	User::LeaveIfError(r.Open(rfs, aResourceFileName, EFileRead | EFileShareReadersOnly));
	CleanupClosePushL(r);
	TInt size;
	User::LeaveIfError(r.Size(size));
	CleanupStack::PopAndDestroy(&r);
	
	CResourceFile* resourceFile = CResourceFile::NewLC(rfs, aResourceFileName, 0, size);
	PlayerApplicationSettingsResourceInit::DefineAttributesL(*iPlayerApplicationSettingsObserver, *resourceFile);
	CleanupStack::PopAndDestroy(resourceFile);
	}

void CTavsrcMtUpdater::KeyPressed(TChar aKey)
	{
	
	switch(aKey)
		{
	case 'e':
		{
		AddEvent(ERegisterNotificationPlaybackPosChanged);
		iNotificationEvents.InsertInOrder(ERegisterNotificationPlaybackPosChanged);
		iMtUpdaterConsole->Console().Printf(_L("Add event EPlaybackPosChanged\n"));
		break;
		}
	case 'c':
		{
		AddCompanyId(0x5678);
		(void)iCompanyIds.InsertInOrder(0x5678);
		iMtUpdaterConsole->Console().Printf(_L("Add CompanyId 0x5678\n"));
		break;
		}
	case '1':
		{
		PlackbackStatusChanged(NextPlaybackStatus());
		iMtUpdaterConsole->Console().Printf(_L("Playback status changed %d\n"),iPlaybackStatus);
		break;
		}
	case '2':
		{
		TrackChanged(++iIndex, KDefaultLength);
		iMtUpdaterConsole->Console().Printf(_L("Track changed\n"));
		break;
		}
	case '3':
		{
		TrackReachedEnd();
		iMtUpdaterConsole->Console().Printf(_L("Track reached end\n"));
		break;
		}
	case '4':
		{
		TrackReachedStart();
		iMtUpdaterConsole->Console().Printf(_L("Track reached end\n"));
		break;
		}
	case '5':
		{
		iPosition += KPositionIncrement;
		SetPlaybackPosition(iPosition);
		iMtUpdaterConsole->Console().Printf(_L("Position %d\n"),iPosition );
		break;
	case '6':
		{
		SetBatteryStatus(NextBatteryStatus());
		iMtUpdaterConsole->Console().Printf(_L("BatteryStatus %d\n"),iBatteryStatus );
		break;
		}
		}
	case '8':
		{
		// change an application setting, 
		// increment the value if attribute ID no 1 for example
		// CPlayerInfoTarget
		iPlayerApplicationSettingsObserver->SetAttributeL(1, 2);
		iMtUpdaterConsole->Console().Printf(_L("Increment the value if attribute ID no 1\n") );
		break;
		}
	case '9':
		{
		// Update NowPlayingList 
		iNowPlaying->NowPlayingContentChanged();

		iMtUpdaterConsole->Console().Printf(_L("Now Playing List updated\n") );
		break;
		}
	case '0':
		{
		User::SafeInc(*reinterpret_cast<TInt*>(&iUidCounter)); // Might get dodgy when reaching 0x80000000
		iMediaBrowseInterface->MrcdamlbMediaLibraryStateChange(iUidCounter);
		iMtUpdaterConsole->Console().Printf(_L("UIDs changed: %d \n"), iUidCounter);
		break;
		}
	case 'a':
		{
		SetAsActivePlayer();
		break;
		}
	default:
		iMtUpdaterConsole->Console().Printf(_L("No such command\n"));
		break;
		};
		
	MtMenu();
	}
		
void CTavsrcMtUpdater::MtMenu()
	{
	DisplayCurrentState();
	
	iMtUpdaterConsole->Console().Printf(_L("e.\tAddEvent\n"));
	iMtUpdaterConsole->Console().Printf(_L("c.\tAddCompanyId\n"));
	iMtUpdaterConsole->Console().Printf(_L("1.\tPlaybackStatusChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("2.\tTrackChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("3.\tTrackReachedEnd\n"));
	iMtUpdaterConsole->Console().Printf(_L("4.\tTrackReachedStart\n"));
	iMtUpdaterConsole->Console().Printf(_L("5.\tPlaybackPositionChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("6.\tSetBatteryStatus\n"));
	iMtUpdaterConsole->Console().Printf(_L("8.\tAttributeChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("9.\tNowPlayingContentChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("0.\tUIDsChanged\n"));
	iMtUpdaterConsole->Console().Printf(_L("\n"));
	
	iMtUpdaterConsole->RequestKey();
	}

TInt CTavsrcMtUpdater::BrowseKeyPressed(TAny* aPtr, TChar aKey)
	{
	TRAPD(err, static_cast<CTavsrcMtUpdater*>(aPtr)->BrowseKeyPressedL(aKey));
	return err;
	}

void CTavsrcMtUpdater::BrowseKeyPressedL(TChar aKey)
	{
	switch(aKey)
		{
	case '0':
		{
		User::SafeInc(*reinterpret_cast<TInt*>(&iUidCounter)); // Might get dodgy when reaching 0x80000000
		iMediaBrowseInterface->MrcdamlbMediaLibraryStateChange(iUidCounter);
		BROWSE_CONSOLE->Console().Printf(_L("UIDs changed: %d \n"), iUidCounter);
		break;
		}
	default:
		BROWSE_CONSOLE->Console().Printf(_L("No such command\n"));
		break;
		};
		
	BrowseMenu();
	}
	
void CTavsrcMtUpdater::BrowseMenu()
	{
	BROWSE_CONSOLE->Console().Printf(_L("0.\tUIDsChanged\n"));
	BROWSE_CONSOLE->Console().Printf(_L("\n"));
	
	BROWSE_CONSOLE->RequestKey();
	}

void CTavsrcMtUpdater::AddEvent(TRegisterNotificationEvent aEvent)
	{
	iPlayerCapabilitiesObserver->AddEvent(aEvent);
	}
	
void CTavsrcMtUpdater::AddCompanyId(TInt aCompanyId)
	{
	// save a Company ID
	iPlayerCapabilitiesObserver->AddCompanyId(aCompanyId);
	}

void CTavsrcMtUpdater::TrackReachedEnd()
	{
	iPlayerEventsObserver->TrackReachedEnd();
	}
	
void CTavsrcMtUpdater::TrackReachedStart()
	{
	iPlayerEventsObserver->TrackReachedStart();
	}
	
void CTavsrcMtUpdater::PlackbackStatusChanged(MPlayerEventsObserver::TPlaybackStatus aPlaybackStatus)
	{ 
	iPlayerEventsObserver->PlaybackStatusChanged(aPlaybackStatus);
	}

void CTavsrcMtUpdater::TrackChanged(TUint64 aIndex, TUint32 aLengthInMilliseconds)
	{ 
	iPlayerEventsObserver->TrackChanged(aIndex, aLengthInMilliseconds);
	}
	
void CTavsrcMtUpdater::SetBatteryStatus(MPlayerEventsObserver::TTargetBatteryStatus aStatus)
	{
	iPlayerEventsObserver->SetBatteryStatus(aStatus);
	}

void CTavsrcMtUpdater::SetPlaybackPosition(TUint32 aPosition)
	{	
	iPlayerEventsObserver->SetPlaybackPosition(aPosition);
	}
	
void CTavsrcMtUpdater::SetAsActivePlayer()
	{
	TInt err = RProperty::Set(TUid::Uid(KRefTspProperty), KRefTspActivePlayer, RProcess().Id());
	iMtUpdaterConsole->Console().Printf(_L("Set as active player %d\n"), err);
	}

MPlayerEventsObserver::TPlaybackStatus CTavsrcMtUpdater::NextPlaybackStatus()
	{
	if(iPlaybackStatus == MPlayerEventsObserver::EStopped)
		{
		iPlaybackStatus = MPlayerEventsObserver::EPlaying;
		}
	else if(iPlaybackStatus == MPlayerEventsObserver::EPlaying)
		{
		iPlaybackStatus = MPlayerEventsObserver::EPaused;
		}
	else if(iPlaybackStatus == MPlayerEventsObserver::EPaused)
		{
		iPlaybackStatus = MPlayerEventsObserver::EFwdSeek;
		}
	else if(iPlaybackStatus == MPlayerEventsObserver::EFwdSeek)
		{
		iPlaybackStatus = MPlayerEventsObserver::ERevSeek;
		}
	else if(iPlaybackStatus == MPlayerEventsObserver::ERevSeek)
		{
		iPlaybackStatus = MPlayerEventsObserver::EStopped;
		}
		
	return iPlaybackStatus;
	}

MPlayerEventsObserver::TTargetBatteryStatus CTavsrcMtUpdater::NextBatteryStatus()
	{
	if(iBatteryStatus == MPlayerEventsObserver::ENormal)
		{
		iBatteryStatus = MPlayerEventsObserver::EWarning;
		}
	else if(iBatteryStatus == MPlayerEventsObserver::EWarning)
		{
		iBatteryStatus = MPlayerEventsObserver::ECritical;
		}
	else if(iBatteryStatus == MPlayerEventsObserver::ECritical)
		{
		iBatteryStatus = MPlayerEventsObserver::EExternal;
		}
	else if(iBatteryStatus == MPlayerEventsObserver::EExternal)
		{
		iBatteryStatus = MPlayerEventsObserver::EFullCharge;
		}
	else if(iBatteryStatus == MPlayerEventsObserver::EFullCharge)
		{
		iBatteryStatus = MPlayerEventsObserver::ENormal;
		}
		
	return iBatteryStatus;
	}

// from MPlayerApplicationSettingsNotify
void CTavsrcMtUpdater::MpasnSetPlayerApplicationValueL(const RArray<TInt>& aAttributeID, const RArray<TInt>& aAttributeValue)

	{
	for (TInt i=0; i < aAttributeID.Count(); i++)
		{
		iMtUpdaterConsole->Console().Printf(_L("SetPlayerApplication attribute:%d value:%d \n"),aAttributeID[i], aAttributeValue[i]);	
		}
	}

void CTavsrcMtUpdater::MrcdanptoPlayItem(const TRemConItemUid& aItem, TRemConFolderScope aScope, TUint16 aUidCounter)
	{
	iMtUpdaterConsole->Console().Printf(_L("* PlayItem %08x %08x\t scope %d remote uidcounter %d\n"), aItem>>32, aItem & 0xffffffff, aScope, aUidCounter);

	if(iUidCounter != aUidCounter)
		{
		iMtUpdaterConsole->Console().Printf(_L("remote uidcounter does not match local(%d)\n"), iUidCounter);
		iNowPlaying->PlayItemResponse(KErrInvalidMediaLibraryStateCookie);
		}
	else
		{
		iNowPlaying->PlayItemResponse(KErrNone);
		}
	}

void CTavsrcMtUpdater::MrcdanptoAddToNowPlaying(const TRemConItemUid& aItem, TRemConFolderScope aScope, TUint16 aUidCounter)
	{
	iMtUpdaterConsole->Console().Printf(_L("* AddToNowPlaying %08x%08x\t scope %d uidcounter %d\n"), aItem>>32, aItem, aScope, aUidCounter);

	if(iUidCounter != aUidCounter)
		{
		iMtUpdaterConsole->Console().Printf(_L("remote uidcounter does not match local(%d)\n"), iUidCounter);
		iNowPlaying->AddToNowPlayingResponse(KErrInvalidMediaLibraryStateCookie);
		}
	else
		{
		iNowPlaying->AddToNowPlayingResponse(KErrNone);
		}
	}

void CTavsrcMtUpdater::MrcdamlboGetFolderListing(TRemConFolderScope aScope, TUint aStartItem, TUint aEndItem)
	{
	BROWSE_CONSOLE->Console().Printf(_L("* GetFolderItems scope %d, start item %d, end item %d\n"), aScope, aStartItem, aEndItem);
	// FIXME handle scopes
	TInt err = aStartItem < KNumberItemsInFolder ? KErrNone : KErrMediaBrowseInvalidOffset;

	TInt numberItems = (iFolderDepth == KMaxFolderDepth) ? KNumberMediaItemsInFolder : KNumberItemsInFolder;
	TInt numberFolderItems = (iFolderDepth == KMaxFolderDepth) ? 0 : KNumberFolderItemsInFolder;
	
	RArray<TRemConItem> folderListing;
	if(!err)
		{
		for(TInt i = aStartItem; (i <= aEndItem) && (i < numberItems) && !err; i++)
			{
			// FIXME handle erro	
			TRemConItem item;
			item.iUid = static_cast<TRemConItemUid>(i) | KUidMarker;
			item.iType = ERemConMediaItem;
			if(i < numberFolderItems)
				{
				item.iUid = static_cast<TRemConItemUid>(i) | KFolderMarker;
				item.iType = ERemConFolderItem;
				}
			
			err = folderListing.Append(item);
			}
		}
	
	if(!err)
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Returning listing of %d items, current uid counter %d\n"), folderListing.Array().Count(), iUidCounter);	
		}
	else
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error %d getting folder listing"));	
		}
	iMediaBrowseInterface->MrcdamlbFolderListing(folderListing.Array(), iUidCounter, err);

	folderListing.Close();
	}

void CTavsrcMtUpdater::SetAttributeL(REAResponse& aAttribute, TRemConItemUid& aUid)
	{
	_LIT8(KTestTitle,     "Test Title 0x%08x%08x");
	aAttribute.iCharset = KUtf8MibEnum;

	switch(aAttribute.iAttributeId)
		{
		case ETitleOfMedia:
			{
			//buffer.Copy(KMediaTitle);
			aAttribute.iString = HBufC8::NewL(29);
			TPtr8 namePtr = aAttribute.iString->Des();
			namePtr.AppendFormat(KTestTitle, (aUid >> 32), aUid);
			aAttribute.iStringLen = namePtr.Length();
			break;
			}
			/*
		case ENameOfArtist:
			buffer.Copy(KArtistName);
			break;
		case ENameOfAlbum:
			buffer.Copy(KAlbumName);
			break;
		case ETrackNumber:
			buffer.Copy(KTrackNumber);
			break;
		case ENumberOfTracks:
			buffer.Copy(KNumberOfTracks);
			break;
		case EGenre:
			buffer.Copy(KGenre);
			break;
		case EPlayingTime:
			buffer.Copy(KPlayingTime);
			break;
			*/
		default:
		//	__DEBUGGER();
			User::Leave(KErrNotFound);
			break;
		}
	}

void CTavsrcMtUpdater::MrcdamlboFolderUp(TUint16 aMediaLibraryStateCookie)
	{
	TInt err = (aMediaLibraryStateCookie == iUidCounter) ? KErrNone : KErrInvalidMediaLibraryStateCookie;

	if(!err && --iFolderDepth < 0)
		{
		iFolderDepth = 0;
		err = KErrMediaBrowseNotADirectory;
		}
	
	if(!err)
		{
		BROWSE_CONSOLE->Console().Printf(_L("* Folder Up\n"));
		}
	else if(err == KErrInvalidMediaLibraryStateCookie)
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error: remote uidcounter (%d) does not match local(%d)\n"), aMediaLibraryStateCookie, iUidCounter);
		}
	else
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error: %d\n"), err);
		}
	iMediaBrowseInterface->MrcdamlbFolderUpResult(KNumberItemsInFolder, err);
	}

void CTavsrcMtUpdater::MrcdamlboFolderDown(const TRemConItemUid& aFolder, TUint16 aMediaLibraryStateCookie)
	{
	TInt err = (aMediaLibraryStateCookie == iUidCounter) ? KErrNone : KErrInvalidMediaLibraryStateCookie;
	
	if(!(aFolder & KFolderMarker))
		{
		err = KErrMediaBrowseNotADirectory;
		}
	else if(!err && ++iFolderDepth > KMaxFolderDepth)
		{
		iFolderDepth = KMaxFolderDepth;
		err = KErrNotSupported;
		}
	
	if(!err)
		{
		BROWSE_CONSOLE->Console().Printf(_L("* Folder down %08x %08x\n"), aFolder >> 32, aFolder);
		}
	else if(err == KErrInvalidMediaLibraryStateCookie)
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error: remote uidcounter (%d) does not match local(%d)\n"), aMediaLibraryStateCookie, iUidCounter);
		}
	else
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error: %d\n"), err);
		}
	iMediaBrowseInterface->MrcdamlbFolderDownResult(KNumberItemsInFolder, err);
	}

void CTavsrcMtUpdater::MrcdamlboGetPath(RPointerArray<HBufC8>& aPath)
	{
	TInt err = KErrNone;
	for(TInt i = 0; (i < iFolderDepth) && (err == KErrNone); i++)
		{
		HBufC8* name = HBufC8::NewL(14);
		name->Des().AppendFormat(KFolderName, iFolderDepth);
		err = aPath.Append(name);
		}
	
	if(!err)
		{
		BROWSE_CONSOLE->Console().Printf(_L("* Player set as browsed: folder items = %d, uid counter = %d\n"), KNumberItemsInFolder, iUidCounter);
		}
	else 
		{
		BROWSE_CONSOLE->Console().Printf(_L("* Error %d setting as browsed player"), err);
		}
	iMediaBrowseInterface->MrcdamlbGetPathResult(KNumberItemsInFolder, iUidCounter, err);
	}

void CTavsrcMtUpdater::MrcdamlboSearch(const TDesC8& /*aSearch*/)
	{
	BROWSE_CONSOLE->Console().Printf(_L("* Search (returning not supported)"));

	iMediaBrowseInterface->MrcdamlbSearchResult(0, iUidCounter, KErrAvrcpAirSearchNotSupported);
	}

void CTavsrcMtUpdater::DisplayCurrentState()
	{
	iMtUpdaterConsole->Console().Printf(_L("**************************************************\n"));
	iMtUpdaterConsole->Console().Printf(_L("* CURRENT STATE\n*\n"));
	
	iMtUpdaterConsole->Console().Printf(_L("* Supported Events:\t"));
	for(TInt i=0; i<iNotificationEvents.Count(); i++)
		{
		iMtUpdaterConsole->Console().Printf(_L("0x%.8x\t"), iNotificationEvents[i]);
		}
	iMtUpdaterConsole->Console().Printf(_L("\n"));
		
	iMtUpdaterConsole->Console().Printf(_L("* Supported Company Ids:\t"));
	for(TInt i=0; i<iCompanyIds.Count(); i++)
		{
		iMtUpdaterConsole->Console().Printf(_L("0x%.8x\t"), iCompanyIds[i]);
		}
	iMtUpdaterConsole->Console().Printf(_L("\n"));
	
	iMtUpdaterConsole->Console().Printf(_L("* Playback Status: "));
	switch(iPlaybackStatus)
		{
	case MPlayerEventsObserver::EStopped:
		iMtUpdaterConsole->Console().Printf(_L("STOPPED\n"));
		break;
	case MPlayerEventsObserver::EPlaying:
		iMtUpdaterConsole->Console().Printf(_L("PLAYING\n"));
		break;
	case MPlayerEventsObserver::EPaused:
		iMtUpdaterConsole->Console().Printf(_L("PAUSED\n"));
		break;
	case MPlayerEventsObserver::EFwdSeek:
		iMtUpdaterConsole->Console().Printf(_L("FWD_SEEK\n"));
		break;
	case MPlayerEventsObserver::ERevSeek:
		iMtUpdaterConsole->Console().Printf(_L("REV_SEEK\n"));
		break;
	default:
		ASSERT(EFalse);
		break;
		}
	
	iMtUpdaterConsole->Console().Printf(_L("* Position: %d\n"), iPosition);
	iMtUpdaterConsole->Console().Printf(_L("* Length: %d\n"), iLength);
	
	iMtUpdaterConsole->Console().Printf(_L("* Battery Status: "), iBatteryStatus);
	switch(iBatteryStatus)
		{
	case MPlayerEventsObserver::ENormal:
		iMtUpdaterConsole->Console().Printf(_L("NORMAL \n"));
		break;
	case MPlayerEventsObserver::EWarning:
		iMtUpdaterConsole->Console().Printf(_L("WARNING \n"));
		break;
	case MPlayerEventsObserver::ECritical:
		iMtUpdaterConsole->Console().Printf(_L("CRITICAL \n"));
		break;
	case MPlayerEventsObserver::EExternal:
		iMtUpdaterConsole->Console().Printf(_L("EXTERNAL \n"));
		break;
	case MPlayerEventsObserver::EFullCharge:
		iMtUpdaterConsole->Console().Printf(_L("FULL CHARGE \n"));
		break;
	case MPlayerEventsObserver::EUnknown:
		iMtUpdaterConsole->Console().Printf(_L("Unknown\n"));
	default:
		ASSERT(EFalse);
		break;
		}	
	
	iMtUpdaterConsole->Console().Printf(_L("**************************************************\n\n"));
	}

void CTavsrcMtUpdater::MrcdanpboGetFolderListing(TUint aStartItem, TUint aEndItem)
	{
	BROWSE_CONSOLE->Console().Printf(_L("* GetFolderItems NowPlaying start item %d, end item %d\n"), aStartItem, aEndItem);

	TInt err = aStartItem < KNumberItemsInFolder ? KErrNone : KErrMediaBrowseInvalidOffset;

	RArray<TRemConItem> folderListing;
	if(!err)
		{
		for(TInt i = aStartItem; (i <= aEndItem) && (i < KNumberItemsInFolder) && !err; i++)
			{
			// FIXME handle erro
			TRemConItem item;
			item.iUid = static_cast<TRemConItemUid>(i) | KUidMarker;
			item.iType = ERemConMediaItem;
			err = folderListing.Append(item);
			}
		}
	
	if(!err)
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Returning listing of %d items, current uid counter %d\n"), folderListing.Array().Count(), iUidCounter);	
		}
	else
		{
		BROWSE_CONSOLE->Console().Printf(_L("  Error %d getting folder listing"));	
		}

	iNowPlayingBrowseInterface->MrcdanpbFolderListing(folderListing.Array(), iUidCounter, err);
	}

TInt CTavsrcMtUpdater::PrepareItemDataL(
		TMediaAttributeIter& aIter,
		const TRemConItemUid& aItemId,
		HBufC8* &aItemName,
		RArray<TMediaElementAttribute>& aItemAttributes)
	{
	/**
	First, construct the name of the media item.
	*/
    _LIT8(KMediaItemName,     "MediaItem 0x%08x%08x");	
	_LIT8(KFolderItemName,     "FolderItem 0x%08x%08x");	
	HBufC8* itemName = HBufC8::NewL(30);
    TPtr8 namePtr = itemName->Des();
	TUint upper = aItemId >> 32;
	TUint lower = aItemId & 0xffffffff;	
	if (aItemId & KUidMarker)
	    {
	    namePtr.AppendFormat(KMediaItemName, upper, lower);
	    }
	else if(aItemId & KFolderMarker)
		{
		namePtr.AppendFormat(KFolderItemName, upper, lower);
		}
	else
		{
		ASSERT(NULL);
		}
	aItemName = itemName;
	/**
	Second, construct attributes of the item.
	*/
	_LIT8(KAttributeName, "Attibute 0x%02x");
	TMediaElementAttribute attribute;
	TMediaAttributeId id;
	aIter.Start();
	while(aIter.Next(id))
		{
		//API takes the ownership.
		HBufC8* attributeName = HBufC8::NewL(30);
		TPtr8 attributeNamePtr = attributeName->Des();
		attributeNamePtr.AppendFormat(KAttributeName, id);
		
		attribute.iAttributeId = id;
		attribute.iString = attributeName;
		aItemAttributes.Append(attribute);
		}
	
	return KErrNone;
	}

TInt CTavsrcMtUpdater::MrcdamlboGetItem(TRemConFolderScope /*aScope*/,
				const TRemConItemUid& aItemId, 
				TMediaAttributeIter& aIter, 
				TUint16 aMediaLibraryStateCookie)
	{
	if(iUidCounter != aMediaLibraryStateCookie)
		{
		BROWSE_CONSOLE->Console().Printf(_L("remote uidcounter does not match local(%d)\n"), iUidCounter);
		return KErrInvalidMediaLibraryStateCookie;
		}

	TInt error = KErrNone;
	RArray<TMediaElementAttribute> itemAttributes;
	HBufC8* itemName = NULL;
	if (aItemId & KUidMarker)
		{
		PrepareItemDataL(aIter, aItemId, itemName, itemAttributes);
		iMediaBrowseInterface->MrcdamlbMediaElementItemResult(aItemId, 
					*itemName, 
					AvrcpBrowsing::KAudio,
					itemAttributes.Array(), 
					KErrNone);
		}
	else if(aItemId & KFolderMarker)
		{
		PrepareItemDataL(aIter, aItemId, itemName, itemAttributes);
		iMediaBrowseInterface->MrcdamlbFolderItemResult(aItemId, 
					*itemName, 
					EFolderPlaylists,
					KFolderNotPlayable,
					itemAttributes.Array(),
					KErrNone);
		}
	else
		{
		error = KErrInvalidUid;
		}
	
	delete itemName;
	return error;
	}


TInt CTavsrcMtUpdater::MrcdanpboGetItem(const TRemConItemUid& aItemId, 
			TMediaAttributeIter& aIter, 
			TUint16 aMediaLibraryStateCookie)
	{
	if(iUidCounter != aMediaLibraryStateCookie)
		{
		BROWSE_CONSOLE->Console().Printf(_L("remote uidcounter does not match local(%d)\n"), iUidCounter);
		return KErrInvalidMediaLibraryStateCookie;
		}
	
	TInt error = KErrNone;
	RArray<TMediaElementAttribute> itemAttributes;
	if (aItemId & KUidMarker)
		{
		HBufC8* itemName;
		PrepareItemDataL(aIter, aItemId, itemName, itemAttributes);
		iNowPlayingBrowseInterface->MrcdanpbMediaElementItemResult(aItemId, 
				*itemName, 
				AvrcpBrowsing::KAudio,
				itemAttributes.Array(), 
				KErrNone);
		delete itemName;
		}
	else
		{
		error = KErrInvalidUid;
		}
	
	return error;
	}

CStopBrowseWatcher* CStopBrowseWatcher::NewL()
	{
	CStopBrowseWatcher* self = new(ELeave) CStopBrowseWatcher;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CStopBrowseWatcher::CStopBrowseWatcher()
	: CActive(EPriorityStandard)
	{
	CActiveScheduler::Add(this);
	}

void CStopBrowseWatcher::ConstructL()
	{
	User::LeaveIfError(iThread.Open(RThread().Id()));
	}

CStopBrowseWatcher::~CStopBrowseWatcher()
	{
	Cancel();
	iThread.Close();
	}

void CStopBrowseWatcher::Start()
	{
	iStatus = KRequestPending;
	SetActive();
	}

void CStopBrowseWatcher::Complete()
	{
	TRequestStatus* status = &iStatus;
	iThread.RequestComplete(status, KErrNone);
	}

void CStopBrowseWatcher::RunL()
	{
	CActiveScheduler::Stop();
	}

void CStopBrowseWatcher::DoCancel()
	{
	TRequestStatus* status = &iStatus;
	iThread.RequestComplete(status, KErrCancel);
	}


