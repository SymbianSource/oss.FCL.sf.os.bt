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

#include "tavsrcUI.h"

//
// class CStreamerUI
//
CStreamerUI* CStreamerUI::NewL(TBool aDisplayPlaylist, TBool aDisplayChunkyIcon)
	{
	CStreamerUI* ui = new (ELeave) CStreamerUI ();
	CleanupStack::PushL(ui);
	ui->ConstructL(aDisplayPlaylist, aDisplayChunkyIcon);
	CleanupStack::Pop(ui);
	return ui;
	}

CStreamerUI::CStreamerUI()
	{	
	}

void CStreamerUI::ConstructL(TBool aDisplayPlaylist, TBool aDisplayChunkyIcon)
	{
	if (aDisplayChunkyIcon)
		{
		iIconConsole = Console::NewL(_L("Icon"), TSize(-1,-1));
		}

	if (aDisplayPlaylist)
		{
		iPlayListConsole = Console::NewL(_L("Play List"), TSize(-1,-1));
		}
	}
	
CStreamerUI::~CStreamerUI()
	{
	delete iIconConsole;
	iIconConsole = NULL;
	delete iPlayListConsole;
	iPlayListConsole = NULL;
	}
	
void CStreamerUI::Play()
	{
	if (iIconConsole)
		{
		TBuf<KIconSize> line;
	
		iIconConsole->ClearScreen();
		
		for (TInt len=1; len<=KIconSize/2; len++)
			{
			line.AppendFill('*',len*2);
			iIconConsole->SetPos(KIconIndent,len);
			iIconConsole->Printf(line);
			iIconConsole->SetPos(KIconIndent,KIconSize-len);
			iIconConsole->Printf(line);
			line.Zero();
			}
		}
	DrawPointer();	
	}

void CStreamerUI::Pause()
	{
	if (iIconConsole)
		{
		TBuf<KIconSize> line;
	
		iIconConsole->ClearScreen();
		line.AppendFill('*',KIconSize/3);
		line.AppendFill(' ',KIconSize/3);
		line.AppendFill('*',KIconSize/3);
		
		for (TInt len=1; len<=KIconSize; len++)
			{
			iIconConsole->SetPos(KIconIndent,len);
			iIconConsole->Printf(line);
			}
		}
	}
	
void CStreamerUI::Stop()
	{
	if (iIconConsole)
		{
		TBuf<KIconSize> line;

		iIconConsole->ClearScreen();
		line.AppendFill('*',KIconSize);
		
		for (TInt len=1; len<=KIconSize; len++)
			{
			iIconConsole->SetPos(KIconIndent,len);
			iIconConsole->Printf(line);
			}
		}
	}

void CStreamerUI::AddTitle(const TDesC& aTitle)
	{
	if (iPlayListConsole)
		{
		iPlayListConsole->SetPos(KPlayListX+2);	
		iPlayListConsole->Printf(_L("%S\n"), &aTitle);
		}
	}
	
void CStreamerUI::Next()
	{
	ClearPointer();
	++iPlayListY;
	DrawPointer();
	}
	
void CStreamerUI::Prev()
	{
	ClearPointer();
	--iPlayListY;
	DrawPointer();	
	}

void CStreamerUI::First()
	{
	ClearPointer();
	iPlayListY = 0;
	DrawPointer();
	}
	
void CStreamerUI::ClearPointer()
	{
	if (iPlayListConsole)
		{
		iPlayListConsole->SetPos(KPlayListX, iPlayListY);
		iPlayListConsole->Printf(_L(" "));
		}
	}
	
void CStreamerUI::DrawPointer()
	{
	if (iPlayListConsole)
		{
		iPlayListConsole->SetPos(KPlayListX, iPlayListY);
		iPlayListConsole->Printf(_L(">"));
		}
	}
	
static const TInt KProgressBarSize = 70;

//
// class CProgressBar
//
CProgressBar::CProgressBar(TInt aMaximum) : iMax(aMaximum)
	{
	}

CProgressBar::~CProgressBar()
	{
	delete iConsole;
	}

CProgressBar* CProgressBar::NewL(TInt aMaximum)
	{
	CProgressBar* p = new (ELeave) CProgressBar(aMaximum);
	CleanupStack::PushL(p);
	p->ConstructL();
	CleanupStack::Pop(p);
	return p;
	}
	
void CProgressBar::Increment(TInt aStep)
	{
	iValue+=aStep;
	if (iValue < 0)
		{
		iValue = 0;
		}
#ifdef __WINS__
	Redraw();
#endif
	}
	
void CProgressBar::ConstructL()
	{
#ifdef __WINS__
	iConsole = Console::NewL(_L("Progress"), TSize(KProgressBarSize+5,1));
	Redraw();
#endif
	}
	
void CProgressBar::Redraw()
	{
	iConsole->SetPos(0);
	
	TBuf<KProgressBarSize> bar;
	TInt numDone = ((iValue*KProgressBarSize) /iMax);
	TInt numToDo = 0;

	if (numDone<bar.MaxLength())
		{
		bar.AppendFill('|',numDone);
		numToDo = KProgressBarSize - numDone;
		}
	bar.AppendFill('-',numToDo);
	
	iConsole->Printf(bar);
	}
