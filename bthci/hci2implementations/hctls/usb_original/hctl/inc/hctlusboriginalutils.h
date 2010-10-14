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


#ifndef HCTLUSBORIGINALUTILS_H
#define HCTLUSBORIGINALUTILS_H

#include <e32base.h>

/**
The panic category for the USB Original HCTL.
*/
_LIT(KUsbOriginalPanic, "HctlUsbOrigPanic");

/**
Panic codes for USB Original HCTL.
*/
enum TUsbOriginalPanic
	{
	EUnexpectedCtrlMgrPowerState		= 0,
	EReadAttemptWhenReadOutstanding		= 1,
	EStateObserverNotAvailable			= 2,
	EStartCalledWhenReadOutstanding		= 3,
	ENoChannelObserver					= 4,
	EPartiallyOpenedInterface			= 5,
	EBadSessionPointer					= 6,
	EStackOnWhenNoDeviceConnected		= 7,
	};

#endif // HCTLUSBORIGINALUTILS_H

