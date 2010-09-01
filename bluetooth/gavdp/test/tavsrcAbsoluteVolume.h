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

#ifndef TAVSRCABSOLUTEVOLUME_H_
#define TAVSRCABSOLUTEVOLUME_H_
#include <absolutevolumeapitargetobserver.h>
#include <absolutevolumeapicontrollerobserver.h>
#include "tavsrcConsole.h"

static const TUint32 KInitialVolume = 1;
static const TUint32 KMaxTgVolume = 10;

static const TUint32 KMaxCtVolume = 100;

class CActiveConsole;
class CRemConInterfaceSelector;
class CRemConAbsoluteVolumeTarget;
class CRemConAbsoluteVolumeController;
class CTavsrcAbsoluteVolume: public CActive, 
	                         public MActiveConsoleNotify,
	                         public MRemConAbsoluteVolumeTargetObserver,
	                         public MRemConAbsoluteVolumeControllerObserver
    {
public:
	static CTavsrcAbsoluteVolume* NewL(CRemConInterfaceSelector& aIfSel);
	~CTavsrcAbsoluteVolume();	

private:
	CTavsrcAbsoluteVolume();
	void ConstructL(CRemConInterfaceSelector& aIfSel);

private://from MActiveConsoleNotify
	void KeyPressed(TChar aKey);
	
private:
	//from MRemConAbsoluteVolumeTargetObserver
	void MrcavtoSetAbsoluteVolumeRequest(TUint32 aVolume, TUint32 aMaxVolume);
	
	//from MRemConAbsoluteVolumeControllerObserver
	void MrcavcoCurrentVolume(TUint32 aVolume, TUint32 aMaxVolume, TInt aError);
	void MrcavcoSetAbsoluteVolumeResponse(TUint32 aVolume, TUint32 aMaxVolume, TInt aError);
	void MrcavcoAbsoluteVolumeNotificationError();
	
private:
	// from CActive
	void RunL();
	void DoCancel();
	
private: 
	void MtMenu();
	TUint32 GetVolume();

private:
	CActiveConsole* iMtAbsoluteVolumeConsole;
	CRemConAbsoluteVolumeTarget* iAbsoluteVolumeTarget;
	CRemConAbsoluteVolumeController* iAbsoluteVolumeController;
	TUint iNumRemotes;
	};

#endif /*TAVSRCABSOLUTEVOLUME_H_*/
