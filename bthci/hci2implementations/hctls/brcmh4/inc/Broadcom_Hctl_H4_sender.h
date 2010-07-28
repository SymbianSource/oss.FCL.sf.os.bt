// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef BROADCOMHCTLH4SENDER_H
#define BROADCOMHCTLH4SENDER_H

#include <e32base.h>

class RBusDevComm;
class MHCTLChannelObserver;

NONSHARABLE_CLASS(CHCTLBcmH4Sender) : public CActive
    {
public:
    static CHCTLBcmH4Sender* NewL(RBusDevComm& aPort);
    ~CHCTLBcmH4Sender();         

    TInt Write(const TDesC8& aData);
	void SetChannelObserver(MHCTLChannelObserver& aObserver);

private:
	// From CActive
	virtual void RunL();     
	virtual void DoCancel();

	CHCTLBcmH4Sender(RBusDevComm& aPort);

private:
	MHCTLChannelObserver* iChannelObserver;
	RBusDevComm& iPort;
    };

#endif // BROADCOMHCTLH4

