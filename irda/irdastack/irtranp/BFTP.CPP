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

#include <e32svr.h>
#include <e32cons.h>
#include <e32base.h>
#include "debug.h"
#include "GLOBAL.H"
#include "comreadwrite.h"
#include "SCEP.H"
#include "tranp.h"
#include "BFTP.H"
#include "tranpprot.h"

BFTP::BFTP() : iPicture(NULL,0), iUPFPicture(NULL,0)
	{
	}

 BFTP* BFTP::NewL(CTranpProtocol* aTranpProtocol, const MTranpNotification* aNotifier)
	{
	DEBUG_OPEN();
	BFTPPRINT(_L("BFTP::NewL()\n"));
	BFTP* self = new(ELeave) BFTP();
	CleanupStack::PushL(self);
	self->ConstructL(aTranpProtocol, aNotifier);
	CleanupStack::Pop();
	return self;
	}

 void BFTP::ConstructL(CTranpProtocol* aTranpProtocol, const MTranpNotification* aNotifier)
	{
	BFTPPRINT(_L("BFTP::ConstructL()\n"));
	iTranpProtocol = aTranpProtocol;
	m_oSCEP = SCEP::NewL(aTranpProtocol,this,aNotifier);
	}

 BFTP::~BFTP()
	{
	delete m_oSCEP;
	}

/*
* Method description:	Called from SCEP with a buffer containing a Request PDU
*
* Parameter:			a_Header - TPtr pointing the command buffer
* Parameter:			a_offset - the offset into the buffer where we should start parsing
*/

