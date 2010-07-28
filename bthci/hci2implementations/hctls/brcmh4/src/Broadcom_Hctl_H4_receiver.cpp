// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

/**
 @file
 @internalComponent
*/

#include "Broadcom_Hctl_H4_receiver.h"

#include "Broadcom_Hctl_H4.h"
#include "Broadcom_Hctl_H4_utils.h"

#include <d32comm.h>
#include <es_prot.h> // For GetLittleEndian methods
#include <bluetooth/logger.h>

// These files are included to get HCI specification defined constants.
#include <bluetooth/hci/hciframe.h>
#include <bluetooth/hci/event.h>



#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_HCTL_BCM_H4);
#endif


CHCTLBcmH4Receiver::CHCTLBcmH4Receiver(CHCTLBcmH4& aHCTLBcmH4,RBusDevComm& aPort)
  : CActive(EPriorityStandard), 
    iHCTLBcmH4(aHCTLBcmH4), 
	iState(EWaitingForHctlHeaderByte),
	iReceiveBufPtr(NULL,0),
    iPort(aPort)
    {
	LOG_FUNC
	CActiveScheduler::Add(this);
    }

CHCTLBcmH4Receiver::~CHCTLBcmH4Receiver()
    {
	LOG_FUNC

	Cancel();
	iReceiveBuffer.Close();

	HCI_LOG_UNLOAD(this);
    }

