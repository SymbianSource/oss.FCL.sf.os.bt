// Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
*/

#include <bluetoothav.h>
#include "gavdpinterface.h"

EXPORT_C TAvdtpMediaTransportCapabilities::TAvdtpMediaTransportCapabilities()
: TAvdtpServiceCapability(EServiceCategoryMediaTransport, 0)
	{
	}
	
EXPORT_C /*virtual*/ TInt TAvdtpMediaTransportCapabilities::AsProtocol(RBuf8& aBuffer) const
	{
	TInt ret = AddHeader(aBuffer);
	// no more to do for this capability!
	return ret;
	}
