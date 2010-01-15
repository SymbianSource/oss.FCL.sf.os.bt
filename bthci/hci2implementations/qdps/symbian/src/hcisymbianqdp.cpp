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

#include "hcisymbianqdp.h"
#include "hcieventmodifiable.h"
#include <bluetooth/hcicommandqitem.h>
#include <bluetooth/hci/hciopcodes.h>
#include <bluetooth/hci/hciconsts.h>
#include <bluetooth/hci/command.h>
#include <bluetooth/hci/event.h>
#include <bluetooth/hci/commandcompleteevent.h>
#include <bluetooth/hci/disconnectioncompleteevent.h>
#include <bluetooth/hci/readclockoffsetevent.h>
#include <bluetooth/hci/authenticationcompleteevent.h>
#include <bluetooth/hci/readlocalversioninfocompleteevent.h>
#include <bluetooth/logger.h>

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_QDP_SYMBIAN);
#endif

/*static*/ CHCISymbianQdp* CHCISymbianQdp::NewL()
	{
	LOG_STATIC_FUNC
	
	CHCISymbianQdp* self = new (ELeave) CHCISymbianQdp();
	return self;
	}

// Private constructor.
CHCISymbianQdp::CHCISymbianQdp()
	{
	LOG_FUNC
	}

TAny* CHCISymbianQdp::Interface(TUid aUid)
	{
	TAny* ret = NULL;
	
	switch(aUid.iUid)
		{
		case KHCICmdQueueDecisionInterfaceUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHCICmdQueueDecisionInterface*>(this));
			break;
		case KHCICmdQueueDecisionEventModifierInterfaceUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHCICmdQueueEventModifierInterface*>(this));
			break;
		case KHCICmdQueueUtilityUserUid:
			ret = reinterpret_cast<TAny*>(static_cast<MHCICmdQueueUtilityUser*>(this));
			break;
		default:
			break;
		};

	return ret;
	}

// MHCICmdQueueDecisionInterface
TBool CHCISymbianQdp::MhcqdiDoesCommandRequireWorkaround(const CHCICommandQItem& /* aParent */)
	{
	LOG_FUNC
	
	// No Workarounds required.
	return EFalse;
	}
	
CHCICommandQItem* CHCISymbianQdp::MhcqdiGetPreChildCommand(const CHCICommandQItem& /* aParent */, 
														   const CHCICommandQItem* /* aPreviousWorkaroundCmd */,
														   const THCIEventBase* /*aPreviousCmdResult*/)
	{
	LOG_FUNC
	
	// No Workarounds required (see MhcqdiDoesCommandRequireWorkaround), should never be called.
	return NULL;
	}

CHCICommandQItem* CHCISymbianQdp::MhcqdiGetPostChildCommand(const CHCICommandQItem& /* aParent */, 
															const CHCICommandQItem* /* aPreviousPostChild */, 
															const THCIEventBase* /*aPreviousCmdResult*/)
	{
	LOG_FUNC
	
	// No Workarounds required (see MhcqdiDoesCommandRequireWorkaround), should never be called.
	return NULL;
	}
	
THCIEventBase* CHCISymbianQdp::MhcqdiGetFakedUnsolicitedEvent(const CHCICommandQItem& /*aParent*/,
															  const THCIEventBase* /*aPreviousFakedEvent*/)
	{
	LOG_FUNC
	
	// No Workarounds required (see MhcqdiDoesCommandRequireWorkaround), should never be called.
	return NULL;
	}
	
void CHCISymbianQdp::MhcqdiCommandAboutToBeDeleted(const CHCICommandQItem& /*aDyingCmd*/)
	{
	LOG_FUNC
	
	// Notification function. No need to do anything.
	}
	
