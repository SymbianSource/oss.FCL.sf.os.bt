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

#ifndef HCTLUSBORIGINALIPC_H
#define HCTLUSBORIGINALIPC_H

#include <e32base.h>

//
// HCTL Server IPC
//

_LIT(KHCTLUsbOrginalSrvName, "!HctlUsbOriginal");

/** Version numbers. */
const TInt8 KHCTLUsbOriginalSrvMajorVersionNumber = 1;
const TInt8 KHCTLUsbOriginalSrvMinorVersionNumber = 0;
const TInt16 KHCTLUsbOriginalSrvBuildNumber = 0;

enum THCTLUsbOriginalSrvFuncs
	{
	EFunctionAttached = 0,
	EFunctionDetached = 1,
	/* Add new server functions before this */
	ENumHctlUsbOriginalSrvFuncs // Never use as an actual function number
	};
	
_LIT(KHCTLUsbOriginalServerPanicCat, "HCTLUsbOriginalServer");

enum THCTLUsbOriginalServerPanics
	{
	EInvalidFunction		= 0,
	EDeviceAlreadyAttached	= 1,
	EDeviceNotAttached		= 2,
	};

#endif // HCTLUSBORIGINALIPC_H
