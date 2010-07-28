// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/

#include "BroadcomControllerInitialisor.h"
#include "ControllerInitialisationPluginImpl.h"

#include <bluetooth/logger.h>



#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_INITIALISOR_BROADCOM);
//_LIT8(KLogComponent, "InitBRCM");

#endif

/*static*/ CConfigFileDownload* CConfigFileDownload::NewL(const TDesC& aConfigFile)
	{
	LOG_STATIC_FUNC
	
	CConfigFileDownload* self = new(ELeave) CConfigFileDownload;
	CleanupStack::PushL(self);
	self->ConstructL(aConfigFile);
	CleanupStack::Pop(self);
	return self;
	}

void CConfigFileDownload::ConstructL(const TDesC& aConfigFile)
	{
	LOG_FUNC

	LEAVEIFERRORL(iFs.Connect());
	LEAVEIFERRORL(iFile.Open(iFs, aConfigFile, EFileRead || EFileShareReadersOnly || EFileStream)); 

	}

CConfigFileDownload::~CConfigFileDownload()
	{
	LOG_FUNC

	iFile.Close();
	iFs.Close();
	}

// Private Constructor
CConfigFileDownload::CConfigFileDownload()
	{
	LOG_FUNC
	}

TInt CConfigFileDownload::ConfigFileSendDataChunk(TBuf8<KConfigFileChunkSize>& aVendorCommandBuffer)
	{
	LOG_STATIC_FUNC
		
	TInt err = KErrNone;
	TInt len;
	
	// read Config file data
	err = iFile.Read(aVendorCommandBuffer, KConfigFileChunkSize);
	len = aVendorCommandBuffer.Length();
	if (KErrNone == err)
		{
		if (aVendorCommandBuffer.Length() == 0)
			{
			// EOF was reached: stop sending HCI commands force err to EOF
			err = KErrEof;			
			}
		}
	
	return err;
	}

TInt CConfigFileDownload::ConfigFileSendData(TBuf8<KHcdConfigFileChunkSize>& aVendorCommandBuffer,TUint8 DataChunkLen)
	{
	LOG_STATIC_FUNC
		
	TInt err = KErrNone;
	TInt len;
	
	aVendorCommandBuffer.SetLength(DataChunkLen);
	
	// read Config file data
	err = iFile.Read(aVendorCommandBuffer, DataChunkLen);
	
	
	//ONLY FOR TEST
	len = aVendorCommandBuffer.Length();
	
	if (KErrNone == err)
		{
		if (aVendorCommandBuffer.Length() == 0)
			{
			// EOF was reached: stop sending HCI commands force err to EOF
			err = KErrEof;			
			}
		}
	//ONLY FOR TEST
	return err;
	}

TInt CConfigFileDownload::ReadCommandOpcode(THCIOpcode & Opcode)
	{
	LOG_STATIC_FUNC
		
	TInt err = KErrNone;
	
	TBuf8<2> OpcodeBytes ;
	//TDes8<2> OpcodeBytes;
	//THCIOpcode Opcode = 0;
	
	
	// read Config file data
	err = iFile.Read(OpcodeBytes, 2);
	
	if(err == KErrNone)
		{
		Opcode = (OpcodeBytes[1]<<8) |OpcodeBytes[0];
		}
	
	return err;
	}

TInt CConfigFileDownload::ReadDataChunkLen(TUint8& DataChunkLen)
	{
	LOG_STATIC_FUNC
		
	TInt err = KErrNone;
	TBuf8<1> DataLen;
	//TUint8 DataChunkLen= 0;
	
	
	// read Config file data
	err = iFile.Read(DataLen, 1);
	//err = iFile.Read(DataChunkLen, 1);
		
	DataChunkLen = DataLen[0];
	
	return err ;
	}

//TInt CConfigFileDownload::SetDataChunkAddress(TBuf8<4> AddressBuf)
TInt CConfigFileDownload::SetDataChunkAddress(TDes8 & AddressBuf)
	{
	LOG_STATIC_FUNC
		
	TInt err = KErrNone;
	//TBuf8 AddressBuf(AddressDesC);
	//TBuf8<4> AddressBuf;
	
	
	// read Config file data
	err = iFile.Read(AddressBuf, 4);
		
	
	//DataChunkLen = DataLen[0];
	
	return err ;
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CConfigFileTimer* CConfigFileTimer::NewL(MConfigFileTimerEventObserver& aTimerObserver)
	{
	LOG_STATIC_FUNC

	CConfigFileTimer* self = new (ELeave) CConfigFileTimer(aTimerObserver);
   	CleanupStack::PushL(self);
   	self->ConstructL();
   	CleanupStack::Pop(self);
   	return self;
	}

void CConfigFileTimer::ConstructL()
	{
	LOG_FUNC

   	CTimer::ConstructL();
   	CActiveScheduler::Add(this);
	}

CConfigFileTimer::CConfigFileTimer(MConfigFileTimerEventObserver& aTimerObserver)
  : CTimer(EPriorityStandard),
    iTimerObserver(aTimerObserver)
	{
	LOG_FUNC
	}

void CConfigFileTimer::Start(const TInt& aDuration)
	{
	LOG_FUNC

	if(!IsActive())
		{
		After(aDuration);
	//	After(KMiniDriverToPatchRamTimerDuration);
	//	TTimeIntervalMicroSeconds32
		}
	}
	
void CConfigFileTimer::RunL()
	{
 	LOG_FUNC

  	iTimerObserver.ConfigFileTimerExpired();
   	}

	
