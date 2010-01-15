// Copyright (c) 1997-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "debug.h"

#include <e32math.h>
#include <e32svr.h> 
#include "comreadwrite.h"
#include "tranp.h"
#include "tranpprot.h"
#include "SCEP.H"
#include "COMMANDP.H"
#include "ABSTRACT.H"
#include "SCONREQC.H"
#include "SCONCONC.H"
#include "SRPSCOMM.H"
#include "GLOBAL.H"
#include "BFTP.H"


SCEP::SCEP()
	{
	}

/*
* Method description:	Leavable constructor for the SCEP layer
*
*/

SCEP* SCEP::NewL(CTranpProtocol* aTranpProtocol, BFTP* a_oBFTP, const MTranpNotification* aNotifier)
	{
	DEBUG_OPEN();
	SCEPPRINT(_L("SCEP::NewL()\n"));
	
	SCEP* self = new(ELeave) SCEP();
	CleanupStack::PushL(self);
	self->ConstructL(aTranpProtocol, a_oBFTP, aNotifier);
	CleanupStack::Pop();
	return self;
	}

/*
* Method description:	Second part of the SCEP construction - creates the CommandProcessor and CComReadWrite.
*
* Parameter:			Pointer to the BFTP class
*
*/

void SCEP::ConstructL(CTranpProtocol* aTranpProtocol, BFTP* a_oBFTP, const MTranpNotification* aNotifier)
	{
	
	SCEPPRINT(_L("SCEP::ConstructL()\n"));
	iNotifier = const_cast<MTranpNotification*>(aNotifier);
	iState = EWaiting;

	m_uiMaxPDU = 4;
	
	m_oCommandP = new(ELeave) CommandProcessor();
	iComReadWrite = CComReadWrite::NewL(*this);
	
	m_oBFTP = a_oBFTP;
	iTranpProtocol = aTranpProtocol;
	
	m_packet = NULL;
	m_iPicSize = 0;
	}

SCEP::~SCEP()
	{
	delete m_oCommandP;
	delete iComReadWrite;
	//delete m_packet; Ownership now belongs to TTranpPicture Object
	}

/*
* Method description:	Pump - receives and sends data, should be called periodically. Due to the way IrTran-P and
*						serial communication works, it also tries to check when it has received a complete SCEP
*						packet. This should be handled in a differerent (and more nice) way.
*
* Returns:				An integer containing the progress from 0 to 100
*/

void SCEP::Abort()
	{
	iComReadWrite->Cancel();
	iComReadWrite->Signal(KErrNone);
	}

TInt SCEP::Pump()
	{
	// Set up the IR port connection with a null write
	//delete m_packet;
	//m_packet = NULL;
	//iState = EWaiting;
	//iSCEPFrame.SetLength(0);
	iComReadWrite->Open();
	return 0;
	}

