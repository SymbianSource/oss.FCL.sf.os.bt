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
#include <bluetooth/hcicommandQueue.h>
#include "broadcomcontrollerinitialisor.h"
#include "controllerinitialisationpluginimpl.h"

#include <bluetooth/hci/vendordebugcommand.h>
#include <bluetooth/hci/controllerinitialisationobserver.h>


#include <bluetooth/hci/commandcompleteevent.h>

#include <bluetooth/hci/vendordebugcompleteevent.h>

#include <bluetooth/hci/hctlinterface.h>

#include <bluetooth/hci/corehci.h>

#include <bluetooth/logger.h>
#include <e32debug.h>
#include <CIniData.h>

#define INITIALISOR_BCM_LOGGER
#define BCM_TRANSPORT
#define UPDATEBAUDRATE_AFTER_PRD

#ifdef INITIALISOR_BCM_LOGGER
#define INITIALISOR_BCM_LOG(A)		RDebug::Print(A);
#define INITIALISOR_BCM_LOG2(A,B)	RDebug::Print(A,B);
#define INITIALISOR_BCM_LOG3(A,B,C)		RDebug::Print(A,B,C);
#else
#define INITIALISOR_BCM_LOG(A)
#define INITIALISOR_BCM_LOG2(A,B)
#define INITIALISOR_BCM_LOG3(A,B,C)
#define INITIALISOR_BCM_LOG4(A,B,C,D)
#endif

const TUint32 BCM2048_RAM_LOCATION = 0x00085D00;
const TUint32 BCM4325_RAM_LOCATION = 0x00086800;
const TUint32 BCM2046_RAM_LOCATION = 0x00086800 ; //0x00087AC8;

TUint32 KDestRamLocation = BCM2048_RAM_LOCATION;	// by default this location is for BCM2048
// 50 mstimer.
const TInt KMiniDriverToPatchRamTimerDuration = 50000;		// 50000;
const TInt KEndDelayTimerDuration = 750000;		//250000;

#define BAUDRATE_921600		921600
#define BAUDRATE_115200		115200
#define BAUDRATE_57600		BAUDRATE_921600		// for now we map the BR 57600 to 921600
#define UART_24MHZ_CLOCK	24000000

TInt iBaudRate = BAUDRATE_115200;		// default baudrate
TInt iBaudRateAfter = BAUDRATE_115200;		// default baudrate
TInt iUartClock = UART_24MHZ_CLOCK;		// default baudrate

#ifdef BCM_TRANSPORT
_LIT(KDefaultIniFileName, "hctl_broadcom.ini");
#else
_LIT(KDefaultIniFileName, "hctl_uart_original.ini");
#endif

_LIT(KConfigFileFileFormat, "%S%S");
#if 0
_LIT(KDefaultConfigFileFileFormat, "%S%S.bin");
#else
_LIT(KDefaultConfigFileFileFormat, "%S%S.hcd");
#endif
_LIT(KDefaultConfigFileName, "BCM2046B1");
_LIT(KDefaultConfigFilePath, "z:\\private\\101f7989\\Bluetooth\\");

_LIT(KGlobalReadySemName, "BCM_BT_READY_010155");
//RSemaphore BCM_DriverReadySem;

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_INITIALISOR_BROADCOM);
//_LIT8(KLogComponent, "InitBRCM");
#endif



/*static*/ CControllerInitialisationPluginImpl* CControllerInitialisationPluginImpl::NewL()
	{
	LOG_STATIC_FUNC

	CControllerInitialisationPluginImpl* self = new (ELeave) CControllerInitialisationPluginImpl();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CControllerInitialisationPluginImpl::~CControllerInitialisationPluginImpl()
	{
	LOG_FUNC
	
	// Delete async CallBacks.	If running, these should be cancelled by the
	// d'tor of CAsyncOneShot.
	delete iAsyncCallBackForPostReset;
	delete iCConfigFileDownload;
	delete iMiniDrive2ConfigFileTimer;
	}

void CControllerInitialisationPluginImpl::ConstructL()
	{
	LOG_FUNC
	
	// Add Async Callbacks	
	TCallBack postResetCallBack(AsyncCallBackForPostReset, this);
	iAsyncCallBackForPostReset = new (ELeave)CAsyncCallBack(postResetCallBack, CActive::EPriorityStandard); 
	iMiniDrive2ConfigFileTimer = CConfigFileTimer::NewL(*this);	
	}

// Protected constructor.
CControllerInitialisationPluginImpl::CControllerInitialisationPluginImpl()
	{
	LOG_FUNC

	}

/*virtual*/ void CControllerInitialisationPluginImpl::MciiPreResetCommand()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iControllerInitialisationObserver, PANIC(KBroadcomControllerInitialisorPanic, ENullInitialisationObserver));
	// Do any pre-reset initialisation...
