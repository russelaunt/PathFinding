// Fill out your copyright notice in the Description page of Project Settings.


#include "AStar.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

Grid::Grid()
{

}

void Grid::BuildGrid(const int32& _Row, const int32& _Column, const float& _Size)
{
	Index.Key = _Row;
	Index.Value = _Column;
	
	Rect.Emplace(_Size * (_Column + 1), 0.f, 0.f);
	Rect.Emplace(0.f, _Size * (_Row + 1), 0.f);
	Rect.Emplace(_Size * _Column, 0.f, 0.f);
	Rect.Emplace(0.f, _Size * _Row, 0.f);

	Center = (Rect[0] + Rect[1] + Rect[2] + Rect[3]) / 2.f;
}

AAStar::AAStar()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObject(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (MeshObject.Succeeded())
	{
		Mesh = MeshObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialObject(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (MaterialObject.Succeeded())
	{
		Material = MaterialObject.Object;
	}
}

void AAStar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<TSharedPtr<Grid>> AAStar::GetPath(const FVector& StartLocation, const FVector& EndLocation)
{
	TPair<int32, int32> EndPair;
	GetGridRowColumn(EndLocation, EndPair.Key, EndPair.Value);
	TPair<int32, int32> CurrentPair;
	GetGridRowColumn(StartLocation, CurrentPair.Key, CurrentPair.Value);

	int32 CurrentIndex = CurrentPair.Key + CurrentPair.Value * Width;
	int32 EndIndex = EndPair.Key + EndPair.Value * Width;

	if (EndPair.Key < 0 || EndPair.Key >= Width || EndPair.Key < 0 || EndPair.Value >= Height ||
		CurrentPair.Key < 0 || CurrentPair.Key >= Width || CurrentPair.Value < 0 || CurrentPair.Value >= Height ||
		!Grids[EndIndex]->bCanPass)
		return TArray<TSharedPtr<Grid>>();

	for (auto& element : Grids) element->bCheck = false;

	ParentGrid = Grids[CurrentIndex];
	EndGrid = Grids[EndIndex];

	//TArray<TSharedPtr<Grid>> OpenList;
	ECustomHeap OpenList(Width * Height);
	TArray<TSharedPtr<Grid>> ClosedList;

	ParentGrid->FCost = 0;
	ParentGrid->GCost = 0;
	ParentGrid->HCost = 0;
	ParentGrid->ParentIndex = TPair<int32, int32>(-1, -1);
	ParentGrid->bCheck = true;
	ClosedList.Add(ParentGrid);
	
	while (ParentGrid != EndGrid)
	{
		// ¡è
		AddOpenList(CurrentPair.Key, CurrentPair.Value + 1, OpenList, 10);
		// ¡æ
		AddOpenList(CurrentPair.Key + 1, CurrentPair.Value, OpenList, 10);
		// ¡é
		AddOpenList(CurrentPair.Key, CurrentPair.Value - 1, OpenList, 10);
		// ¡ç
		AddOpenList(CurrentPair.Key - 1, CurrentPair.Value, OpenList, 10);
		// ¢Ö
		AddOpenList(CurrentPair.Key + 1, CurrentPair.Value + 1, OpenList, 14);
		// ¢Ù
		AddOpenList(CurrentPair.Key + 1, CurrentPair.Value - 1, OpenList, 14);
		// ¢×
		AddOpenList(CurrentPair.Key - 1, CurrentPair.Value - 1, OpenList, 14);
		// ¢Ø
		AddOpenList(CurrentPair.Key - 1, CurrentPair.Value + 1, OpenList, 14);

		//OpenList.Sort([](const TSharedPtr<Grid>& a, const TSharedPtr<Grid>& b) { return a->FCost > b->FCost;  });
		
		//ParentGrid = OpenList[OpenList.Num() - 1];
		//OpenList.RemoveAt(OpenList.Num() - 1);
		ParentGrid = OpenList.Delete();
		ClosedList.Add(ParentGrid);
		CurrentPair = ParentGrid->GetIndex();
	}

	TArray<TSharedPtr<Grid>> Result;
	TSharedPtr<Grid> ResultGrid = ClosedList[ClosedList.Num() - 1];
	while (ResultGrid->ParentIndex != TPair<int32, int32>(-1, -1))
	{
		Result.Add(ResultGrid);
		ResultGrid = Grids[ResultGrid->ParentIndex.Key + ResultGrid->ParentIndex.Value * Width];
	}

	return Result;
}

void AAStar::BeginPlay()
{
	Super::BeginPlay();
	
	BuildAStar();
	//BuildNeighbor();
	BuildObstacles();
}

void AAStar::BuildAStar()
{
	for (int32 j = 0; j < Height; ++j)
	{
		for (int32 i = 0; i < Width; ++i)
		{
			TSharedPtr<Grid> NewGrid = MakeShared<Grid>();
			NewGrid->BuildGrid(i, j, Size);
			Grids.Add(NewGrid);
		}
	}

	CustomHeap = MakeUnique<ECustomHeap>(Width * Height);
}

void AAStar::BuildObstacles()
{
	TArray<AActor*> Obstacles;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), FName("Obstacle"), Obstacles);
	for (auto& element : Obstacles)
	{
		SetupObstacle(element);
	}
}