CHCTLBcmH4Receiver* CHCTLBcmH4Receiver::NewL(CHCTLBcmH4& aHCTLBcmH4, RBusDevComm& aPort) 
    {
	LOG_STATIC_FUNC

	CHCTLBcmH4Receiver* self=new(ELeave)CHCTLBcmH4Receiver(aHCTLBcmH4, aPort);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

void CHCTLBcmH4Receiver::ConstructL()
	{
	LOG_FUNC

	HCI_LOG_LOADL(this, KHCILoggerDatalinkTypeH4);

	// Create the receive buffer
    HBufC8* buf = HBufC8::NewMaxL(KHCTLRecvBufSize);
    
    // HBufC8 is not modifiable.  Create a RBuf8 object that
    // allows the data to be modified.
    iReceiveBuffer.Assign(buf);
	}

void CHCTLBcmH4Receiver::QueueReadForNextFrame(TUint16 aOffset, TUint16 aBytesRequired)
	{
	LOG_FUNC

	__ASSERT_DEBUG(!IsActive(), PANIC(KBcmHctlH4Panic, EPortReadAttemptWhenReadOutstanding));
	__ASSERT_DEBUG(aBytesRequired != 0, PANIC(KBcmHctlH4Panic, EAttemptToReadZeroBytes));
	__ASSERT_DEBUG(aOffset + aBytesRequired <= iReceiveBuffer.MaxLength(), PANIC(KBcmHctlH4Panic, EHctlReceiverBufferOverflow));
 
    SetActive();

	// Read the required number of bytes into the buffer starting at the specified offset.
	iReceiveBufPtr.Set(iReceiveBuffer.MidTPtr(aOffset, aBytesRequired));
	iPort.Read(iStatus, iReceiveBufPtr, aBytesRequired);
	}

void CHCTLBcmH4Receiver::ProcessData()	
    {
	LOG_FUNC

	TUint16 bytesRequiredForNextRead = 0;
	TUint16 currentReadOffset = 0;
	
	switch (iState)
		{
		case EWaitingForHctlHeaderByte:
			{
			// reads the first byte of the Packet, to decide the 
			// type of Packet and set the next state
			__ASSERT_ALWAYS(iReceiveBufPtr.Length() == KHctlHeaderSize, 
			                PANIC(KBcmHctlH4Panic, EReadCompletedWithInsufficientBytes));

			// Store the HCI packet.
			iCurrentHCIPacketType = iReceiveBufPtr[KHctlPacketTypeOffset];
			iState = EWaitingForHciHeader;
			
			// Request the appropriate number of header bytes.  The HCI packet
			// type will be overwritten when the HCI header bytes are read.
		    switch(iCurrentHCIPacketType)
			    {
				case CHCTLBcmH4::EACLDataPacket:
			    	{
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerACLDataFrame | KHCILoggerFrameFragmented,
								  iReceiveBufPtr.Left(KHctlHeaderSize));
			    	
			    	bytesRequiredForNextRead = CHctlAclDataFrame::KHCIACLDataPacketHeaderLength;
			    	}
			    	break;

				case CHCTLBcmH4::ESynchronousDataPacket:
					{
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerSynchronousDataFrame | KHCILoggerFrameFragmented,
								  iReceiveBufPtr.Left(KHctlHeaderSize));

			    	bytesRequiredForNextRead = CHctlSynchronousDataFrame::KHCISynchronousDataPacketHeaderLength;
					}
					break;

				case CHCTLBcmH4::EEventPacket:
			    	{
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerCommandOrEvent | KHCILoggerFrameFragmented,
								  iReceiveBufPtr.Left(KHctlHeaderSize));

			    	bytesRequiredForNextRead = THCIEventBase::KEventCommonFieldsLength;
			    	}
			    	break;

				default:
					///////////////////////////////////////
					//CHRIS MODIF
					if((iHCTLBcmH4.iInitpluginState == EMiniDrvToHcdConfigFileDelay) || (iHCTLBcmH4.iInitpluginState == EMiniDrvToBinConfigFileDelay)) 
						{
						if((iCurrentHCIPacketType == 0x34) || (iCurrentHCIPacketType == 0x31) )
							{
							bytesRequiredForNextRead = 1;
							iState = EWaitingForHctlHeaderByte;
							break;
							}
						}
					else
						{
					/////////////////////////////////////////
					// unexpected/unsupported data Received
					iState = EInvalidDataReceived;
						}
					break;
			    };
			}
			break;
			
		case EWaitingForHciHeader:
			{
			iState = EWaitingForHciPayload;
			
			// Read the packet length.
			switch(iCurrentHCIPacketType)
				{
				case CHCTLBcmH4::EACLDataPacket:
					{
					__ASSERT_ALWAYS(iReceiveBufPtr.Length() == CHctlAclDataFrame::KHCIACLDataPacketHeaderLength, 
					                PANIC(KBcmHctlH4Panic, EReadCompletedWithInsufficientBytes));

					bytesRequiredForNextRead = LittleEndian::Get16(iReceiveBufPtr.Ptr() + CHctlDataFrameBase::KHCIDataPacketLengthFieldOffset);
					
					// Check that the size of the ACL data does not exceed the internal buffer.
					if(bytesRequiredForNextRead > (KHCTLRecvBufSize - CHctlAclDataFrame::KHCIACLDataPacketHeaderLength))
						{
						// This is unexpected from the controller.  Try restarting it.
						iState = EInvalidDataReceived;
						}
					else
						{
						HCI_LOG_FRAME(this,
									  KHCILoggerControllerToHost | KHCILoggerACLDataFrame | KHCILoggerFrameFragmented,
									  iReceiveBufPtr.Left(CHctlAclDataFrame::KHCIACLDataPacketHeaderLength));
						currentReadOffset = CHctlAclDataFrame::KHCIACLDataPacketHeaderLength;
						}
					}
					break;

				case CHCTLBcmH4::ESynchronousDataPacket: 
					{
					__ASSERT_ALWAYS(iReceiveBufPtr.Length() == CHctlSynchronousDataFrame::KHCISynchronousDataPacketHeaderLength, 
					                PANIC(KBcmHctlH4Panic, EReadCompletedWithInsufficientBytes));

					bytesRequiredForNextRead = iReceiveBufPtr[CHctlDataFrameBase::KHCIDataPacketLengthFieldOffset];
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerSynchronousDataFrame | KHCILoggerFrameFragmented,
								  iReceiveBufPtr.Left(CHctlSynchronousDataFrame::KHCISynchronousDataPacketHeaderLength));
					currentReadOffset = CHctlSynchronousDataFrame::KHCISynchronousDataPacketHeaderLength;
					}
					break;

				case CHCTLBcmH4::EEventPacket:
					{
					__ASSERT_ALWAYS(iReceiveBufPtr.Length() == THCIEventBase::KEventCommonFieldsLength, 
					                PANIC(KBcmHctlH4Panic, EReadCompletedWithInsufficientBytes));

					bytesRequiredForNextRead = iReceiveBufPtr[THCIEventBase::KTotalParameterLengthOffset];
					HCI_LOG_FRAME(this, 
					              KHCILoggerControllerToHost | KHCILoggerCommandOrEvent | KHCILoggerFrameFragmented,
					              iReceiveBufPtr.Left(THCIEventBase::KEventCommonFieldsLength));
					currentReadOffset = THCIEventBase::KEventCommonFieldsLength;
					}
					break;

				default:
					{
					// Invalid state.
					PANIC(KBcmHctlH4Panic, EIllegalState);
					break;
					}
				};
			}
			break;
	
		case EWaitingForHciPayload:
			{
			TUint16 payloadLength = iReceiveBufPtr.Length();
			switch(iCurrentHCIPacketType)
				{
				case CHCTLBcmH4::EACLDataPacket:
					{
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerACLDataFrame, 
								  iReceiveBufPtr);
					iHCTLBcmH4.ProcessACLData(iReceiveBuffer.Left(payloadLength + CHctlAclDataFrame::KHCIACLDataPacketHeaderLength));
					}
					break;

				case CHCTLBcmH4::ESynchronousDataPacket:
					{
					HCI_LOG_FRAME(this, 
							      KHCILoggerControllerToHost | KHCILoggerSynchronousDataFrame, 
								  iReceiveBufPtr);
					iHCTLBcmH4.ProcessSynchronousData(iReceiveBuffer.Left(payloadLength + CHctlSynchronousDataFrame::KHCISynchronousDataPacketHeaderLength));
					}
					break;

				case CHCTLBcmH4::EEventPacket:
					{
					HCI_LOG_FRAME(this, 
								  KHCILoggerControllerToHost | KHCILoggerCommandOrEvent, 
						          iReceiveBufPtr);
					iHCTLBcmH4.ProcessEvent(iReceiveBuffer.Left(payloadLength + THCIEventBase::KEventCommonFieldsLength));
					}
					break;

				default:  
					// Invalid state
					PANIC(KBcmHctlH4Panic, EIllegalState);
				};
				
			// Starting a new Packet. Bytes Required now is 1, offset is 0
			// We finished with this packet so we're back in 'wait for next mode'
			currentReadOffset = 0;
			bytesRequiredForNextRead = KHctlHeaderSize;  
			iState = EWaitingForHctlHeaderByte;  
			}
			break;

		default:  
			// must never get here
			PANIC(KBcmHctlH4Panic, EIllegalState);
			break;
		}

	if(iState == EInvalidDataReceived)
		{
		// The HCTL can not recover from this.  Reset the controller and restart the host.
		iHCTLBcmH4.MhriStartHardReset();
		}
	else
		{
		// Request the next read on the port.
		QueueReadForNextFrame(currentReadOffset, bytesRequiredForNextRead);
		}
	}


void CHCTLBcmH4Receiver::RunL()
    {
	LOG_FUNC
	LOG1(_L8("\tiStatus = %d"), iStatus.Int());

	// Only process the read if it has completed successfully.
    if (iStatus.Int() == KErrNone)
        {
        ProcessData();	
        }
    else
    	{
		// The HCTL can not recover from this.  Reset the controller and restart the host.
		iHCTLBcmH4.MhriStartHardReset();
    	}
	}

void CHCTLBcmH4Receiver::DoCancel()
	{
	LOG_FUNC
	
	// Cancel the outstanding read request.
	iPort.ReadCancel();
	}

void CHCTLBcmH4Receiver::Start()
	{
	LOG_FUNC
	__ASSERT_DEBUG(!IsActive(), PANIC(KBcmHctlH4Panic, EStartCalledWhenReadOutstanding));
	
	// Reset this object state and make an initial read on the UART.
	iState = EWaitingForHctlHeaderByte;

	// Read first byte on next frame (HCI packet type) into offset zero of the buffer.
	QueueReadForNextFrame(0, KHctlHeaderSize);
	}
