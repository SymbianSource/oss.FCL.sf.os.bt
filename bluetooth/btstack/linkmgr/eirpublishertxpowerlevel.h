// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef EIRPUBLISHERTXPOWERLEVEL_H
#define EIRPUBLISHERTXPOWERLEVEL_H

#include <e32base.h>
#include <bluetooth/eirpublisherbase.h>

#define KSizeOfTxPowerLevelData 1

//**********************************
// CEirPublisherTxPowerLevel
//**********************************
/**
Provides functionality to publish TxPowerLevel to EIR.
**/
NONSHARABLE_CLASS(CEirPublisherTxPowerLevel) : public CEirPublisherBase
	{
public:
	static CEirPublisherTxPowerLevel* NewL();
	~CEirPublisherTxPowerLevel();
	void UpdateTxPowerLevel(TInt8 aTxPowerLevel);

private:
	CEirPublisherTxPowerLevel();
	void ConstructL();
	
	// From MEirPublisherNotifier
	virtual void MepnSpaceAvailable(TUint aBytesAvailable);
	virtual void MepnSetDataError(TInt aResult);

private:
	TInt8 iTxPowerLevel;
	TBuf8<KSizeOfTxPowerLevelData> iPublishBuf;
	};
	
#endif	//EIRPUBLISHERTXPOWERLEVEL_H
