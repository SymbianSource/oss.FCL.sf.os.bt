/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  Redefinition of used CenRep and PS keys.
 *
 */

#ifndef LIMITEDPDPPLUGINCOPYKEYS_H
#define LIMITEDPDPPLUGINCOPYKEYS_H

/**
 * NOTE: THIS FILE CONTAINS COPIES OF CENREP AND PS KEYS DEFINED
 *       IN IPCONNMGMT PACKAGE. DO NOT CHANGE!!!
 * 
 * These needs to be copied to avoid dependency from OS layer (this module)
 * to middleware layer (ipconnmgmt). Bad design choices...
 */

/**
 * UID of CmManager repository
 */
const TUid KCopyOfCRUidCmManager =
    {
    0x10207376
    };

/**
 * Dial-up override setting.
 */
const TUint32 KCopyOfDialUpOverride = 0x00000002;

/**
 * Connection Monitor RProperty category.
 */
const TUid KCopyOfConnectionMonitorPS =
    {
    0x101F6CF0
    };

/**
 * Dial-Up override key.
 */
const TUint KCopyOfDialUpConnection = 0x00000001;

/**
 * Values for KDialUpConnection key.
 */
enum TCopyOfDialUpStates
    {
    EConnMonDialUpClosed,
    EConnMonDialUpInit,
    EConnMonReady
    };

#endif // LIMITEDPDPPLUGINCOPYKEYS_H
