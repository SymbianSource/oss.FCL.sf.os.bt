/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/


#if (!defined __T_DATA_SDP_UTIL_H__ )
#define __T_DATA_SDP_UTIL_H__

// User includes
#include "DataWrapperBase.h"

// EPOC includes
#include <btsdp.h>


/**
 * Test Active Notification class
 *
 */
class CT_DataSdpUtil : public CDataWrapperBase
	{
public:
	/**
	* Two phase constructor
	*/
	static CT_DataSdpUtil*	NewL();

	/**
	* Public destructor
	*/
	~CT_DataSdpUtil();

  /**
  * Process a command read from the ini file
  *
  * @param aCommand			The command to process
  * @param aSection			The section in the ini containing data for the command
  * @param aAsyncErrorIndex	Command index for async calls to return errors to
  *
  * @return					ETrue if the command is processed
  *
  * @leave					System wide error
  */
	virtual TBool	DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);

	/**
	* Return a pointer to the object that the data wraps
	*
	* @return	pointer to the object that the data wraps
	*/
	virtual TAny*	GetObject()	{ return NULL; }

	/**
	* Set the object that the data wraps
	*
	* @param	aObject object that the wrapper is testing
	*
	*/
	virtual void	SetObjectL(TAny* aAny);

protected:
	/**
	* Protected constructor. First phase construction
	*/
	CT_DataSdpUtil();

	/**
	* Second phase construction
	*/
	void	ConstructL();

private:
	/**
	* Helper methods
	*/
	void			DestroyData();
	
	inline void		DoCmdPutUint(const TDesC& aSection);
	inline void		DoCmdGetUint(const TDesC& aSection);
	inline void		DoCmdPutUint64(const TDesC& aSection);
	inline void		DoCmdGetUint64(const TDesC& aSection);
	inline void		DoCmdPutUint128(const TDesC& aSection);
	inline void		DoCmdGetUint128(const TDesC& aSection);
	};

#endif /* __T_DATA_SDP_UTIL_H__*/
