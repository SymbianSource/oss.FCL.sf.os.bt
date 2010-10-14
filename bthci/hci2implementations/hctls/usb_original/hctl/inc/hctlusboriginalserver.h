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

#ifndef HCTLUSBORIGINALSERVER_H
#define HCTLUSBORIGINALSERVER_H

#include <e32base.h>

class CHCTLUsbOriginal;

NONSHARABLE_CLASS(CHCTLUsbOriginalSession)
	: public CSession2
	{
public:
	CHCTLUsbOriginalSession(CHCTLUsbOriginal& aUsbOriginal);
	~CHCTLUsbOriginalSession();
	
private:
	void ServiceL(const RMessage2& aMessage);
	
private:
	CHCTLUsbOriginal& iUsbOriginal;
	TBool iDeviceAttached;
	};


NONSHARABLE_CLASS(CHCTLUsbOriginalServer)
	: public CPolicyServer
	{
public:
	static CHCTLUsbOriginalServer* NewL(CHCTLUsbOriginal& aUsbOriginal);
	~CHCTLUsbOriginalServer();
	
	void DropSession(CHCTLUsbOriginalSession* aSession) const;
	
private:
	CHCTLUsbOriginalServer(CHCTLUsbOriginal& aUsbOriginal);
	void ConstructL();
	
	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
	
private:
	CHCTLUsbOriginal& iUsbOriginal;
	mutable CHCTLUsbOriginalSession* iSession;
	};

#endif // HCTLUSBORIGINALSERVER_H