void AAStar::AddObstacle(FVector Location)
{
	int32 Row, Column;
	GetGridRowColumn(Location, Row, Column);
	int32 Index = Row + Column * Width;
	if (!Grids[Index]->bCanPass) return;

	FVector ObstacleLocation = GetGridCenter(Location);
	ObstacleLocation.Z = 4.500093;
	FRotator ObstacleRotation = FRotator::ZeroRotator;
	FVector ObstacleScale = FVector(1.f, 1.f, 0.1f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = nullptr;
	SpawnParams.Owner = nullptr;

	AStaticMeshActor* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), ObstacleLocation, ObstacleRotation, SpawnParams);
	if (SpawnedActor)
	{
		SpawnedActor->SetMobility(EComponentMobility::Movable);
		SpawnedActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
		SpawnedActor->GetStaticMeshComponent()->SetMaterial(0, Material);
		SpawnedActor->SetActorScale3D(ObstacleScale);
		SetupObstacle(SpawnedActor);
	}
}

void AAStar::SetupObstacle(AActor* Obstacle)
{
	FVector Scale = Obstacle->GetActorScale() * 100.f * 0.5f;
	FVector Location = Obstacle->GetActorLocation();

	FVector Top = FVector(Location.X, 0.f, 0.f) + FVector(Scale.X, 0.f, 0.f);
	FVector Right = FVector(0.f, Location.Y, 0.f) + FVector(0.f, Scale.Y, 0.f);
	FVector Bottom = FVector(Location.X, 0.f, 0.f) + FVector(-Scale.X, 0.f, 0.f);
	FVector Left = FVector(0.f, Location.Y, 0.f) + FVector(0.f, -Scale.Y, 0.f);

	int32 MinRow, MinColumn;
	GetGridRowColumn(Bottom + Left, MinRow, MinColumn);
	MinRow = MinRow < 0 ? 0 : MinRow;
	MinColumn = MinColumn < 0 ? 0 : MinColumn;

	int32 MaxRow, MaxColumn;
	GetGridRowColumn(Top + Right, MaxRow, MaxColumn);
	MaxRow = MaxRow > Width - 1 ? Width - 1 : MaxRow;
	MaxColumn = MaxColumn > Height - 1 ? Height - 1 : MaxColumn;

	for (int32 j = MinColumn; j <= MaxColumn; ++j)
	{
		for (int32 i = MinRow; i <= MaxRow; ++i)
		{
			int32 Index = j * Width + i;
			FVector Center = Grids[Index]->GetCenter();

			if (Center.X <= Top.X && Center.X >= Bottom.X && Center.Y >= Left.Y && Center.Y <= Right.Y)
			{
				Grids[Index]->bCanPass = false;
			}
		}
	}
}

FVector AAStar::GetGridCenter(FVector Location)
{
	TPair<int32, int32> Pair;
	GetGridRowColumn(Location, Pair.Key, Pair.Value);

	return Grids[Pair.Key + Pair.Value * Width]->GetCenter();
}

void AAStar::GetGridRowColumn(const FVector& Location, int32& Row, int32& Column) const
{
	Row = static_cast<int32>(Location.Y / Size);
	Column = static_cast<int32>(Location.X / Size);
}