TInt CHCISymbianQdp::MhcqdiCanSend(CHCICommandQItem& /*aCommand*/, const TDblQue<const CHCICommandQItem>& aSentCommands)
	{
	LOG_FUNC

   if (!aSentCommands.IsEmpty())
   		{
        // Def088959 - The following unhandled commands are blocked to avoid operational errors.
        // Note: This workaround currently resides in this Symbian QDP, but may require further
        // modification or placement depending on target hardware characteristics. This workaround
        // may not be required for all controllers.
        
        THCIOpcode opcode=aSentCommands.Last()->Command().Opcode();
        if (opcode == KHoldModeOpcode ||
        			opcode == KSniffModeOpcode ||
        			opcode == KExitSniffModeOpcode ||
        			opcode == KSwitchRoleOpcode ||
        			opcode == KParkModeOpcode ||
        			opcode == KExitParkModeOpcode)
	        {
        	return EBlock;
	        }
   		}
   //otherwise allow command queue to proceed   
	return EContinue;
	}
	
TUint CHCISymbianQdp::MhcqdiTimeoutRequired(const CHCICommandQItem& /* aCmdAboutToBeSent */)
	{
	LOG_FUNC
	
	// No timeout required.
	return MHCICmdQueueDecisionInterface::KNoTimeoutRequired;
	}
	
void CHCISymbianQdp::MhcqdiMatchedEventReceived(const THCIEventBase& aEvent, const CHCICommandQItem& /*aRelatedCommand*/)
	{
	LOG_FUNC
	
	// Cache the HCI version number of the controller. This allows
	// us to ignore errors from specific versions of controllers
	if (   aEvent.EventCode() == ECommandCompleteEvent
		&& THCICommandCompleteEvent::Cast(aEvent).CommandOpcode() == KReadLocalVersionInfoOpcode)
		{
		const TReadLocalVersionInfoCompleteEvent& readLocalVersionCompleteEvent = TReadLocalVersionInfoCompleteEvent::Cast(aEvent);
		iHCIVersion = readLocalVersionCompleteEvent.Version();
		}
	}

void CHCISymbianQdp::MhcqemiMatchedEventReceived(THCIEventBase& aEvent, const CHCICommandQItem& aRelatedCommand)
	{
	LOG_FUNC
	
#ifdef BROKEN_CASIRA_1_1
	FirmwareFixIgnoreErrorOnSetEventMaskForCasira(aEvent);
#endif // BROKEN_CASIRA_1_1
	
#ifdef BROKEN_BELKIN_2_1
	FirmwareFixFakeCompletionEventsOnDisconnectionForBelkin(aEvent);
#endif // BROKEN_BELKIN_2_1

	MhcqdiMatchedEventReceived(aEvent, aRelatedCommand);
	}


MHCICmdQueueDecisionInterface::TCommandErroredAction CHCISymbianQdp::MhcqdiMatchedErrorEventReceived(const THCIEventBase& /*aErrorEvent*/, 
																									 const CHCICommandQItem& /*aRelatedCommand*/)
	{
	LOG_FUNC
	
	// Never resend.
	return MHCICmdQueueDecisionInterface::EContinueWithError;
	}
	
void CHCISymbianQdp::MhcqdiUnmatchedEventReceived(const THCIEventBase& /*aEvent*/)
	{
	LOG_FUNC
	
	// Notification function. No need to do anything.
	}

void CHCISymbianQdp::FirmwareFixIgnoreErrorOnSetEventMaskForCasira(THCIEventBase& aEvent)
	{
	LOG_FUNC
	// Casiras with 1.1 firmware return an EInvalidHCIParameter error
	// when SetEventMask is called. We still want to call SetEventMask but
	// on this (old/buggy) firmware, ignore the returned EInvalidHCIParameter
	
	if (    aEvent.ErrorCode() == EInvalidHCIParameter
	    &&  aEvent.EventCode() == ECommandCompleteEvent
		&& KSetEventMaskOpcode == THCICommandCompleteEvent::Cast(aEvent).CommandOpcode()
		&&         iHCIVersion == EHWHCIv1_1)
		{
		THCIEventBase& modevent = const_cast<THCIEventBase&>(aEvent);
		THCIEventModifiable& event = reinterpret_cast<THCIEventModifiable&>(modevent);
		event.SetErrorCode(EOK);
		}
	}

