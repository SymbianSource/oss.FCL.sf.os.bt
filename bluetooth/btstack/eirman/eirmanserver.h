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

/**
 @file
 @internalComponent
*/

#ifndef _EIRMANSERVER_H
#define _EIRMANSERVER_H

#include <e32base.h>

class CEirManSession;
class CEirManager;
class MHCICommandQueue;
class CLinkMgrProtocol;

enum TEirFeatureState
	{
	EEirFeaturePending = 0,
	EEirFeatureReady = 1,
	EEirFeatureNotSupported = 2,
	};

/**
The EIR Manager server class.
*/
NONSHARABLE_CLASS(CEirManServer) : public CPolicyServer
	{
public:
	static CEirManServer* NewL(MHCICommandQueue& aCommandQueue, CLinkMgrProtocol& aLinkMgrProtocol);
	~CEirManServer();
	
	inline CEirManager& EirManager() const;
	
	void AddSession();
	void DropSession();
	void NotifyFeaturesReady();
	TEirFeatureState EirFeatureState();

private:
	CEirManServer(MHCICommandQueue& aCommandQueue, CLinkMgrProtocol& aLinkMgrProtocol);
	void ConstructL();

	// from CPolicyServer
	virtual CSession2* NewSessionL(const TVersion &aVersion, const RMessage2& aMessage) const;
	TCustomResult CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing);
	
private:
	// unowned
	MHCICommandQueue&	iCommandQueue;
	CLinkMgrProtocol& 	iLinkMgrProtocol;
	//owned
	CEirManager* 		iEirManager;
	TInt				iSessionCount;
	TBool				iIsFeaturesReady;
	TBool				iIsEirSupported;
	};
#include "eirmanserver.inl"

#endif // _EIRMANSERVER_H