#ifndef __WINSCW__

	INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : Set Initial BaudRate to %d before reset"), iBaudRate)
    
	MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
	__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
	hctlConfigInterface->MhciUpdateBaudRate(iBaudRate);
#endif
	
	INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : call McioPreResetCommandComplete(KErrNone)"))
	// Inform stack that we have finished
	iControllerInitialisationObserver->McioPreResetCommandComplete(KErrNone);
	
	}
/*virtual*/ TAny* CControllerInitialisationPluginImpl::Interface(TUid aUid)
	{
	LOG_FUNC

	TAny* ret = NULL;
	switch(aUid.iUid)
		{
		case KControllerInitialisationInterfaceUid:
			ret = reinterpret_cast<TAny*>(static_cast<MControllerInitialisationInterface*>(this));
			break;

		default:
			break;
		};

	return ret;
	}

/*static*/ TInt CControllerInitialisationPluginImpl::AsyncCallBackForPostReset(TAny* aInitPlugin)
	{
	LOG_STATIC_FUNC

	INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : CControllerInitialisationPluginImpl::AsyncCallBackForPostReset(TAny* aInitPlugin)"))

	CControllerInitialisationPluginImpl* initPlugin = static_cast<CControllerInitialisationPluginImpl*>(aInitPlugin);
	initPlugin->DoPostReset();
	return EFalse;	// Don't call back any more.
	}

