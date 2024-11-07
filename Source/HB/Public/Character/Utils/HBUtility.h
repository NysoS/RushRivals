#pragma once

#include <initializer_list>
#include <variant>

template <typename... types>
struct container
{
	static inline std::initializer_list<const char*> Names = { types::name... };
};

#define GEN_SKIN(x) struct x { static constexpr const char* name = #x; };

GEN_SKIN(SkinTorso)
GEN_SKIN(SkinHat)
GEN_SKIN(SkinHand)
GEN_SKIN(SkinHead)
GEN_SKIN(SkinFeets)
GEN_SKIN(SkinLegs)
GEN_SKIN(SkinBack)

using SkinListNames = container<SkinTorso, SkinHat, SkinHand, SkinHead, SkinFeets, SkinLegs, SkinBack>;

namespace SkinListLib
{
	template <typename key, typename type>
	static void LoadFromPath(const FString& path, TMap<FString, TObjectPtr<type>>& targetMap)
	{
		const FString keyName = key::name;
		const ConstructorHelpers::FObjectFinder<type> CurrentDataTable(*path);
		check(CurrentDataTable.Object);
		targetMap.Add(keyName, CurrentDataTable.Object);
	}

	template <typename type>
	static void SetAllValue(TMap<FString, type>& targetMap, type value)
	{
		for (const char* currentName : SkinListNames::Names)
		{
			targetMap.FindOrAdd(currentName) = value;
		}
	}

	template <typename type>
	static FString SerializeMap(const TMap<FString, type>& targetMap)
	{
		FString targetVal;
		for (const auto& [key, value] : targetMap)
		{
			targetVal.Append( "key:" + key + ",value:" + value.ToString() + "\n");
		}
		return targetVal;
	}

	template <typename type>
	static void UnserializeMap(TMap<FString, type>& target, const FString& info)
	{
		target.Empty();

		TArray<FString> InfoList;

		info.ParseIntoArrayLines(InfoList);
		for (const auto& inf : InfoList)
		{
			FString LeftInf;
			FString RightInf;
			inf.Split(",", &LeftInf, &RightInf);
			LeftInf.RemoveFromStart("key:");
			RightInf.RemoveFromStart("value:");
			target.Add(LeftInf, FName(RightInf));
		}
	}
}
