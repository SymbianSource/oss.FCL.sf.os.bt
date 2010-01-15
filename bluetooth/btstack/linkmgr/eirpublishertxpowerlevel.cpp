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

#include "eirpublishertxpowerlevel.h"
#include <bluetooth/eirpublisherbase.h>

//**********************************
// CEirPublisherTxPowerLevel
//**********************************
/**
Provides functionality to publish transmission power level to EIR.
**/
CEirPublisherTxPowerLevel* CEirPublisherTxPowerLevel::NewL()
	{
	CEirPublisherTxPowerLevel* self = new (ELeave) CEirPublisherTxPowerLevel();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CEirPublisherTxPowerLevel::CEirPublisherTxPowerLevel()
:	CEirPublisherBase(EEirTagTxPowerLevel)
	{
	}

void CEirPublisherTxPowerLevel::ConstructL()
	{
	CEirPublisherBase::ConstructL();
	}

CEirPublisherTxPowerLevel::~CEirPublisherTxPowerLevel()
	{
	}

void CEirPublisherTxPowerLevel::UpdateTxPowerLevel(TInt8 aTxPowerLevel)
	{
	iTxPowerLevel = aTxPowerLevel;
	iPublisher->PublishData(KSizeOfTxPowerLevelData);
	}
	
// From MEirPublisherNotifier
void CEirPublisherTxPowerLevel::MepnSpaceAvailable(TUint aBytesAvailable)
	{
	if (aBytesAvailable >= KSizeOfTxPowerLevelData)
		{
		// only publish TxPowerLevel if enough space
		iPublishBuf.Zero();
		iPublishBuf.Append(iTxPowerLevel);
		iPublisher->SetData(iPublishBuf, EEirDataComplete);
		}
	else
		{
		// Otherwise publish zero length string
		iPublishBuf.Zero();
		iPublisher->SetData(iPublishBuf, EEirDataPartial);
		}
	}

void CEirPublisherTxPowerLevel::MepnSetDataError(TInt /*aResult*/)
	{
	}
