// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef TAVSRCSOCK_H
#define TAVSRCSOCK_H

#include "tavsrc.h"
#include "tavsrcConsole.h"

#include <e32base.h>

class CActivePacketDropIoctl : public CActive
	{
public:
	static CActivePacketDropIoctl* NewL(CActiveConsole* aLogConsole, RSocketArray aPendingSockets);
	~CActivePacketDropIoctl();
	void DoCancel();
	void RunL();
	void Start();

private:
	CActivePacketDropIoctl(CActiveConsole* aLogConsole, RSocketArray aPendingSockets);

private:
	CActiveConsole* iLogConsole;
	RSocketArray iPendingSockets;
	TPckgBuf<TInt> iPacketsLostPkgBuf;
	};

class CActiveSockWriter : public CActive
	{
public:
	static CActiveSockWriter* NewL(RSocket aSock, TAvdtpTransportSessionType aType);
	~CActiveSockWriter();
	void Send();
	void DoCancel();
	void RunL();

private:
	CActiveSockWriter(RSocket aSock);
	void ConstructL(TAvdtpTransportSessionType aType);
	
private:
	RSocket	iSock;
	CConsoleBase* iConsole;
	HBufC8* iSendBuffer;
	TAvdtpTransportSessionType iType;
	};

class CActiveSockReader : public CActive
	{
public:
	static CActiveSockReader* NewL(RSocket aSock, TAvdtpTransportSessionType aType);
	~CActiveSockReader();
	void Start();

private:
	CActiveSockReader(RSocket aSock);
	void ConstructL(TAvdtpTransportSessionType aType);
	void DoCancel();
	void RunL();

private:
	RSocket	iSock;
	CConsoleBase* iConsole;
	HBufC8*	iBuffer;
	TPtr8 iBufferDes;
	TSockXfrLength iLen;
	TAvdtpTransportSessionType iType;
	};

#endif // TAVSRCSOCK_H
