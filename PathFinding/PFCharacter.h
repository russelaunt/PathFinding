// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PFCharacter.generated.h"

class Grid;
class AAStar;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;

#define STOP_DISTANCE 3.f

UCLASS()
class PATHFINDING_API APFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APFCharacter();
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void ZoomIn();
	void ZoomOut();
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

private:
	UPROPERTY(VisibleAnywhere, Category = Custom)
	UInputMappingContext* IMC;

	UPROPERTY(VisibleAnywhere, Category = Custom)
	UInputAction* MoveAction;

	void MoveButtonPressed(const FInputActionValue& Value);

	void LocalStartMove(const FVector& InDestination);
	UFUNCTION(Server, Reliable)
	void ServerStartMove(const FVector_NetQuantize& InDestination);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartMove(const FVector_NetQuantize& InDestination);
	
	void TickMove(float DeltaTime);

private:
	bool bMove = false;
	AAStar* AStarAlgorithm;
	TArray<TSharedPtr<Grid>> Path;
	FVector FinalDestination;
	FVector Destination;
	FVector MovementDirection;
private:
	UPROPERTY(EditAnywhere)
	float Speed = 300.f;
public:
	FORCEINLINE AAStar* GetAStar() const { return AStarAlgorithm; }
};
