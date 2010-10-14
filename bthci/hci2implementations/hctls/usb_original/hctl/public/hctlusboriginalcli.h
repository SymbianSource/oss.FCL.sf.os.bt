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

#ifndef HCTLUSBORIGINALCLI_H
#define HCTLUSBORIGINALCLI_H

#include <e32base.h>
#include <hctlusboriginalipc.h>

NONSHARABLE_CLASS(RHCTLUsbOriginal) : public RSessionBase
	{
public:
	inline RHCTLUsbOriginal();

	inline TInt Connect();
	inline void Close();

	inline TInt DeviceAttached(TUint32 aAclToken, TUint32 aScoToken);
	inline void DeviceDetached();

private:
	TAny* iExtension;
	};


inline RHCTLUsbOriginal::RHCTLUsbOriginal()
	: iExtension(NULL)
	{}

inline TInt RHCTLUsbOriginal::Connect()
	{
	// We don't want to start the server - that is a function of loading the bluetooth
	// stack.  Instead we just want to try and connect and report on whether we succeeded.
	return CreateSession(
						KHCTLUsbOrginalSrvName,
						TVersion(
							KHCTLUsbOriginalSrvMajorVersionNumber,
							KHCTLUsbOriginalSrvMinorVersionNumber,
							KHCTLUsbOriginalSrvBuildNumber
							)
						);
	}

inline void RHCTLUsbOriginal::Close()
	{
	RSessionBase::Close();
	}

inline TInt RHCTLUsbOriginal::DeviceAttached(TUint32 aAclToken, TUint32 aScoToken)
	{
	return SendReceive(EFunctionAttached, TIpcArgs(aAclToken, aScoToken));
	}

inline void RHCTLUsbOriginal::DeviceDetached()
	{
	Send(EFunctionDetached);
	}

#endif // HCTLUSBORIGINALCLI_H