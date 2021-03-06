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
// This file was generated automatically from the template commandheader.tmpl
// on Wed, 25 Jul 2007 17:00:41 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef WRITEDEFAULTLINKPOLICYSETTINGSCOMMAND_H
#define WRITEDEFAULTLINKPOLICYSETTINGSCOMMAND_H

#include <bluetooth/hci/command.h>
#include <bluetooth/hci/hcitypes.h> // in case the generated class uses Bluetooth types

/**
This class represents the WriteDefaultLinkPolicySettings HCI Command
*/

class CWriteDefaultLinkPolicySettingsCommand : public CHCICommandBase
	{
public:
	// Construction
	IMPORT_C static CWriteDefaultLinkPolicySettingsCommand* NewL(TUint16 aDefaultLinkPolicySettings);
	IMPORT_C static CWriteDefaultLinkPolicySettingsCommand* NewL();
	// Destructor
	~CWriteDefaultLinkPolicySettingsCommand();

	

	// Assign new values to the parameters of this command
	IMPORT_C void Reset(TUint16 aDefaultLinkPolicySettings);

	// Accessor methods for the parameters of the command
	IMPORT_C TUint16 DefaultLinkPolicySettings() const;
	
	// Extension function
	virtual TInt Extension_(TUint aExtensionId, TAny*& aInterface, TAny* aData);

private:
	CWriteDefaultLinkPolicySettingsCommand(TUint16 aDefaultLinkPolicySettings);
	CWriteDefaultLinkPolicySettingsCommand();
	
	// From CHCICommandBase - knows how to format the parameters of this specific command
	// into the HCTL frame
	virtual void Format(CHctlCommandFrame& aCommandFrame) const;
  
private:
	TUint16 iDefaultLinkPolicySettings;
	
	};

#endif // WRITEDEFAULTLINKPOLICYSETTINGSCOMMAND_H
