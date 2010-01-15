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

#ifndef TAVSRCUI_H
#define TAVSRCUI_H

#include <e32base.h>
#include <e32cons.h>

static const TUint KIconSize = 36;
static const TUint KIconIndent = 36;
static const TUint KPlayListX = 2;

class CStreamerUI : public CBase
	{
public:
	static CStreamerUI* NewL(TBool aDisplayPlaylist, TBool aDisplayChunkyIcon);
	~CStreamerUI();
	void Play();
	void Pause();
	void Stop();
	void AddTitle(const TDesC& aTitle);
	void Next();
	void Prev();
	void First();
	
private:
	CStreamerUI();
	void ConstructL(TBool aDisplayPlaylist, TBool aDisplayChunkyIcon);
	void ClearPointer();
	void DrawPointer();

private:
	CConsoleBase* iIconConsole;
	CConsoleBase* iPlayListConsole;
	TInt iPlayListY;
	};

class CProgressBar : public CBase
	{
public:
	static CProgressBar* NewL(TInt aMaximum);
	void Increment(TInt aStep);
	~CProgressBar();
	
private:
	CProgressBar(TInt aMaximum);
	void ConstructL();
	void Redraw();
	
private:
	TInt	iMax;
	TInt	iValue;
	CConsoleBase*	iConsole;
	};

#endif // TAVSRCUI_H