void CHCISymbianQdp::FirmwareFixFakeCompletionEventsOnDisconnectionForBelkin(THCIEventBase& aEvent)
	{
	LOG_FUNC
	// For Belkin 2.1 controllers, if we receive a "Disconnection Complete Event"
	// then look for a "Authentication Requested" command or "Read Clock Offset" on
	// the sent queue, and if found, then fake up an completion event (with reason
	// code copied from the Disconnection Complete Event) and inject this into the
	// queue. This is because the Belkin 2.1 controllers fail to send a completion
	// event for "Read Clock Offset" and "Request Authentication" (and maybe others)
	// themselves (i.e. these are firmware bugs we're working around)

	if (aEvent.EventCode() == EDisconnectionCompleteEvent && iHCIVersion == EHWHCIv2_1)
		{
		const TDisconnectionCompleteEvent& disconnEvent = TDisconnectionCompleteEvent::Cast(aEvent);
		THCIConnectionHandle handle = disconnEvent.ConnectionHandle();
		THCIErrorCode reason = static_cast<THCIErrorCode>(disconnEvent.Reason());
		
		if (iProvider->FindOutstandingCommand(KAuthenticationRequestedOpcode) != NULL)
			{
			TBuf8<KHCIMaxEventSize> eventBuf1;
			TAuthenticationCompleteEvent authenticationCompleteEvent(reason, handle, eventBuf1);
			iProvider->InjectEvent(authenticationCompleteEvent);
			}
		
		if (iProvider->FindOutstandingCommand(KReadClockOffsetOpcode) != NULL)
			{
			TBuf8<KHCIMaxEventSize> eventBuf2;
			THCIClockOffset clockOffset = 0;
			TReadClockOffsetEvent readClockOffsetEvent(reason, handle, clockOffset, eventBuf2);
			iProvider->InjectEvent(readClockOffsetEvent);
			}
		}
	}

void CHCISymbianQdp::MhcqemiUnmatchedEventReceived(THCIEventBase& aEvent)
	{
	LOG_FUNC
	
#ifdef BROKEN_BELKIN_2_1
	FirmwareFixFakeCompletionEventsOnDisconnectionForBelkin(aEvent);
#endif // BROKEN_BELKIN_2_1
	
	MhcqdiUnmatchedEventReceived(aEvent);
	}
	
MHCICmdQueueDecisionInterface::TCommandTimedOutAction CHCISymbianQdp::MhcqdiCommandTimedOut(const CHCICommandQItem& /*aCommand*/,
																							const TDblQue<const CHCICommandQItem>& /*aSentCommands*/,
																							TUint /*aCurrentCommandCredits*/,
																							TUint& aCreditsToBeRefunded)
	{
	LOG_FUNC
	
	// No Timeout ever set, should never be called.
	aCreditsToBeRefunded = KHCIDefaultCmdCredits;
	return EContinueWithTimeoutEvent;
	}
	
void CHCISymbianQdp::MhcqdiSetPhysicalLinksState(const MPhysicalLinksState& /*aPhysicalLinkState*/)
	{
	LOG_FUNC
	}
	
void CHCISymbianQdp::MhcqdiSetHardResetInitiator(const MHardResetInitiator& /*aHardResetInitiator*/)
	{
	LOG_FUNC
	}
	
void CHCISymbianQdp::MhcqdiSetHCICommandQueue(MHCICommandQueue& /*aHCICommandQueue*/)
	{
	LOG_FUNC
	}

void CHCISymbianQdp::MhcqdiSetTimeouts(TUint /*aQueueStarvationTimeout*/,
                                       TUint /*aMaxHciCommandTimeout*/)
	{
	LOG_FUNC
	}
	
TUint CHCISymbianQdp::MhcqdiReset()
	{
	LOG_FUNC
	
	// Return the initial number of command credits for the queue.
	return KHCIDefaultCmdCredits;
	}

void CHCISymbianQdp::MhcquuSetUtilitiesProvider(MHCICmdQueueUtilities& aProvider)
	{
	LOG_FUNC
	
	iProvider = &aProvider;
	}
