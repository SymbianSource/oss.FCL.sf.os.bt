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

#include <hal.h>
#include <hal_data.h>

#include "tavsrcTimer.h"
#include "tavsrc.h"

static const TUint KMillion = 1000000;

CAdaptiveHighResPeriodic* CAdaptiveHighResPeriodic::NewL(MAdaptiveHighResPeriodicClient& aClient)
	{
	CAdaptiveHighResPeriodic* p = new (ELeave) CAdaptiveHighResPeriodic(aClient);
	CleanupStack::PushL(p);
	p->ConstructL();
	CleanupStack::Pop(p);
	return p;
	}
	
CAdaptiveHighResPeriodic::~CAdaptiveHighResPeriodic()
	{
	Cancel();
	}

CAdaptiveHighResPeriodic::CAdaptiveHighResPeriodic(MAdaptiveHighResPeriodicClient& aClient)
: CTimer(EPriorityStandard+1), iClient(aClient)
	{
	CActiveScheduler::Add(this);
	}
	
void CAdaptiveHighResPeriodic::ConstructL()
	{
	CTimer::ConstructL();
	
	HAL::Get(HALData::EFastCounterFrequency, iFastCounterFreq);
	HAL::Get(HALData::EFastCounterCountsUp, iFastCounterIncreases);

	RDebug::Printf("Timer HAL: FC Freq %d", iFastCounterFreq);
	
	if (iFastCounterIncreases)
		{
		RDebug::Printf("Timer HAL: FC increases");
		}
	else
		{
		RDebug::Printf("Timer HAL: FC decreases");
		}

	iFastCounterFreqUs = (TReal)iFastCounterFreq / KMillion;
	}
	
	
void CAdaptiveHighResPeriodic::Start(TTimeIntervalMicroSeconds32 aInterval)
	{
	RDebug::Printf("*** Start Timer");
	iInterval = aInterval;

	// calculate the number of fast counter ticks for the interval
	TReal intervalInCounts = iInterval.Int()*iFastCounterFreqUs;		
	TInt countChangeExpected = (TInt) intervalInCounts;

	// store any extra fractions of fast counter ticks
	iExtraCounts = intervalInCounts - countChangeExpected;

	iIntendedCountOnCallback = iFastCounterIncreases
								? User::FastCounter() + countChangeExpected
								: User::FastCounter() - countChangeExpected;

	StartTimer(aInterval);
	}
	
void CAdaptiveHighResPeriodic::RunL()
	{
	User::LeaveIfError(iStatus.Int());
	iClient.TimerEvent(*this);

	if (!IsActive())
		{
		TUint endCount = User::FastCounter();

		// are we fast or slow? positive = late, negative = early
		TInt varianceCount = iFastCounterIncreases
				? (TInt)((endCount - iIntendedCountOnCallback))
				: (TInt)((iIntendedCountOnCallback - endCount));

		// convert count to microsecs
		TInt varianceUs = varianceCount/iFastCounterFreqUs;

		TInt nextInterval = iInterval.Int() - varianceUs; // in musecs

		if (nextInterval < 0)
			{
			nextInterval = 0;
			}

		// calculate the number of fast counter ticks for the interval
		TReal intervalInCounts = nextInterval*iFastCounterFreqUs;		
		TInt countChangeExpected = (TInt) intervalInCounts;

		// update extra fractions of fast counter ticks
		iExtraCounts += intervalInCounts - countChangeExpected;

		if (iExtraCounts >= 1)
			{
			// we have more than a whole tick, do the adjustment
			countChangeExpected++;
			iExtraCounts--;
			nextInterval = countChangeExpected/iFastCounterFreqUs;
			}

		iIntendedCountOnCallback = iFastCounterIncreases
									? User::FastCounter() + countChangeExpected
									: User::FastCounter() - countChangeExpected;
		
		StartTimer(nextInterval);
		}
	}
	
TInt CAdaptiveHighResPeriodic::RunError(TInt aError)
	{
	iClient.TimerError(*this, aError);
	return KErrNone;
	}

void CAdaptiveHighResPeriodic::SetInterval(TTimeIntervalMicroSeconds32 aInterval)
	{
	iInterval = aInterval;
	}
	
void CAdaptiveHighResPeriodic::StartTimer(TTimeIntervalMicroSeconds32 aInterval)
	{
	HighRes(aInterval);
	}
