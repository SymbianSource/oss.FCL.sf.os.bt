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

#ifndef TAVSRCUTILS_H
#define TAVSRCUTILS_H

#include <e32def.h>
#include <btsdp.h>
#include <f32file.h>

class TTavsrcUtils
	{
public:
	// SDP helper functions
	static void RegisterSinkSDPRecordL(RSdpDatabase& aDB, TSdpServRecordHandle& aRecHandle,
									   TBool aHeadphone, TBool aSpeaker,TBool aRecorder,TBool aAmp);
	static void RegisterSourceSDPRecordL(RSdpDatabase& aDB, TSdpServRecordHandle& aRecHandle,
										 TBool aPlayer, TBool aMic, TBool aTuner, TBool aMixer);

	// User input helper functions
	static TInt GetIntFromUser(CConsoleBase& aConsole);
	static TBool GetYNFromUser(CConsoleBase& aConsole, const TDesC& aDes);

	static void GetDeviceAddressL(TBTDevAddr& aAddr);

	static TInt GetCodecSettingsFromSBCFile(RFile& aFile, TInt aPos, TInt& aChannelMode,
							TInt& aNumChannels, TInt& aNumSubbands, TInt& aBlkLen,
							TInt& aBitPool, TInt& aFreq, TInt& aAllocMethod);

	static TInt GetCodecSettingsFromSBCFile(const TDesC& aFileName, TInt& aChannelMode,
							TInt& aNumChannels, TInt& aNumSubbands, TInt& aBlkLen,
							TInt& aBitPool, TInt& aFreq, TInt& aAllocMethod);

	static TInt CEIL(TReal aX);
	};

#endif // TAVSRCUTILS_H

