// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// PhysLinksManHelper.cpp
// 
//

#include <bluetooth/logger.h>
#include "PhysicalLinkHelper.h"
#include "physicallinksmanager.h"
#include "AclDataQController.h"
#include "ProxySAP.h"
#include "linkmgr.h"

#ifdef __FLOG_ACTIVE
_LIT8(KLogComponent, LOG_COMPONENT_LINKMGR);
#endif

CRoleSwitcher::CRoleSwitcher(CPhysicalLinksManager& aLinkMgr, CPhysicalLink& aLink, TBTBasebandRole aRole)
	: CTimer(CActive::EPriorityStandard)
	, iLinkMgr(aLinkMgr)
	, iLink(aLink)
	, iRole(aRole)
	, iIsEncryptionDisabledForRoleSwitch(EFalse)
	{
	LOG_FUNC
	iState = &iLinkMgr.RoleSwitcherStateFactory().GetState(CRoleSwitcherStateFactory::EIdle);
	}

CRoleSwitcher* CRoleSwitcher::NewL(CPhysicalLinksManager& aLinkMgr, CPhysicalLink& aLink, TBTBasebandRole aRole)
	{
	LOG_STATIC_FUNC
	CRoleSwitcher* self = new(ELeave) CRoleSwitcher(aLinkMgr, aLink, aRole);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}
	
void CRoleSwitcher::ConstructL()
	{
	LOG_FUNC
	
	// create Proxy telling it the possible PHY
	iBTProxySAP = CBTProxySAP::NewL(iLinkMgr, &iLink);
	iBTProxySAP->SetNotify(this);
		
	TCallBack cb(EventReceivedCallBack, this);
	iEventReceivedCallBack = new (ELeave)CAsyncCallBack(cb, EActiveHighPriority);

	CTimer::ConstructL();
	CActiveScheduler::Add(this);

	// add ourselves to the list in LinkMgr, LinkMgr will kick off the role change state machine 	
	iLinkMgr.AddRoleSwitcher(*this);
	iAddedToLinkMgr = ETrue;
	}
	
CRoleSwitcher::~CRoleSwitcher()
	{
	LOG_FUNC
	if (iAddedToLinkMgr)
		{
		iLinkMgr.RemoveRoleSwitcher(*this);
		}
		
	Cancel(); // watchdog timer
	delete iBTProxySAP;
	if (iEventReceivedCallBack)
		{
		iEventReceivedCallBack->Cancel();
		delete iEventReceivedCallBack;
		}
	}

void CRoleSwitcher::DisableLPM()
   	{
	LOG_FUNC
	TPckgBuf<TInt> optionBuf;	
   	iBTProxySAP->SAPSetOption(KSolBtLMProxy, EBBRequestPreventAllLowPowerModes, optionBuf);
   	}

void CRoleSwitcher::EnableLPM()
   	{
	LOG_FUNC
	TPckgBuf<TInt> optionBuf;	
   	iBTProxySAP->SAPSetOption(KSolBtLMProxy, EBBRequestAllowAllLowPowerModes, optionBuf);
   	}

void CRoleSwitcher::DisableEncryption()
  	{
	LOG_FUNC
 	// data traffic suspended
	iLinkMgr.LinkManagerProtocol().ACLController().SetParked(iLink.Handle(), ETrue);
   	TBTBasebandEvent event(ENotifyEncryptionChangeOff);
   	iBTProxySAP->Ioctl(KSolBtLMProxy, KLMBasebandEventOneShotNotificationIoctl, &event);
	iLinkMgr.Encrypt(EFalse, iLink);

	// set flag here, it's too late when we receive the event as AccessReqester
	// might receive the baseband notification earlier then the flag is set!	
	iIsEncryptionDisabledForRoleSwitch = ETrue;
  	}

void CRoleSwitcher::EnableEncryption()
  	{
	LOG_FUNC  	
   	TBTBasebandEvent event(ENotifyEncryptionChangeOn);
   	iBTProxySAP->Ioctl(KSolBtLMProxy, KLMBasebandEventOneShotNotificationIoctl, &event);
	iLinkMgr.Encrypt(ETrue, iLink);	
	// data traffic is enabled in IoctlComplete
  	}
   	
void CRoleSwitcher::ChangeRole()
	{
	LOG_FUNC
   	TBTBasebandEvent event(ENotifyAnyRole);
   	iBTProxySAP->Ioctl(KSolBtLMProxy, KLMBasebandEventOneShotNotificationIoctl, &event);
	iLinkMgr.ChangeRole(iRole, iLink);
	}

