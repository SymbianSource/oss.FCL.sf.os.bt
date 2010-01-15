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

#ifndef TAVSRCTIMER_H
#define TAVSRCTIMER_H

#include <e32base.h>

class CAdaptiveHighResPeriodic;

class MAdaptiveHighResPeriodicClient
	{
public:
	virtual void TimerEvent(CAdaptiveHighResPeriodic& aTimer)=0;
	virtual void TimerError(CAdaptiveHighResPeriodic& aTimer, TInt aError)=0;
	};

//adapts the callback to counter drift	
class CAdaptiveHighResPeriodic : public CTimer
	{
public:
	static CAdaptiveHighResPeriodic* NewL(MAdaptiveHighResPeriodicClient& aClient);
	~CAdaptiveHighResPeriodic();
	void Start(TTimeIntervalMicroSeconds32 aPeriod);

	void SetInterval(TTimeIntervalMicroSeconds32 aInterval);
	
private:
	CAdaptiveHighResPeriodic(MAdaptiveHighResPeriodicClient& aClient);
	void ConstructL();
	void RunL();
	TInt RunError(TInt aError);
	void StartTimer(TTimeIntervalMicroSeconds32 aInterval);
	
private:
	MAdaptiveHighResPeriodicClient& iClient;

	// HAL stuff
	TBool	iFastCounterIncreases;
	TInt	iFastCounterFreq;
	TReal	iFastCounterFreqUs;
	TReal	iExtraCounts;
	
	// what the client asked for in musecs
	TTimeIntervalMicroSeconds32	iInterval;

	TUint	iIntendedCountOnCallback ;	// the hoped for fast counter reading on callback
	};

#endif // TAVSRCTIMER_H
