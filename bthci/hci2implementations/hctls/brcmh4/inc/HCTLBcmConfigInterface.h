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
 @internalTechnology
*/

#ifndef HCTLBCMCONFIGINTERFACE_H
#define HCTLBCMCONFIGINTERFACE_H

#include <e32def.h>

/**
The UID of the HCTL configuration interface.
*/
const TInt KHctlBcmConfigInterfaceUid = 0xE0000030;

/**
Mixin for the HCTL configuration interface.
*/ 

class MHctlBcmConfigInterface
	{
public:
	/**
	Update the BAUD rate.
	
	@param aBaudRate New value for BAUD rate
	@return Standard error value.
	*/
	virtual TInt MhciUpdateBaudRate(TUint32 aBaudRate) = 0;
	
	virtual void MhciSetInitPluginState(TInitState InitState) = 0;
	};

#endif // HCTLBCMCONFIGINTERFACE_H