void SCEP::ReceiveComplete(TDesC8& aBuffer)
	{
	SCEPPRINT_2(_L("SCEP::ReceiveComplete(%d)\n"),aBuffer.Length());

	iSCEPFrame.Append(aBuffer); // Add new data to cached packet

	TBool loopagain;
	do
		{
		loopagain = EFalse;
		
		if ((iSCEPFrame[0]==0x00)&&(iSCEPFrame.Length()>=3))
			{
			TInt tempLength = 0;
			TInt additionalLength = 0;
			TInt lengthPos = 3;

			if ((iSCEPFrame[1]==KTranpMsgTypeCER) && (iSCEPFrame[2]==KTranpInfTypeVer))
				{
				lengthPos = 5;
				additionalLength = 4; // ... and that same packet has 4 trailing bytes
				}
				
			if (iSCEPFrame.Length()>lengthPos+2)// Otherwise we might miss out on the last packet
				{
				if(iSCEPFrame[0] == 0x00) // Let's guess it's an SCEP header - HARDCODED AND UGLY!!!
					{
					tempLength = iSCEPFrame[lengthPos];
					if(tempLength == 0xff)
						{
						tempLength = IrTranpUtil::DExtract(iSCEPFrame, lengthPos+1);
						lengthPos = 5;
						}
					SCEPPRINT_2(_L("tempLength = %d"), tempLength);
					}

				tempLength += ((lengthPos+1) + additionalLength);
				
				SCEPPRINT_3(_L("iSCEPFrame.length(%d) >= templength(%d)"),iSCEPFrame.Length(),tempLength);

					switch (iSCEPFrame[1])
					{
				case KTranpMsgTypeCER: // MsgType = Connection establishment request
				case KTranpMsgTypeCEC: // MsgType = Connection establishment confirmation
				case KTranpMsgTypeData: // Data (Command)
				case KTranpMsgTypeDisc: // Disconnection
					{
					switch (iSCEPFrame[2])
						{
					case KTranpInfTypeVer: // Version of MsgType
					case KTranpInfTypeNeg: // Negotiation Information
					case KTranpInfTypeUser: // UserData
					case KTranpInfTypeExt: // Extend in the future
					case KTranpInfTypeRsn: // Reason
						break;
					default:
						// bogus frame
						SCEPPRINT(_L("SCEP::Bogus Inf Type in frame\n"));
						iSCEPFrame.SetLength(0);
						}
					}
					break;
				default:
					// bogus frame
					SCEPPRINT(_L("SCEP::Bogus Msg Type in frame\n"));
					iSCEPFrame.SetLength(0);
					}

				if(iSCEPFrame.Length() >= tempLength) // yes, complete packet
					{
					TInt oldLength = iSCEPFrame.Length();
					iSCEPFrame.SetLength(tempLength); // We only want exactly one packet
					ParseCommand(iSCEPFrame);
					
					iSCEPFrame.SetLength(oldLength);
					iSCEPFrame.Delete(0, tempLength); // The surplus data belongs to the next packet
					SCEPPRINT(_L("\r\n***\r\n"));
					//IrTranpUtil::HexOut(iSCEPFrame);
					SCEPPRINT(_L("***\r\n"));
					loopagain = (oldLength > tempLength) ? ETrue : EFalse;
					}		
				}	
			}
		else
			{
			SCEPPRINT(_L("SCEP:: Bogus Frame header\n"));
			iSCEPFrame.SetLength(0);
			}
		} while (loopagain);


	// let the commandprocessor send whatever's in queue
	
	m_oCommandP->DoCommand();

	if ( (!(iComReadWrite->IsActive())) && !(iState == EError) ) //Make sure that thre is always an outstanding request
		iComReadWrite->Receive(); 

	switch(iState)
		{
	case EConnected:
		iNotifier->Connected();
		iState = EProgress;
		SCEPPRINT(_L("SCEP: EConnected -> EProgress\r\n"));
		break;
	case EDisconnected:
		iNotifier->Disconnected();
		iComReadWrite->Close();
		iState = EWaiting;
		SCEPPRINT(_L("SCEP: EDisconnected -> EWaiting\r\n"));
		break;
	case EProgress:
		{
		SCEPPRINT(_L("SCEP: EProgress\r\n"));

		TInt32 progress = 0;
		if(m_packet!=NULL)
			{
			if(m_packet->Length() != 0)
				{
				Math::Int(progress, m_packet->Length()*100 / m_iPicSize);
				}
			iNotifier->ProgressIndication(progress);
			}
		}
		break;
	case EWaiting:
		SCEPPRINT(_L("SCEP: EWaiting\r\n"));
		break;
	case EError:
		SCEPPRINT(_L("SCEP: EError\r\n"));
		break;
	case ECommunicationInt:
		SCEPPRINT(_L("SCEP: ECommunicationInt\r\n"));
		iNotifier->Disconnected();
		iComReadWrite->Close();
		iState = EWaiting;
		break;
	default:
		SCEPPRINT(_L("SCEP: Unknown state\r\n"));
		break;
		}
	SCEPPRINT(_L("SCEP: Exit ReceiveComplete\r\n"));
	}

void SCEP::SendComplete()
	{
	// Post Read Request
	SCEPPRINT(_L("SCEP: Send Complete\n"));
	iComReadWrite->Receive();
	}

