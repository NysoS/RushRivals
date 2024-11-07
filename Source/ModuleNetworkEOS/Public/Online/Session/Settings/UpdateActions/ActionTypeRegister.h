#pragma once

#include <any>

#include "Online/Session/Settings/UpdateActions/Actions/SessionSettingActionBase.h"
#include <unordered_map>

// Function to register template specializations with string names
using ExecuteFunction = void(*)();

//class ActionTypeRegister
//{
//public:
//    // Function to execute based on string
//    static void executeFromString(const FString& typeName, std::any... args) {
//        auto it = m_executeMap.find(typeName);
//        if (it != m_executeMap.end()) {
//            std::forward<ExecuteFunction>(it->second(), args...);  // Call the appropriate specialized function
//        }
//    }
//
//    template <typename Type>
//    static void addExecuteAction(const FString& typeName)
//    {
//        m_executeMap.emplace(typeName, &OnSettingUpdateAction<Type>::execute);
//    }
//
//private:
//    static std::unordered_map<FString, ExecuteFunction> m_executeMap;
//};