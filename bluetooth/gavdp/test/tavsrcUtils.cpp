// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "tavsrcUtils.h"
#include "tavsrcConsole.h"

#include <bluetoothav.h>
#include <btextnotifiers.h>

#include <f32file.h>
#include <e32math.h>

void TTavsrcUtils::RegisterSinkSDPRecordL(RSdpDatabase& aDB, TSdpServRecordHandle& aRecHandle,
										  TBool aHeadphone, TBool aSpeaker, TBool aRecorder,
										  TBool aAmp)
	{
	CSdpAttrValue* attrVal = 0;
	CSdpAttrValueDES* attrValDES = 0;

	// Set Attr 1 (service class list) to list with UUID = Audio Sink
	aDB.CreateServiceRecordL(TUUID(KAudioSinkUUID), aRecHandle);

	// Protocol Descriptor List
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);

	attrValDES->StartListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAPUUID))) // L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(KAVDTP)) // PSM = AVDTP
			->EndListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAvdtpUUID))) // Avdtp UUID
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // Version
			->EndListL()
		->EndListL();
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdProtocolDescriptorList, *attrValDES);
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
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBrowseGroupList, *attrValDES); //attribute 5
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
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdLanguageBaseAttributeIDList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;

	// BT Profile Description
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);

	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(KAdvancedAudioDistributionUUID)
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // version
		->EndListL()
	->EndListL();
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBluetoothProfileDescriptorList, *attrValDES);

	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	
	// provider name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Symbian Software Ltd"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetProviderName, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Advanced audio distribution sink"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service description
	attrVal = CSdpAttrValueString::NewStringL(_L8("kick back and listen to some fine tunes with this neat service"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// supported features
	TUint16 supportedFeatures = aHeadphone ? 1:0;
	supportedFeatures|=aSpeaker ? 2:0;
	supportedFeatures|=aRecorder ? 4:0;
	supportedFeatures|=aAmp ? 8:0;
	attrVal = CSdpAttrValueUint::NewUintL(TSdpIntBuf<TUint16>(supportedFeatures));
	CleanupStack::PushL(attrVal);
	
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdSupportedFeatures, *attrVal);	
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;
	}

void TTavsrcUtils::RegisterSourceSDPRecordL(RSdpDatabase& aDB, TSdpServRecordHandle& aRecHandle,
											TBool aPlayer, TBool aMic, TBool aTuner, TBool aMixer)
	{
	CSdpAttrValue* attrVal = 0;
	CSdpAttrValueDES* attrValDES = 0;

	// Set Attr 1 (service class list) to list with UUID = Audio Sink
	aDB.CreateServiceRecordL(TUUID(KAudioSourceUUID), aRecHandle);

	// Protocol Descriptor List
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);

	attrValDES->StartListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KL2CAPUUID))) // L2CAP
			->BuildUintL(TSdpIntBuf<TUint16>(KAVDTP)) // PSM = AVDTP
			->EndListL()
		->BuildDESL()
			->StartListL()
			->BuildUUIDL(TUUID(TUint16(KAvdtpUUID))) // Avdtp UUID
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // Version
			->EndListL()
		->EndListL();
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdProtocolDescriptorList, *attrValDES);
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
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBrowseGroupList, *attrValDES); //attribute 5
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
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdLanguageBaseAttributeIDList, *attrValDES);
	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;

	// BT Profile Description
	attrValDES = CSdpAttrValueDES::NewDESL(0);
	CleanupStack::PushL(attrValDES);

	attrValDES->StartListL()
		->BuildDESL()->StartListL()
			->BuildUUIDL(KAdvancedAudioDistributionUUID)
			->BuildUintL(TSdpIntBuf<TUint16>(0x0100)) // version
		->EndListL()
	->EndListL(); 
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBluetoothProfileDescriptorList, *attrValDES);

	CleanupStack::PopAndDestroy(attrValDES);
	attrValDES = 0;
	
	// provider name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Symbian Software Ltd"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetProviderName, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service name
	attrVal = CSdpAttrValueString::NewStringL(_L8("Advanced audio distribution source"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceName, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// service description
	attrVal = CSdpAttrValueString::NewStringL(_L8("plug your wireless cans into me!"));
	CleanupStack::PushL(attrVal);
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdBasePrimaryLanguage + KSdpAttrIdOffsetServiceDescription, *attrVal);
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;

	// supported features									
	TUint16 supportedFeatures = aPlayer ? 1:0;
	supportedFeatures|=aMic ? 2:0;
	supportedFeatures|=aTuner ? 4:0;
	supportedFeatures|=aMixer ? 8:0;
	attrVal = CSdpAttrValueUint::NewUintL(TSdpIntBuf<TUint16>(supportedFeatures));
	CleanupStack::PushL(attrVal);
	
	aDB.UpdateAttributeL(aRecHandle, KSdpAttrIdSupportedFeatures, *attrVal);	
	CleanupStack::PopAndDestroy(attrVal);
	attrVal = 0;
	}