void SCEP::SendError(TInt aError)
	{
	SCEPPRINT_2(_L("SCEP: Send Error %d\n"), aError);
	iNotifier->Error(aError);
	iState = EError;
	}

void SCEP::ReceiveError(TInt aError)
	{
	SCEPPRINT_2(_L("SCEP: Receive Error %d\n"), aError);
	iNotifier->Error(aError);
	iState = EError;
	}
void SCEP::Error(TInt aError)
	{
	SCEPPRINT_2(_L("SCEP: Error %d\n"), aError);
	iNotifier->Error(aError);
	iState = EError;
	}


/*
* Method description:	Called from BFTP when a connection request should be sent
*
*/

void SCEP::SCONreqL()
	{
	SCONreqCommand* command = new(ELeave) SCONreqCommand(iComReadWrite);
	command->SetPMID(m_PMID.Des());
	command->SetSMID(m_SMID.Des());
	command->SetPDU(m_uiMaxPDU);
	command->Create();
	//IrTranpUtil::HexOut(command->GetData());
	m_oCommandP->Add(command);
	}

/*
* Method description:	Called from BFTP when a connection confirmed should be sent
*
*/

void SCEP::SCONconL()
	{
	SCONconCommand* command = new(ELeave) SCONconCommand(iComReadWrite);
	command->SetPMID(m_PMID.Des());
	command->SetSMID(m_SMID.Des());
	command->SetPDU(m_uiMaxPDU);
	command->Create();
//	IrTranpUtil::HexOut(command->GetData());
	m_oCommandP->Add(command);
	}

/*
* Method description:	Called from BFTP when a common BFTP packet should be sent
*
*/

void SCEP::SCEPSendCommand(TDes8& a_userData, TUint8 a_pduType)
	{
	SRPSCommand* command = new (ELeave) SRPSCommand(iComReadWrite);
	command->SetPMID(m_PMID.Des());
	command->SetSMID(m_SMID.Des());
	command->SetDPID(m_DPID);
	command->SetSPID(m_SPID);
	command->SetCmdId(m_CmdId);
	command->SetPDUType(a_pduType);
	command->SetUserData(a_userData);
	command->Create();
//	IrTranpUtil::HexOut(command->GetData());
	m_oCommandP->Add(command);
	}

/*
* Method description:	Private method called when a Connection Request packet should be parsed
*
* Parameter:			a_Header - pointer to the packet buffer
* Parameter:			offset - where in the buffer to start parsing
*/

void SCEP::SCEPConRequest(TDes8& a_Header, TInt offset)
	{
	SCEPPRINT(_L("Inside SCEPConRequest\n"));
	
	TInt length = a_Header.Length();

	while(offset<length)
		{
		if(a_Header[offset] == KTranpInfTypeVer)
			//		case IrTranpUtil::InfTypeVer:
			{
			offset+=2;
			}
		else if(a_Header[offset] == KTranpInfTypeNeg)
			{
			// Negotiation information
			//				TUint8 length = a_Header[offset+1];
			//				TUint8 InfVer = a_Header[offset+2];
			//				TUint8 CFLG = a_Header[offset+3];
			
			//TPtr8 SMID((unsigned char*)a_Header.Ptr()+offset+4, 8, a_Header.MaxLength());
			TPtr8 PMID((unsigned char*)a_Header.Ptr()+offset+12, 8, a_Header.MaxLength());
			
			m_PMID = PMID;
			//				m_SMID = SMID; // We want to keep ours
			
			NegInf(a_Header, offset+20);
			
			offset+=(length+2);
			}
		else if(a_Header[offset] == KTranpInfTypeExt)
			{
			// Ehh .. dunno .. "extend in the future"
			offset+=4;
			}
		else
			{
			// Error Not a Valid SCEP connection request...
			SCEPPRINT(_L("Not a Valid SCEPConRequest\n"));
			iNotifier->Error(KErrIrtranpPeerDoesNotHaveIrtranp); //KErrIrtranpPeerDoesNotHaveIrtranp
			break;
			}
		}
	SCEPPRINT(_L("Exiting SCEPConRequest\n"));
	}

