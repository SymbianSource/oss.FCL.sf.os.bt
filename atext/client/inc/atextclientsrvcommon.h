/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Client-Server message passing declarations
*
*/


#ifndef C_ATEXTCLIENTSRVCOMMON_H
#define C_ATEXTCLIENTSRVCOMMON_H

/**  Opcodes used in message passing between client and server */
enum TATExtIpc
    {
    EATExtSetCommonInterface,
    EATExtSynchronousClose,
    EATExtGetMode,
    EATExtReceiveModeStatusChange,
    EATExtCancelReceiveModeStatusChange,
    EATExtGetNvramStatus,
    EATExtReceiveNvramStatusChange,
    EATExtCancelReceiveNvramStatusChange,
    EInvalidIpc
    };

/**  Server name */
_LIT( KATExtSrvName, "atextcommon" );

/**  A version must be specifyed when creating a session with the server */
const TUint KCommonServerMajorVersionNumber = 2;
const TUint KCommonServerMinorVersionNumber = 0;
const TUint KCommonServerBuildVersionNumber = 0;

#endif  // C_ATEXTCLIENTSRVCOMMON_H
