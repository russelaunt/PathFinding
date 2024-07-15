// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PFPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PATHFINDING_API APFPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APFPlayerController();
	virtual void Tick(float DeltaTime) override;

	const bool GetPickingLocation(FHitResult& OutHitResult);
	UFUNCTION(BlueprintCallable)
	void AddObstacle();
	UFUNCTION(BlueprintCallable)
	void SetMouseWheel(float Param);
protected:
	virtual void BeginPlay() override;

private:
	bool bMakeObstacle = false;
	int32 MouseWheelUpDown = 0;
};
