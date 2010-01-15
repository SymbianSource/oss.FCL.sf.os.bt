// Copyright (c) 1999-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// BT protocol wide types (Published Partner)
// 
//

#ifndef _BTTYPESPARTNER_H
#define _BTTYPESPARTNER_H

#include <e32def.h>
#include <e32cmn.h>

/**
The size of a simple pairing hash in bytes.
@publishedPartner
@released
*/
const TInt KBluetoothSimplePairingHashSize = 16;
/**
A representation of the simple pairing hash.
@publishedPartner
@released
*/
typedef TBuf8<KBluetoothSimplePairingHashSize> TBluetoothSimplePairingHash;

/**
The size of a simple pairing hash in randomizer.
@publishedPartner
@released
*/
const TInt KBluetoothSimplePairingRandomizerSize = 16;
/**
A representation of the simple pairing randomizer.
@publishedPartner
@released
*/
typedef TBuf8<KBluetoothSimplePairingRandomizerSize> TBluetoothSimplePairingRandomizer;

#endif	//_BTTYPESPARTNER_H
