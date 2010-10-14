// Copyright (c) 2007-2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "fdchctloriginal.h"

#include <d32usbdescriptors.h>
#include <usbhost/internal/fdcpluginobserver.h>
#include <usbhosterrors.h>
#include <bluetooth/logger.h>

#include "fdchctloriginalserver.h"

#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, "fdchctloriginal");
#endif


CFdcHctlOriginal* CFdcHctlOriginal::NewL(MFdcPluginObserver& aObserver)
	{
	LOG_STATIC_FUNC
	CFdcHctlOriginal* self = new(ELeave) CFdcHctlOriginal(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CFdcHctlOriginal::CFdcHctlOriginal(MFdcPluginObserver& aObserver)
	: CFdcPlugin(aObserver)
	{
	LOG_FUNC
	}

CFdcHctlOriginal::~CFdcHctlOriginal()
	{
	LOG_FUNC
	iHctlSession.Close();
	delete iServer;
	}

void CFdcHctlOriginal::ConstructL()
	{
	LOG_FUNC
	iServer = CFdcHctlOriginalServer::NewL(*this);
	}

TAny* CFdcHctlOriginal::GetInterface(TUid aUid)
	{
	LOG_FUNC
	LOG1(_L8("\taUid = 0x%08x"), aUid);
	
	TAny* ret = NULL;
	if(aUid == TUid::Uid(KFdcInterfaceV1))
		{
		ret = reinterpret_cast<TAny*>(static_cast<MFdcInterfaceV1*>(this));
		}
	
	LOG1(_L8("\tret = [0x%08x]"), ret);
	return ret;
	}

TInt CFdcHctlOriginal::Mfi1NewFunction(TUint aDeviceId,
		const TArray<TUint>& aInterfaces,
		const TUsbDeviceDescriptor& aDeviceDescriptor,
		const TUsbConfigurationDescriptor& aConfigurationDescriptor)
	{
	LOG_FUNC
	
	// Normally an FDC is required to claim it's interfaces first.  In this
	// case we need to parse the descriptor tree to determine if the device
	// firmware update interface is present. (We don't do firmware updates but
	// we claim it until to provide successful device attachments. If a
	// firmware update FDC is provided this should be removed.)
#ifdef SYMBIAN_FDC_HCTL_ORIGINAL_ACCEPT_FIRMWARE_UPDATE
	TBool firmwareIntFound = EFalse;
	TUint8 firmwareIntNum = 0xff;
	
	TUint8 KDfuInterfaceClass = 0xfe;
	TUint8 KDfuInterfaceSubClass = 0x01;
	
	// Drop down a level from the configuration descriptor.
	TUsbGenericDescriptor* descriptor = aConfigurationDescriptor.iFirstChild;
	// Search across the interface tier (note doesn't handle DFU in IAD).
	while(descriptor)
		{
		TUsbInterfaceDescriptor* interface;
		if (interface = TUsbInterfaceDescriptor::Cast(descriptor), interface)
			{
			if(	interface->InterfaceClass() == KDfuInterfaceClass &&
				interface->InterfaceSubClass() == KDfuInterfaceSubClass)
				{
				firmwareIntNum = interface->InterfaceNumber();
				firmwareIntFound = ETrue;
				break;
				}
			}
		descriptor = descriptor->iNextPeer;
		}
#endif // SYMBIAN_FDC_HCTL_ORIGINAL_ACCEPT_FIRMWARE_UPDATE
	
	// We claim the interfaces we are to represent, we must claim the
	// first interface given to us as FDF has already determined that
	// we are to use it.
	const TUint KNumOfHctlInterfaces = 2;
	const TUint8 KAclInterfaceNum = 0x00, KScoInterfaceNum = 0x01;
	TBool gotAcl = EFalse, gotSco = EFalse, fatalError = EFalse;
	
	for(TInt i=0; i<aInterfaces.Count(); ++i)
		{
		TUint intNum = aInterfaces[i];
		if(intNum == KAclInterfaceNum)
			{
			fatalError = (fatalError || gotAcl); // The USB device should only report one ACL interface
			iAclToken = Observer().TokenForInterface(intNum);
			gotAcl = ETrue;
			}
		else if(intNum == KScoInterfaceNum)
			{
			fatalError = (fatalError || gotSco); // The USB device should only report one ACL interface
			iScoToken = Observer().TokenForInterface(intNum);
			gotSco = ETrue;
			}
		else if(i == 0)
			{
			// We always need to claim the first interface, this should have
			// been claimed already, but if we have a funny device then this
			// might not be the case.  We will need to error.
			TUint32 unknownToken = Observer().TokenForInterface(intNum);
			return KErrCorrupt;
			}
#ifdef SYMBIAN_FDC_HCTL_ORIGINAL_ACCEPT_FIRMWARE_UPDATE
		else if(firmwareIntFound && intNum == firmwareIntNum)
			{
			TUint32 dfuToken = Observer().TokenForInterface(intNum);
			}
#endif // SYMBIAN_FDC_HCTL_ORIGINAL_ACCEPT_FIRMWARE_UPDATE
		}
	// At this point we will have claimed to the interface mandated by FDF and
	// so we are at liberty to return an error.
	
	// firstly, check to see if a fatal error occured.
	if(fatalError)
		{
		LOG(_L8("\tFatal error when retrieving interfaces for driver instance..."));
		return KErrGeneral;
		}
	
	// Now we perform some validation that the function is what we expect.
	// There should be two interfaces as part of the function. One is
	// the standard data and control planes.  The other for the sync-
	// chronous connections.
	if(aConfigurationDescriptor.NumInterfaces() < KNumOfHctlInterfaces)
		{
		LOG(_L8("\tInsufficent interfaces in USB config. descriptor..."));
		return KErrUsbBadDescriptor;
		}
	if(aInterfaces.Count() < KNumOfHctlInterfaces)
		{
		LOG(_L8("\tInsufficient interfaces provided to FDC..."));
		return KErrUnderflow;
		}
	
	// Ensure that we got both interfaces, otherwise the device is malformed.
	if(!gotAcl || !gotSco)
		{
		LOG2(_L8("\tMissing Token [ACL=%d] [SCO=%d]"), gotAcl, gotSco);
		return KErrNotFound;
		}
	
	// At this point we are set-up to use the device.
	iReady = ETrue;
	
	// We try our best; the Bluetooth stack may not be running, it may not be even using
	// the USB HCTL.  So we accept the tokens and try the best to set-up the HCTL.
	// If we fail now the HCTL should later inform us to try again.
	RequestConnection();
	
	return KErrNone;
	}

void CFdcHctlOriginal::Mfi1DeviceDetached(TUint aDeviceId)
	{
	LOG_FUNC
	
	iReady = EFalse;
	
	if(iHctlSession.Handle())
		{
		// Inform of disconnection.
		iHctlSession.DeviceDetached();
		}
	// Close Hctl Handle
	iHctlSession.Close();
	}

void CFdcHctlOriginal::RequestConnection()
	{
	LOG_FUNC
	// Trigger the attempt to connect to the USB HCTL Server.
	
	if(!iReady)
		{
		// For whatever reason, we have not got the tokens for the USB interfaces.
		LOG(_L8("\tFDC is not ready"));
		return;
		}
	
	// Note that this will error if, the bt thread or usb hctl is not running;
	// or if there is another FDC already connected to it.
	TInt err = iHctlSession.Connect();
	if(err != KErrNone)
		{
		LOG1(_L8("\tRHCTLUsbOriginal::Connect error = %d"), err);
		return;
		}
	
	// Now inform the stack that we have a connected device.
	err = iHctlSession.DeviceAttached(iAclToken, iScoToken);
	if(err != KErrNone)
		{
		LOG1(_L8("\tRHCTLUsbOriginal::DeviceAttached error = %d"), err);
		iHctlSession.Close();
		return;
		}
	}
	