void CControllerInitialisationPluginImpl::InitialiseAndLoadMiniDriver()
	{
	LOG_FUNC

	CVendorDebugCommand* cmd = NULL;
	TInt err = KErrNone;

	INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : InitialiseAndLoadMiniDriver()"))

	// Chinda - Check for specific patch ram file in patch_ram section in hctl_bcm.ini file
	TFileName fileName;
	CIniData* pIniFile = NULL;
	
	TName name;
	
	name.Copy(KDefaultConfigFilePath);
	name.Append(KDefaultIniFileName);
	
	_LIT(KFileHcdExtension,"*.hcd");
	_LIT(KFileBinExtension,"*.bin");
	
	_LIT(KPatchramSection, "patch_ram");
	_LIT(KPatchramPath, "path");
	_LIT(KPatchramFile, "file");
	_LIT(KPatchramLocation, "location");
	TInt iRamLocation;	

	
	TRAP(err, pIniFile = CIniData::NewL(name));
	if (err != KErrNone)
		{
		// Attempt to open the config file.
		fileName.Format(KDefaultConfigFileFileFormat,&KDefaultConfigFilePath, &KDefaultConfigFileName);
		iBaudRate = BAUDRATE_115200;
		iBaudRateAfter = BAUDRATE_115200;
		} 
	else
		{
		// First read COM port and baudrate
		if (pIniFile)
			{
			_LIT(KCOMPortSection, "hctl");
			_LIT(KCOMBaudRate, "baud_rate");
			_LIT(KCOMBaudRateAfter, "functional_baud_rate");
			_LIT(KCOMUARTClock, "uart_clock");
			
			if (!pIniFile->FindVar(KCOMPortSection, KCOMBaudRate, iBaudRate))
				{
				iBaudRate = BAUDRATE_115200;
				}

			if (!pIniFile->FindVar(KCOMPortSection, KCOMBaudRateAfter, iBaudRateAfter))
				{
					iBaudRateAfter = BAUDRATE_115200;
				}
			if (!pIniFile->FindVar(KCOMPortSection, KCOMUARTClock, iUartClock))
				{
					iUartClock = UART_24MHZ_CLOCK;
				}

			TPtrC PathText;
			TPtrC FileText;			
		
			//TBufC8<256> strFileExtension;

			if (!pIniFile->FindVar(KPatchramSection, KPatchramPath, PathText))
				{
				PathText.Set(KDefaultConfigFilePath);
				}
#if 1
			if (!pIniFile->FindVar(KPatchramSection, KPatchramFile, FileText))
				{
				FileText.Set(KDefaultConfigFileName);
				fileName.Format(KDefaultConfigFileFileFormat,&PathText, &FileText);					
				}
			else
				{
				fileName.Format(KConfigFileFileFormat,&PathText, &FileText);
				}
			}
		else
			{
			//FileText.Set(KDefaultConfigFileName);
			fileName.Format(KDefaultConfigFileFileFormat,&KDefaultConfigFilePath, &KDefaultConfigFileName);
			// ADD EXTENSION CHECK HERE
			}
		
		// Check if we have an hcd file to download
		if(fileName.Match(KFileHcdExtension) != KErrNotFound)
			{
			iFileFormat = EHcdFileFormat;
			delete pIniFile;
			}
		else
			{
			// Check if we have a binary file to download
			if(fileName.Match(KFileBinExtension) != KErrNotFound)
				{
				iFileFormat = EBinFileFormat;
				// check for ram location in ini file
				if (pIniFile->FindVar(KPatchramSection, KPatchramLocation, iRamLocation) != EFalse)
					KDestRamLocation = iRamLocation;
				else if (fileName.Find(_L("2048")) != KErrNotFound)
					KDestRamLocation = BCM2048_RAM_LOCATION;
				else if (fileName.Find(_L("4325")) != KErrNotFound)
					KDestRamLocation = BCM4325_RAM_LOCATION;
				else if (fileName.Find(_L("2046")) != KErrNotFound)
					KDestRamLocation = BCM2046_RAM_LOCATION;
				delete pIniFile;
				}
			else	//Unknown file type SHOULD NEVER FALL IN THIS CONDITION
				{
					iInitState = EIdle;
					MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
					__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
					hctlConfigInterface->MhciSetInitPluginState(EIdle);
					
					iFileFormat = EUnknownFileFormat;
					delete pIniFile;
					return;
				}			
			}
		}			
			
#else
		
		if (!pIniFile->FindVar(KPatchramSection, KPatchramFile, FileText))
			{
			FileText.Set(KDefaultConfigFileName);
			fileName.Format(KDefaultConfigFileFileFormat,&PathText, &FileText);
			}
		else
			{
			fileName.Format(KConfigFileFileFormat,&PathText, &FileText);
			}

		// check for ram location in ini file
		if (pIniFile->FindVar(KPatchramSection, KPatchramLocation, iRamLocation) != EFalse)
			KDestRamLocation = iRamLocation;
		else if (fileName.Find(_L("2048")) != KErrNotFound)
			KDestRamLocation = BCM2048_RAM_LOCATION;
		else if (fileName.Find(_L("4325")) != KErrNotFound)
			KDestRamLocation = BCM4325_RAM_LOCATION;
		else if (fileName.Find(_L("2046")) != KErrNotFound)
			KDestRamLocation = BCM2046_RAM_LOCATION;
		delete pIniFile;
		}
	else
		{
		fileName.Format(KDefaultConfigFileFileFormat,&KDefaultConfigFilePath, &KDefaultConfigFileName);
		}
	
	}
		
			
