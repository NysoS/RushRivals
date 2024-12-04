#pragma once

#include "CoreMinimal.h"
#include "HitboxECustomMovement.h"

#include <functional>
#include <variant>


template<typename... Types>
struct Overload : public Types...
{
	using Types::operator()...;
};

template<typename... Types>
Overload(Types...) -> Overload<Types...>;

#define CREATE_MOVEMENT(x) \
class x : public DefaultMovement<x> \
{ \
	using Super = typename DefaultMovement<x>; \
public: \
	explicit x(UHitboxMovement* owner = nullptr); \
	virtual void Update(double Deltatime) override; \
	ECustomMovementMode CurrentMovement() const { return CMOVE_##x; }; \
};

class AActor;
class UHitboxMovement;

template <typename Type>
class DefaultMovement
{
public:

	DefaultMovement() = delete;

	explicit DefaultMovement(UHitboxMovement* owner = nullptr)
		: m_Owner(owner)
	{
		if(IsServer())
			SetCustomMovement(CurrentMovement());
	}

	virtual ~DefaultMovement() = default;

	virtual void Update(double Deltatime)
	{
		
	}

	virtual void Cancel() {};

	ECustomMovementMode CurrentMovement()
	{
		if (Type* tmp = static_cast<Type*>(this))
		{
			return tmp->CurrentMovement();
		}
		return CMOVE_None;
	}

	bool IsServer() const
	{
		if (m_Owner && m_Owner->HasBegunPlay())
		{
			return m_Owner->GetOwner()->HasAuthority();
		}
		return false;
	}

	void SetCustomMovement(ECustomMovementMode MovementMode)
	{
		m_Owner->SetMovementMode(MOVE_Custom, MovementMode);
	}

	virtual void ResetMovement()
	{
		m_Owner->ResetMovementMode();
	}

protected:

	UWorld* GetWorld() const { return m_Owner->GetWorld(); }

	AActor* GetOwner() const { return m_Owner->GetOwner(); }

	UHitboxMovement* m_Owner;
};

//CREATE_MOVEMENT(Quickfall)
class Quickfall : public DefaultMovement<Quickfall>
{
using Super = typename DefaultMovement<Quickfall>;
public:
	explicit Quickfall(UHitboxMovement* owner = nullptr);
	virtual void Update(double Deltatime) override;
	ECustomMovementMode CurrentMovement() const { return CMOVE_Quickfall; }; 
};

class Slide : public DefaultMovement<Slide>
{
	using Super = typename DefaultMovement<Slide>;

public:
	explicit Slide(UHitboxMovement* owner = nullptr);
	virtual void Update(double Deltatime) override;

	ECustomMovementMode CurrentMovement() const { return CMOVE_Slide; };

	virtual void ResetMovement() override;

private:

	bool UpdateSlideTime(double Deltatime);

private:

	FVector SlideDirection;

	FRotator SlideRotation;

	double CurrentSlideTime;

	FTimerHandle SlideTimerHandle;
};

class WallRunHorizontal : public DefaultMovement<WallRunHorizontal>
{
	using Super = typename DefaultMovement<WallRunHorizontal>;

public: 
	explicit WallRunHorizontal(UHitboxMovement* owner = nullptr);
	virtual void Update(double Deltatime) override;
	ECustomMovementMode CurrentMovement() const { return CMOVE_WallRunHorizontal; };

	void ProcessDirection(const FVector& A, const FVector& B);

private:

	FVector WallDirection;

	FVector Direction;

};

class WallRunVertical : public DefaultMovement<WallRunVertical>
{
	using Super = typename DefaultMovement<WallRunVertical>;
public:
	explicit WallRunVertical(UHitboxMovement* owner = nullptr);
	virtual void Update(double Deltatime) override;
	ECustomMovementMode CurrentMovement() const { return CMOVE_WallRunVertical; };

	void JumpOfWall();

	FVector WallDirection;
};

class Vault : public DefaultMovement<Vault>
{
	using Super = typename DefaultMovement<Vault>;
public:
	explicit Vault(UHitboxMovement* owner = nullptr);
	virtual ~Vault() override;
	virtual void Update(double Deltatime) override;

	bool CalcVault();
	ECustomMovementMode CurrentMovement() const { return CMOVE_Vault; };

private:

	FVector TargetDir;

	FVector TargetLocation;
};

using FCustomMovement = std::variant<Quickfall, Slide, WallRunHorizontal, WallRunVertical, Vault>;

#define SWITCH_ON_MOVE(x) \
std::visit(Overload{ \
[&](typename Quickfall) {}, \
[&](typename Slide) {}, \
[&](typename WallRunHorizontal) {}, \
[&](typename WallRunVertical) {}, \
[&](typename Vault) {} \
}, x);

