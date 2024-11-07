#pragma once

#define CREATE_ACTION(ActionType) \
	template <> \
	struct OnSettingUpdateAction<ActionType> {
	
#define END_ACTION() };

struct ActionTypeBase
{};

#define CREATE_ACTION_TYPE(ActionType) \
	struct ActionType : public ActionTypeBase { \
		static FString GetClassName(){ \
			return #ActionType; \
		} \
	}; \

#define REGISTER_ACTION_TYPE(ActionType) \
	ActionTypeRegister::addExecuteAction<ActionType>(#ActionType);

#define EXECUTE_ACTION_TYPE(ActionType, ...) \
	ActionTypeRegister::executeFromString(ActionType, __VA_ARGS__);

CREATE_ACTION_TYPE(SearchMatchmakingAction)
CREATE_ACTION_TYPE(JoinMatchmakingAction)
CREATE_ACTION_TYPE(TravelMatchmakingAction)