#endif
			
				
	// Open the config file 
	TRAP(err, iCConfigFileDownload = CConfigFileDownload::NewL(fileName));
	
	if (err != KErrNone)
		{
		iControllerInitialisationObserver->McioPostResetCommandComplete(err);
		iInitState = EIdle;
		MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
		__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
		hctlConfigInterface->MhciSetInitPluginState(EIdle);

		
		}
	else
		{
		// initiate Dwl config file
		TRAP(err, cmd = CVendorDebugCommand::NewL(KHciVSDownloadMiniDriver));
		if (err == KErrNone)
			{
			TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
			if(err != KErrNone)
				{
				// Inform the stack of the initialisation status.
				iControllerInitialisationObserver->McioPostResetCommandComplete(err);
				iInitState = EIdle;
				MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
				__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
				hctlConfigInterface->MhciSetInitPluginState(EIdle);
				}
			else
				{
				if(iFileFormat == EBinFileFormat)
					{
					iDestRam = KDestRamLocation;
					iTxPatchLength = 0;
					}
				iInitState = EloadingMiniDriver;
				MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
				__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
				hctlConfigInterface->MhciSetInitPluginState(EloadingMiniDriver);

				}
			}
		else
			{
			iControllerInitialisationObserver->McioPostResetCommandComplete(err);
			iInitState = EIdle;
			MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
			__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
			hctlConfigInterface->MhciSetInitPluginState(EIdle);
			}
		}
	}


void CControllerInitialisationPluginImpl::DoPostReset()
	{
	LOG_FUNC

	INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : CControllerInitialisationPluginImpl::DoPostReset()"))

	InitialiseAndLoadMiniDriver();

	}

/*virtual*/ void CControllerInitialisationPluginImpl::MciiPostResetCommand()
	{
	LOG_FUNC

	__ASSERT_DEBUG(iControllerInitialisationObserver, PANIC(KBroadcomControllerInitialisorPanic, ENullInitialisationObserver));

	// Use async callback
	iAsyncCallBackForPostReset->CallBack();
	}

