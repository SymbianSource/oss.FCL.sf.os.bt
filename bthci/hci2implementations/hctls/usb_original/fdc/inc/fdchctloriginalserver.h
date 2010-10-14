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

#ifndef FDCHCTLORIGINALSERVER_H
#define FDCHCTLORIGINALSERVER_H

#include <e32base.h>

class CFdcHctlOriginal;

NONSHARABLE_CLASS(CFdcHctlOriginalSession)
	: public CSession2
	{
public:
	CFdcHctlOriginalSession(CFdcHctlOriginal& aHctlOriginal);
	~CFdcHctlOriginalSession();
	
private:
	void ServiceL(const RMessage2& aMessage);
	
private:
	CFdcHctlOriginal& iHctlOriginal;
	};


NONSHARABLE_CLASS(CFdcHctlOriginalServer)
	: public CPolicyServer
	{
public:
	static CFdcHctlOriginalServer* NewL(CFdcHctlOriginal& aHctlOriginal);
	~CFdcHctlOriginalServer();
	
	void DropSession(CFdcHctlOriginalSession* aSession) const;
	
private:
	CFdcHctlOriginalServer(CFdcHctlOriginal& aHctlOriginal);
	void ConstructL();
	
	CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
	
private:
	CFdcHctlOriginal& iHctlOriginal;
	mutable CFdcHctlOriginalSession* iSession;
	};

#endif // FDCHCTLORIGINALSERVER_H