void CRoleSwitcher::CancelIoctl()
	{
	LOG_FUNC
	iBTProxySAP->CancelIoctl(KSolBtLMProxy, KLMBasebandEventOneShotNotificationIoctl);
	}
	
// Timer
void CRoleSwitcher::RunL()
	{
	LOG_FUNC
	iState->TimerExpired(*this);
	}
	
TInt CRoleSwitcher::RunError(TInt aError)
	{
	LOG_FUNC
	iState->Error(*this, aError);
	return KErrNone;
	}
   	   	
// From MSocketNotify
void CRoleSwitcher::NewData(TUint /*aCount*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::CanSend()
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::ConnectComplete()
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::ConnectComplete(const TDesC8& /*aConnectData*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::ConnectComplete(CServProviderBase& /*aSSP*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::ConnectComplete(CServProviderBase& /*aSSP*/,const TDesC8& /*aConnectData*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::CanClose(TDelete /*aDelete*/)
	{
	LOG_FUNC
	
	}

void CRoleSwitcher::CanClose(const TDesC8& /*aDisconnectData*/,TDelete /*aDelete*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::Error(TInt /*aError*/,TUint /*aOperationMask*/)
	{
	LOG_FUNC
	
	}
	
void CRoleSwitcher::Disconnect(void)
	{
	LOG_FUNC
	iState->Error(*this, KErrDisconnected);
	}

void CRoleSwitcher::Disconnect(TDesC8& /*aDisconnectData*/)
	{
	LOG_FUNC
	iState->Error(*this, KErrDisconnected);
	}

void CRoleSwitcher::Start()
	{
	LOG_FUNC
	iState->Start(*this);
	}

void CRoleSwitcher::Finish()
	{
	LOG_FUNC
	// async call to delete this class
	iLink.AsyncDeleteRoleSwitcher();
	}
		
void CRoleSwitcher::SaveEncryption()
	{
	LOG_FUNC
	iIsEncrypted = iLink.Encrypted();
	}
	
TBool CRoleSwitcher::IsEPRSupported() const
	{
	LOG_FUNC
	// For Lisbon (Bluetooth 2.1), if EPR is supported both locally and remotely,
	// then the controllers will disable/enable encryption automatically for us,
	// so skip some states.
	return iLink.IsEncryptionPauseResumeSupported();
	}

void CRoleSwitcher::LogRoleSwitchSuccessful() const
	{
	LOG_FUNC
	TInt eventType;
	eventType = (iRole == EMaster ? ENotifyMaster :ENotifySlave);
	
	if (iBasebandEvent.EventType()==eventType &&
	    iBasebandEvent.ErrorCode()==KErrNone)
		{
		LOG(_L("CRoleSwitcher RoleSwitch OK"));	
		}
	else 
		{
		LOG(_L("CRoleSwitcher RoleSwitch failed"));
		}
	}

void CRoleSwitcher::IoctlComplete(TDesC8 *aBuf)
	{
	LOG_FUNC
	const TBTBasebandEventNotification* event = reinterpret_cast<const TBTBasebandEventNotification*>(aBuf->Ptr());
	iBasebandEvent = *event;
	iEventReceivedCallBack->CallBack();
	}
	
/*static*/ TInt CRoleSwitcher::EventReceivedCallBack(TAny* aRoleSwitcher)
	{
	LOG_STATIC_FUNC
	CRoleSwitcher* roleSwitcher = static_cast<CRoleSwitcher*>(aRoleSwitcher);
	roleSwitcher->iState->EventReceived(*roleSwitcher);
	return EFalse;
	}
		

//----------------------------------------------------------------------------------
// STATE FACTORY
//----------------------------------------------------------------------------------

