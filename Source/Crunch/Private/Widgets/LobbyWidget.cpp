// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "AbilityListView.h"
#include "CharacterDisplay.h"
#include "CharacterEntryWidget.h"
#include "PlayerTeamLayoutWidget.h"
#include "TeamSelectionWidget.h"
#include "Character/PA_CharacterDefinition.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Framework/CAssetManager.h"
#include "Framework/CGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Network/CNetStatics.h"
#include "Player/CPlayerState.h"
#include "Player/LobbyPlayerController.h"


void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearAndPopulateTeamSelectionSlots();
	ConfigureGameState();
	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
	if (LobbyPlayerController)
	{
		LobbyPlayerController->OnSwitchToHeroSelection.BindUObject(this, &ULobbyWidget::SwitchToHeroSelection);
	}
	StartHeroSelectionButton->SetIsEnabled(false);
	StartHeroSelectionButton->OnClicked.AddDynamic(this, &ULobbyWidget::StartHeroSelectionButtonClicked);
	StartMatchButton->SetIsEnabled(false);
	StartMatchButton->OnClicked.AddDynamic(this, &ULobbyWidget::StartMatchButtonClicked);
	
	UCAssetManager::Get().LoadCharacterDefinitions(FStreamableDelegate::CreateUObject(this, &ULobbyWidget::CharacterDefinitionLoaded));
	if (CharacterSelectionTileView)
	{
		CharacterSelectionTileView->OnItemSelectionChanged().AddUObject(this, &ULobbyWidget::CharacterSelected);
	}
	SpawnCharacterDisplay();
}

void ULobbyWidget::ClearAndPopulateTeamSelectionSlots()
{
	TeamSelectionSlotGridPanel->ClearChildren();

	for (int i = 0; i < UCNetStatics::GetPlayerCountPerTeam() * 2; ++i)
	{
		UTeamSelectionWidget* NewSelectionSlot = CreateWidget<UTeamSelectionWidget>(this, TeamSelectionWidgetClass);
		if (NewSelectionSlot)
		{
			NewSelectionSlot->SetSlotID(i);
			UUniformGridSlot* NewGridSlot = TeamSelectionSlotGridPanel->AddChildToUniformGrid(NewSelectionSlot);
			if (NewGridSlot)
			{
				int Row = i % UCNetStatics::GetPlayerCountPerTeam();
				int Column = i < UCNetStatics::GetPlayerCountPerTeam() ? 0 : 1;

				NewGridSlot->SetRow(Row);
				NewGridSlot->SetColumn(Column);
			}

			NewSelectionSlot->OnSlotClicked.AddUObject(this, &ULobbyWidget::SlotSelected);
			TeamSelectionSlots.Add(NewSelectionSlot);
		}
	}
}

void ULobbyWidget::SlotSelected(uint8 NewSlotID)
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotID);
	}
}

void ULobbyWidget::ConfigureGameState()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	CGameState = World->GetGameState<ACGameState>();
	if (!CGameState)
	{
		World->GetTimerManager().SetTimer(ConfigureGameStateTimerHandle, this, &ULobbyWidget::ConfigureGameState, 1.f);
	}
	else
	{
		CGameState->OnPlayerSelectionUpdated.AddUObject(this, &ULobbyWidget::UpdatePlayerSelectionDisplay);
		UpdatePlayerSelectionDisplay(CGameState->GetPlayerSelection());
	}
}

void ULobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections)
{
	for (UTeamSelectionWidget* SelectionSlot : TeamSelectionSlots)
	{
		SelectionSlot->UpdateSlotInfo("Empty");
	}
	for (UUserWidget* CharacterEntryAsWidget : CharacterSelectionTileView->GetDisplayedEntryWidgets())
	{
		if (UCharacterEntryWidget* CharacterEntryWidget = Cast<UCharacterEntryWidget>(CharacterEntryAsWidget))
		{
			CharacterEntryWidget->SetSelected(false);
		}
	}
	
	for (const FPlayerSelection& PlayerSelection : PlayerSelections)
	{
		if (!PlayerSelection.IsValid())
			continue;

		TeamSelectionSlots[PlayerSelection.GetPlayerSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickName());
		UCharacterEntryWidget* SelectedEntry = CharacterSelectionTileView->GetEntryWidgetFromItem<UCharacterEntryWidget>(PlayerSelection.GetCharacterDefinition());
		if (SelectedEntry)
		{
			SelectedEntry->SetSelected(true);
		}
		if (PlayerSelection.IsForPlayer(GetOwningPlayerState()))
		{
			UpdateCharacterDisplay(PlayerSelection);
		}
	}
	
	if (CGameState)
	{
		StartHeroSelectionButton->SetIsEnabled(CGameState->CanStartHeroSelection());
		StartMatchButton->SetIsEnabled(CGameState->CanStartMatch());
	}
	if (PlayerTeamLayoutWidget)
	{
		PlayerTeamLayoutWidget->UpdatePlayerSelection(PlayerSelections);
	}
}

void ULobbyWidget::StartHeroSelectionButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_StartHeroSelection();
	}
}

void ULobbyWidget::SwitchToHeroSelection()
{
	MainSwitcher->SetActiveWidget(HeroSelectionRoot);
}

void ULobbyWidget::CharacterDefinitionLoaded()
{
	TArray<UPA_CharacterDefinition*> LoadedCharacterDefinitions;
	if (UCAssetManager::Get().GetLoadedCharacterDefinitions(LoadedCharacterDefinitions))
	{
		CharacterSelectionTileView->SetListItems(LoadedCharacterDefinitions);
	}
}

void ULobbyWidget::CharacterSelected(UObject* SelectedUObject)
{
	if (!CPlayerState)
	{
		CPlayerState = GetOwningPlayerState<ACPlayerState>();
	}

	if (!CPlayerState)
		return;

	if (const UPA_CharacterDefinition* CharacterDefinition = Cast<UPA_CharacterDefinition>(SelectedUObject))
	{
		CPlayerState->Server_SetSelectedCharacterDefinition(CharacterDefinition);
	}
}

void ULobbyWidget::SpawnCharacterDisplay()
{
	if (CharacterDisplay)
		return;

	if (!CharacterDisplayClass)
		return;

	FTransform CharacterDisplayTransform = FTransform::Identity;
	AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
	if (PlayerStart)
	{
		CharacterDisplayTransform = PlayerStart->GetActorTransform();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CharacterDisplay = GetWorld()->SpawnActor<ACharacterDisplay>(CharacterDisplayClass, CharacterDisplayTransform, SpawnParams);
	GetOwningPlayer()->SetViewTarget(CharacterDisplay);
}

void ULobbyWidget::UpdateCharacterDisplay(const FPlayerSelection& PlayerSelection)
{
	if (!PlayerSelection.GetCharacterDefinition())
		return;

	CharacterDisplay->ConfigureWithCharacterDefinition(PlayerSelection.GetCharacterDefinition());
	AbilityListView->ClearListItems();
	const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>* Abilities = PlayerSelection.GetCharacterDefinition()->GetAbilities();
	if (Abilities)
	{
		AbilityListView->ConfigureAbilities(*Abilities);
	}
}

void ULobbyWidget::StartMatchButtonClicked()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->Server_RequestStartMatch();
	}
}
