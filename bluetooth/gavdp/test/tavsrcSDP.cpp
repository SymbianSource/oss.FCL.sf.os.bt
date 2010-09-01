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
//

#include "tavsrc.h"

void CAVTestApp::RegisterSinkSDPRecordL(RSdpDatabase& aDB, TBool aHeadphone,
										TBool aSpeaker,TBool aRecorder,TBool aAmp)
	{
	CSdpAttrValue* attrVal = 0;
	CSdpAttrValueDES* attrValDES = 0;

	// Set Attr 1 (service class list) to list with UUID = Audio Sink
	aDB.CreateServiceRecordL(TUUID(KAudioSinkUUID), iSnkHandle); 

	// Protocol Descriptor List
	attrValDES = CSdpAttrValueDES::NewDESL(0); 
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAPUUID)))					// L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(KAVDTP))					// PSM = AVDTP
			->EndListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAvdtpUUID)))					// Avdtp UUID
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100))					// Version
			->EndListL()
		->EndListL(); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdProtocolDescriptorList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	
	//BrowseGroupList
	/*
	This has been added in order to be interoperable with remote devices which only look for the 
	service in the PublicBrowseGroup (the root of the browse hierarchy). This is not a mandatory feature. 
	*/
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	
	attrValDES->StartListL()
		->BuildUUIDL(TUUID(TUint16(KPublicBrowseGroupUUID))) // Public browse group (the root)
	    ->EndListL();
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdBrowseGroupList, *attrValDES); //attribute 5
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = NULL;

	// Language
	attrValDES = CSdpAttrValueDES::NewDESL(0); 
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildUintL(TSdpIntBuf<TUint16>(KLanguageEnglish))
		->BuildUintL(TSdpIntBuf<TUint16>(KSdpAttrIdCharacterEncodingUTF8))
		->BuildUintL(TSdpIntBuf<TUint16>(KSdpAttrIdBasePrimaryLanguage))
	->EndListL(); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdLanguageBaseAttributeIDList, *attrValDES); 
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;

	// BT Profile Description
	attrValDES = CSdpAttrValueDES::NewDESL(0); 
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(KAdvancedAudioDistributionUUID)
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100))					// version
		->EndListL()
	->EndListL(); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdBluetoothProfileDescriptorList, *attrValDES); 

	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	

	// provider name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Symbian Software Ltd")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetProviderName, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Advanced audio distribution sink")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service description
	attrVal = CSdpAttrValueString::NewStringL(_L8("kick back and listen to some fine tunes with this neat service")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// supported features
	TUint16 supportedFeatures = aHeadphone ? 1:0;
	supportedFeatures|=aSpeaker ? 2:0;
	supportedFeatures|=aRecorder ? 4:0;
	supportedFeatures|=aAmp ? 8:0;
	attrVal = CSdpAttrValueUint::NewUintL(TSdpIntBuf<TUint16>(supportedFeatures)); 
	CleanupStack::PushL(attrVal); 
	
	aDB.UpdateAttributeL(iSnkHandle, KSdpAttrIdSupportedFeatures, *attrVal); 	
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;
	}


void CAVTestApp::RegisterSourceSDPRecordL(RSdpDatabase& aDB, TBool aPlayer,
										TBool aMic,TBool aTuner,TBool aMixer)
	{
	CSdpAttrValue* attrVal = 0;
	CSdpAttrValueDES* attrValDES = 0;

	// Set Attr 1 (service class list) to list with UUID = Audio Sink
	aDB.CreateServiceRecordL(TUUID(KAudioSourceUUID), iSrcHandle); 

	// Protocol Descriptor List
	attrValDES = CSdpAttrValueDES::NewDESL(0); 
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAPUUID)))					// L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(KAVDTP))					// PSM = AVDTP
			->EndListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAvdtpUUID)))					// Avdtp UUID
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100))					// Version
			->EndListL()
		->EndListL(); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdProtocolDescriptorList, *attrValDES); 
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	
	//BrowseGroupList
	/*
	This has been added in order to be interoperable with remote devices which only look for the 
	service in the PublicBrowseGroup (the root of the browse hierarchy). This is not a mandatory feature. 
	*/
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);
	
	attrValDES->StartListL()
		->BuildUUIDL(TUUID(TUint16(KPublicBrowseGroupUUID))) // Public browse group (the root)
	    ->EndListL();
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdBrowseGroupList, *attrValDES); //attribute 5
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = NULL;

	// Language
	attrValDES = CSdpAttrValueDES::NewDESL(0); 
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildUintL(TSdpIntBuf<TUint16>(KLanguageEnglish))
		->BuildUintL(TSdpIntBuf<TUint16>(KSdpAttrIdCharacterEncodingUTF8))
		->BuildUintL(TSdpIntBuf<TUint16>(KSdpAttrIdBasePrimaryLanguage))
	->EndListL(); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdLanguageBaseAttributeIDList, *attrValDES); 
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;

	// BT Profile Description
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES); 

	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(KAdvancedAudioDistributionUUID)
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100))					// version
		->EndListL()
	->EndListL(); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdBluetoothProfileDescriptorList, *attrValDES); 

	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	

	// provider name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Symbian Software Ltd")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetProviderName, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Advanced audio distribution source")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service description
	attrVal = CSdpAttrValueString::NewStringL(_L8("plug your wireless cans into me!")); 
	CleanupStack::PushL(attrVal); 
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription, *attrVal); 
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// supported features
										
	TUint16 supportedFeatures = aPlayer ? 1:0;
	supportedFeatures|=aMic ? 2:0;
	supportedFeatures|=aTuner ? 4:0;
	supportedFeatures|=aMixer ? 8:0;
	attrVal = CSdpAttrValueUint::NewUintL(TSdpIntBuf<TUint16>(supportedFeatures)); 
	CleanupStack::PushL(attrVal); 
	
	aDB.UpdateAttributeL(iSrcHandle, KSdpAttrIdSupportedFeatures, *attrVal); 	
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;
	}