CRoleSwitcherStateFactory* CRoleSwitcherStateFactory::NewL()
	{
	LOG_STATIC_FUNC
	CRoleSwitcherStateFactory* ret=new (ELeave) CRoleSwitcherStateFactory();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

void CRoleSwitcherStateFactory::ConstructL()
	{
	LOG_FUNC	
	iStates[EIdle]					=new (ELeave) TRSStateIdle(*this);
	iStates[EDisablingLPM]			=new (ELeave) TRSStateDisablingLPM(*this);
	iStates[EDisablingEncryption]	=new (ELeave) TRSStateDisablingEncryption(*this);
	iStates[EChangingRole]			=new (ELeave) TRSStateChangingRole(*this);
	iStates[EChangingRoleWithEPR]	=new (ELeave) TRSStateChangingRoleWithEPR(*this);
	iStates[EEnablingEncryption]	=new (ELeave) TRSStateEnablingEncryption(*this);
	}

CRoleSwitcherStateFactory::CRoleSwitcherStateFactory()
	{
	LOG_FUNC
	iStates.DeleteAll();
	}

TRoleSwitcherState& CRoleSwitcherStateFactory::GetState(CRoleSwitcherStateFactory::TRoleSwitcherStates aState)
	{
	LOG_FUNC
	__ASSERT_DEBUG(iStates[aState],  Panic(ERoleSwitcherInvalidState));
	return *iStates[aState];
	}

TInt CRoleSwitcherStateFactory::StateIndex(const TRoleSwitcherState* aState) const
	{
	LOG_FUNC
	TInt state;
	for (state = 0; state < ERoleSwitcherMaxState; state++)
		{
		if (iStates[state] == aState)
			{
			return state;
			}
		}
	
	return KUnknownState;
	}


//----------------------------------------------------------------------------------
// STATES
//----------------------------------------------------------------------------------

TRoleSwitcherState::TRoleSwitcherState(CRoleSwitcherStateFactory& aFactory)
: iFactory(aFactory)
	{
	LOG_FUNC
	}

void TRoleSwitcherState::PanicInState(TLinkPanic aPanic) const
	{
	LOG_FUNC
	Panic(aPanic, iFactory.StateIndex(this));
	}

void TRoleSwitcherState::ChangeState(CRoleSwitcher& aContext, CRoleSwitcherStateFactory::TRoleSwitcherStates aState) const
	{
	LOG_FUNC
	
	aContext.iState->Exit(aContext);

#ifdef __FLOG_ACTIVE
	TRoleSwitcherState* state=&iFactory.GetState(aState);
	LOG2(_L("RoleSwitcher: State %S -> %S"), &aContext.iState->iName, &state->iName);
#endif //__FLOG_ACTIVE
	aContext.iState=&iFactory.GetState(aState);

	aContext.iState->Enter(aContext);
	}

void TRoleSwitcherState::Enter(CRoleSwitcher& /*aContext*/) const
	{
	LOG_FUNC
	// do nothing
	}

void TRoleSwitcherState::Exit(CRoleSwitcher& /*aContext*/) const
	{
	LOG_FUNC
	// do nothing
	}

void TRoleSwitcherState::Start(CRoleSwitcher& /*aContext*/) const
	{
	LOG_FUNC
	PanicInState(ERoleSwitcherStateMachineInvalidEvent);
	}

void TRoleSwitcherState::Error(CRoleSwitcher& aContext, TInt /*aErr*/) const
	{
	LOG_FUNC
	aContext.CancelIoctl();
	aContext.Cancel();
	ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
	}

void TRoleSwitcherState::EventReceived(CRoleSwitcher& /*aContext*/) const
	{
	LOG_FUNC
	// do nothing
	}

void TRoleSwitcherState::TimerExpired(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
	}

//----------------------------------------------------------------------------------

TRSStateIdle::TRSStateIdle(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateIdle");
	}

void TRSStateIdle::Start(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.After(KTimeoutRoleSwitch);	// watchdog timer
	ChangeState(aContext, CRoleSwitcherStateFactory::EDisablingLPM);
	}	

void TRSStateIdle::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.Finish();
	}	

//----------------------------------------------------------------------------------

TRSStateDisablingLPM::TRSStateDisablingLPM(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateDisablingLPM");
	}

void TRSStateDisablingLPM::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	// DisableLPM even if link is active to prevent possible LPM requests during encryption disabling

	if (aContext.iLink.LinkMode() == EActiveMode)
		{
		aContext.DisableLPM();
		if (aContext.IsEPRSupported())
			{
			ChangeState(aContext, CRoleSwitcherStateFactory::EChangingRoleWithEPR);
			}
		else
			{
			ChangeState(aContext, CRoleSwitcherStateFactory::EDisablingEncryption);
			}
		// don't wait for notification
		}
	else
		{
		TBTBasebandEvent event(ENotifyActiveMode);
		aContext.iBTProxySAP->Ioctl(KSolBtLMProxy, KLMBasebandEventOneShotNotificationIoctl, &event);
		aContext.DisableLPM();
		}
	}

void TRSStateDisablingLPM::EventReceived(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	if (aContext.iBasebandEvent.EventType()==ENotifyActiveMode &&
		aContext.iBasebandEvent.ErrorCode()==KErrNone)
		{
		if (aContext.IsEPRSupported())
			{
			ChangeState(aContext, CRoleSwitcherStateFactory::EChangingRoleWithEPR);
			}
		else
			{
			ChangeState(aContext, CRoleSwitcherStateFactory::EDisablingEncryption);
			}
		}
	else 
		{
		LOG(_L("CRoleSwitcher RoleSwitch failed in DisableLPM"));
		// we can quit SM, don't need to rewind
		ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
		}
	}

