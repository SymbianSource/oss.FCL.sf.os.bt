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

#include "tavsrcOperations.h"

// The opcodes used internally to CTavsrcOperations
static const TInt KTavsrcOpCodeConnect = 0x00000001;
static const TInt KTavsrcOpCodeCancel = 0x00000002;
static const TInt KTavsrcOpCodeDiscoverSEPs = 0x00000004;
static const TInt KTavsrcOpCodeCreateBearers = 0x00000008;
static const TInt KTavsrcOpCodeCloseBearers = 0x00000010;
static const TInt KTavsrcOpCodeContentProtection = 0x00000020;
static const TInt KTavsrcOpCodeGetCapabilities = 0x00000040;
static const TInt KTavsrcOpCodeStartStreams = 0x00000080;
static const TInt KTavsrcOpCodeSuspendStreams = 0x00000100;
static const TInt KTavsrcOpCodeEchoStorm = 0x00000200;
static const TInt KTavsrcOpCodeAbort = 0x00000400;
static const TInt KTavsrcOpCodeStream = 0x00000800;
static const TInt KTavsrcOpCodeStreamFaster = 0x00001000;
static const TInt KTavsrcOpCodeStreamSlower = 0x00002000;
static const TInt KTavsrcOpCodeAutoStream = 0x00004000;
static const TInt KTavsrcOpCodeStopStream = 0x00008000;
static const TInt KTavsrcOpCodeRegisterSEP = 0x00010000;
static const TInt KTavsrcOpCodeRegisterMultipleSEPs = 0x00020000;
static const TInt KTavsrcOpCodeStartSrc = 0x00040000;
static const TInt KTavsrcOpCodeStopSrc = 0x00080000;
static const TInt KTavsrcOpCodeDisconnectSrc = 0x00100000;
static const TInt KTavsrcOpCodeConfigureSEP = 0x00200000;
static const TInt KTavsrcOpCodeReconfigureSEP = 0x00400000;
static const TInt KTavsrcOpCodePacketDropIoctl = 0x00800000;
static const TInt KTavsrcOpCodeVolumeUp = 0x01000000;
static const TInt KTavsrcOpCodeVolumeDown = 0x02000000;
static const TInt KTavsrcOpCodeBackwards = 0x04000000;
static const TInt KTavsrcOpCodeForwards = 0x08000000;
static const TInt KTavsrcOpCodePlay = 0x10000000;
static const TInt KTavsrcOpCodeStop = 0x20000000;
static const TInt KTavsrcOpCodeExit = 0x40000000;
static const TInt KTavsrcOpCodeToggleSafeMode = 0x80000000;

// all the AVRCP opcodes
static const TInt KTavsrcOpCodeAvrcp = KTavsrcOpCodeVolumeUp | KTavsrcOpCodeVolumeDown | KTavsrcOpCodeBackwards | KTavsrcOpCodeForwards | KTavsrcOpCodePlay | KTavsrcOpCodeStop;

// all always available opcodes
static const TInt KTavsrcOpCodeAlwaysAvailable = KTavsrcOpCodeAvrcp | KTavsrcOpCodeToggleSafeMode | KTavsrcOpCodeCancel | KTavsrcOpCodeExit | KTavsrcOpCodeStopSrc | KTavsrcOpCodeRegisterSEP | KTavsrcOpCodeRegisterMultipleSEPs;

// all possible opcodes
static const TInt KTavsrcOpCodeAll = 0xffffffff;