void AAStar::AddOpenList(const int32& _Row, const int32& _Column, ECustomHeap& OpenList/*TArray<TSharedPtr<Grid>>& OpenList*/, const int32& _GCost)
{
	if (_Row >= Width || _Row < 0 || _Column >= Height || _Column < 0) return;
	if (!Grids[_Row + _Column * Width]->bCanPass) return;

	int32 Index = _Row + _Column * Width;

	TPair<int32, int32> Distance = TPair<int32, int32>(Grids[Index]->GetIndex().Key - ParentGrid->GetIndex().Key, Grids[Index]->GetIndex().Value - ParentGrid->GetIndex().Value);

	int32 a = (ParentGrid->GetIndex().Key + Distance.Key) + ParentGrid->GetIndex().Value * Width;
	int32 b = ParentGrid->GetIndex().Key + (Distance.Value + ParentGrid->GetIndex().Value) * Width;

	if ((Distance.Key == 0 && Distance.Value == 0) ||
		!Grids[(ParentGrid->GetIndex().Key + Distance.Key) + ParentGrid->GetIndex().Value * Width]->bCanPass ||
		!Grids[ParentGrid->GetIndex().Key + (Distance.Value + ParentGrid->GetIndex().Value) * Width]->bCanPass)
	{
		return;
	}

	if (!Grids[Index]->bCheck)
	{
		TPair<int32, int32> EndIndex = EndGrid->GetIndex();
		TPair<int32, int32> CurrentIndex = Grids[Index]->GetIndex();
		int32 WidthRange = FMath::Abs(EndIndex.Key - CurrentIndex.Key);
		int32 HeightRange = FMath::Abs(EndIndex.Value - CurrentIndex.Value);
		
		Grids[Index]->bCheck = true;
		Grids[Index]->GCost = ParentGrid->GCost + _GCost;
		Grids[Index]->HCost = (WidthRange + HeightRange) * 10;
		Grids[Index]->FCost = Grids[Index]->GCost + Grids[Index]->HCost;
		Grids[Index]->ParentIndex = ParentGrid->GetIndex();

		OpenList.Add(Grids[Index]);
	}
	else
	{
		if (ParentGrid->GCost + _GCost < Grids[Index]->GCost)
		{
			TPair<int32, int32> EndIndex = EndGrid->GetIndex();
			TPair<int32, int32> CurrentIndex = Grids[Index]->GetIndex();
			int32 WidthRange = FMath::Abs(EndIndex.Key - CurrentIndex.Key);
			int32 HeightRange = FMath::Abs(EndIndex.Value - EndIndex.Value);

			Grids[Index]->bCheck = true;
			Grids[Index]->GCost = ParentGrid->GCost + _GCost;
			Grids[Index]->HCost = (WidthRange + HeightRange) * 10;
			Grids[Index]->FCost = Grids[Index]->GCost + Grids[Index]->HCost;
			Grids[Index]->ParentIndex = ParentGrid->GetIndex();
		}
	}
}

ECustomHeap::ECustomHeap()
{
}

ECustomHeap::ECustomHeap(int32 HeapSize)
{
	Container.SetNum(HeapSize + 1);
}

void ECustomHeap::Add(TSharedPtr<Grid> NewTarget)
{
	int32 Index = NumOfData + 1;

	while (Index != 1)
	{
		if (NewTarget->FCost < Container[GetParentIndex(Index)]->FCost)
		{
			Container[Index] = Container[GetParentIndex(Index)];
			Index = GetParentIndex(Index);
		}
		else
		{
			break;
		}
	}

	Container[Index] = NewTarget;
	++NumOfData;
}

TSharedPtr<Grid> ECustomHeap::Delete()
{
	TSharedPtr<Grid> retGrid = Container[1];
	TSharedPtr<Grid> lastGrid = Container[NumOfData];

	int32 ParentIndex = 1;
	int32 ChildIndex = GetHighPriorityChildIndex(ParentIndex);

	while (ChildIndex)
	{
		if (lastGrid->FCost <= Container[ChildIndex]->FCost)
		{
			break;
		}
		Container[ParentIndex] = Container[ChildIndex];
		ParentIndex = ChildIndex;

		ChildIndex = GetHighPriorityChildIndex(ParentIndex);
	}

	Container[ParentIndex] = lastGrid;
	--NumOfData;
	return retGrid;
}

void ECustomHeap::Clear()
{
	NumOfData = 0;
}

int32 ECustomHeap::GetParentIndex(int32 ChildIndex)
{
	return ChildIndex / 2;
}

int32 ECustomHeap::GetLeftChildIndex(int32 ParentIndex)
{
	return ParentIndex * 2;
}

int32 ECustomHeap::GetRightChildIndex(int32 ParentIndex)
{
	return GetLeftChildIndex(ParentIndex) + 1;
}

int32 ECustomHeap::GetHighPriorityChildIndex(int32 ParentIndex)
{
	if (GetLeftChildIndex(ParentIndex) > NumOfData)
	{
		return 0;
	}
	else if (GetLeftChildIndex(ParentIndex) == NumOfData)
	{
		return GetLeftChildIndex(ParentIndex);
	}
	else
	{
		if (Container[GetLeftChildIndex(ParentIndex)]->FCost > Container[GetRightChildIndex(ParentIndex)]->FCost)
		{
			return GetRightChildIndex(ParentIndex);
		}
		else
		{
			return GetLeftChildIndex(ParentIndex);
		}
	}
	return int32();
}
