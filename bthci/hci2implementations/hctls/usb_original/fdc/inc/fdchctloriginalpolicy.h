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
 
#ifndef FDCHCTLORIGINALPOLICY_H
#define FDCHCTLORIGINALPOLICY_H

#include <e32def.h>
#include <fdchctloriginalipc.h>

/** FDC HCTL Original Server Security Policy Definition */
const TUint KFdcHctlOriginalRangeCount = 2;

const TInt KFdcHctlOriginalRanges[KFdcHctlOriginalRangeCount] =
	{
	EFunctionRequestConnection,			// ESock SID
	ENumFdcHctlOriginalSrvFuncs,		// not supported
	};

/** Index numbers into KFdcHctlOriginalElements[] */
const TInt KPolicyEsockSid = 0;

/** Mapping IPCs to policy element */
const TUint8 KFdcHctlOriginalElementsIndex[KFdcHctlOriginalRangeCount] =
	{
	KPolicyEsockSid,				// All (valid) APIs
	CPolicyServer::ENotSupported	// Undefined functions
	};

/** Individual policy elements */
const CPolicyServer::TPolicyElement KFdcHctlOriginalElements[] = 
	{
		{ _INIT_SECURITY_POLICY_S0(0x101f7989) },
	};

/** Main policy */
const CPolicyServer::TPolicy KFdcHctlOriginalPolicy =
	{
	KPolicyEsockSid, // connection requirements
	KFdcHctlOriginalRangeCount,
	KFdcHctlOriginalRanges,
	KFdcHctlOriginalElementsIndex,
	KFdcHctlOriginalElements,
	};

#endif // FDCHCTLORIGINALPOLICY_H
