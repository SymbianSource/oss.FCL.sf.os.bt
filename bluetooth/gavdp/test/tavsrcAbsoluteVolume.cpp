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

#include "tavsrcAbsoluteVolume.h"
#include <absolutevolumeapicontroller.h>
#include <absolutevolumeapitarget.h>

#ifdef __WINS__
GLDEF_D TSize gAbsoluteVolumeConsole(75,30);
#else
GLDEF_D TSize gAbsoluteVolumeConsole(KConsFullScreen,KConsFullScreen);
#endif

	
CTavsrcAbsoluteVolume* CTavsrcAbsoluteVolume::NewL(CRemConInterfaceSelector& aIfSel)
	{
	CTavsrcAbsoluteVolume* self = new (ELeave) CTavsrcAbsoluteVolume();
	CleanupStack::PushL(self);
	self->ConstructL(aIfSel);
	CleanupStack::Pop();
	return self;
	}

CTavsrcAbsoluteVolume::~CTavsrcAbsoluteVolume()
	{
	Cancel();
	delete iMtAbsoluteVolumeConsole;
	}

CTavsrcAbsoluteVolume::CTavsrcAbsoluteVolume()
	: CActive(EPriorityStandard)
	{
	CActiveScheduler::Add(this);
	}

void CTavsrcAbsoluteVolume::ConstructL(CRemConInterfaceSelector& aIfSel)
	{
	iAbsoluteVolumeTarget = CRemConAbsoluteVolumeTarget::NewL(aIfSel, *this, KInitialVolume, KMaxTgVolume);
	iAbsoluteVolumeController = CRemConAbsoluteVolumeController::NewL(aIfSel, *this, KMaxCtVolume);
	iMtAbsoluteVolumeConsole = CActiveConsole::NewL(*this,_L("AbsoluteVolume"),gAbsoluteVolumeConsole);
	MtMenu();
	}

void CTavsrcAbsoluteVolume::MtMenu()
	{
	iMtAbsoluteVolumeConsole->Console().Printf(_L("1.\tCT SetAbsoluteVolume \n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("2.\tCT Cancel SetAbsoluteVolume \n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("3.\tCT RegisterAbsoluteVolumeNotification \n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("4.\tCT CancelAbsoluteVolumeNotification\n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("5.\tTG SetAbsoluteVolumeResponse\n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("6.\tTG AbsoluteVolumeChanged\n"));
	iMtAbsoluteVolumeConsole->Console().Printf(_L("\n"));
	
	iMtAbsoluteVolumeConsole->RequestKey();
	}

void CTavsrcAbsoluteVolume::KeyPressed(TChar aKey)
	{
	switch(aKey)
			{
		case '1':
			{
			TRequestStatus status;
			iAbsoluteVolumeController->SetAbsoluteVolume(iStatus, GetVolume(), iNumRemotes);
			SetActive();
			break;
			}
		case '2':
			{
			iAbsoluteVolumeController->CancelSetAbsoluteVolume();
			iMtAbsoluteVolumeConsole->Console().Printf(_L("CT Cancelled Set Absolute Volume\n"));
			break;
			}

		case '3':
			{
			TRequestStatus status;
			iAbsoluteVolumeController->RegisterAbsoluteVolumeNotification();
			iMtAbsoluteVolumeConsole->Console().Printf(_L("CT Registered Absolute Volume Notification \n"));
			break;
			}
		case '4':
			{
			iAbsoluteVolumeController->CancelAbsoluteVolumeNotification();
			iMtAbsoluteVolumeConsole->Console().Printf(_L("CT Cancelled Absolute Volume Notification  \n"));
			break;
			}
		case '5':
			{
			iAbsoluteVolumeTarget->SetAbsoluteVolumeResponse(GetVolume(), KErrNone);
			iMtAbsoluteVolumeConsole->Console().Printf(_L("TG SetAbsoluteVolumeResponse \n"));
			break;
			}
		case '6':
			{
			iAbsoluteVolumeTarget->AbsoluteVolumeChanged(GetVolume());
			iMtAbsoluteVolumeConsole->Console().Printf(_L("TG AbsoluteVolumeChanged \n"));
			break;
			}
		default:
			iMtAbsoluteVolumeConsole->Console().Printf(_L("No such command\n"));
			break;
			};
			
	MtMenu();
	}
	
void CTavsrcAbsoluteVolume::MrcavtoSetAbsoluteVolumeRequest(TUint32 aVolume, 
		TUint32 aMaxVolume)
	{
	TUint32 scaledVolume = (aVolume * 10)/(aMaxVolume);
	iMtAbsoluteVolumeConsole->Console().Printf(_L("MrcavtoSetAbsoluteVolumeRequest vol= %d, maxVol= %d, setting volume  = %d\n"), aVolume, aMaxVolume, scaledVolume);
	iMtAbsoluteVolumeConsole->Console().Printf(_L("Provide volume for response relative to local max volume (%d)\n"), KMaxTgVolume);
	iAbsoluteVolumeTarget->SetAbsoluteVolumeResponse(GetVolume(), KErrNone); 
	}
	
void CTavsrcAbsoluteVolume::MrcavcoCurrentVolume(TUint32 aVolume, 
		TUint32 aMaxVolume,
		TInt aError)
	{
	iMtAbsoluteVolumeConsole->Console().Printf(
			_L("MrcavcoCurrentVolume vol= %d, maxVol= %d, err= %d\n"), 
			aVolume, aMaxVolume, aError);
	}

void CTavsrcAbsoluteVolume::MrcavcoAbsoluteVolumeNotificationError()
	{
	iMtAbsoluteVolumeConsole->Console().Printf(_L("MrcavcoAbsoluteVolumeNotificationError\n"));
	}

void CTavsrcAbsoluteVolume::MrcavcoSetAbsoluteVolumeResponse(TUint32 aVolume,
		TUint32 aMaxVolume,
		TInt aError)
	{
	iMtAbsoluteVolumeConsole->Console().Printf(
			_L("MrcavcoSetAbsoluteVolumeResponse vol= %d, maxVol= %d, err= %d\n"), 
			aVolume, aMaxVolume, aError);
	}

void CTavsrcAbsoluteVolume::RunL()
	{
	iMtAbsoluteVolumeConsole->Console().Printf(_L("CT SetAbsoluteVolume Sent %d to %u remotes\n"), iStatus.Int(), iNumRemotes);
	}

void CTavsrcAbsoluteVolume::DoCancel()
	{
	iAbsoluteVolumeController->CancelSetAbsoluteVolume();
	}

TUint32 CTavsrcAbsoluteVolume::GetVolume()
	{
	iMtAbsoluteVolumeConsole->Console().Printf(_L("Enter volume: "));
	TUint32 volume = iMtAbsoluteVolumeConsole->GetIntFromUser();
	iMtAbsoluteVolumeConsole->Console().Printf(_L("\n"));
	
	return volume;
	}