/*
* Method description:	Private method called when a commond Data packet should be parsed
*
* Parameter:			a_Header - pointer to the packet buffer
* Parameter:			offset - where in the packet to start parsing
*/

void SCEP::SCEPDataCommand(TDes8& a_Header, TInt offset)
	{
	SCEPPRINT(_L("Inside SCEPDataCommand\n"));
	
	if(a_Header[offset] == KTranpInfTypeUser)
		{
		// Always InfTypeUser when in SCEPDataCommand
		TInt packetLength = a_Header[offset+1];
		if(packetLength == 0xff)
			{
			// since length1 was 0xff, look at length2
			packetLength = ((a_Header[offset+2] << 8) + (a_Header[offset+3])); // 16bit
			offset+=4; // include inftype and length2
			}
		else
			{
			offset+=2; // include inftype and length1
			}
		
		// TUint8 InfVer = a_Header[offset];
		TUint8 DFLG = a_Header[offset+1];
		// TUint16 length3 = IrTranpUtil::DExtract(a_Header, offset+2);
		
		if(DFLG == KTranpDFLGrcon)
			{
			// Reject to request connection
			SCEPPRINT(_L("Connection rejected\n"));
			}
		else if(DFLG == KTranpDFLGnseg)
			{
			// PDU not segmented (single PDU)
			DoCommand(a_Header, offset+4);
			//					break;
			}
		else if(DFLG == KTranpDFLGfPDU)
			{
			// First segmented PDU
			// We do all reassembling in SCEP, and then send the complete (big) packet to bFTP
			// restno * PDU must be able to hold the complete packet, right?
			
			// Ownership of this data [m_packet] belongs to the TTranpPicture object
			// so must ask the correct object to delete its heap.
			// If we were already building a packet, get rid of it
			iTranpProtocol->DeleteHeapData();
			
			TUint32 seqno = IrTranpUtil::LExtract(a_Header, offset+4);
			TUint32 restno = IrTranpUtil::LExtract(a_Header, offset+8);
			
			TUint32 PDUSize;
			switch(m_uiMaxPDU)
				{
				case '1':
					{
					PDUSize = 512;
					break;
					}
				case '2':
					{
					PDUSize = 1024;
					break;
					}
				case '3':
					{
					PDUSize = 2048;
					break;
					}
				case '4':
					{
					PDUSize = 4096;
					break;
					}
				default:
					{
					PDUSize = 512;
					break;
					}
				}
			
			// This should not leave and report an error back...
			//__UHEAP_FAILNEXT(1);
			TRAPD( err , m_packet = HBufC8::NewL(PDUSize*restno)); // should be enough
			if (err != KErrNone)
				{
				iNotifier->Error(err);
				iState = EError;
				return;
				}
			else
				iTranpProtocol->SetHeapData(m_packet);
			
			m_iPicSize = PDUSize*restno;
			
			SCEPPRINT(_L("ResizeL went well\n"));
			
			m_seq = seqno;
			AddSegment(a_Header, offset+12);
			}
		else if(DFLG == KTranpDFLGiPDU)
			{
			// Intermediate segmented PDUs
			
			TUint32 seqno = IrTranpUtil::LExtract(a_Header, offset+4);
			if(seqno == m_seq+1)
				{
				AddSegment(a_Header, offset+12);
				m_seq++;
				}
			else
				{
				SCEPPRINT(_L("Error in intermediate PDU\n"));
				}
			}
		else if(DFLG == KTranpDFLGlPDU)
			{
			// Last segmented PDU
			
			m_seq++;
			TUint32 restno = IrTranpUtil::LExtract(a_Header, offset+8);
			if(restno == 1)
				{
				AddSegment(a_Header, offset+12);
				//TPtr8 packetPtr = m_packet->Des();
				DoCommand(*m_packet, 0); // Now let's play
                iNotifier->ProgressIndication(100); // Makes sure 100% is reached
		        iNotifier->GetComplete();
                TPtr8 temp2 = m_packet->Des();
	            temp2.Zero();
				}
			else
				{
				SCEPPRINT(_L("Error in last PDU\n"));
				}
			}
		else if(DFLG == KTranpDFLGcint)
			{
			SCEPPRINT(_L("Communication interrupted\n"));
			iState = ECommunicationInt;
			}
		else
			{
			// Ooops.
			}
		}
	else 
		{
		// oops
		}
			
	SCEPPRINT(_L("Exiting SCEPDataCommand\n"));
}

