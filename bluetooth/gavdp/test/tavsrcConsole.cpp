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

#include "tavsrcConsole.h"
#include "tavsrc.h"

#include <e32test.h>

#if defined (__WINS__)
#define PDD_NAME _L("ECDRV")
#define LDD_NAME _L("ECOMM")
#else  // __GCC32__
#define PDD_NAME _L("EUART1")
#define LDD_NAME _L("ECOMM")
#endif

static RTest testOutcome(_L("Test results"));	

CActiveConsole::CActiveConsole(MActiveConsoleNotify& aNotify)
	: CActive(EPriorityStandard), iNotify(aNotify)
	{
	CActiveScheduler::Add(this);
	}


CActiveConsole::~CActiveConsole()
	{
	Cancel();
	delete iConsole;
	}

CActiveConsole* CActiveConsole::NewL(MActiveConsoleNotify& aNotify,const TDesC& aTitle,const TSize& aSize)
	{
	CActiveConsole* console = new (ELeave) CActiveConsole(aNotify);
	CleanupStack::PushL(console);
	console->ConstructL(aTitle,aSize);
	CleanupStack::Pop();
	return console;
	}

void CActiveConsole::ConstructL(const TDesC& aTitle,const TSize& aSize)
	{
	iConsole = Console::NewL(aTitle,aSize);
	}

void CActiveConsole::DoCancel()
	{
	iConsole->ReadCancel();
	}

void CActiveConsole::RequestKey()
	{
	DrawCursor();
	iConsole->Read(iStatus);
	SetActive();
	}

void CActiveConsole::DrawCursor()
	{
	iConsole->Printf(_L(">>"));
	}

void CActiveConsole::RunL()
	{
	// key has been pressed
	TChar ch = iConsole->KeyCode();
	iNotify.KeyPressed(ch);
	}

TInt CActiveConsole::RunError(TInt aError)
	{
	iConsole->Printf(_L("Console error %d\nTrying again...\n"), aError);
	RequestKey();
	return KErrNone;
	}

TInt CActiveConsole::GetIntFromUser()
	{
	TBool requeRead = EFalse;
	if(IsActive())
		{
		Cancel();
		requeRead = ETrue;
		}

	TKeyCode key;
	
	TBuf<10> intBuf;
	
	while((key = iConsole->Getch())!=EKeyEnter)
		{
		if(key == EKeyBackspace&&intBuf.Length()!=0)
			{
			intBuf.SetLength(intBuf.Length()-1);
			iConsole->Printf(_L("%c"), key);
			}
		else if ( intBuf.Length() < intBuf.MaxLength())
			{
			intBuf.Append(key);
			iConsole->Printf(_L("%c"), key);
			}
		}
	
	iConsole->Printf(_L("\n"));	
	
	TLex lex(intBuf);
	TInt value = 0;
	
	TInt err = lex.Val(value);
	if(err)
		{
		iConsole->Printf(_L("Error %d reading value from console\n"), err);	
		}

	if(requeRead)
		{
		RequestKey();
		}
	return value;
	}

void LoadLDD_PDDL()
	{
	TInt err = KErrNone;
	testOutcome.Printf(_L("Loading PDD... "));
	err = User::LoadPhysicalDevice(PDD_NAME);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		User::Leave(err);
		}
	testOutcome.Printf(_L("Loading LDD... "));
	err = User::LoadLogicalDevice(LDD_NAME);
	if(err != KErrNone && err != KErrAlreadyExists)
		{
		User::Leave(err);
		}
	}

void StartL()
	{
	LoadLDD_PDDL();
		
	CAVTestApp* app = CAVTestApp::NewL();
	app->StartL();

	delete app;
	}
	

TInt E32Main()
	{
	__UHEAP_MARK;

	CTrapCleanup* cleanupStack=CTrapCleanup::New();	// Get CleanupStack

	CActiveScheduler* activescheduler=new CActiveScheduler;
	CActiveScheduler::Install(activescheduler);

	testOutcome.Printf(_L("Starting tests... "));
	
	TRAPD(err, StartL());

	delete activescheduler;
	delete cleanupStack;	

	testOutcome.Printf(_L("Test completed result %d"), err);
	if(err != KErrNone)
		{
		testOutcome.Getch();
		}
	testOutcome.Close();


	__UHEAP_MARKEND;

	return err ;
	}


