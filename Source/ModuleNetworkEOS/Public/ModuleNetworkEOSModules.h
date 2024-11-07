#pragma once
#include "CoreMinimal.h"
DECLARE_LOG_CATEGORY_EXTERN(ModuleNetworkEOS, Log, All);

#define WARNING_LOG(Format, ...) \
		UE_LOG(ModuleNetworkEOS, Warning, Format, ##__VA_ARGS__)

#define ERROR_LOG(Format, ...) \
		UE_LOG(ModuleNetworkEOS, Error, Format, ##__VA_ARGS__)
		