//----------------------------------------------------------------------------------
TRSStateDisablingEncryption::TRSStateDisablingEncryption(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateDisablingEncryption");
	}

void TRSStateDisablingEncryption::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.SaveEncryption();
	if (aContext.iIsEncrypted)
		{
		aContext.DisableEncryption();
		}
	else
		{
		ChangeState(aContext, CRoleSwitcherStateFactory::EChangingRole);
		}
	}

void TRSStateDisablingEncryption::EventReceived(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	if (aContext.iBasebandEvent.EventType()==ENotifyEncryptionChangeOff &&
	    aContext.iBasebandEvent.ErrorCode()==KErrNone)
		{
		ChangeState(aContext, CRoleSwitcherStateFactory::EChangingRole);
		}
	else 
		{
		LOG(_L("CRoleSwitcher RoleSwitch failed in DisableEncryption"));
		// before quiting SM , try to enable LPM
		aContext.EnableLPM();
		ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
		}
	}

void TRSStateDisablingEncryption::TimerExpired(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.CancelIoctl();
	ChangeState(aContext, CRoleSwitcherStateFactory::EEnablingEncryption);
	}

//----------------------------------------------------------------------------------
TRSStateChangingRole::TRSStateChangingRole(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateChangingRole");
	}

void TRSStateChangingRole::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.ChangeRole();
	}

void TRSStateChangingRole::EventReceived(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.Cancel();	// cancel watchdog timer

	FTRACE(aContext.LogRoleSwitchSuccessful());
	

	ChangeState(aContext, CRoleSwitcherStateFactory::EEnablingEncryption);
	}

void TRSStateChangingRole::TimerExpired(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.CancelIoctl();
	ChangeState(aContext, CRoleSwitcherStateFactory::EEnablingEncryption);
	}

//----------------------------------------------------------------------------------
TRSStateChangingRoleWithEPR::TRSStateChangingRoleWithEPR(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateChangingRoleWithEPR");
	}

void TRSStateChangingRoleWithEPR::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.ChangeRole();
	}

void TRSStateChangingRoleWithEPR::EventReceived(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.Cancel();	// cancel watchdog timer

	FTRACE(aContext.LogRoleSwitchSuccessful());
		
	ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
	}

void TRSStateChangingRoleWithEPR::TimerExpired(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.CancelIoctl();
	ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
	}

//----------------------------------------------------------------------------------
TRSStateEnablingEncryption::TRSStateEnablingEncryption(CRoleSwitcherStateFactory& aFactory)
: TRoleSwitcherState(aFactory)
	{
	LOG_FUNC
	STATENAME("TRSStateEnablingEncryption");
	}

void TRSStateEnablingEncryption::Enter(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	if (aContext.iIsEncrypted)
		{
		aContext.After(KTimeoutOneCommand);
		aContext.EnableEncryption();
		}
	else
		{
		aContext.EnableLPM();
		ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
		}
	}
	
void TRSStateEnablingEncryption::Exit(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	if (aContext.iIsEncrypted)
		{
		// enable data traffic
		aContext.iLinkMgr.LinkManagerProtocol().ACLController().SetParked(aContext.iLink.Handle(), EFalse);
		}
	}

void TRSStateEnablingEncryption::EventReceived(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	aContext.Cancel(); // watchdog timer
	if (aContext.iBasebandEvent.EventType()==ENotifyEncryptionChangeOn &&
		aContext.iBasebandEvent.ErrorCode()==KErrNone)
		{
		aContext.EnableLPM();		
		ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
		aContext.iIsEncryptionDisabledForRoleSwitch = EFalse;
		}
	else 
		{
		LOG(_L("CRoleSwitcher SetEncryption failed, disconnect link"));
		if (aContext.iLink.Terminate(ERemoteUserEndedConnection) != KErrNone) 
			{
			LOG(_L("CRoleSwitcher OOM"));
			}
		ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
		}
	}

void TRSStateEnablingEncryption::TimerExpired(CRoleSwitcher& aContext) const
	{
	LOG_FUNC
	LOG(_L("CRoleSwitcher Timeout in EncryptionEnable, disconnect"));
	aContext.CancelIoctl();			
	if (aContext.iLink.Terminate(ERemoteUserEndedConnection) != KErrNone)
			{
			LOG(_L("CRoleSwitcher OOM"));
			}
	ChangeState(aContext, CRoleSwitcherStateFactory::EIdle);
	}