/*
* Method description:	Adds segmented packets into a complete packet
*
* Parameter:			a_Header - pointer to the segment
* Parameter:			a_offset - where in the segment data to be assembled is
*/

void SCEP::AddSegment(TDes8& a_Header, TInt a_offset)
	{
	SCEPPRINT(_L("Entered AddSegment\n"));
	TPtr8 temp((unsigned char*)a_Header.Ptr()+a_offset, a_Header.Length()-a_offset, a_Header.MaxLength());
	TPtr8 temp2 = m_packet->Des();
	temp2.Append(temp);
	SCEPPRINT(_L("Exited AddSegment\n"));
	}

	/*
	* Method description:	Private method called to parse an SCEP disconnetion packet
	*
	* Parameter:			a_Header - pointer to the packet
	* Parameter:			offset - where in the packet to start parsing
*/

void SCEP::SCEPDisconnect(TDes8& a_Header, TInt offset)
	{
	SCEPPRINT(_L("SCEP:  SCEPDisconnect\n"));
	SCEPPRINT_2(_L("SCEP:  %d -> EDisconnected\r\n"), iState);
	
	if(a_Header[offset] == KTranpInfTypeRsn)
		{
		TUint16 reason = IrTranpUtil::DExtract(a_Header, offset+2);
		if(reason == KTranpDisPDUrsnUR)
			{
			SCEPPRINT(_L("SCEP: Disconnect: Unspecified Reason\n"));
			iState = EDisconnected;

			}
		else if(reason == KTranpDisPDUrsnUD)
			{
			SCEPPRINT(_L("SCEP: Disconnect: User Disconnect\n"));
			iState = EDisconnected;

			}
		else if(reason == KTranpDisPDUrsnPD)
			{
			SCEPPRINT(_L("SCEP: Disconnect: Provider Disconnect\n"));
			iState = EDisconnected;

			}
		else
			{
			// reserved
			}
		}
	else
		{
		// oops
		}
	SCEPPRINT(_L("SCEP: Exiting SCEPDisconnect\n"));
	return;
	}

/*
* Method description:	Private method called when parsing a Connection Confirmed packet
*
* Parameter:			a_Header - pointer to the packet
* Parameter:			offset - where in the packet to start parsing
*/

void SCEP::SCEPConConfirm(TDes8& a_Header, TInt offset)
	{
	SCEPPRINT(_L("Inside SCEPConConfirm\n"));
	
	TInt length = a_Header.Length();
	
	while(offset<length)
		{
		if(a_Header[offset] == KTranpInfTypeVer)
			{
			// Should always be 01?
			offset+=2;
			}
		else if(a_Header[offset] == KTranpInfTypeNeg)
			{
			// Negotiation information
			TUint8 length = a_Header[offset+1];
			//				TUint8 InfVer = a_Header[offset+2];
			//				TUint8 CFLG = a_Header[offset+3];
			
			TPtr8 PMID((unsigned char*)a_Header.Ptr()+offset+4, 8, a_Header.MaxLength());
			TPtr8 SMID((unsigned char*)a_Header.Ptr()+offset+12, 8, a_Header.MaxLength());
			
			m_PMID = PMID;
			m_SMID = SMID;
			
			NegInf(a_Header, offset+20);
			offset+=(length+2);
			}
		else if(a_Header[offset] == KTranpInfTypeExt)
			{
			// "extend in the future"
			
			SCEPPRINT(_L("InfTypeExt appeared in ConConfirm - error in protocol\n"));
			// Should not appear in a ConConfirm!
			offset+=4;
			}
		else //default
			{
			}
		} // End of While loop
	}

