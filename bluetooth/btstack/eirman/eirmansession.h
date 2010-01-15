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

#ifndef _EIRMANSESSION_H
#define _EIRMANSESSION_H

#include <e32base.h>
#include <e32std.h>
#include <bluetooth/eirmanshared.h>
#include "eirmanager.h"

class CEirManServer;

NONSHARABLE_CLASS(CEirManSession) : public CSession2, public MEirManagerNotifiee
	{
public:
	static CEirManSession* NewL(CEirManServer& aServer);
	~CEirManSession();
	void NotifyEirFeatureState(TInt aResult);
private:
	CEirManSession(CEirManServer& aServer);
	void ConstructL();

	// ServiceL handlers
	void RegisterTag(const RMessage2& aMessage);
	TInt RegisterSpaceAvailableListener(const RMessage2& aMessage, TBool& aComplete);
	TInt CancelSpaceAvailableListener();
	TInt SetData(const RMessage2& aMessage);
	TInt NewData(const RMessage2& aMessage);
	
	void CompleteSpaceAvailableRequest(TUint aBytesAvailable);
	void DeregisterTag();

private: // CSession2 virtuals
	virtual void ServiceL(const RMessage2& aMessage);

private: // from MEirManagerNotifiee
	virtual void MemnEirBlockAvailable(TEirTag aTag, TUint aSpaceForTag);

private:
	CEirManServer& 		iEirManServer; // unowned
	TEirTag 			iEirTag;
	TEirTag				iPendingEirTag;
	TUint 				iLastSpaceOffered;
	TBool 				iSpaceAvailableListenerOutstanding;
	RMessage2			iRegisterMessage;
	RMessage2			iDataAvailableListenerMessage;
	};

#endif // _EIRMANSESSION_H

