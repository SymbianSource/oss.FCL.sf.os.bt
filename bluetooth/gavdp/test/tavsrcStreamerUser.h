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

#ifndef TAVSRCSTREAMERUSER_H
#define TAVSRCSTREAMERUSER_H

#include <bluetoothav.h>

// used to notify changes to the configuration of the media being streamed
class MActiveStreamerUser
	{
public:
	virtual void MediaCodecConfigurationRequired(TSBCCodecCapabilities& aConfig)=0;
	};

#endif // TAVSRCSTREAMERUSER_H