void CControllerInitialisationPluginImpl::HandleVendorSpecificCompleteEvent(const TVendorDebugCompleteEvent& aEvent)
	{
	LOG_FUNC
	
	CVendorDebugCommand* cmd = NULL;
	TInt err = KErrNone;
	THCIErrorCode hcierr = EOK;
	

	switch(iInitState)
		{
		
		case EloadingMiniDriver:
			{	    			    	
			if (aEvent.CommandOpcode() == KHciVSDownloadMiniDriver )
				{
				INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : HCI VSC Download Mini Driver event (%04x)"), aEvent.CommandOpcode())

				hcierr = aEvent.ErrorCode();
				if (hcierr == EOK)
					{
				   	// Delay before actual patch ram download
				   	iMiniDrive2ConfigFileTimer->Start(KMiniDriverToPatchRamTimerDuration);
				   	if(iFileFormat == EBinFileFormat)
				   		{
						iInitState = EMiniDrvToBinConfigFileDelay;
						MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
						__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
						hctlConfigInterface->MhciSetInitPluginState(EMiniDrvToBinConfigFileDelay);
				   		}
				   	else if(iFileFormat == EHcdFileFormat)
				   		{
						iInitState = EMiniDrvToHcdConfigFileDelay;	
						MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
						__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
						hctlConfigInterface->MhciSetInitPluginState(EMiniDrvToHcdConfigFileDelay);
				   		}
					}
				}
			}
			break;
			
		case ELoadingHcdConfigFileData:
			{
			INITIALISOR_BCM_LOG(_L("WRITE RAM COMPLETE EVENT"))

			if (aEvent.CommandOpcode() == KHciVSWriteRam )
				{
				hcierr = aEvent.ErrorCode();
				if (hcierr == EOK)
					{
					THCIOpcode CommandOpcode;
					TUint8 DataChunkLen = 0;			
				
					iCConfigFileDownload->ReadCommandOpcode(CommandOpcode);
				
					TRAP(err, cmd = CVendorDebugCommand::NewL(CommandOpcode));
					if (err == KErrNone)	
						{
						TDes8& cmdData = cmd->Command();
				
						iCConfigFileDownload->ReadDataChunkLen(DataChunkLen);
										
						if(DataChunkLen != 0)
							{
							TBuf8<4> AddressBuf;
							TBuf8<KHcdConfigFileChunkSize> patchRamData;
							if(CommandOpcode == KHciVSLaunchRam)
								{
								
								err = iCConfigFileDownload->ConfigFileSendData(patchRamData,DataChunkLen);
								if (KErrNone == err)
									{
									cmd->Reset(KHciVSLaunchRam);
									cmdData.Append(patchRamData);
									INITIALISOR_BCM_LOG(_L("LAUNCH RAM"))
									iInitState = ELoadingconfigFileDataDone;
									TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
									
									MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
									__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
									hctlConfigInterface->MhciSetInitPluginState(ELoadingconfigFileDataDone);
									}
								}
							else
								{
								err = iCConfigFileDownload->SetDataChunkAddress(AddressBuf);
					
								if (KErrNone == err)
									{
									cmdData.FillZ(4);
									cmdData[0] = AddressBuf[0];
									cmdData[1] = AddressBuf[1];
									cmdData[2] = AddressBuf[2];
									cmdData[3] = AddressBuf[3];
																
									err = iCConfigFileDownload->ConfigFileSendData(patchRamData,(DataChunkLen-4));
									
									if (KErrNone == err)
										{
										cmdData.Append(patchRamData);
										TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
										INITIALISOR_BCM_LOG(_L("WRITE RAM"))						
										}
									}
								}
							}
						}
						
					if(KErrNone != err)
						{
							// something went wrong during the file read
							delete cmd; //command was not sent due to read file error
							
							// Inform the stack of the initialisation status.
							iControllerInitialisationObserver->McioPostResetCommandComplete(err);
							iInitState = EIdle;
							MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
							__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
							hctlConfigInterface->MhciSetInitPluginState(EIdle);
						}					
					}//hcierr == EOK
				}//aEvent.CommandOpcode() == KHciVSWriteRam
			else
				{
				err = KErrGeneral; //complete event is not the expected one
				}
			}
			break;
			
		case ELoadingBinConfigFileData:
			{
			if (aEvent.CommandOpcode() == KHciVSWriteRam )
				{
				hcierr = aEvent.ErrorCode();
				if (hcierr == EOK)
				    {
		    	    //Update the Destination Ram address
		    		iDestRam = iDestRam + iTxPatchLength;
		    		
				    TRAP(err, cmd = CVendorDebugCommand::NewL(KHciVSWriteRam));
					if (err == KErrNone)	
						{
						TBuf8<KConfigFileChunkSize> patchRamData;
						
						// Try to read config file data
						err = iCConfigFileDownload->ConfigFileSendDataChunk(patchRamData);
						if (KErrEof == err)
							{
							// EOF reached
							iInitState = ELoadingconfigFileDataDone;
							
							MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
							__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
							hctlConfigInterface->MhciSetInitPluginState(ELoadingconfigFileDataDone);
							
							// Launch Patch ram
							cmd->Reset(KHciVSLaunchRam); 
							// reuse allocated cmd
							
							TDes8& cmdData = cmd->Command();

							cmdData.FillZ(4);
							cmdData[0] = 0xFF;
							cmdData[1] = 0xFF;
							cmdData[2] = 0xFF;
							cmdData[3] = 0xFF;
							
							TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
							// Wait for the launch config file complete event 
							
							}
					    else if (KErrNone == err) 
					    	{					    										

					    		TDes8& cmdData = cmd->Command();

								INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : HCI VSC Write RAM event (%04x)"), aEvent.CommandOpcode())

								cmdData.FillZ(4);
								cmdData[0] =  (iDestRam)        & 0xFF;
								cmdData[1] = ((iDestRam) >>  8) & 0xFF;
								cmdData[2] = ((iDestRam) >> 16) & 0xFF;
								cmdData[3] = ((iDestRam) >> 24) & 0xFF;

								cmdData.Append(patchRamData);
								
					    		// 	Send another chunk of config file data
								TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
								if (KErrNone == err)
									{
									// Update length of Tx'd data
									iTxPatchLength = patchRamData.Length();
									iInitState = ELoadingBinConfigFileData;
									
									MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
									__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
									hctlConfigInterface->MhciSetInitPluginState(ELoadingBinConfigFileData);
									// Wait for the VSC complete event
									}
					    	}
					    else if (KErrNone != err)
					    	{
					    	// something went wrong during the file read
					    	delete cmd; //command was not sent due to read file error
					    	}
						}
				    }
				else
					{
					err = KErrGeneral; //complete event is not the expected one
					}

				if (err != KErrNone)
					{
					iControllerInitialisationObserver->McioPostResetCommandComplete(err);
					iInitState = EIdle;
					MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
					__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
					hctlConfigInterface->MhciSetInitPluginState(EIdle);
					}
				}
			}
			break;
			
		case ELoadingconfigFileDataDone:
			{
			INITIALISOR_BCM_LOG(_L("LAUNCH RAM COMPLETE EVENT"))
			if (aEvent.CommandOpcode() == KHciVSLaunchRam)
				{
				INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : HCI VSC Launch RAM event (%04x)"), aEvent.CommandOpcode())

				hcierr = aEvent.ErrorCode();
				if (hcierr == EOK)
					{
				    iMiniDrive2ConfigFileTimer->Start(KEndDelayTimerDuration);
				    iInitState = EWaitEndDelay;
				    
					MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
					__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
					hctlConfigInterface->MhciSetInitPluginState(EWaitEndDelay);
					}
				}
			}
			break;
			
			
		case EWaitUpdateBaudRateAfterPrm:
			{
#ifdef UPDATEBAUDRATE_AFTER_PRD
			INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : VSC Update BaudRate event (%04x)"), aEvent.CommandOpcode())

			MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
			if (aEvent.CommandOpcode() == KHciVSUpdateUartHCIBaudRate)
				{
				hcierr = aEvent.ErrorCode();
				if (hcierr == EOK)
					{
					INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : Change UART BaudRate to %d bps after launching RAM"), iBaudRateAfter)

					err = hctlConfigInterface->MhciUpdateBaudRate(iBaudRateAfter);
					}
				else
					{
					err = KErrGeneral;
					}		
				}
			else			
				{
				err = KErrGeneral;
				}
#else
			err = KErrNone;
#endif			
			// BCM initialisation is completed- resume generic init
			iControllerInitialisationObserver->McioPostResetCommandComplete(err);
			iInitState = EIdle;
			hctlConfigInterface->MhciSetInitPluginState(EIdle);
			
			//iInitState = EReset;	
			// Signal to everyone that we are ready 
			//BCM_DriverReadySem.CreateGlobal(KGlobalReadySemName,1,EOwnerProcess);
			}
			break;
			
		default:
		INITIALISOR_BCM_LOG2(_L("Broadcom: CControllerInitialisationPluginImpl::HandleVendorSpecificCompleteEvent - Default state (%d)"),iInitState)

			break;
	    }
	}

