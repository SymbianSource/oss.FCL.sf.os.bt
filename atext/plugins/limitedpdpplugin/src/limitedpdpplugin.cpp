/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  Main handler for incoming requests
 *
 */

#include "limitedpdpplugin.h"
#include <centralrepository.h>
#include "limitedpdpplugincopiedkeys.h"
#include "debug.h"

// Internal constants
const TInt KDialupOverrideEnabled = 1;
const TInt KCloseTimeoutInterval = 15000000; // 15 secs
const TInt KCleanUpWaitTimeout = 300000; // 300 ms
#ifdef _DEBUG 
const TInt KUsecToMSecDivider = 1000;
#endif

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CLimitedPdpPlugin* CLimitedPdpPlugin::NewL()
    {
    CLimitedPdpPlugin* self = new ( ELeave ) CLimitedPdpPlugin();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CLimitedPdpPlugin::~CLimitedPdpPlugin()
    {
    TRACE_FUNC_ENTRY

    // Check validity of handles so that Cancel functions can be called
    if ( iProperty.Handle() != NULL )
        {
        // Cancel just in case in spite of the fact that with current
        // architecture cancelling of the operation is not possible
        iProperty.Cancel();
        iProperty.Close();
        }
    if ( iCancelTimer.Handle() != NULL )
        {
        iCancelTimer.Cancel();
        iCancelTimer.Close();
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CLimitedPdpPlugin::CLimitedPdpPlugin
// ---------------------------------------------------------------------------
//
CLimitedPdpPlugin::CLimitedPdpPlugin() :
    CATExtPluginBase()
    {
    // Nothing to do here, resources are initialized in ConstructL
    }

// ---------------------------------------------------------------------------
// CLimitedPdpPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY

    // No need to use cleanup stack here, since these will be closed in
    // destructor anyway.
    User::LeaveIfError( iCancelTimer.CreateLocal() );
    User::LeaveIfError( iProperty.Attach( KCopyOfConnectionMonitorPS,
                                          KCopyOfDialUpConnection ) );
    iFeatureSupported = EFalse;
    iFeatureSupported = CheckFeatureEnablement();

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports connection identifier name to the extension plugin.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ReportConnectionName( const TDesC8& /*aName*/)
    {
    }

// ---------------------------------------------------------------------------
// Reports the support status of an AT command. This is a synchronous API.
// ---------------------------------------------------------------------------
//
TBool CLimitedPdpPlugin::IsCommandSupported( const TDesC8& /*aCmd*/)
    {
    TRACE_FUNC_ENTRY
    // Set the return value according to feature support
    TRACE_INFO(( _L("Returning feature support value %d"),
                    static_cast<TInt>(iFeatureSupported)));
    TRACE_FUNC_EXIT
    return iFeatureSupported;
    }

// ---------------------------------------------------------------------------
// Handles an AT command. Cancelling of the pending request is done by
// HandleCommandCancel(). The implementation in the extension plugin should
// be asynchronous, but in this case we must use blocking behavior. This has
// a couple of drawbacks. First, the ATEXT thread is blocked and possible,
// which is against the requirements, and secondly and more importantly, the
// cancelling of this operation is not supported.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::HandleCommand( const TDesC8& /*aCmd*/,
                                       RBuf8& /*aReply*/,
                                       TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY

    // Double check that we are actually supporting the feature
    if ( iFeatureSupported )
        {
        // Request ConnMon to close existing packet data connections
        TInt retTemp = CloseExistingConnections();
        if ( retTemp == KErrNone )
            {
            retTemp = BlockThreadUntilConnectionsClosed();
            if ( retTemp == KErrNone )
                {
                // Internal connections closed. We need to wait for a while
                // to make sure that lower layer resources are properly 
                // cleaned in order to avoid conflicts in resource
                // allocation. Again thread needs to be blocked, therefore
                // User::After.
                // NOTE: This is definitely a bad workaround and unreliable
                // approach, but there aren't too many options...
                TRACE_INFO(( _L("Block for %d ms for resource cleanup"),
                                ( KCleanUpWaitTimeout / KUsecToMSecDivider )));
                User::After( KCleanUpWaitTimeout );
                TRACE_INFO(( _L("Cleanup wait completed, exit")));
                }
            }
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels a pending HandleCommand request.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Next reply part's length.
// The value must be equal or less than KDefaultCmdBufLength.
// When the reply from this method is zero, ATEXT stops calling
// GetNextPartOfReply().
// ---------------------------------------------------------------------------
//
TInt CLimitedPdpPlugin::NextReplyPartLength()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return KErrNotSupported;
    }

// ---------------------------------------------------------------------------
// Gets the next part of reply initially set by HandleCommandComplete().
// Length of aNextReply must be equal or less than KDefaultCmdBufLength.
// ---------------------------------------------------------------------------
//
TInt CLimitedPdpPlugin::GetNextPartOfReply( RBuf8& /*aNextReply*/)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return KErrNotSupported;
    }

// ---------------------------------------------------------------------------
// Receives unsolicited results. Cancelling of the pending request is done by
// by ReceiveUnsolicitedResultCancel(). The implementation in the extension
// plugin should be asynchronous.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ReceiveUnsolicitedResult()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels a pending ReceiveUnsolicitedResult request.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ReceiveUnsolicitedResultCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports NVRAM status change to the plugins.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ReportNvramStatusChange( const TDesC8& /*aNvram*/)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports about external handle command error condition.
// This is for cases when for example DUN decided the reply contained an
// error condition but the plugin is still handling the command internally.
// Example: "AT+TEST;+TEST2" was given in command line; "AT+TEST" returns
// non-EReplyTypeError condition and "AT+TEST2" returns EReplyTypeError.
// As the plugin(s) returning the non-EReplyTypeError may still have some
// ongoing operation then these plugins are notified about the external
// EReplyTypeError in command line processing. It is to be noted that
// HandleCommandCancel() is not sufficient to stop the processing as the
// command handling has already finished.
// ---------------------------------------------------------------------------
//
void CLimitedPdpPlugin::ReportExternalHandleCommandError()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reads CenRep key to check whether requested functionality is active.
// ---------------------------------------------------------------------------
//
TBool CLimitedPdpPlugin::CheckFeatureEnablement()
    {
    TRACE_FUNC_ENTRY
    TBool enabled( EFalse );
    CRepository* cmRepository = NULL;
    TRAP_IGNORE( cmRepository = CRepository::NewL ( KCopyOfCRUidCmManager ) );
    if ( cmRepository )
        {
        TInt overrideValue = KErrNotFound;
        TInt retTemp = cmRepository->Get( KCopyOfDialUpOverride,
                                          overrideValue );

        if ( ( retTemp == KErrNone ) && ( overrideValue
                == KDialupOverrideEnabled ) )
            {
            enabled = ETrue;
            TRACE_INFO(( _L("Dialup override feature enabled")));
            }
        }
    delete cmRepository;
    TRACE_FUNC_EXIT
    return enabled;
    }

// ---------------------------------------------------------------------------
// Ask ConnMon to close all existing packet data connections
// ---------------------------------------------------------------------------
//
TInt CLimitedPdpPlugin::CloseExistingConnections()
    {
    TRACE_FUNC_ENTRY

    TInt dialupState( EConnMonReady );
    TInt retVal( KErrNone );

    retVal = iProperty.Get( dialupState );
    if ( retVal == KErrNone )
        {
        TRACE_INFO(( _L("Current dialup connection PS key value %d"),
                        dialupState ));
        if ( dialupState != EConnMonDialUpInit )
            {
            TRACE_INFO(( _L("Setting dialup connection PS key to value %d"),
                            EConnMonDialUpInit ));
            retVal = iProperty.Set( EConnMonDialUpInit );
            }
        else
            {
            // Error situation, we should not end up to this function if
            // the connection closing is already in init state.
            retVal = KErrNotReady;
            }
        }
    TRACE_INFO(( _L("Closing existing connections done with status %d"),
                    retVal ));
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// Synchronously block thread until ConnMon indicates that connections are
// closed or operation timeouts.
// ---------------------------------------------------------------------------
//
TInt CLimitedPdpPlugin::BlockThreadUntilConnectionsClosed()
    {
    TRACE_FUNC_ENTRY

    TInt dialupState( EConnMonDialUpInit );
    TInt retVal( KErrNone );

    // Read the dialup value just to check if ConnMon has been able to update
    // it already to reflect correct state.
    retVal = iProperty.Get( dialupState );

    if ( retVal == KErrNone )
        {
        TRACE_INFO(( _L("Dialup connection PS key value before wait: %d"),
                        dialupState ));
        if ( dialupState == EConnMonDialUpInit )
            {
            // Block thread until value changes
            TRequestStatus propertyStatus;
            TRequestStatus timeoutStatus;
            // Set operation to timeout if closing internal contexts fails.
            // If timeout expires before PS key is updated, this plugin
            // will pass the ATD*99# command to modem. Most probably
            // that will fail due to lack of resources. However, if
            // internal connections are not closed within this timeframe
            // there will be problems anyway. So this is only to hasten
            // end user feedback on error situation. 
            iCancelTimer.After( timeoutStatus, KCloseTimeoutInterval );

            // Loop for property subscription just in case the ConnMon does
            // not set correct value in first attempt. The loop exiting is
            // done separately below, but this condition is better than using
            // while ( ETrue )
            while ( dialupState != EConnMonReady )
                {
                iProperty.Subscribe( propertyStatus );

                TRACE_INFO(( _L("Blocking thread to wait connection closing") ));
                User::WaitForRequest( propertyStatus, timeoutStatus );

                // Wait completed, check which condition was valid
                if ( propertyStatus != KRequestPending )
                    {
                    if ( propertyStatus == KErrNone )
                        {
                        // ConnMon changed the value, check if it is valid
                        TInt retVal = iProperty.Get( dialupState );
                        if ( retVal == KErrNone )
                            {
                            if ( dialupState == EConnMonReady )
                                {
                                TRACE_INFO(( _L("Existing connections closed successfully") ));
                                }
                            else
                                {
                                // Otherwise retry
                                TRACE_INFO(( _L("Wrong internal connection state (%d), retry"),
                                                dialupState ));
                                continue;
                                }
                            }
                        }
                    else
                        {
                        retVal = propertyStatus.Int();
                        }
                    // We should exit the loop, either due success of error
                    // Cancel and wait for timeout request to complete
                    TRACE_INFO((_L("Existing connections closed (status: %d), cancel timer"),
                                    retVal ));
                    iCancelTimer.Cancel();
                    // Wait... If the completion of Cancel() is not waited
                    // here, CActiveScheduler will panic due to stray signal
                    // (E32USER-CBase panic code 46)
                    User::WaitForRequest( timeoutStatus );
                    TRACE_INFO(( _L("Timeout cancelled (timeoutStatus: %d), exit wait"),
                                    timeoutStatus.Int() ));
                    break;
                    }
                else if ( timeoutStatus != KRequestPending )
                    {
                    // Timeout or error, exit
                    TRACE_INFO(( _L("Wait for existing connections timeouted (status: %d)"),
                                    timeoutStatus.Int() ));
                    // Cancel PS subscription and wait for its completion
                    iProperty.Cancel();
                    User::WaitForRequest( propertyStatus );
                    retVal = iProperty.Set( EConnMonDialUpClosed );
                    TRACE_INFO(( _L("Subscription cancelled (status: %d) and state reset, exit"),
                                    propertyStatus.Int() ));
                    retVal = KErrTimedOut;
                    break;
                    }
                }
            }
        }
    // All done, exit
    TRACE_INFO(( _L("Exiting from wait with status %d"), retVal ));

    TRACE_FUNC_EXIT
    return retVal;
    }

