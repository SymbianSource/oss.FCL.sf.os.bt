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
// on Mon, 26 Nov 2007 13:49:47 (time stamp)
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef SETEVENTMASKCOMMAND_H
#define SETEVENTMASKCOMMAND_H

#include <bluetooth/hci/command.h>
#include <bluetooth/hci/hcitypes.h> // in case the generated class uses Bluetooth types

/**
This class represents the SetEventMask HCI Command
*/

class CSetEventMaskCommand : public CHCICommandBase
	{
public:
	// Construction
	IMPORT_C static CSetEventMaskCommand* NewL(const TDesC8& aEventMask);
	IMPORT_C static CSetEventMaskCommand* NewL();
	// Destructor
	~CSetEventMaskCommand();

	

	// Assign new values to the parameters of this command
	IMPORT_C void Reset(const TDesC8& aEventMask);

	// Accessor methods for the parameters of the command
	IMPORT_C TPtrC8 EventMask() const;
	
	// Extension function
	virtual TInt Extension_(TUint aExtensionId, TAny*& aInterface, TAny* aData);

private:
	CSetEventMaskCommand(const TDesC8& aEventMask);
	CSetEventMaskCommand();
	
	// From CHCICommandBase - knows how to format the parameters of this specific command
	// into the HCTL frame
	virtual void Format(CHctlCommandFrame& aCommandFrame) const;
  
private:
	TBuf8<8> iEventMask;
	
	};

#endif // SETEVENTMASKCOMMAND_H