// The next opcodes for every command, 0 means to not change state
static const TInt KTavsrcOpConnectNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeDiscoverSEPs | KTavsrcOpCodeAutoStream;
static const TInt KTavsrcOpCancelNextOperations = 0;
static const TInt KTavsrcOpDiscoverSEPsNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeGetCapabilities;
static const TInt KTavsrcOpCreateBearersNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeStartStreams | KTavsrcOpCodeCloseBearers;
static const TInt KTavsrcOpCloseBearersNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeCreateBearers;
static const TInt KTavsrcOpContentProtectionNextOperations = 0;
static const TInt KTavsrcOpGetCapabilitiesNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeConfigureSEP;
static const TInt KTavsrcOpStartStreamsNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeStream | KTavsrcOpCodeCloseBearers | KTavsrcOpCodeSuspendStreams | KTavsrcOpCodeAbort;
static const TInt KTavsrcOpSuspendStreamsNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeStartStreams | KTavsrcOpCodeAbort | KTavsrcOpCodeReconfigureSEP;
static const TInt KTavsrcOpEchoStormNextOperations = 0;
static const TInt KTavsrcOpAbortNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeStartStreams | KTavsrcOpCodeCloseBearers;
static const TInt KTavsrcOpStreamNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeStopStream | KTavsrcOpCodeCloseBearers | KTavsrcOpCodeSuspendStreams | KTavsrcOpCodeAbort | KTavsrcOpCodeStreamFaster | KTavsrcOpCodeStreamSlower;
static const TInt KTavsrcOpStreamFasterNextOperations = 0;
static const TInt KTavsrcOpStreamSlowerNextOperations = 0;
static const TInt KTavsrcOpAutoStreamNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeStopStream | KTavsrcOpCodeCloseBearers | KTavsrcOpCodeSuspendStreams;
static const TInt KTavsrcOpStopStreamNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeCloseBearers | KTavsrcOpCodeStream | KTavsrcOpCodeSuspendStreams | KTavsrcOpCodeAbort;
static const TInt KTavsrcOpRegisterSEPNextOperations = 0;
static const TInt KTavsrcOpRegisterMultipleSEPsNextOperations = 0;
static const TInt KTavsrcOpStartSrcNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeConnect;
static const TInt KTavsrcOpStopSrcNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeStartSrc;
static const TInt KTavsrcOpDisconnectSrcNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeConnect;
static const TInt KTavsrcOpConfigureSEPNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeCreateBearers;
static const TInt KTavsrcOpReconfigureSEPNextOperations = KTavsrcOpCodeAlwaysAvailable | KTavsrcOpCodeDisconnectSrc | KTavsrcOpCodeStartStreams | KTavsrcOpCodeAbort | KTavsrcOpCodeReconfigureSEP;
static const TInt KTavsrcOpPacketDropIoctlNextOperations = 0;
static const TInt KTavsrcOpVolumeUpNextOperations = 0;
static const TInt KTavsrcOpVolumeDownNextOperations = 0;
static const TInt KTavsrcOpBackwardsNextOperations = 0;
static const TInt KTavsrcOpForwardsNextOperations = 0;
static const TInt KTavsrcOpPlayNextOperations = 0;
static const TInt KTavsrcOpStopNextOperations = 0;
static const TInt KTavsrcOpExitNextOperations = 0;
static const TInt KTavsrcOpToggleSafeModeNextOperations = 0;

// TTavsrcOperation
TTavsrcOperation::TTavsrcOperation(TInt aOpCode, TChar aOperation, TInt aNextOpCodes)
	: iOpCode(aOpCode), iOperation(aOperation), iNextOpCodes(aNextOpCodes)
	{	
	}

/*static*/ TBool TTavsrcOperation::OperationMatchesAndAllowed(const TTavsrcOperation& aOpA, const TTavsrcOperation& aOpB)
	{
	return ((aOpA.iOperation == aOpB.iOperation) && (aOpA.iNextOpCodes & aOpB.iOpCode));
	}

TInt TTavsrcOperation::GetNextOpCodes() const
	{
	return iNextOpCodes;
	}

// CTavsrcOperations
CTavsrcOperations* CTavsrcOperations::NewL()
	{
	CTavsrcOperations* operations = new (ELeave) CTavsrcOperations;
	CleanupStack::PushL(operations);
	operations->ConstructL();
	CleanupStack::Pop(operations);
	return operations;
	}

void CTavsrcOperations::ConstructL()
	{
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeConnect, KTavsrcOpConnect, KTavsrcOpConnectNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeCancel, KTavsrcOpCancel, KTavsrcOpCancelNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeDiscoverSEPs, KTavsrcOpDiscoverSEPs, KTavsrcOpDiscoverSEPsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeCreateBearers, KTavsrcOpCreateBearers, KTavsrcOpCreateBearersNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeCloseBearers, KTavsrcOpCloseBearers, KTavsrcOpCloseBearersNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeContentProtection, KTavsrcOpContentProtection, KTavsrcOpContentProtectionNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeGetCapabilities, KTavsrcOpGetCapabilities, KTavsrcOpGetCapabilitiesNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStartStreams, KTavsrcOpStartStreams, KTavsrcOpStartStreamsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeSuspendStreams, KTavsrcOpSuspendStreams, KTavsrcOpSuspendStreamsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeEchoStorm, KTavsrcOpEchoStorm, KTavsrcOpEchoStormNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeAbort, KTavsrcOpAbort, KTavsrcOpAbortNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStream, KTavsrcOpStream, KTavsrcOpStreamNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStreamFaster, KTavsrcOpStreamFaster, KTavsrcOpStreamFasterNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStreamSlower, KTavsrcOpStreamSlower, KTavsrcOpStreamSlowerNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeAutoStream, KTavsrcOpAutoStream, KTavsrcOpAutoStreamNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStopStream, KTavsrcOpStopStream, KTavsrcOpStopStreamNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeRegisterSEP, KTavsrcOpRegisterSEP, KTavsrcOpRegisterSEPNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeRegisterMultipleSEPs, KTavsrcOpRegisterMultipleSEPs, KTavsrcOpRegisterMultipleSEPsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStartSrc, KTavsrcOpStartSrc, KTavsrcOpStartSrcNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStopSrc, KTavsrcOpStopSrc, KTavsrcOpStopSrcNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeDisconnectSrc, KTavsrcOpDisconnectSrc, KTavsrcOpDisconnectSrcNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeConfigureSEP, KTavsrcOpConfigureSEP, KTavsrcOpConfigureSEPNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeReconfigureSEP, KTavsrcOpReconfigureSEP, KTavsrcOpReconfigureSEPNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodePacketDropIoctl, KTavsrcOpPacketDropIoctl, KTavsrcOpPacketDropIoctlNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeVolumeUp, KTavsrcOpVolumeUp, KTavsrcOpVolumeUpNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeVolumeDown, KTavsrcOpVolumeDown, KTavsrcOpVolumeDownNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeBackwards, KTavsrcOpBackwards, KTavsrcOpBackwardsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeForwards, KTavsrcOpForwards, KTavsrcOpForwardsNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodePlay, KTavsrcOpPlay, KTavsrcOpPlayNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeStop, KTavsrcOpStop, KTavsrcOpStopNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeExit, KTavsrcOpExit, KTavsrcOpExitNextOperations));
	iOperations.AppendL(TTavsrcOperation(KTavsrcOpCodeToggleSafeMode, KTavsrcOpToggleSafeMode, KTavsrcOpToggleSafeModeNextOperations));
	
	// tavsrc always starts the src initially
	iLastOperationIndex = GetAllowedOperationIndex(KTavsrcOpStartSrc, KTavsrcOpCodeAll);
	User::LeaveIfError(iLastOperationIndex);
	}