void BFTP::ReqPDU(const TDesC8& a_Header, TInt a_offset)
	{
	BFTPPRINT(_L("BFTP: ReqPDU\n"));
	
	TUint16 AttNum = IrTranpUtil::DExtract(a_Header, a_offset);
	a_offset+=2;
	
	for(TInt i=0;i<AttNum;i++) // Loop all AttNames
		{
		TBuf8<4> attName; // BFTP Attribute name
		for(TInt j=0;j<4;j++) // we don't like it .. hardcoded
			{
			attName.Append(a_Header[a_offset+j]);
			}
		
		a_offset+=4;
		
		if(attName.Compare(KTranpFIL0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute FIL0\n"));
			
			m_FileName.SetLength(0);
			m_FileName.Append((unsigned char*)a_Header.Ptr() + a_offset + 6, length-2);
			
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpLFL0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute LFL0\n"));

			iLongFilename.Append((unsigned char*)a_Header.Ptr() + a_offset + 6, length-2);

			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpTIM0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute TIM0\n"));
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpTYP0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute TYP0\n"));
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpTMB0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute TMB0\n"));
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpBDY0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute BDY0\n"));
			
			//iPicture.Set((unsigned char*)a_Header.Ptr() + a_offset + 6 + KTranpUPFHeader, length-2, length-2);
			iUPFPicture.Set((unsigned char*)a_Header.Ptr() + a_offset + 6, length-2, length-2);
			
			iTranpProtocol->SetPicture(iPicture, m_FileName, iUPFPicture, iLongFilename);

			a_offset+=(length+4);
			/**/
			TBuf8<256> userData;
		
			// AttNum
			userData.Append(0x00);
			userData.Append(0x01);
			// attName
			userData.Append(KTranpRPL0);
			// AttLength
			userData.Append(0x00);
			userData.Append(0x00);
			userData.Append(0x00);
			userData.Append(2 + m_FileName.Length());
			// AttType
			userData.Append(0x01);
			// AFLG
			userData.Append(0x00);
			// AttValue
			userData.Append(m_FileName);
			
			TUint8 pduType = KTranpPduTypeRplAck;
			m_oSCEP->SCEPSendCommand(userData, pduType); // We want SCEP to send a packet with this command
			}
		else if(attName.Compare(KTranpCMD0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute CMD0\n"));
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpWHT0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute WHT0\n"));
			
			// how to avoid copying of the information we put in the command??? Don't avoid it for now.
			TPtr8 queryType((unsigned char*)a_Header.Ptr() + a_offset+6, 4, 4);

			TBuf8<256> userData;
			//TPtr8 userDataPtr = userData.Des();
			
			if (queryType.Compare(KTranpRIMG) == 0)
				{
				//This reply is for an RIMG query
				// Its instructs the peer to send the image in its orginal lattice size

				// AttNum
				userData.Append(0x00);
				userData.Append(0x01);
				// AttName
				userData.Append(KTranpBDY0);
				// AttLength
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x0d);
				// AttType
				userData.Append(0x00);
				// AFLG
				userData.Append(0x00);
				
				/*userData.Append(0x00);// <tag-pix-aspect> 3 bytes
				userData.Append(0x01);
				userData.Append(0x01);	*/

				switch(iLatticeSize)
					{
				case EQVGA:
					//QVGA:=320x240  (0x0140 x 0x00f0)
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0x40); // width [2 bytes]
//					userData.Append(0x01);
//					userData.Append(0xf0);// height [2 bytes]
//					userData.Append(0x00);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0x40);// width [2 bytes]
					userData.Append(0x01);
					userData.Append(0xf0);// height [2 bytes]
					userData.Append(0x00);
				break;
				case EVGA :
					// VGA:=640x480 ( 0x0280 x 0x01e0)
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0x80); // width [2 bytes]
//					userData.Append(0x02);
//					userData.Append(0xe0);// height [2 bytes]
//					userData.Append(0x01);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0x80); // width [2 bytes]
					userData.Append(0x02);
					userData.Append(0xe0);// height [2 bytes]
					userData.Append(0x01);
				break;
				case ESVGA:
					// SVGA:=800x600 (0x0320 x 0x0258)
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0x20);// width [2 bytes]
//					userData.Append(0x03);
//					userData.Append(0x58);// height [2 bytes]
//					userData.Append(0x02);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0x20);// width [2 bytes]
					userData.Append(0x03);
					userData.Append(0x58);// height [2 bytes]
					userData.Append(0x02);
					break;
				case EXGA:
					// XGA:=1024x768 ( 0x0400 x 0x0300)
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0x00);// width [2 bytes]
//					userData.Append(0x04);
//					userData.Append(0x00);// height [2 bytes]
//					userData.Append(0x30);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0x00);// width [2 bytes]
					userData.Append(0x04);
					userData.Append(0x00);// height [2 bytes]
					userData.Append(0x03);	
					break;
				case ESXGA: 
					// SXGA:=1280x960 (0x0500 x 0x03c0)
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0x00);// width [2 bytes]
//					userData.Append(0x05);
//					userData.Append(0xc0);// height [2 bytes]
//					userData.Append(0x03);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0x00);// width [2 bytes]
					userData.Append(0x05);
					userData.Append(0xc0);// height [2 bytes]
					userData.Append(0x03);
				break;
				case EFREE:
					// FREE:=m x n
				default:
					// Send in orginal size
//					userData.Append(0x01);// <tag-org-size>     5 bytes
//					userData.Append(0xFF);
//					userData.Append(0xFF);
//					userData.Append(0xFF);
//					userData.Append(0xFF);
				
					userData.Append(0x02);// <tag-acc-size>     6 bytes
					userData.Append(0x01);// <num-acc-size>
					userData.Append(0xFF);// width [2 bytes]
					userData.Append(0xFF);
					userData.Append(0xFF);// height [2 bytes]
					userData.Append(0xFF);	
					};

				/*userData.Append(0x03);// <tag-org-samp>   3 bytes
				userData.Append(0xC4);
				userData.Append(0x20);
				
				userData.Append(0x04);// <tag-acc-samp>   3 bytes
				userData.Append(0xC4);
				userData.Append(0x20);*/
				
				userData.Append(0x05);//<tag-acc-filesize> 5 bytes
				userData.Append(0xFF);
				userData.Append(0xFF);
				userData.Append(0xFF);
				userData.Append(0xFF);	
				}
			else if (queryType.Compare(KTranpRINF) == 0)
				{
				// This  reply is for an RINF query
				userData.Append(0x00);
				userData.Append(0x01);
				// AttName
				userData.Append(KTranpBDY0);
				// AttLength
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x08);
				// AttType
				userData.Append(0x00);
				// AFLG
				userData.Append(0x00);
				// AttValue
				userData.Append(0x10); // memory
				userData.Append(0xff);
				userData.Append(0xff);
				userData.Append(0x11); // battery
				userData.Append(0xff);
				userData.Append(0xff);	
				}
			else if (queryType.Compare(KTranpRCMD) == 0)
				{
                // This  reply is for an RINF query
				userData.Append(0x00);
				userData.Append(0x01);
				// AttName
				userData.Append(KTranpBDY0);
				// AttLength
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x00);
				userData.Append(0x07);
				// AttType
				userData.Append(0x00);
				// AFLG
				userData.Append(0x00);
				// AttValue
				userData.Append(0x20); // tag-opt-func
                userData.Append(0x00);
				userData.Append(0x01);
				userData.Append(0x00);
				userData.Append(0x01); // func-multi-command
				}

			a_offset+=(length+4);

			TUint8 pduType = KTranpPduTypeRplAck;
			m_oSCEP->SCEPSendCommand(userData, pduType); // We want SCEP to send a packet with this command
			}
		else if(attName.Compare(KTranpERR0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute ERR0\n"));
			a_offset+=(length+4);
			}
		else if(attName.Compare(KTranpRPL0) == 0)
			{
			TUint32 length = IrTranpUtil::LExtract(a_Header, a_offset);
			BFTPPRINT(_L("BFTP: Attribute RPL0\n"));
			a_offset+=(length+4);
			}
	}
	
	BFTPPRINT(_L("BFTP: Exiting ReqPDU\n"));
}

