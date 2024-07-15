// Fill out your copyright notice in the Description page of Project Settings.


#include "PFCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PFPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AStar.h"
#include "Kismet/GameplayStatics.h"

APFCharacter::APFCharacter()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->SetIsReplicated(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetupAttachment(GetRootComponent());

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);


	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCObject(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC.IMC'"));
	if (IMCObject.Succeeded()) IMC = IMCObject.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionObject(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Move.IA_Move'"));
	if (MoveActionObject.Succeeded()) MoveAction = MoveActionObject.Object;
}

void APFCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			SubSystem->AddMappingContext(IMC, 0);
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AAStar::StaticClass(), Actors);
	if (Actors.IsEmpty() == false)
	{
		AStarAlgorithm = Cast<AAStar>(Actors[0]);
	}

	int32 a = sizeof(APFCharacter);

	int32 b = 20;
}

void APFCharacter::MoveButtonPressed(const FInputActionValue& Value)
{
	APFPlayerController* PlayerController = Cast<APFPlayerController>(Controller);
	if (PlayerController == nullptr) return;

	FHitResult HitResult;
	if (!PlayerController->GetPickingLocation(HitResult)) return;

	LocalStartMove(HitResult.ImpactPoint);
	ServerStartMove(HitResult.ImpactPoint);
}

void APFCharacter::LocalStartMove(const FVector& InDestination)
{
	FinalDestination = InDestination;
	FinalDestination.Z = 0.f;

	Path = AStarAlgorithm->GetPath(GetActorLocation(), InDestination);

	if (Path.IsEmpty())
	{
		bMove = true;
		Destination = FinalDestination;
		MovementDirection = FinalDestination - GetActorLocation();
		MovementDirection.Z = 0.f;
		MovementDirection.Normalize();
	}
	else
	{
		Destination = Path.Last()->GetCenter();
		Path.RemoveAt(Path.Num() - 1);
		MovementDirection = Destination - GetActorLocation();
		MovementDirection.Z = 0.f;
		MovementDirection.Normalize();
		bMove = true;
	}
}

void APFCharacter::ServerStartMove_Implementation(const FVector_NetQuantize& InDestination)
{
	if (HasAuthority() && IsLocallyControlled()) return;

	FinalDestination = InDestination;
	FinalDestination.Z = 0.f;

	Path = AStarAlgorithm->GetPath(GetActorLocation(), InDestination);

	if (Path.IsEmpty())
	{
		bMove = true;
		Destination = FinalDestination;
		MovementDirection = FinalDestination - GetActorLocation();
		MovementDirection.Z = 0.f;
		MovementDirection.Normalize();
	}
	else
	{
		Destination = Path.Last()->GetCenter();
		Path.RemoveAt(Path.Num() - 1);
		MovementDirection = Destination - GetActorLocation();
		MovementDirection.Z = 0.f;
		MovementDirection.Normalize();
		bMove = true;
	}
}

void APFCharacter::MulticastStartMove_Implementation(const FVector_NetQuantize& InDestination)
{
}

void APFCharacter::TickMove(float DeltaTime)
{
	if (!bMove || !HasAuthority()) return;

	FVector CurrentLocation = GetActorLocation();

	FVector DestToCur = CurrentLocation - Destination;
	DestToCur.Z = 0.f;
	if (MovementDirection.Dot(DestToCur) >= 0.f || DestToCur.Length() < STOP_DISTANCE)
	{
		if (Path.Num() > 0)
		{
			Destination = Path.Last()->GetCenter();
			Path.RemoveAt(Path.Num() - 1);
			MovementDirection = Destination - GetActorLocation();
			MovementDirection.Z = 0.f;
			MovementDirection.Normalize();
		}
		else
		{
			bMove = false;
			return;
		}
	}

	if (Path.IsEmpty())
	{
		Destination = FinalDestination;
		MovementDirection = Destination - GetActorLocation();
		MovementDirection.Z = 0.f;
		MovementDirection.Normalize();
	}

	SetActorLocation(CurrentLocation + MovementDirection * DeltaTime * Speed);

	//AddMovementInput(MovementDirection);
}

void APFCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickMove(DeltaTime);
}

void APFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APFCharacter::MoveButtonPressed);
	}
}

void APFCharacter::ZoomIn()
{
	CameraBoom->TargetArmLength -= 200.f;
}

void APFCharacter::ZoomOut()
{
	CameraBoom->TargetArmLength += 200.f;
}

