// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"
#include <Network/CNetStatics.h>
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

void UCGameInstance::StartMatch()
{
	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		LoadLevelAndListen(GameLevel);
	}
}

void UCGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("### GI Init Entered ###"));
	if (GetWorld()->IsEditorWorld())
		return;

	if (UCNetStatics::IsSessionServer(this))
	{
		CreateSession();
	}
}

void UCGameInstance::CreateSession()
{
	IOnlineSessionPtr SessionPtr = UCNetStatics::GetSessionPtr();
	if (SessionPtr)
	{
		ServerSessionName = UCNetStatics::GetSessionNameStr();
		FString SessionSearchId = UCNetStatics::GetSessionSearchIdStr();
		SessionServerPort = UCNetStatics::GetSessionPort();
		UE_LOG(LogTemp, Warning, TEXT("#### Create Session With Name: %s, ID: %s, Port: %d"), *(ServerSessionName), *(SessionSearchId), SessionServerPort)

		FOnlineSessionSettings OnlineSessionSetting = UCNetStatics::GenerateOnlineSessionSettings(FName(ServerSessionName), SessionSearchId, SessionServerPort);
		if (!SessionPtr->CreateSession(0, FName(ServerSessionName), OnlineSessionSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Sesison Creating Failed Right away!!!!"))
		}
	}
}

void UCGameInstance::LoadLevelAndListen(TSoftObjectPtr<UWorld> Level)
{
	const FName LevelURL = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));

	if (LevelURL != "")
	{
		GetWorld()->ServerTravel(LevelURL.ToString() + "?listen");
	}
}
