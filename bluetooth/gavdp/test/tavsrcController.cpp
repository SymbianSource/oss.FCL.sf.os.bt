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

#include "tavsrcController.h"

#include <remconinterfaceselector.h>
#include <remconcoreapicontroller.h>
#include <remconbeareravrcp.h>

CTavsrcController* CTavsrcController::NewL(CRemConInterfaceSelector& aIfSel, CActiveConsole& aConsole)
	{
	CTavsrcController* controller = new(ELeave) CTavsrcController(aIfSel, aConsole);
	CleanupStack::PushL(controller);
	controller->ConstructL();
	CleanupStack::Pop(controller);
	return controller;
	}
	
CTavsrcController::CTavsrcController(CRemConInterfaceSelector& aIfSel, CActiveConsole& aConsole)
	: CActive(EPriorityStandard), iState(EControllerNotConnected), iSelector(&aIfSel), iConsole(aConsole)
	{
	CActiveScheduler::Add(this);
	}

void CTavsrcController::ConstructL()
	{
	iCoreController = CRemConCoreApiController::NewL(*iSelector, *this);
	iGoConnectionOrientedSent = EFalse;
	}
	
CTavsrcController::~CTavsrcController()
	{
	Cancel();
	}

TInt CTavsrcController::Command(TRemConCoreApiOperationId aOpId)
	{
	TInt err = KErrNotReady;
	
	if(!IsActive())
		{
		err = KErrNone;
		
		if(iState == EControllerConnected)
			{
			IssueCommand(aOpId);
			}
		else
			{
			// Need to connect first
			TRAP(err, ConnectL());
			}
		}
	
	return err;
	}


void CTavsrcController::MrccacoResponse(TRemConCoreApiOperationId aOperationId, TInt aError)
	{
	iConsole.Console().Printf(_L("Received Response for Operation %d, Result %d\n"), aOperationId, aError);
	}

void CTavsrcController::IssueCommand(TRemConCoreApiOperationId aOpId)
	{
	switch(aOpId)
		{
	case ERemConCoreApiVolumeDown:
		iCoreController->VolumeDown(iStatus, iNumRemotes, ERemConCoreApiButtonClick);
		break;
	case ERemConCoreApiVolumeUp:
		iCoreController->VolumeUp(iStatus, iNumRemotes, ERemConCoreApiButtonPress);
		break;
	case ERemConCoreApiForward:
		iCoreController->Forward(iStatus, iNumRemotes, ERemConCoreApiButtonClick);
		break;
	case ERemConCoreApiBackward:
		iCoreController->Backward(iStatus, iNumRemotes, ERemConCoreApiButtonClick);
		break;
	case ERemConCoreApiPlay:
		iCoreController->Play(iStatus, iNumRemotes, ERemConCoreApiButtonClick);
		break;
	case ERemConCoreApiStop:
		iCoreController->Stop(iStatus, iNumRemotes, ERemConCoreApiButtonClick);
		break;
	default:
		// Cos we don't want to SetActive(), alright?
		return;
		}
	
	iConsole.Console().Printf(_L("Sending Command Operation %d\n"), aOpId);	
	SetActive();
	}

void CTavsrcController::ConnectL()
	{
	//Ask user which device address we should connect to...	
	iState = ESearchingForDevice;
	User::LeaveIfError(iNotify.Connect());
	iNotify.StartNotifierAndGetResponse(iStatus, KDeviceSelectionNotifierUid, iPckg, iResPckg);
	SetActive();
	}
	
void CTavsrcController::RunL()
	{
	switch(iState)
		{
	case EControllerConnected:
		{
		// We don't care what the result was really
		// Ask Tim about the display logging and 
		// log it
		break;
		}
	case EControllerNotConnected:
		{
		iConsole.Console().Printf(_L("AVRCP connection completed, result = %d\n"), iStatus.Int());

		if(iStatus.Int() == KErrNone)
			{
			// Connected ok, now issue command
			iState = EControllerConnected;
			IssueCommand(iOutstandingOperation);	
			}
		break;
		}
	case ESearchingForDevice:
		{
		if (iStatus.Int() == KErrNone)
			{
			iState = EControllerNotConnected;
			
			iNotify.CancelNotifier(KDeviceSelectionNotifierUid);
			iNotify.Close();
			
			iConsole.Console().Printf(_L("Opening AVRCP connection...\n"));

			TBTDevAddr btAddr = iResPckg().BDAddr();
			// Store as 8 bit machine readable
			RBuf8 bearerData;
			bearerData.CreateL(btAddr.Des());

			// Form the RemCon Addr from the AVRCP Uid and the 
			// bluetooth address
			TRemConAddress addr;
			addr.BearerUid() = TUid::Uid(KRemConBearerAvrcpImplementationUid);
			addr.Addr() = bearerData;

			if (iGoConnectionOrientedSent)
				{
				// Only allowed to call GoConnectionOrientedL() once on pain of RemCon panic,
				// ERemConClientPanicAlreadyConnectionOriented. Solution is to call GoConnectionlessL()
				// before recalling GoConnectionOrientedL()
				iSelector->GoConnectionlessL();
				}
			iSelector->GoConnectionOrientedL(addr);
			iSelector->ConnectBearer(iStatus);
			iGoConnectionOrientedSent = ETrue;

			SetActive();
			break;
			}
		}
	default:
		{
		// hmm shouldn't be here
		__DEBUGGER();
		break;
		}
		}
	}
	
void CTavsrcController::DoCancel()
	{
	iCoreController->Cancel();
	}