CTavsrcOperations::CTavsrcOperations()
	: iPendingOperationIndex(KErrNotFound), iSafeMode(ETrue)
	{
	}

CTavsrcOperations::~CTavsrcOperations()
	{
	iOperations.Close();
	}

// Returns the index into iOperations array if aOperation is allowed as the next operation
// according to aAllowedNextOpcodes or an error
TInt CTavsrcOperations::GetAllowedOperationIndex(TChar aOperation, TInt aAllowedNextOpcodes)
	{
	TIdentityRelation<TTavsrcOperation> matchFunc(TTavsrcOperation::OperationMatchesAndAllowed);
	return (iOperations.Find(TTavsrcOperation(0, aOperation, aAllowedNextOpcodes), matchFunc));
	}

TBool CTavsrcOperations::IsOperationAllowed(TChar aOperation)
	{
	TBool ret = EFalse;
	
	if (!SafeMode())
		{
		// if we are not in safe mode then any operation is allowed
		ret = ETrue;
		}
	else
		{ 
		ret = (GetAllowedOperationIndex(aOperation, iOperations[iLastOperationIndex].GetNextOpCodes()) >= 0);
		}
	return ret;
	}

TInt CTavsrcOperations::BeginOperation(TChar aOperation)
	{
	TInt rerr = KErrNone;

	if ((iPendingOperationIndex >= 0) && (aOperation != KTavsrcOpCancel))
		{
		// an operation is in progress, block until it has completed
		rerr = KErrNotReady;
		}
	else
		{
		TInt allowedNextOpCodes = iOperations[iLastOperationIndex].GetNextOpCodes();
		
		if (!SafeMode())
			{
			// not in safe mode so any operation should be allowed
			allowedNextOpCodes = KTavsrcOpCodeAll;
			}
		
		TInt index = GetAllowedOperationIndex(aOperation, allowedNextOpCodes);

		if (index >= 0)
			{
			// we have a valid operation
			iPendingOperationIndex = index;
			}
		else
			{
			// aOperation is not a supported command, return error
			rerr = index;
			}
		}
	return rerr;
	}

void CTavsrcOperations::EndOperation(TChar aOperation, TInt aError)
	{
	// if an error has occured the state hasn't moved on
	if (!aError)
		{
		// check to see if there is an outstanding operation
		if (iPendingOperationIndex < 0)
			{
			// we have no pending operation, this means that the remote has moved our state on,
			// to keep the allowed operations in step pretend that we moved the state on ourselves
			iPendingOperationIndex = GetAllowedOperationIndex(aOperation, KTavsrcOpCodeAll);
			}
	
		// update current state if there is any valid operations in the pending operation
		if (iOperations[iPendingOperationIndex].GetNextOpCodes() != 0)
			{
			iLastOperationIndex = iPendingOperationIndex;
			}
		}
	iPendingOperationIndex = KErrNotFound;
	}

TBool CTavsrcOperations::SafeMode()
	{
	return iSafeMode;
	}

void CTavsrcOperations::SetSafeMode(TBool aSafeMode)
	{
	iSafeMode = aSafeMode;
	}