/*
* Method description:	Private method called when parsing the Command-section of a packet
*
* Parameter:			a_Header - pointer to the packet
* Parameter:			a_offset - where in the packet the command is
*/

void SCEP::DoCommand(const TDesC8& a_Header, TInt a_offset)
	{
	SCEPPRINT(_L("SCEP: DoCommand\n"));
	
	if(a_Header[a_offset] != 0x58) // Always 0x58 at the start of a command
		{
		SCEPPRINT(_L("SCEP: Command did not start with 0x58\n"));
		return;
		}
	
	TUint8 switchVar = (TUint8)(a_Header[a_offset+1] & 0xc0); // we only want the two highest bits for PDU type
	if(switchVar == KTranpPduTypeReq)
		{
		// Don't care about the MachineIDs for now
		SCEPPRINT(_L("SCEP: PduTypeReq\n"));

		m_SPID = IrTranpUtil::DExtract(a_Header, 22 + a_offset);
		m_DPID = IrTranpUtil::DExtract(a_Header, 24 + a_offset);
		m_CmdId = IrTranpUtil::DExtract(a_Header,26 + a_offset);
		// Pass SCEP packet Payload to BFTP layer
		m_oBFTP->ReqPDU(a_Header, a_offset + 28); // hardcoded, but always 28
		}
	else if(switchVar == KTranpPduTypeRplAck)
		{
		SCEPPRINT(_L("SCEP: PduTypeRplAck\n"));
		}
	else if(switchVar == KTranpPduTypeRplNack)
		{
		SCEPPRINT(_L("SCEP: PduTypeRplNack\n"));
		}
	else if(switchVar == KTranpPduTypeAbt)
		{
		SCEPPRINT(_L("SCEP: PduTypeAbt\n"));
		}
	else
		{
		SCEPPRINT(_L("SCEP: PduType Broken\n"));
		}
	
	SCEPPRINT(_L("SCEP: Exiting DoCommand\n"));
	}

/*
* Method description:	Private method called to parse the NegInf section of a packet
*
* Parameter:			a_Header - pointer to the packet
* Parameter:			a_offset - where in the packet the NegInf is
*/

void SCEP::NegInf(TDes8& a_Header, TInt a_offset)
	{
	SCEPPRINT(_L("SCEP: NegInf\n"));
	
	if(a_Header[a_offset] != KTranpNegVer)
		{
		return; // skip the NegInf totally
		}
	TInt length = a_Header.Length();
	while(a_offset<length) // Parse all NegContent attributes
		{
		TBuf<8> l(8);
		l.Num(a_offset);
		TBuf<8> k(8);
		k.Num(length);
		
		TBufC8<4> temp;
		TPtr8 temp2 = temp.Des();
		for(TInt j=0;j<3;j++) // we don't like it .. hardcoded
			{
			temp2.Append(a_Header[a_offset+1+j]);
			}
		
		TInt valueOffset = a_offset+1+temp.Length();
		if(temp.Compare(KTranpFR) == 0)
			{
			SCEPPRINT(_L("SCEP: Found FR attribute\n"));
			// pump whitespace
			if(a_Header[valueOffset] == ' ')
				{
				valueOffset++;
				}
			
			m_uiMaxPDU = a_Header[valueOffset];
			
			// Now skip CRLF - pump
			TBool found = FALSE;
			while(found == FALSE)
				{
				if(a_Header[valueOffset] == KTranpLF) // Why look for both CR and LF? LF always follows CR .. 
					{
					found = TRUE;
					}
				else
					valueOffset++;
				}
			}
		else if(temp.Compare(KTranpID) == 0)
			{
			SCEPPRINT(_L("SCEP: Found ID attribute\n"));
			TBool found = FALSE;
			TPtr8 temp = m_szPIS.Des();
			temp.Zero();
			do
				{
				if(a_Header[valueOffset] != KTranpCR)
					{
					temp.Append(a_Header[valueOffset++]);
					}
				else
					{
					found = TRUE;
					valueOffset+=2; // skip LF too
					}
				}
				while(found == FALSE);
			}
		else if(temp.Compare(KTranpNM) == 0)
			{
			SCEPPRINT(_L("SCEP: Found NM attribute\n"));
			TBool found = FALSE;
			TPtr8 temp = m_szUserName.Des();
			temp.Zero();
			do
				{
				if(a_Header[valueOffset] != KTranpCR)
					{
					temp.Append(a_Header[valueOffset++]);
					}
				else
					{
					found = TRUE;
					valueOffset+=2; // skip LF too
					}
				}
				while(found == FALSE);
			}
		else if(temp.Compare(KTranpPW) == 0)
			{
			SCEPPRINT(_L("SCEP: Found PW attribute\n"));
			TBool found = FALSE;
			TPtr8 temp = m_szPassword.Des();
			temp.Zero();
			do
				{
				if(a_Header[valueOffset] != KTranpCR)
					{
					temp.Append(a_Header[valueOffset++]);
					}
				else
					{
					found = TRUE;
					valueOffset+=2; // skip LF too
					}
				}
				while(found == FALSE);
			}
		else
			{
			// Something that wasn't an attribute
			}
		a_offset = valueOffset;
		
	}
	iState = EConnected;
	//iNotifier->Connected();
	SCEPPRINT(_L("SCEP: Exiting NegInf\n"));
}