/*virtual*/ void CControllerInitialisationPluginImpl::ConfigFileTimerExpired()
	{
	LOG_FUNC
	
	TInt err = KErrNone;
	CVendorDebugCommand* cmd = NULL;

	switch(iInitState)
		{
		case EMiniDrvToHcdConfigFileDelay:
			{
			THCIOpcode CommandOpcode;
			TUint8 DataChunkLen = 0;			
			
			INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : Start patch ram download"))
			
			iCConfigFileDownload->ReadCommandOpcode(CommandOpcode);
			
			TRAP(err, cmd = CVendorDebugCommand::NewL(CommandOpcode));
			TDes8& cmdData = cmd->Command();
			
			iCConfigFileDownload->ReadDataChunkLen(DataChunkLen);
									
			if(DataChunkLen != 0)
				{
				//TBuf8<4> AddressBuf;
				TBuf8<4> AddressBuf;
				
				err = iCConfigFileDownload->SetDataChunkAddress(AddressBuf);
				if (KErrNone == err)
					{
					cmdData.FillZ(4);
					cmdData[0] = AddressBuf[0];
					cmdData[1] = AddressBuf[1];
					cmdData[2] = AddressBuf[2];
					cmdData[3] = AddressBuf[3];
				
					TBuf8<KHcdConfigFileChunkSize> HcdpatchRamData;
					
					// Substract 4 from DataChunkLen for download address
					err = iCConfigFileDownload->ConfigFileSendData(HcdpatchRamData,(DataChunkLen-4));
					if (KErrNone == err)
						{
						cmdData.Append(HcdpatchRamData);
						TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
						INITIALISOR_BCM_LOG(_L("FIRST WRITE RAM"))
						if (KErrNone == err)
							{
							iInitState = ELoadingHcdConfigFileData;
							
							MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
							__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
							hctlConfigInterface->MhciSetInitPluginState(ELoadingHcdConfigFileData);
							}
						}
					}

				if(KErrNone != err)
					{
					// something went wrong during the file read
					delete cmd; //command was not sent due to read file error
						
					// Inform the stack of the initialisation status.
					iControllerInitialisationObserver->McioPostResetCommandComplete(err);
					iInitState = EIdle;
					MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
					__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
					hctlConfigInterface->MhciSetInitPluginState(EIdle);
					}
				
				}
			}
			break;
			
		case EMiniDrvToBinConfigFileDelay:
			{
			INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : Waiting for Mini download Driver"))
			// initiate actual downloading of the Config file
		    TRAP(err, cmd = CVendorDebugCommand::NewL(KHciVSWriteRam));
			if (err == KErrNone)	
				{				
				TBuf8<KConfigFileChunkSize> patchRamData;
				TDes8& cmdData = cmd->Command();

				err = iCConfigFileDownload->ConfigFileSendDataChunk(patchRamData);
				
				// Start sending config file data
				if (KErrNone == err)
					{
					cmdData.FillZ(4);
					cmdData[0] =  (KDestRamLocation)        & 0xFF;
					cmdData[1] = ((KDestRamLocation) >>  8) & 0xFF;
					cmdData[2] = ((KDestRamLocation) >> 16) & 0xFF;
					cmdData[3] = ((KDestRamLocation) >> 24) & 0xFF;
					
					cmdData.Append(patchRamData);
					TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
					if (KErrNone == err)
						{
						// Update length of Tx'd data
						iTxPatchLength = iTxPatchLength + patchRamData.Length();
						iInitState = ELoadingBinConfigFileData;
						
						MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
						__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
						hctlConfigInterface->MhciSetInitPluginState(ELoadingBinConfigFileData);
						// Wait for the VSC complete event
						}
					}
				else
					{
			    	// something went wrong during the file read
			    	delete cmd; //command was not sent due to read file error
					}
				}
			
				
				if (KErrNone != err)
					{
					// Inform the stack of the initialisation status.
					iControllerInitialisationObserver->McioPostResetCommandComplete(err);
					iInitState = EIdle;
					
					MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
					__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
					hctlConfigInterface->MhciSetInitPluginState(EIdle);
					}
				}
			break;
			
		case EWaitEndDelay:
			{
#ifdef UPDATEBAUDRATE_AFTER_PRD			
			// BCM initialisation is completed- resume generic init
			INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : END Initialisation steps (PatchRAM downloaded OK))"))
			
			// Request baud rate update.
			CVendorDebugCommand* cmd = NULL;
			TRAP(err, cmd = CVendorDebugCommand::NewL(KHciVSUpdateUartHCIBaudRate));
			TDes8& cmdData = cmd->Command();

			if(err == KErrNone)
			{	
				INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : Change BaudRate to %d bps after launchRAM"), iBaudRateAfter)
				INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : UART Clock is %d"), iUartClock)
				cmdData.FillZ(6);
				cmdData[2] = iBaudRateAfter 		& 0xFF;
				cmdData[3] = (iBaudRateAfter >> 8)	& 0xFF;
				cmdData[4] = (iBaudRateAfter >> 16)	& 0xFF;
				cmdData[5] = (iBaudRateAfter >> 24)	& 0xFF;
				
				TRAP(err, iHCICommandQueue->MhcqAddInitCommandL(cmd, *this));
				// error handling if the method above fails is handled in at the end of the function
//				INITIALISOR_BCM_LOG2(_L("Initialisor_Broadcom : Send %x %x %x %x %x %x after launchRAM OK"), iBaudRateAfter)
				RDebug::Print(_L("Initialisor_Broadcom : Send %x %x %x %x %x %x after launchRAM OK"), 
						cmdData[0],
						cmdData[1],
						cmdData[2],
						cmdData[3],
						cmdData[4],
						cmdData[5]);
				iInitState = EWaitUpdateBaudRateAfterPrm;
				
				MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
				__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
				hctlConfigInterface->MhciSetInitPluginState(EWaitUpdateBaudRateAfterPrm);
			}
#else
			iInitState = EWaitUpdateBaudRateAfterPrm;
			MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
			__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
			hctlConfigInterface->MhciSetInitPluginState(EWaitUpdateBaudRateAfterPrm);
			
			iControllerInitialisationObserver->McioPostResetCommandComplete(err);
#endif


			if(err != KErrNone)
			{
				iInitState = EIdle;	
				
				MHctlBcmConfigInterface* hctlConfigInterface = static_cast<MHctlBcmConfigInterface*>(iCoreHci->MchHctlPluginInterface(TUid::Uid(KHctlBcmConfigInterfaceUid)));	
				__ASSERT_DEBUG(hctlConfigInterface, PANIC(KBroadcomControllerInitialisorPanic, EConfigInterfaceNull));			
				hctlConfigInterface->MhciSetInitPluginState(EIdle);
				
				iControllerInitialisationObserver->McioPostResetCommandComplete(err);
			}
			
			// Signal to everyone that we are ready 
			//BCM_DriverReadySem.CreateGlobal(KGlobalReadySemName,1,EOwnerProcess);

			}
			break;
			
		default:
		break;
		}
	}


