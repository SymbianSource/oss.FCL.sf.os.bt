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

#ifndef HCTLUSBORIGINALCONTROL_H
#define HCTLUSBORIGINALCONTROL_H

#include <e32base.h>
#include <d32usbdi.h>

class MHCTLChannelObserver;

/**
Control pipe in the spec. defined USB transport is one way.
*/
NONSHARABLE_CLASS(CHCTLUsbOriginalCommand)
	: public CActive
	{
public:
	static CHCTLUsbOriginalCommand* NewL(RUsbInterface& aInterface);
	~CHCTLUsbOriginalCommand();
	
	TInt Write(const TDesC8& aData);
	void SetChannelObserver(MHCTLChannelObserver& aObserver);
	
private:
	CHCTLUsbOriginalCommand(RUsbInterface& aInterface);
	
private: // from CActive
	virtual void RunL();
	virtual void DoCancel();
	
private:
	MHCTLChannelObserver* iChannelObserver;
	RUsbInterface& iInterface;
	RUsbInterface::TUsbTransferRequestDetails iEp0Details;
	TBuf8<255> iRecvData; // TODO test if we can remove this.
    };

#endif // HCTLUSBORIGINALCONTROL_H

