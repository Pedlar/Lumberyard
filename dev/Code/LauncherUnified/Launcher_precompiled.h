/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once

#include <AzCore/PlatformDef.h>
#include <Launcher_Traits_Platform.h>

#if AZ_TRAIT_LAUNCHER_USE_CRY_NEW_AND_DELETE_FOR_MONOLITHIC_BULDS
    #if defined(AZ_MONOLITHIC_BUILD)
        #define USE_CRY_NEW_AND_DELETE
    #endif // defined(AZ_MONOLITHIC_BUILD)
#endif // AZ_TRAIT_LAUNCHER_USE_CRY_NEW_AND_DELETE_FOR_MONOLITHIC_BULDS

#define _LAUNCHER  // should be defined in launcher, because launcher has to have every symbols in platform_impl.h

#include <platform.h>

#include <ISystem.h>

#if defined(AZ_ENABLE_TRACING) || defined(RELEASE_LOGGING)
    #define ENABLE_LOGGING
#endif // defined(AZ_ENABLE_TRACING) || defined(RELEASE_LOGGING)
