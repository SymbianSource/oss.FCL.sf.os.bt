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


#if (!defined __T_AVDTP_CONTENTPROTECT_CAPABILITIES_DATA_H__)
#define __T_AVDTP_CONTENTPROTECT_CAPABILITIES_DATA_H__

class CT_AvdtpServiceCapabilityData;
class TAvdtpContentProtectionCapabilities;

/**
 * Test Representing the Content Protection Capability of AVDTP
 *
 */
class CT_AvdtpContentProtectionCapabilitiesData :
	public CT_AvdtpServiceCapabilityData
	{
public:
	/**
	 * Two phase constructor
	 */
	static CT_AvdtpContentProtectionCapabilitiesData* NewL();

	/**
	 * Public destructor
	 */
	~CT_AvdtpContentProtectionCapabilitiesData();

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
	virtual TBool DoCommandL(const TTEFFunction& aCommand,
			const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);

	/**
	 * Return a pointer to the object that the data wraps
	 *
	 * @return	pointer to the object that the data wraps
	 */
	virtual TAny* GetObject()
		{
		return iAvdtpContentProtectionCapabilities;
		}

	/**
	 * Set the object that the data wraps
	 *
	 * @param	aObject object that the wrapper is testing
	 *
	 */
	virtual void SetObjectL(TAny* aAny);

	/**
	 * The object will no longer be owned by this
	 *
	 * @leave	KErrNotSupported if the the function is not supported
	 */
	virtual void DisownObjectL();

protected:
	/**
	 * Protected constructor. First phase construction
	 */
	CT_AvdtpContentProtectionCapabilitiesData();

	virtual TAvdtpServiceCapability* GetAvdtpServiceCapability() const;

	/**
	 * Second phase construction
	 */
	void ConstructL();

private:
	/**
	 * Helper methods
	 */
	void DestroyData();

	inline void DoCmdnewL();
	inline void DoCmdContentProtectionType(const TDesC& aSection);
	inline void DoCmdSetContentProtectionType(const TDesC& aSection);
	inline void DoCmdContentProtectionData(const TDesC& aSection);
	inline void DoCmdSetContentProtectionData(const TDesC& aSection);

private:
	TAvdtpContentProtectionCapabilities * iAvdtpContentProtectionCapabilities;
	};

#endif /* __T_AVDTP_CONTENTPROTECT_CAPABILITIES_DATA_H__ */
