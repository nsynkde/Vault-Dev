// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VaultStyle.h"
#include "Vault.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FVaultStyle::StyleInstance = NULL;



void FVaultStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVaultStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FVaultStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VaultStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

const FVector2D Icon8x8(8.f, 8.f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon32x32(32.0f, 32.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FVaultStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("VaultStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Vault")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Vault.PluginAction", new IMAGE_BRUSH(TEXT("Vault_Up"), Icon40x40));

	Style->Set("MetaTitleText", FTextBlockStyle()
		.SetFont(DEFAULT_FONT("Fonts/Roboto - Regular", 14.f))
		.SetColorAndOpacity(FLinearColor(FLinearColor::White))
		);

	{

		const FMargin BarPadding = FMargin(0.f, 8);

		const FEditableTextBoxStyle AssetSearchBox = FEditableTextBoxStyle()
				.SetPadding(BarPadding)
				;

		Style->Set("AssetSearchBar", FSearchBoxStyle()
			.SetTextBoxStyle(AssetSearchBox)
			.SetUpArrowImage(IMAGE_BRUSH("UpArrow", Icon8x8))
			.SetDownArrowImage(IMAGE_BRUSH("DownArrow", Icon8x8))
			.SetGlassImage(IMAGE_BRUSH("SearchGlass", Icon16x16))
			.SetClearImage(IMAGE_BRUSH("X", Icon16x16))
			

			);
	}
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
#undef DEFAULT_FONT

void FVaultStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FVaultStyle::Get()
{
	return *StyleInstance;
}
