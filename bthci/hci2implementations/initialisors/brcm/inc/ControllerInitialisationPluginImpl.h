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

#ifndef CONTROLLERINITIALISATIONPLUGINIMPL_H
#define CONTROLLERINITIALISATIONPLUGINIMPL_H

#include <bluetooth/hci/controllerinitialisationplugin.h>
#include <bluetooth/hci/controllerinitialisationinterface.h>

#include <bluetooth/hcicommandqueueclient.h>
#include <bluetooth/logger.h>

#include <BroadcomControllerInitialisor.h>

#include <bluetooth/hci/hctlbcmconfiginterface.h>

class TVendorDebugCompleteEvent;
class MCoreHci;

const TUint16 KHciVSUpdateUartHCIBaudRate = 0xFC18;
const TUint16 KHciVSDownloadMiniDriver = 0xFC2E; //HCI_BRCM_DOWNLOAD_MINI_DRV
const TUint16 KHciVSWriteRam = 0xFC4C;           //HCI_BRCM_WRITE_RAM
const TUint16 KHciVSLaunchRam = 0xFC4E;          //HCI_BRCM_LAUNCH_RAM

class CControllerInitialisationPluginImpl : public CControllerInitialisationPlugin,
											public MControllerInitialisationInterface,
											public MHCICommandQueueClient,
											public MConfigFileTimerEventObserver
	{
	
private:
	enum TPatchRamFormat
		{
		EUnknownFileFormat,
		EHcdFileFormat,
		EBinFileFormat
		};
			
public:
	static CControllerInitialisationPluginImpl* NewL();
	~CControllerInitialisationPluginImpl();
	virtual TAny* Interface(TUid aUid);

protected:
	CControllerInitialisationPluginImpl();

private:
	void ConstructL();
	
	// Static async CallBack methods.
	static TInt AsyncCallBackForPostReset(TAny* aInitPlugin);
		
	// MControllerInitialisationInterface
	virtual void MciiPreResetCommand();
	virtual void MciiPostResetCommand();
	virtual void MciiSetHCICommandQueue(MHCICommandQueue& aHCICommandQueue);
	virtual void MciiSetControllerInitialisationObserver(MControllerInitialisationObserver& aObserver);
	virtual void MciiSetCoreHci(MCoreHci& aCoreHci);

	// MHCICommandQueueClient
	virtual void MhcqcCommandEventReceived(const THCIEventBase& aEvent,
										   const CHCICommandBase* aRelatedCommand);
	virtual void MhcqcCommandErrored(TInt aErrorCode,const CHCICommandBase* aCommand);
    virtual void ConfigFileTimerExpired();
    void HandleVendorSpecificCompleteEvent(const TVendorDebugCompleteEvent& aEvent);
 	void DoPostReset();
 	void InitialiseAndLoadMiniDriver();
// 	TInt MapBaudRate(TInt iMapfrom, TInt iMapTo);

private:
// Async Callback
	CAsyncCallBack* iAsyncCallBackForPostReset;
	TInitState iInitState;			
	CConfigFileDownload* iCConfigFileDownload;
 	MHCICommandQueue* iHCICommandQueue; 	
	MControllerInitialisationObserver* iControllerInitialisationObserver;
	MCoreHci* iCoreHci;
	TUint32 iDestRam;
	TUint16 iTxPatchLength;
	CConfigFileTimer* iMiniDrive2ConfigFileTimer;
	
	TPatchRamFormat iFileFormat ;
	};



#endif
