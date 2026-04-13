#ifndef GAME_CLIENT_KEYVALUESCOMPAT_H
#define GAME_CLIENT_KEYVALUESCOMPAT_H

#include "tier1/interface.h"

/**
*	Initializes the KeyValuesSystem accessor.
*	@return Whether initialization succeeded.
*/
bool KV_InitKeyValuesSystem( CreateInterfaceFn* pFactories, int iNumFactories );

#endif //GAME_CLIENT_KEYVALUESCOMPAT_H
