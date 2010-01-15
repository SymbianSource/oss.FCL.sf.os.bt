// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef EIRPUBLISHERLOCALNAME_H
#define EIRPUBLISHERLOCALNAME_H

#include <e32base.h>
#include <bluetooth/eirpublisherbase.h>
#include <bluetooth/hci/hciconsts.h>

//**********************************
// CEirPublisherLocalName
//**********************************
/**
Provides functionality to publish 16 bit UUIDs to EIR.
**/
NONSHARABLE_CLASS(CEirPublisherLocalName) : public CEirPublisherBase
	{
public:
	static CEirPublisherLocalName* NewL();
	~CEirPublisherLocalName();
	void UpdateName(const TDesC8& aName);

private:
	CEirPublisherLocalName();
	void ConstructL();

	// From MEirPublisherNotifier
	virtual void MepnSpaceAvailable(TUint aBytesAvailable);
	virtual void MepnSetDataError(TInt aResult);

private:
	TBuf8<KHCILocalDeviceNameMaxLength> iLocalName;
	HBufC8* iPublishBuf;
	};
	
#endif	// EIRPUBLISHERLOCALNAME_H