/*virtual*/ void CControllerInitialisationPluginImpl::MciiSetHCICommandQueue(MHCICommandQueue& aHCICommandQueue)
	{
	LOG_FUNC

	__ASSERT_DEBUG(iHCICommandQueue == 0, PANIC(KBroadcomControllerInitialisorPanic, EHciCommandQueueOverwritten));
	iHCICommandQueue = &aHCICommandQueue;
	}

/*virtual*/ void CControllerInitialisationPluginImpl::MciiSetControllerInitialisationObserver(MControllerInitialisationObserver& aObserver)
	{
	LOG_FUNC

	__ASSERT_DEBUG(iControllerInitialisationObserver == 0, PANIC(KBroadcomControllerInitialisorPanic, EInitialisationObserverOverwritten));
	iControllerInitialisationObserver = &aObserver;
	}
void CControllerInitialisationPluginImpl::MhcqcCommandErrored(TInt aErrorCode,
					  										  const CHCICommandBase* /*aCommand*/)
	{
	LOG_FUNC

	switch(iInitState)
		{
		default:
			PANIC(KBroadcomControllerInitialisorPanic, EInvalidInitialisorState);
			break;
		};
	}
	
void CControllerInitialisationPluginImpl::MhcqcCommandEventReceived(const THCIEventBase& aEvent,
																	const CHCICommandBase* /*aRelatedCommand*/)
	{
	LOG_FUNC

	if (aEvent.EventCode() == ECommandCompleteEvent)
		{
		// Switch on the opcode.
		THCICommandCompleteEvent& event = THCICommandCompleteEvent::Cast(aEvent);
		switch(event.CommandOpcode())
			{

			default:
				// Check for VS commands.
				if((event.CommandOpcode() & KVendorDebugOGF) == KVendorDebugOGF)
					{
					TVendorDebugCompleteEvent& vEvent = TVendorDebugCompleteEvent::Cast(event);
					HandleVendorSpecificCompleteEvent(vEvent);
					}						
				break;	
			}
		}
	}	

/*virtual*/ void CControllerInitialisationPluginImpl::MciiSetCoreHci(MCoreHci& aCoreHci)
	{
	LOG_FUNC

	INITIALISOR_BCM_LOG(_L("Initialisor_Broadcom : Setting Core HCI"))
	__ASSERT_DEBUG(iCoreHci == 0, PANIC(KBroadcomControllerInitialisorPanic, ECoreHciOverwritten));
    iCoreHci = &aCoreHci;
	}

