// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FVaultStyle
{
public:

	static void Initialize();

	static void Shutdown();

	static bool CacheThumbnailsLocally();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:

	static TSharedRef< class FSlateStyleSet > Create();

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};