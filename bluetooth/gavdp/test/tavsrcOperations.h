
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

#ifndef TAVSRCOPERATIONS_H
#define TAVSRCOPERATIONS_H

#include <e32def.h>
#include <e32cmn.h>
#include <e32base.h>
#include <e32keys.h>

// all key operations available for user input
static const TInt KTavsrcOpConnect = ']';
static const TInt KTavsrcOpCancel = '[';
static const TInt KTavsrcOpDiscoverSEPs = 'd';
static const TInt KTavsrcOpCreateBearers = '1';
static const TInt KTavsrcOpCloseBearers = '!';
static const TInt KTavsrcOpContentProtection = 'p';
static const TInt KTavsrcOpGetCapabilities = 'g';
static const TInt KTavsrcOpStartStreams = 's';
static const TInt KTavsrcOpSuspendStreams = 'e';
static const TInt KTavsrcOpEchoStorm = 'E';
static const TInt KTavsrcOpAbort = 'a';
static const TInt KTavsrcOpStream = '@';
static const TInt KTavsrcOpStreamFaster = '+';
static const TInt KTavsrcOpStreamSlower = '-';
static const TInt KTavsrcOpAutoStream = 'A';
static const TInt KTavsrcOpStopStream = '\'';
static const TInt KTavsrcOpRegisterSEP = 't';
static const TInt KTavsrcOpRegisterMultipleSEPs = 'T';
static const TInt KTavsrcOpStartSrc = 'o';
static const TInt KTavsrcOpStopSrc = 'c';
static const TInt KTavsrcOpDisconnectSrc = 'f';
static const TInt KTavsrcOpConfigureSEP = 'x';
static const TInt KTavsrcOpReconfigureSEP = 'x';
static const TInt KTavsrcOpPacketDropIoctl = 'i';
static const TInt KTavsrcOpVolumeUp = EKeyUpArrow;
static const TInt KTavsrcOpVolumeDown = EKeyDownArrow;
static const TInt KTavsrcOpBackwards = EKeyLeftArrow;
static const TInt KTavsrcOpForwards = EKeyRightArrow;
static const TInt KTavsrcOpPlay = '}';
static const TInt KTavsrcOpStop = '{';
static const TInt KTavsrcOpExit = EKeyEscape;
static const TInt KTavsrcOpToggleSafeMode = 'W';

// describes one operation including key to press and next possible operations
class TTavsrcOperation
	{
public:
	TTavsrcOperation(TInt aOpCode, TChar aOperation, TInt aNextOpCodes);

	static TBool OperationMatchesAndAllowed(const TTavsrcOperation& aOpA, const TTavsrcOperation& aOpB);
	TInt GetNextOpCodes() const;

private:
	// iOpCode is the internal representation of the operation and can be used in a bitmask to define
	// all possible next operations 
	TInt iOpCode;
	
	// iOperation is the external representation of the operation, i.e. the key pressed by the user defined
	// above
	TChar iOperation;
	
	// iNextOpCodes is a bitmask of all the next possible operations after this operation
	TInt iNextOpCodes;
	};

// describes all operations and is used to only allow acceptable, as defined by GAVDP, operations
// to be selected by the user. All checking of whether an operation is allowed can be disabled
// by turning off safe mode.
class CTavsrcOperations : public CBase
	{
public:
	static CTavsrcOperations* NewL();
	~CTavsrcOperations();

	TBool IsOperationAllowed(TChar aOperation);
	TInt BeginOperation(TChar aOperation);
	void EndOperation(TChar aOperation, TInt aError);

	TBool SafeMode();
	void SetSafeMode(TBool aSafeMode);
	
private:
	CTavsrcOperations();
	void ConstructL();

	TInt GetAllowedOperationIndex(TChar aOperation, TInt aAllowedNextOpcodes);
	TBool IsOperationAllowed(TInt aIndex);
	
private:
	TInt iLastOperationIndex;
	TInt iPendingOperationIndex;

	TBool iSafeMode;
	
	RArray<TTavsrcOperation> iOperations;
	};

#endif // TAVSRCOPERATIONS_H
