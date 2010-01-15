// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef ACTIVECALLBACKCONSOLE_H
#define ACTIVECALLBACKCONSOLE_H

#include <e32base.h>
#include <e32cons.h>



class CActiveCallBackConsole : public CActive
	{
public:
	static CActiveCallBackConsole* NewL(TInt(*aFunction)(TAny *aPtr, TChar aKey), TAny* aPtr, const TDesC& aTitle,const TSize& aSize);
	void RequestKey();
	
	inline CConsoleBase& Console() const 
		{
		return *iConsole;
		};

	~CActiveCallBackConsole();

private:
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();
	void DrawCursor();
	CActiveCallBackConsole(TInt(*aFunction)(TAny *aPtr, TChar aKey), TAny* aPtr);
	void ConstructL(const TDesC& aTitle,const TSize& aSize);

private:
	CConsoleBase*	iConsole;
	TInt(*iFunction)(TAny *aPtr, TChar aKey);
	TAny* iPtr;
	};

#endif //ACTIVECALLBACKCONSOLE_H