/*
* Method description:	Called from the application with a picture to send
*
* Parameter:			a_picture - TPtr pointing to the picture
* Parameter:			a_fileName - filename of the picture
*/

void BFTP::Put(TPtr8* /*a_picture*/, TPtr /*a_fileName*/)
	{
	// Not fully implemented as of now
	}

/*
* Method description:	Called from the application, returns a TPtr with length>0 when picture is ready.
*						Use Pump() (returnvalue = progress from 0 to 100) to run the module!
*
* Returns:				Pointer to a TPtr pointing to the picture
*/

// TPtr8* BFTP::Get()
//	{
//	return m_oPicture;	// Null if no picture has been received yet
//	}

	/*
	* Method description:	Called from the application, returns a pointer to the filename
	*
	* Returns:				Pointer to the filename
*/

// TPtr8 BFTP::GetFileName()
void BFTP::GetFileName()
	{
	// Deprecated Function - No Longer used
	return ;//m_FileName.Des();
	}

/*
* Method description:	Runs the module, call at intervals like 200 ms
*
* Returns:				Integer containing the progress from 0 to 100
*/

TInt BFTP::Pump(TLatticeSize aLatticeSize)
	{
	iLatticeSize = aLatticeSize;
	return m_oSCEP->Pump();
	}

void BFTP::Abort()
	{
	m_oSCEP->Abort();
	}

/*
* Method description:	Passing through initialization data to SCEP
*
* Parameter:			a_oPMID - Primary Machine ID
*/

void BFTP::SetPMID(const TDesC8& aPMID)
	{
	m_oSCEP->SetPMID(aPMID);
	}

/*
* Method description:	Passing through initialization data to SCEP
*
* Parameter:			a_oSMID - Secondary Machine ID
*/

void BFTP::SetSMID(const TDesC8& aSMID)
	{
	m_oSCEP->SetSMID(aSMID);
	}

/*
* Method description:	Passing through initialization data to SCEP
*
* Parameter:			a_iPDU - Maximum receivable PDU size
*/

void BFTP::SetPDU(TUint8 a_iPDU)
	{
	m_oSCEP->SetPDU(a_iPDU);
	}

