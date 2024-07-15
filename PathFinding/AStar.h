// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStar.generated.h"

#define TOP 0 
#define RIGHT 1
#define BOTTOM 2
#define LEFT 3

class Grid
{
public:
	Grid();
	void BuildGrid(const int32& _Row, const int32& _Column, const float& _Size);
public:
	bool bCanPass = true;

	int32 FCost;
	int32 GCost;
	int32 HCost;
	TPair<int32, int32> ParentIndex;
	bool bCheck = false;
private:
	TArray<FVector> Rect;
	TPair<int32, int32> Index;
	FVector Center;

public:
	FORCEINLINE FVector GetCenter() const { return Center; }
	TPair<int32, int32> GetIndex() const { return Index; }
};

class ECustomHeap
{
public:
	ECustomHeap();
	ECustomHeap(int32 HeapSize);

	void Add(TSharedPtr<Grid> NewTarget);
	TSharedPtr<Grid> Delete();
	void Clear();
protected:
	int32 GetParentIndex(int32 ChildIndex);
	int32 GetLeftChildIndex(int32 ParentIndex);
	int32 GetRightChildIndex(int32 ParentIndex);
	int32 GetHighPriorityChildIndex(int32 ParentIndex);
private:
	TArray<TSharedPtr<Grid>> Container;
	int32 NumOfData = 0;
};

UCLASS()
class PATHFINDING_API AAStar : public AActor
{
	GENERATED_BODY()
	
public:	
	AAStar();
	virtual void Tick(float DeltaTime) override;

	TArray<TSharedPtr<Grid>> GetPath(const FVector& StartLocation, const FVector& EndLocation);

	void AddObstacle(FVector Location);
	FVector GetGridCenter(FVector Location);
protected:
	virtual void BeginPlay() override;

	void BuildAStar();
	void BuildObstacles();
	
	void SetupObstacle(AActor* Obstacle);

	void GetGridRowColumn(const FVector& Location, int32& Row, int32& Column) const;
	void AddOpenList(const int32& _Row, const int32& _Column, ECustomHeap& OpenList/*TArray<TSharedPtr<Grid>>& OpenList*/, const int32& _GCost);
private:
	TArray<TSharedPtr<Grid>> Grids;
	UPROPERTY(EditAnywhere, Category = Custom)
	float Size;
	UPROPERTY(EditAnywhere, Category = Custom)
	int32 Width;
	UPROPERTY(EditAnywhere, Category = Custom)
	int32 Height;
private:
	TSharedPtr<Grid> ParentGrid;
	TSharedPtr<Grid> EndGrid;
private:
	TUniquePtr<ECustomHeap> CustomHeap;
private:
	UStaticMesh* Mesh;
	UMaterial* Material;
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetWidth() const { return Width; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetHeight() const { return Height; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetSize() const { return Size; }
};
