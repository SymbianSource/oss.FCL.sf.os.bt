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
 
#ifndef HCTLUSBORIGINALPOLICY_H
#define HCTLUSBORIGINALPOLICY_H

#include <e32def.h>
#include <hctlusboriginalipc.h>

/** HCTL USB Original Server Security Policy Definition */
const TUint KHCTLUsbOriginalRangeCount = 2;

const TInt KHCTLUsbOriginalRanges[KHCTLUsbOriginalRangeCount] =
	{
	EFunctionAttached,					// FDF SID
//	EFunctionDetached,
	ENumHctlUsbOriginalSrvFuncs,		// not supported
	};

/** Index numbers into KHCTLUsbOriginalElements[] */
const TInt KPolicyFdfSid = 0;

/** Mapping IPCs to policy element */
const TUint8 KHCTLUsbOriginalElementsIndex[KHCTLUsbOriginalRangeCount] =
    {
    KPolicyFdfSid,					// All (valid) APIs
    CPolicyServer::ENotSupported	// Undefined functions
    };

/** Individual policy elements */
const CPolicyServer::TPolicyElement KHCTLUsbOriginalElements[] =
    {
        { _INIT_SECURITY_POLICY_S0(0x10282b48) },
    };

/** Main policy */
const CPolicyServer::TPolicy KHCTLUsbOriginalPolicy =
    {
    KPolicyFdfSid, // connection requirements
    KHCTLUsbOriginalRangeCount,
    KHCTLUsbOriginalRanges,
    KHCTLUsbOriginalElementsIndex,
    KHCTLUsbOriginalElements,
    };
    
#endif // HCTLUSBORIGINALPOLICY_H