TInt TTavsrcUtils::GetIntFromUser(CConsoleBase& aConsole)
	{
	TBuf<4> inpb;
	aConsole.Printf(_L(":"));
	TInt x = aConsole.WhereX();
	TInt y = aConsole.WhereY();
	TRequestStatus stat;

	TChar ch(0); // set it to anything to prevent warning
	while ((ch!='\n')&&(ch!='\r') && inpb.Length()==0)
		{
		aConsole.Read(stat);
		User::WaitForRequest(stat);
		ch = aConsole.KeyCode();
		
		if ((ch=='\b') && (inpb.Length()>0))
			{
			inpb.Delete(inpb.Length()-1,1);
			}
			
		if ((inpb.Length()<4)&&((ch>='0')&&(ch<='9')))
			{
			inpb.Append(ch);
			}
			
		aConsole.SetPos(x,y);
		aConsole.Printf(_L("%S"),&inpb);
		}

	TLex lex(inpb);
	TInt res;
	return (lex.Val(res)==KErrNone) ? res : 0;
	}
	
TBool TTavsrcUtils::GetYNFromUser(CConsoleBase& aConsole, const TDesC& aDes)
	{
	TBuf<4> inpb;
	aConsole.Printf(_L("%S (y/n):"),&aDes);
	TRequestStatus stat;

	TChar ch(0); // set it to anything to prevent warning
	while ((ch!='n')&&(ch!='y'))
		{
		aConsole.Read(stat);
		User::WaitForRequest(stat);
		ch = aConsole.KeyCode();
		}
	aConsole.Printf(_L("%c"), static_cast<TUint>(ch));
	return (ch=='y') ? ETrue : EFalse;
	}

void TTavsrcUtils::GetDeviceAddressL(TBTDevAddr& aAddr)
	{
	//Ask user which device address we should connect to...
	RNotifier notify;
	User::LeaveIfError(notify.Connect());
	TBTDeviceSelectionParamsPckg pckg;
	TBTDeviceResponseParamsPckg resPckg;
	TRequestStatus stat;
	notify.StartNotifierAndGetResponse(stat, KDeviceSelectionNotifierUid, pckg, resPckg);
	User::WaitForRequest(stat);
	notify.CancelNotifier(KDeviceSelectionNotifierUid);
	notify.Close();
	User::LeaveIfError(stat.Int()); 

	aAddr = resPckg().BDAddr();
	}

TInt TTavsrcUtils::GetCodecSettingsFromSBCFile(RFile& aFile, TInt aPos, TInt& aChannelMode,
							TInt& aNumChannels, TInt& aNumSubbands, TInt& aBlkLen,
							TInt& aBitPool, TInt& aFreq, TInt& aAllocMethod)
	{
	TInt err = KErrNone;
	
	TBuf8<4> header;
	aFile.Read(aPos, header);
	TInt syncWord = header[0];
	__ASSERT_ALWAYS(syncWord==0x9C, User::Invariant());
	
	TInt sampleFreq = (header[1]&0xC0)>>6;
	TInt blockLen = (header[1]&0x30)>>4;
	aChannelMode = (header[1]&0x0C)>>2;
	aAllocMethod = (header[1]&0x02)>>1;
	TInt subBands = (header[1]&0x01);
	aBitPool = header[2];
	
	aNumChannels = (aChannelMode==0) ? 1 : 2;
	aNumSubbands = (subBands==1) ? 8 : 4;
	
	switch (sampleFreq)
		{
		case 0x00:
			aFreq = 16000;
			break; 
		case 0x01:
			aFreq = 32000;
			break; 
		case 0x02:
			aFreq = 44100;
			break; 
		case 0x03:
			aFreq = 48000;
			break; 
		}
		
	switch (blockLen)
		{
		case 0x00:
			aBlkLen = 4;
			break; 
		case 0x01:
			aBlkLen = 8;
			break; 
		case 0x02:
			aBlkLen = 12;
			break; 
		case 0x03:
			aBlkLen = 16;
			break; 
		}
		
	return err;
	}

TInt TTavsrcUtils::GetCodecSettingsFromSBCFile(const TDesC& aFileName, TInt& aChannelMode,
							TInt& aNumChannels, TInt& aNumSubbands, TInt& aBlkLen,
							TInt& aBitPool, TInt& aFreq, TInt& aAllocMethod)
	{
	TInt err = KErrNone;
	
	RFs fileserver;
	RFile file;
	
	err = fileserver.Connect();
	if (!err) 
		{
		err = file.Open(fileserver,aFileName,EFileRead | EFileShareReadersOnly);			
		}
	if (err)
		{
		return err;			
		}
	
	err = GetCodecSettingsFromSBCFile(file, 0, aChannelMode, aNumChannels, aNumSubbands,
										aBlkLen, aBitPool, aFreq, aAllocMethod);	
	file.Close();
	fileserver.Close();
	
	return err;
	}
	
TInt TTavsrcUtils::CEIL(TReal aX)
	{
	TReal frac;
	Math::Frac(frac,aX);
	return (frac < 0.5) ? aX :aX+1;
	}
