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

#ifndef FDCHCTLORIGINALIPC_H
#define FDCHCTLORIGINALIPC_H

#include <e32base.h>

//
// FDC Server IPC
//

_LIT(KFdcHctlOrigSrvName, "!FdcHctlOriginal");

/** Version numbers. */
const TInt8 KFdcHctlOrigSrvMajorVersionNumber = 1;
const TInt8 KFdcHctlOrigSrvMinorVersionNumber = 0;
const TInt16 KFdcHctlOrigSrvBuildNumber = 0;

enum TFdcHctlOriginalSrvFuncs
	{
	EFunctionRequestConnection = 0,
	/* Add new server functions before this */
	ENumFdcHctlOriginalSrvFuncs // Never use as an actual function number
	};


_LIT(KFdcHctlOriginalSrvPanicCat, "FdcHctlOriginalSrv");

enum TFdcHctlOriginalSrvPanics
	{
	EFdcHctlInvalidFunction	= 0,
	};

#endif // FDCHCTLORIGINALIPC_H