//
// Method description:	Private method called when a packet has been received. Here is where the parsing begins.
//						Also creates the necessary reply-commands at SCEP level.
//
// Parameter:			a_TempBuffer - the packet itself
//

void SCEP::ParseCommand(TDes8& a_TempBuffer)
	{
	SCEPPRINT(_L("SCEP: ParseCommand\n"));
	// parse the incoming buffer and see what command we're talking about. Return an instance of such a command.
	
	// Look at the first bytes, see what command it might be, call the correct private method and let it take care of the rest?
	
	if(a_TempBuffer[1] == KTranpMsgTypeCER)
		// MsgType = Connection establishment request
		{
		SCEPPRINT(_L("SCEP: Got a connection request"));
		SCEPConRequest(a_TempBuffer, 2);
		
		// Create and add a SCEPConConfirm!
		SCONconL();
		}
	else if(a_TempBuffer[1] == KTranpMsgTypeCEC)
		// MsgType = Connection establishment confirmation
		{
		SCEPPRINT(_L("SCEP: Got a connect ack"));
		SCEPConConfirm(a_TempBuffer, 2);
		}
	else if(a_TempBuffer[1] == KTranpMsgTypeData)
		// Data (Command)
		{
		SCEPPRINT(_L("SCEP: Got a data packet\n"));
		SCEPDataCommand(a_TempBuffer, 2);
		}
	else if(a_TempBuffer[1] == KTranpMsgTypeDisc)
		// Disconnection
		{
		SCEPPRINT(_L("SCEP: Got a disconnect packet\n"));
		SCEPDisconnect(a_TempBuffer, 2);
		if (m_packet != NULL)
			m_iPicSize = m_packet->Length(); // Otherwise we'll never get to 100%
		iSCEPFrame.Zero();
		}
	else
		{
		SCEPPRINT(_L("SCEP: Wacko! Default!\n"));
		// Reserved
		}
	
	
	SCEPPRINT(_L("SCEP: Exiting ParseCommand\n"));
	
	return;
	}

//
// Method description:	Sets the Primary Machine ID
//
// Parameter:			a_PMID - the new Machine ID
//

void SCEP::SetPMID(const TDesC8& aPMID)
	{
	m_PMID = aPMID;
	}

//
// Method description:	Sets the Secondary Machine ID
//
// Parameter:			a_SMID - the new Machine ID
//

void SCEP::SetSMID(const TDesC8& aSMID)
	{
	m_SMID = aSMID;
	}

//
// Method description:	Sets the maximum receivable PDU size
//
// Parameter:			a_PDU - the new PDU size
//
void SCEP::SetPDU(TUint8 a_PDU)
	{
	m_uiMaxPDU = a_PDU;
	}

