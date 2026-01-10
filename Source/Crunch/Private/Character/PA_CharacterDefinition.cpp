// Fill out your copyright notice in the Description page of Project Settings.


#include "PA_CharacterDefinition.h"

#include "CCharacter.h"

FPrimaryAssetId UPA_CharacterDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetCharacterDefinitionAssetType(), GetFName());
}

FPrimaryAssetType UPA_CharacterDefinition::GetCharacterDefinitionAssetType()
{
	return FPrimaryAssetType("CharacterDefinition");
}

UTexture2D* UPA_CharacterDefinition::LoadIcon() const
{
	CharacterIcon.LoadSynchronous();
	if (CharacterIcon.IsValid())
		return CharacterIcon.Get();

	return nullptr;
}

TSubclassOf<ACCharacter> UPA_CharacterDefinition::LoadCharacterClass() const
{
	CharacterClass.LoadSynchronous();
	if (CharacterClass.IsValid())
		return CharacterClass.Get();
	
	return TSubclassOf<ACCharacter>();

}

TSubclassOf<UAnimInstance> UPA_CharacterDefinition::LoadDisplayAnimationBP() const
{
	DisplayAnimBP.LoadSynchronous();
	if (DisplayAnimBP.IsValid())
		return DisplayAnimBP.Get();

	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPA_CharacterDefinition::LoadDisplayMesh() const
{
	TSubclassOf<ACCharacter> LoadedCharaterClass = LoadCharacterClass();
	if (!LoadedCharaterClass)
		return nullptr;

	ACharacter* Character = Cast<ACharacter>(LoadedCharaterClass.GetDefaultObject());
	if (!Character)
		return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}