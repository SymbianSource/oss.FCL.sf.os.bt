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

#ifndef C_LIMITEDPDPPLUGIN_H
#define C_LIMITEDPDPPLUGIN_H

#include <atextpluginbase.h>
#include <e32base.h>
#include <e32property.h>

/**
 *  Class for selecting handlers for different AT commands
 *
 *  @since TB9.2
 */
NONSHARABLE_CLASS( CLimitedPdpPlugin ) : public CATExtPluginBase
    {

public:

    /**
     * Two-phased constructor.
     * @return Instance of self
     */
    static CLimitedPdpPlugin* NewL();

    /**
     * Destructor.
     */
    virtual ~CLimitedPdpPlugin();

private:

    CLimitedPdpPlugin();

    void ConstructL();

    /**
     * Reports connection identifier name to the extension plugin.
     *
     * @since TB9.2
     * @param aName Connection identifier name
     * @return None
     */
    void ReportConnectionName( const TDesC8& aName );

    /**
     * Reports the support status of an AT command. This is synchronous API.
     *
     * @since TB9.2
     * @param aCmd The AT command. Its format may vary depending on the
     *             specification. 
     *             a character carriage return (<cr>) in the end.
     * @return ETrue if the command is supported; EFalse otherwise.
     */
    TBool IsCommandSupported( const TDesC8& aCmd );

    /**
     * Handles an AT command. Cancelling of the pending request is done by
     * HandleCommandCancel(). The implementation in the extension plugin
     * should be asynchronous. However, in this case, WE MUST IMPLEMENT THE 
     * PLUGIN synchronously and BLOCK ATEXT THREAD WHILE EXISTING CONNECTIONS
     * ARE BEING CLOSED. This is due to the fact that this plugin is an 
     * observer and the modem AT command handler is issued immediately after
     * this command returns. This has a couple of drawbacks:
     * 1. ATEXT thread is blocked, which is against the requirements
     * 2. Cancelling of this operation is not supported.
     * 
     * The extension plugin which accepts this command is responsible to
     * supply result codes and response and to format result codes properly.
     *
     * After an extension plugin has handled or decided to reject given AT
     * command, it must inform ATEXT by HandleCommandCompleted() with proper
     * error code.
     *
     * @since TB9.2
     * @param aCmd The AT command to be handled. 
     * @param aReply When passed in, contains the built in answer filled by
     *               ATEXT if it is not empty; when command handling
     *               completes successfully, contains the result codes and 
     *               responses to this command 
     * @param aReplyNeeded Reply needed if ETrue, no reply otherwise. 
     * @return None
     */
    void
            HandleCommand( const TDesC8& aCmd,
                           RBuf8& aReply,
                           TBool aReplyNeeded );

    /**
     * Cancels a pending HandleCommand request.
     *
     * @since TB9.2
     * @return None
     */
    void HandleCommandCancel();

    /**
     * Next reply part's length.
     * The value must be equal or less than KDefaultCmdBufLength.
     * When the reply from this method is zero, ATEXT stops calling
     * GetNextPartOfReply().
     *
     * @since TB9.2
     * @return Next reply part's length if zero or positive
     */
    TInt NextReplyPartLength();

    /**
     * Gets the next part of reply initially set by HandleCommandComplete().
     * Length of aNextReply must be equal or less than KDefaultCmdBufLength.
     *
     * @since TB9.2
     * @param aNextReply Next reply
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt GetNextPartOfReply( RBuf8& aNextReply );

    /**
     * Receives unsolicited results. Cancelling of pending request is done
     * by ReceiveUnsolicitedResultCancel(). The implementation in extension
     * plugin should be asynchronous.
     *
     * @since TB9.2
     * @return None
     */
    void ReceiveUnsolicitedResult();

    /**
     * Cancels a pending ReceiveUnsolicitedResult request.
     *
     * @since TB9.2
     * @return None
     */
    void ReceiveUnsolicitedResultCancel();

    /**
     * Reports NVRAM status change to the plugins.
     *
     * @since TB9.2
     * @param aNvram New NVRAM status. Each call of this function is a result
     *               of DUN extracting the form notified by
     *               CATExtCommonBase::SendNvramStatusChange(). Each of the
     *               settings from SendNvramStatusChange() is separated to
     *               one call of ReportNvramStatusChange().
     * @return None
     */
    void ReportNvramStatusChange( const TDesC8& aNvram );

    /**
     * Reports about external handle command error condition.
     * This is for cases when for example DUN decided the reply contained an
     * error condition but plugin is still handling the command internally.
     * This solution keeps the pointer to the last AT command handling plugin
     * inside ATEXT and calls this function there to report the error.
     * It is to be noted that HandleCommandCancel() is not sufficient to stop
     * the processing as the command handling has already finished.
     *
     * @since TB9.2
     * @return None
     */
    void ReportExternalHandleCommandError();

    /**
     * Checks whether this feature is on or off. Reads the value from CenRep
     * and stores it to the internal variable.
     *
     * @since TB9.2
     * @return ETrue if enabled, EFalse otherwise
     */
    TBool CheckFeatureEnablement();

    /**
     * Closes existing connections by setting PS key to ConnMon
     *
     * @since TB9.2
     * @return KErrNone if successful, otherwise Symbian error code
     */
    TInt CloseExistingConnections();

    /**
     * Blocks thread until ConnMon indicates that existing packet data
     * connections are closed. Operation is guarded by a timeout.
     *
     * @since TB9.2
     * @return KErrNone if successful, otherwise Symbian error code
     */
    TInt BlockThreadUntilConnectionsClosed();

private:
    // data

    /**
     * Property handle used for communication with Connection monitor
     */
    RProperty iProperty;

    /**
     * Timer handle used for timeouting the ongoinging operation. Used for
     * cancelling the closing of existing PDP contexts in case the operation
     * takes too long.
     */
    RTimer iCancelTimer;

    /**
     * Value indicating whether this feature is enabled
     */
    TBool iFeatureSupported;

    };

#endif  // C_LIMITEDPDPPLUGIN_H
