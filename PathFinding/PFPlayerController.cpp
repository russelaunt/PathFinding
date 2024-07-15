// Fill out your copyright notice in the Description page of Project Settings.


#include "PFPlayerController.h"
#include "PFCharacter.h"
#include "AStar.h"

APFPlayerController::APFPlayerController()
{
	bShowMouseCursor = true;
}

void APFPlayerController::Tick(float DeltaTime)
{
	if (IsLocalPlayerController())
	{
		FHitResult PickingHitResult;
		GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, PickingHitResult);
	}

	APFCharacter* PFCharacter = Cast<APFCharacter>(GetPawn());

	if (bMakeObstacle)
	{
		if (PFCharacter)
		{
			AAStar* AStarAlgorithm = PFCharacter->GetAStar();
			if (AStarAlgorithm)
			{
				FHitResult HitResult;
				GetPickingLocation(HitResult);
				if (HitResult.bBlockingHit)
				{
					AStarAlgorithm->AddObstacle(HitResult.ImpactPoint);
				}
			}
		}
	}

	if (MouseWheelUpDown > 0)
	{
		PFCharacter->ZoomOut();
	}
	else if (MouseWheelUpDown < 0)
	{
		PFCharacter->ZoomIn();
	}
}

const bool APFPlayerController::GetPickingLocation(FHitResult& OutHitResult)
{
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, OutHitResult);

	if (OutHitResult.bBlockingHit) return true;
	else return false;
}

void APFPlayerController::AddObstacle()
{
	bMakeObstacle = !bMakeObstacle;
}

void APFPlayerController::SetMouseWheel(float Param)
{
	if (Param > 0.f) MouseWheelUpDown = 1;
	else if (Param < 0.f) MouseWheelUpDown = -1;
	else MouseWheelUpDown = 0;
}

void APFPlayerController::BeginPlay()
{
	
}
