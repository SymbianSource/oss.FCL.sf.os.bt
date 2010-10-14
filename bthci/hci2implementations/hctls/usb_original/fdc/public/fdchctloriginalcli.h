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

#ifndef FDCHCTLORIGINALCLI_H
#define FDCHCTLORIGINALCLI_H

#include <e32base.h>
#include <fdchctloriginalipc.h>

NONSHARABLE_CLASS(RFdcHctlOriginal)
	: public RSessionBase
	{
public:
	inline RFdcHctlOriginal();
	
	inline TInt Connect();
	inline void Close();
	
	inline void RequestConnection();
	
private:
	TAny* iExtension;
	};

inline RFdcHctlOriginal::RFdcHctlOriginal()
	: iExtension(NULL)
	{}

inline TInt RFdcHctlOriginal::Connect()
	{
	// We are not able to "start" the server, as it is set running when the function driver
	// is loaded. Instead we just want to try and connect and ask for the provision of any
	// hardware that is available.
	return CreateSession(
						KFdcHctlOrigSrvName,
						TVersion(
							KFdcHctlOrigSrvMajorVersionNumber,
							KFdcHctlOrigSrvMinorVersionNumber,
							KFdcHctlOrigSrvBuildNumber
							)
						);
	}

inline void RFdcHctlOriginal::Close()
	{
	RSessionBase::Close();
	}

inline void RFdcHctlOriginal::RequestConnection()
	{
	// We can always fail here - this is a best effort request.
	Send(EFunctionRequestConnection);
	}

#endif // FDCHCTLORIGINALCLI_H