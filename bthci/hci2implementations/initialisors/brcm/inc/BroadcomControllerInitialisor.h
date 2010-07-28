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

#ifndef BROADCOMCONTROLLERINITIALISOR_H
#define BROADCOMCONTROLLERINITIALISOR_H

#include <e32base.h>
#include <f32file.h>

#include <bluetooth/hci/hciopcodes.h> 

/**
The panic category for the Broadcom controller initialisor.
*/
_LIT(KBroadcomControllerInitialisorPanic, "Broadcom Ctlr Init");

/**
Panic codes for the Symbian reference controller initialisor
*/
enum TBroadcomControllerInitialisorPanic
	{
	EConfigInterfaceNull					= 0,
	EInvalidInitialisorState				= 1,
	ENullInitialisationObserver				= 2,
	EHciCommandQueueOverwritten				= 3,
	EInitialisationObserverOverwritten		= 4,
	ECoreHciOverwritten						= 5
	};
	static const TUint8 KHcdConfigFileChunkSize = 0xff;	
    static const TUint8 KConfigFileChunkSize = 250;	

enum TInitState
   	{
   	EIdle,
   	EUpdateBaudRateBeforePrmDelay,
   	EloadingMiniDriver,
   	EMiniDrvToBinConfigFileDelay,
   	EMiniDrvToHcdConfigFileDelay,
   	ELoadingBinConfigFileData,
   	ELoadingHcdConfigFileData,
   	ELoadingconfigFileDataDone,
   	EWaitEndDelay,
   	EWaitUpdateBaudRateAfterPrm		
   	};
    /**
	Observer class for ConfigFileTimer
*/
class MConfigFileTimerEventObserver
	{
public:
   	virtual void ConfigFileTimerExpired() = 0;
	};  
	
/**
	Timer class
*/
class CConfigFileTimer : public CTimer
	{
public:
	static CConfigFileTimer* NewL(MConfigFileTimerEventObserver& aTimerObserver);
	void Start(const TInt& aDuration);

private:
	CConfigFileTimer(MConfigFileTimerEventObserver& aTimerObserver);
	void ConstructL();
	void RunL();

private:	
	MConfigFileTimerEventObserver& iTimerObserver;
	};
	
	
NONSHARABLE_CLASS(CConfigFileDownload) : public CBase
	{
public:
	static CConfigFileDownload* NewL(const TDesC& aConfigFile);
	~CConfigFileDownload();
	TInt ConfigFileSendDataChunk(TBuf8<KConfigFileChunkSize>& aVendorCommandBuffer);
	
	TInt ConfigFileSendData(TBuf8<KHcdConfigFileChunkSize>& aVendorCommandBuffer,TUint8 DataChunkLen);
	TInt ReadCommandOpcode(THCIOpcode & Opcode);
	TInt ReadDataChunkLen(TUint8 & DataChunkLen);
	TInt SetDataChunkAddress(TDes8 &  AddressBuf);
//	TInt SetDataChunkAddress(TBuf8<4>  AddressBuf);
	
private:
	CConfigFileDownload();
	void ConstructL(const TDesC& aConfigFile);
	
private:

 	RFile iFile;
	RFs iFs;
	

	};


#endif // BROADCOMCONTROLLERINITIALISOR_H

