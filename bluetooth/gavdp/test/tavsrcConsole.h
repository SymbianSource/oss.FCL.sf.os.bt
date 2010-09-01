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

#ifndef TAVSRCCONSOLE_H
#define TAVSRCCONSOLE_H

#include <e32base.h>
#include <e32cons.h>

class MActiveConsoleNotify
	{
public:
	virtual void KeyPressed(TChar aKey) =0;
	};

class CActiveConsole : public CActive
	{
public:
	static CActiveConsole* NewL(MActiveConsoleNotify& aNotify,const TDesC& aTitle,const TSize& aSize);
	void RequestKey();
	TInt GetIntFromUser();
	
	inline CConsoleBase& Console() const 
		{
		return *iConsole;
		};

	~CActiveConsole();

private:
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();
	void DrawCursor();
	CActiveConsole(MActiveConsoleNotify& aNotify);
	void ConstructL(const TDesC& aTitle,const TSize& aSize);

private:
	CConsoleBase*	iConsole;
	MActiveConsoleNotify&	iNotify;	
	};

#endif // TAVSRCCONSOLE_H
