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

#ifndef TAVSRCCONTROLLER_H
#define TAVSRCCONTROLLER_H

#include "tavsrcConsole.h"

#include <e32base.h>
#include <btextnotifiers.h>
#include <remconcoreapicontrollerobserver.h>

class CRemConInterfaceSelector;
class CRemConCoreApiController;

class CTavsrcController : public CActive, public MRemConCoreApiControllerObserver
	{
public:
	static CTavsrcController* NewL(CRemConInterfaceSelector& aIfSel, CActiveConsole& aConsole);
	~CTavsrcController();

	TInt Command(TRemConCoreApiOperationId aOpId);
	void MrccacoResponse(TRemConCoreApiOperationId aOperationId, TInt aError);

private:
	CTavsrcController(CRemConInterfaceSelector& aIfSel, CActiveConsole& aConsole);
	void ConstructL();
		
	void RunL();
	void DoCancel();

	void ConnectL();
	void IssueCommand(TRemConCoreApiOperationId aOpId);
		
private:
	enum TTavsrcControllerState
		{
		EControllerConnected,
		EControllerNotConnected,
		ESearchingForDevice,
		};

private:
	TTavsrcControllerState	  iState;
	CRemConInterfaceSelector* iSelector;
	TBool                     iGoConnectionOrientedSent;
	CRemConCoreApiController* iCoreController;
	TUint					  iNumRemotes;	
	TRemConCoreApiOperationId iOutstandingOperation;
	CActiveConsole&			  iConsole;
	RNotifier 				  iNotify;
	TBTDeviceSelectionParamsPckg iPckg;
	TBTDeviceResponseParamsPckg iResPckg;
	};

#endif // TAVSRCCONTROLLER_H
