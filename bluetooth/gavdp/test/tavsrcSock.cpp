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

#include "tavsrcSock.h"
#include "tavsrc.h"

#ifdef __WINS__
static const TSize KReaderConsole(55,10);
static const TSize KWriterConsole(55,12);
#else
static const TSize KReaderConsole(KConsFullScreen,KConsFullScreen);
static const TSize KWriterConsole(KConsFullScreen,KConsFullScreen);
#endif

//
// class CActivePacketDropIoctl
//
CActivePacketDropIoctl* CActivePacketDropIoctl::NewL(CActiveConsole* aLogConsole, RSocketArray aPendingSockets)
	{
	CActivePacketDropIoctl* self = new (ELeave) CActivePacketDropIoctl(aLogConsole, aPendingSockets);
	return self;
	}

CActivePacketDropIoctl::CActivePacketDropIoctl(CActiveConsole* aLogConsole, RSocketArray aPendingSockets) : CActive(0)
	{
	iPendingSockets = aPendingSockets;
	iLogConsole = aLogConsole;
	CActiveScheduler::Add(this);
	}

CActivePacketDropIoctl::~CActivePacketDropIoctl()
	{
	}

void CActivePacketDropIoctl::DoCancel()
	{
	Cancel();
	}

void CActivePacketDropIoctl::Start()
	{
	iPendingSockets[0].Ioctl(ENotifyAvdtpMediaPacketDropped, iStatus, &iPacketsLostPkgBuf, KSolBtAVDTPMedia);	
	SetActive();
	}

void CActivePacketDropIoctl::RunL()
	{
	__LOG(_L("\n**Packets Dropped Notification! %d packets lost.\n"), iPacketsLostPkgBuf());		
	}

//
// class CActiveSockReader
//
CActiveSockReader* CActiveSockReader::NewL(RSocket aSock, TAvdtpTransportSessionType aType)
	{
	CActiveSockReader* self = new (ELeave) CActiveSockReader(aSock);
	CleanupStack::PushL(self);
	self->ConstructL(aType);
	CleanupStack::Pop(self);
	return self;
	}

CActiveSockReader::CActiveSockReader(RSocket aSock)
: CActive(0), iSock(aSock), iBufferDes(NULL, NULL)
	{
	CActiveScheduler::Add(this);
	}
	
void CActiveSockReader::Start()
	{
	iSock.Read(iBufferDes,iStatus);
	SetActive();
	}
	
void CActiveSockReader::ConstructL(TAvdtpTransportSessionType aType)
	{
	iType = aType;
	switch (aType)
		{
	case EMedia:
		iConsole = Console::NewL(_L("Incoming Sink Data"),KReaderConsole);
		break;
	case EReporting:
		iConsole = Console::NewL(_L("Incoming Reporting Channel Data"),KReaderConsole);
		break;
	case ERecovery:
		iConsole = Console::NewL(_L("Incoming Recovery Channel Data"),KReaderConsole);
		break;
		}

	iBuffer = HBufC8::NewL(40);
	iBuffer->Des().SetMax();
	iBufferDes.Set(iBuffer->Des());
	}

CActiveSockReader::~CActiveSockReader()
	{
	Cancel();
	delete iConsole;
	iSock.Close();
	}
	
void CActiveSockReader::DoCancel()
	{
	iSock.CancelAll();
	}
	
void CActiveSockReader::RunL()
	{
	if (iStatus.Int()==KErrNone)
		{
		iConsole->Printf(_L("length: [%04d]\n"),iBufferDes.Length());
		for (TInt i=0;i<=iBufferDes.Length()-1;i++)
			{
			iConsole->Printf(_L("%c"),iBufferDes[i]);
			}
		iConsole->Printf(_L("\n\n"));
		iBuffer->Des().SetMax();
		iBufferDes.Set(iBuffer->Des());
		iSock.Read(iBufferDes,iStatus);
		SetActive();
		}
	else
		{
		iConsole->Printf(_L("SOCKET ERROR: %d"),iStatus.Int());
		}
	
	}

//
// class CActiveSockWriter
//
CActiveSockWriter* CActiveSockWriter::NewL(RSocket aSock,
													TAvdtpTransportSessionType aType)
	{
	CActiveSockWriter* writer = new (ELeave) CActiveSockWriter(aSock);
	CleanupStack::PushL(writer);
	writer->ConstructL(aType);
	CleanupStack::Pop();
	return writer;
	}

CActiveSockWriter::~CActiveSockWriter()
	{
	delete iSendBuffer;
	delete iConsole;
	// test for normal shutdown on media socket
	TRequestStatus stat;
	iSock.Shutdown(RSocket::ENormal, stat);
	User::WaitForRequest(stat);
	iSock.Close();
	}
	
void CActiveSockWriter::Send()
	{
	// periodic callback to send from app
	if (IsActive())
		{
		iConsole->Printf(_L("Info: Transport session %d tried to send, but active\n"), iType);
		}
	else
		{
		iSock.Write(*iSendBuffer, iStatus);
		SetActive();
		}
	}
	
CActiveSockWriter::CActiveSockWriter(RSocket aSock)
: CActive(-1), iSock(aSock)
	{
	CActiveScheduler::Add(this);
	}

void CActiveSockWriter::ConstructL(TAvdtpTransportSessionType aType)
	{
	//	Writer console to be used in the future if necessary for debugging
	//	iConsole = Console::NewL(_L("Writer"), KWriterConsole);
	
	iSendBuffer = HBufC8::NewL(512);
	TPtr8 des = iSendBuffer->Des();
	iType = aType;
	switch (aType)
		{
	case EMedia:
		des.Copy(_L("Media channel connected!"));
		break;
	case EReporting:
		des.Copy(_L("Reporting channel connected!"));
		break;
	case ERecovery:
		des.Copy(_L("Recovery channel connected!"));
		break;
	default:
		des.Copy(_L("Unknown channel "));
		des.AppendNum(aType);
		des.Append(_L(" connected!"));
		break;
		}
	}
	
void CActiveSockWriter::DoCancel()
	{
	iSock.CancelWrite();
	}
	
void CActiveSockWriter::RunL()
	{
	//	iConsole->Printf(_L("Info: Transport session %d SENT, result %d"), iType, iStatus.Int());
	}
