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

#include "activecallbackconsole.h"

CActiveCallBackConsole::CActiveCallBackConsole(TInt(*aFunction)(TAny *aPtr, TChar aKey), TAny* aPtr)
	: CActive(EPriorityStandard), iFunction(aFunction), iPtr(aPtr)
	{
	CActiveScheduler::Add(this);
	}


CActiveCallBackConsole::~CActiveCallBackConsole()
	{
	Cancel();
	delete iConsole;
	}

CActiveCallBackConsole* CActiveCallBackConsole::NewL(TInt(*aFunction)(TAny *aPtr, TChar aKey), TAny* aPtr, const TDesC& aTitle,const TSize& aSize)
	{
	CActiveCallBackConsole* console = new (ELeave) CActiveCallBackConsole(aFunction, aPtr);
	CleanupStack::PushL(console);
	console->ConstructL(aTitle,aSize);
	CleanupStack::Pop();
	return console;
	}

void CActiveCallBackConsole::ConstructL(const TDesC& aTitle,const TSize& aSize)
	{
	iConsole = Console::NewL(aTitle,aSize);
	}

void CActiveCallBackConsole::DoCancel()
	{
	iConsole->ReadCancel();
	}

void CActiveCallBackConsole::RequestKey()
	{
	DrawCursor();
	iConsole->Read(iStatus);
	SetActive();
	}

void CActiveCallBackConsole::DrawCursor()
	{
	iConsole->Printf(_L(">>"));
	}

void CActiveCallBackConsole::RunL()
	{
	// key has been pressed
	TChar ch = iConsole->KeyCode();
	(*iFunction)(iPtr, ch);
	}

TInt CActiveCallBackConsole::RunError(TInt aError)
	{
	iConsole->Printf(_L("Console error %d\nTrying again...\n"), aError);
	RequestKey();
	return KErrNone;
	}
