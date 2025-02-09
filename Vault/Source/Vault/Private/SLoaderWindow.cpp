// Copyright Daniel Orchard 2020

#include "SLoaderWindow.h"

#include "Vault.h"
#include "VaultSettings.h"
#include "MetadataOps.h"
#include "SAssetPackTile.h"
#include "VaultStyle.h"
#include "AssetPublisher.h"
#include "VaultTypes.h"
#include "VaultCommands.h"

#include "ImageUtils.h"
#include "EditorStyleSet.h"
#include "PakFileUtilities.h"
#include "EditorFramework/AssetImportData.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include <AssetRegistryModule.h>
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "EditorStyleSet.h"
#include "Kismet/KismetStringLibrary.h"

#include "Widgets/Layout/SScalebox.h"

#include "EditorAssetLibrary.h"


#define LOCTEXT_NAMESPACE "SVaultLoader"

const int32 SLoaderWindow::THUMBNAIL_BASE_HEIGHT = 415;
const int32 SLoaderWindow::THUMBNAIL_BASE_WIDTH = 415;
const int32 SLoaderWindow::TILE_BASE_HEIGHT = 465;
const int32 SLoaderWindow::TILE_BASE_WIDTH = 415;

namespace VaultColumnNames
{
	static const FName TagCheckedColumnName(TEXT("Flag"));
	static const FName TagNameColumnName(TEXT("Tag Name"));
	static const FName TagCounterColumnName(TEXT("Used"));
};

class VAULT_API SCategoryFilterRow : public SMultiColumnTableRow<FCategoryFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(SCategoryFilterRow) {}
	SLATE_ARGUMENT(FCategoryFilteringItemPtr, CategoryData)
	SLATE_ARGUMENT(TSharedPtr<SLoaderWindow>, ParentWindow)
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		CategoryData = InArgs._CategoryData;
		ParentWindow = InArgs._ParentWindow;

		SMultiColumnTableRow<FCategoryFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}

	void OnCheckBoxStateChanged(ECheckBoxState NewCheckedState)
	{
		const bool Filter = NewCheckedState == ECheckBoxState::Checked;
		ParentWindow->ModifyActiveCategoryFilters(CategoryData->Category, Filter);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox)
				.IsChecked(false)
				.OnCheckStateChanged(this, &SCategoryFilterRow::OnCheckBoxStateChanged);
		}
		else if (ColumnName == VaultColumnNames::TagNameColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FVaultMetadata::CategoryToString(CategoryData->Category)));
		}
		else if (ColumnName == VaultColumnNames::TagCounterColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(CategoryData->UseCount)));
		}

		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:

	FCategoryFilteringItemPtr CategoryData;
	TSharedPtr<SLoaderWindow> ParentWindow;

};

class VAULT_API STagFilterRow : public SMultiColumnTableRow<FTagFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(STagFilterRow) {}
	SLATE_ARGUMENT(FTagFilteringItemPtr, TagData)
	SLATE_ARGUMENT(TSharedPtr<SLoaderWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		TagData = InArgs._TagData;
		ParentWindow = InArgs._ParentWindow;

		SMultiColumnTableRow<FTagFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}

	void OnCheckBoxStateChanged(ECheckBoxState NewCheckedState)
	{
		const bool Filter = NewCheckedState == ECheckBoxState::Checked;
		ParentWindow->ModifyActiveTagFilters(TagData->Tag, Filter);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox)
				.IsChecked(false)
				.OnCheckStateChanged(this, &STagFilterRow::OnCheckBoxStateChanged);
		}
		else if (ColumnName == VaultColumnNames::TagNameColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(TagData->Tag));
		}
		else if (ColumnName == VaultColumnNames::TagCounterColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(TagData->UseCount)));
		}

		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:

	FTagFilteringItemPtr TagData;
	TSharedPtr<SLoaderWindow> ParentWindow;

};

class VAULT_API SDeveloperFilterRow : public SMultiColumnTableRow<FDeveloperFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(SDeveloperFilterRow) {}

	SLATE_ARGUMENT(FDeveloperFilteringItemPtr, Entry)
	SLATE_ARGUMENT(TSharedPtr<SLoaderWindow>, ParentWindow)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		Entry = InArgs._Entry;
		ParentWindow = InArgs._ParentWindow;
		SMultiColumnTableRow<FDeveloperFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}

	
	void OnCheckBoxStateChanged(ECheckBoxState NewCheckedState)
	{
		const bool Filter = NewCheckedState == ECheckBoxState::Checked;
		ParentWindow->ModifyActiveDevFilters(Entry->Developer, Filter);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox)
				.IsChecked(false)
				.OnCheckStateChanged(this, &SDeveloperFilterRow::OnCheckBoxStateChanged);
		}
		else if (ColumnName == VaultColumnNames::TagNameColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromName(Entry->Developer));
		}
		else if (ColumnName == VaultColumnNames::TagCounterColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(Entry->UseCount)));
		}
		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:
	FDeveloperFilteringItemPtr Entry;
	TSharedPtr<SLoaderWindow> ParentWindow;
};

void SLoaderWindow::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	RefreshAvailableFiles();
	PopulateBaseAssetList();
	PopulateCategoryArray();
	PopulateTagArray();
	PopulateDeveloperNameArray();

	ActiveSortingType = SortingTypes::Filename;
	bSortingReversed = false;

	// TODO: put this in the settings file and load it.
	bHideBadHierarchyAssets = false;

	SortFilteredAssets(ActiveSortingType, bSortingReversed);

	LastSearchTextLength = 0;

	// Bind to our publisher so we can refresh automatically when the user publishes an asset (they wont need to import it, but its a visual feedback for the user to check it appeared in the library
	UAssetPublisher::OnVaultPackagingCompletedDelegate.BindRaw(this, &SLoaderWindow::OnAssetUpdateHappened);

	// Bind to vault module update delegate to automatically refresh when an asset was updated
	FVaultModule::Get().OnAssetWasUpdated.BindRaw(this, &SLoaderWindow::OnAssetUpdateHappened);
	
	// Construct the Holder for the Metadata List
	MetadataWidget = SNew(SVerticalBox);

	// Set the Default Scale for Sliders. 
	TileUserScale = 0.5;

	const float TILE_SCALED_WIDTH = TILE_BASE_WIDTH * TileUserScale;
	const float TILE_SCALED_HEIGHT = TILE_BASE_HEIGHT * TileUserScale;


	// Main Widget
	TSharedRef<SVerticalBox> LoaderRoot = SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			// Primary 3 Boxes go in Here
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
			
				+SSplitter::Slot()
				.Value(0.2f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(4.0f, 4.0f))
					[
						// Left Sidebar!
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.Padding(0,3,0,5)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("VaultLoaderSidebarHeaderLabel", "FILTERING"))
							.TextStyle(FVaultStyle::Get(), "MetaTitleText")
						]

						// Asset List Amount
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Top)
						.Padding(0,0,0,5)
						[
							SNew(STextBlock)
							.Text(this, &SLoaderWindow::DisplayTotalAssetsInLibrary)
						]

						// Category filtering
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SListView<FCategoryFilteringItemPtr>)
							.SelectionMode(ESelectionMode::Single)
							.ListItemsSource(&CategoryCloud)
							.OnGenerateRow(this, &SLoaderWindow::MakeCategoryFilterViewWidget)
							.HeaderRow
							(
								SNew(SHeaderRow)
								+ SHeaderRow::Column(VaultColumnNames::TagCheckedColumnName)
								.DefaultLabel(LOCTEXT("FilteringBoolLabel", "Filter"))
								.FixedWidth(40.0f)

								+ SHeaderRow::Column(VaultColumnNames::TagNameColumnName)
								.DefaultLabel(LOCTEXT("CategoryFilteringTagNameLabel", "Category"))

								+ SHeaderRow::Column(VaultColumnNames::TagCounterColumnName)
								.DefaultLabel(LOCTEXT("FilteringCounterLabel", "Count"))
							)

						]

						// Tag filtering
						+ SVerticalBox::Slot()
						.FillHeight(0.95f)
						[
							SNew(SListView<FTagFilteringItemPtr>)
							.SelectionMode(ESelectionMode::Single)
							.ListItemsSource(&TagCloud)
							.OnGenerateRow(this, &SLoaderWindow::MakeTagFilterViewWidget)
							.HeaderRow
							(
								SNew(SHeaderRow)
								+ SHeaderRow::Column(VaultColumnNames::TagCheckedColumnName)
								.DefaultLabel(LOCTEXT("FilteringBoolLabel", "Filter"))
								.FixedWidth(40.0f)

								+ SHeaderRow::Column(VaultColumnNames::TagNameColumnName)
								.DefaultLabel(LOCTEXT("TagFilteringTagNameLabel", "Tags"))

								+ SHeaderRow::Column(VaultColumnNames::TagCounterColumnName)
								.DefaultLabel(LOCTEXT("FilteringCounterLabel", "Count"))
							)

						]

						// Developer Filtering
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SListView<FDeveloperFilteringItemPtr>)
							.SelectionMode(ESelectionMode::Single)
							.ListItemsSource(&DeveloperCloud)
							.OnGenerateRow(this, &SLoaderWindow::MakeDeveloperFilterViewWidget)
							.HeaderRow
							(
								SNew(SHeaderRow)
								+ SHeaderRow::Column(VaultColumnNames::TagCheckedColumnName)
								.DefaultLabel(LOCTEXT("FilteringBoolLabel", "Filter"))
								.FixedWidth(40.0f)

								+ SHeaderRow::Column(VaultColumnNames::TagNameColumnName)
								.DefaultLabel(LOCTEXT("DevFilteringTagNameLabel", "Developer"))

								+ SHeaderRow::Column(VaultColumnNames::TagCounterColumnName)
								.DefaultLabel(LOCTEXT("FilteringCounterLabel", "Count"))
							)
						]
					] // close SBorder
					] // ~Close Left Splitter Area
							

				// Center Area!
				+SSplitter::Slot()
					.Value(0.6f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(4.0f, 4.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.Padding(FMargin(0,0,0,0))
							[
								// Center content area
								SAssignNew(SearchBox, SSearchBox)
								.HintText(LOCTEXT("SearchBoxHintText", "Search..."))
								.OnTextChanged(this, &SLoaderWindow::OnSearchBoxChanged)
								.OnTextCommitted(this, &SLoaderWindow::OnSearchBoxCommitted)
								.DelayChangeNotificationsWhileTyping(false)
								.Visibility(EVisibility::Visible)
								.IsEnabled_Lambda([this] {
									return IsConnected;
								})
								.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("AssetSearch")))
							]

							+ SHorizontalBox::Slot()
							.Padding(FMargin(15.f,0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SAssignNew(StrictSearchCheckBox, SCheckBox)
								.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
								.Padding(FMargin( 5.f,0.f ))
								.ToolTipText(LOCTEXT("StrictSearchToolTip", "Search only the Pack Names"))
								[
									SNew(SBox)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Center)
									.Padding(FMargin(4.f,2.f))
									[
										SNew(STextBlock)
										.Text(LOCTEXT("StrictSearchCheckBox", "Strict Search"))
									]
								]
							]

							+ SHorizontalBox::Slot()
							.Padding(FMargin(5.f, 0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SNew(SButton)
								//.Text(LOCTEXT("RefreshLibraryScreenBtnLbl", "Refresh Library"))
								.ButtonStyle(FEditorStyle::Get(), "FlatButton")
								.OnClicked(this, &SLoaderWindow::OnRefreshLibraryClicked)
								.Content()
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.Content()
									[
										SNew(SImage)
										.ColorAndOpacity(FLinearColor::White)
										.Image(FVaultStyle::Get().GetBrush("Refresh"))
									]
								]
							]

							+ SHorizontalBox::Slot()
							.Padding(FMargin(5.f, 0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SNew(SComboButton)
								.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
								.ButtonStyle(FEditorStyle::Get(), "FlatButton")
								.ForegroundColor(FLinearColor::White)
								.OnGetMenuContent(this, &SLoaderWindow::OnSortingOptionsMenuOpened)
								.HasDownArrow(false)
								.ButtonContent()
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									.Content()
									[
										SNew(SImage)
										.ColorAndOpacity(FSlateColor::UseForeground())
										.Image(FVaultStyle::Get().GetBrush("Sorting"))
									]
								]
							]
						]

						// Not Connected Message
						+ SVerticalBox::Slot()
						.AutoHeight()
							[
								SNew(SBox)
								.MinDesiredHeight(30.f)
								.MaxDesiredHeight(50.f)
								[
									SNew(SMultiLineEditableTextBox)
									.IsReadOnly(true)
									.AllowMultiLine(true)
									.AlwaysShowScrollbars(false)
									.BackgroundColor(FLinearColor::Black)
									.ForegroundColor(FLinearColor::Red)
									.Text(LOCTEXT("VaultFailedConnection", "Couldn't connect to vault folder!\nCheck if the location set in your local settings file is reachable."))
									.Visibility_Lambda([this]
									{
										if (IsConnected)
										{
											return EVisibility::Collapsed;
										}
										else
										{
											return EVisibility::Visible;
										}
									})
								]
							]

						// Tile View
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
							[
								SNew(SBox)
								.Padding(FMargin(5,5,5,5))
								[
									SAssignNew(TileView, STileView<TSharedPtr<FVaultMetadata>>)
									.ItemWidth(TILE_SCALED_WIDTH)
									.ItemHeight(TILE_SCALED_HEIGHT)
									.ItemAlignment(EListItemAlignment::EvenlyDistributed)
									.ListItemsSource(&FilteredAssetItems)
									.OnGenerateTile(this, &SLoaderWindow::MakeTileViewWidget)
									.SelectionMode(ESelectionMode::Single)
									.OnSelectionChanged(this, &SLoaderWindow::OnAssetTileSelectionChanged)
									.OnMouseButtonDoubleClick(this, &SLoaderWindow::OnAssetTileDoubleClicked)
									.OnContextMenuOpening(this, &SLoaderWindow::OnAssetTileContextMenuOpened)
								]
							
							]
						
						// Bottom Bar
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
								[
									SNew(SSpacer)
								]
							+ SHorizontalBox::Slot()
								[
									// Scale Slider
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									.Padding(FMargin(0, 0, 0, 0))
									[
										SNew(STextBlock)
										.Text(LOCTEXT("ScaleSliderLabel", "Thumbnail Scale"))
										.Justification(ETextJustify::Right)
									]
									+ SHorizontalBox::Slot()
										[
											SAssignNew(UserScaleSlider, SSlider)
											.Value(TileUserScale)
											.MinValue(0.2)
											.OnValueChanged(this, &SLoaderWindow::OnThumbnailSliderValueChanged)
										]
								]
						]
					] // close border for center area
				] // ~ Close Center Area Splitter
				

				// Metadata Zone
				+ SSplitter::Slot()
					.Value(0.2f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Padding(FMargin(4.0f, 4.0f))
						[
							// Left Sidebar!
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("VaultLoaderRightSidebarHeaderLabel", "METADATA"))
								.TextStyle(FVaultStyle::Get(), "MetaTitleText")
					
							]
							+ SVerticalBox::Slot()
							.FillHeight(1)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SNew(SBox)
									[
										MetadataWidget.ToSharedRef()
									]
								]
							]
						]
					]
				] // ~ hbox			
				]; // ~ LoaderRoot
	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(2.f,2.f))
			[
				LoaderRoot
			]
		];

}

void SLoaderWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

void SLoaderWindow::PopulateBaseAssetList()
{
	FilteredAssetItems.Empty();

	for (FVaultMetadata Meta : FVaultModule::Get().MetaFilesCache)
	{
		FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Meta)));
	}
}

// Show all categories, even if they are unused
void SLoaderWindow::PopulateCategoryArray()
{
	CategoryCloud.Empty();

	TMap<FVaultCategory, FCategoryFilteringItemPtr> CategoryCloudMap;

	for (int i = 0; i <= FVaultCategory::Unknown; i++)
	{
		// current category
		FVaultCategory CurCat = static_cast<FVaultCategory>(i);

		FCategoryFilteringItemPtr CatTemp = MakeShareable(new FCategoryFilteringItem);
		CatTemp->Category = CurCat;
		CatTemp->UseCount = 0;
		CategoryCloudMap.Add(CurCat, CatTemp);
	}

	for (auto Asset : FVaultModule::Get().MetaFilesCache)
	{
		if (Asset.IsMetaValid() && CategoryCloudMap.Contains(Asset.Category))
		{
			CategoryCloudMap.Find(Asset.Category)->Get()->UseCount++;
		}
		else
		{
			CategoryCloudMap.Find(FVaultCategory::Unknown)->Get()->UseCount++;
		}
	}


	CategoryCloudMap.GenerateValueArray(CategoryCloud);
}

// Only shows tags that are actually used, no empty tags will appear. 
void SLoaderWindow::PopulateTagArray()
{
	// Empty Tag Container
	TagCloud.Empty();

	// Create a map version of the array for more efficient searching.
	TMap<FString, FTagFilteringItemPtr> TagCloudMap;

	// For each Asset in our global list of assets...
	for (auto Asset : FVaultModule::Get().MetaFilesCache)
	{
		// Get each tag belonging to that asset...
		for (const FString AssetTag : Asset.Tags)
		{
			// If we already have a tag stored for it, increment the use counter
			if (TagCloudMap.Contains(AssetTag))
			{
				TagCloudMap.Find(AssetTag)->Get()->UseCount++;
			}

			// otherwise, add a new tag to our list. 
			else
			{

				FTagFilteringItemPtr TagTemp = MakeShareable(new FTagFilteringItem);
				TagTemp->Tag = AssetTag;
				TagTemp->UseCount = 1;
				TagCloudMap.Add(AssetTag, TagTemp);
			}
		}
	}

	TagCloudMap.KeySort([](const FString& A, const FString& B) 
		{
			return A < B;
		});

	// The Map version is easier to work with during generation, but since we have to use an Array, we convert our cloud map into an array now:
	TagCloudMap.GenerateValueArray(TagCloud);

}

void SLoaderWindow::PopulateDeveloperNameArray()
{
	// Developer Array
	DeveloperCloud.Empty();

	TMap<FName, int32> DevAssetCounter;

	for (auto AssetItem : FilteredAssetItems)
	{
		if (DevAssetCounter.Contains(AssetItem->Author))
		{
			int Count = *DevAssetCounter.Find(AssetItem->Author);
			Count++;
			DevAssetCounter.Add(AssetItem->Author, Count);
		}
		else
		{
			DevAssetCounter.Add(AssetItem->Author, 1);
		}
	}

	for (auto dev : DevAssetCounter)
	{
		FDeveloperFilteringItemPtr DevTemp = MakeShareable(new FDeveloperFilteringItem);
		DevTemp->Developer = dev.Key;
		DevTemp->UseCount = dev.Value;
		DeveloperCloud.AddUnique(DevTemp);
	}
}

TSharedRef<ITableRow> SLoaderWindow::MakeTileViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FVaultMetadata>>, OwnerTable)
		.Style(FEditorStyle::Get(), "ContentBrowser.AssetListView.TableRow")
		.Padding(FMargin(5.0f, 5.0f, 5.0f, 25.0f))
		[
			SNew(SAssetTileItem)
			.AssetItem(AssetItem)
		];
}

TSharedRef<ITableRow> SLoaderWindow::MakeCategoryFilterViewWidget(FCategoryFilteringItemPtr inCategory, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SCategoryFilterRow, OwnerTable)
		.CategoryData(inCategory)
		.ParentWindow(SharedThis(this));
}

TSharedRef<ITableRow> SLoaderWindow::MakeTagFilterViewWidget(FTagFilteringItemPtr inTag, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STagFilterRow, OwnerTable)
		.TagData(inTag)
		.ParentWindow(SharedThis(this));

}

TSharedRef<ITableRow> SLoaderWindow::MakeDeveloperFilterViewWidget(FDeveloperFilteringItemPtr Entry, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SDeveloperFilterRow, OwnerTable)
		.Entry(Entry)
		.ParentWindow(SharedThis(this));
}

void SLoaderWindow::OnAssetTileSelectionChanged(TSharedPtr<FVaultMetadata> InItem, ESelectInfo::Type SelectInfo)
{
	// Checks if anything is selected
	if (TileView->GetNumItemsSelected() > 0)
	{
		ConstructMetadataWidget(InItem);
		return;
	}

	// If no selection, clear the active metadata.
	MetadataWidget->ClearChildren();

}

void SLoaderWindow::OnAssetTileDoubleClicked(TSharedPtr<FVaultMetadata> InItem)
{
	// #todo Add Item to Project on Double Click
	LoadAssetPackIntoProject(InItem);
}

TSharedPtr<SWidget> SLoaderWindow::OnAssetTileContextMenuOpened()
{

	// Lets check we have stuff selected, we only want the context menu on selected items. No need to open on a blank area
	
	if (TileView->GetNumItemsSelected() == 0)
	{
		return SNullWidget::NullWidget;
	}
	
	// Store our selected item for any future operations.
	TSharedPtr<FVaultMetadata> SelectedAsset = TileView->GetSelectedItems()[0];

	FMenuBuilder MenuBuilder(true, nullptr, nullptr, true);

	//FVaultModule::Get().PluginCommands->MapAction(
	//	FVaultCommands::Get().Rename,
	//	FExecuteAction::CreateLambda([this, SelectedAsset]()
	//		{
	//			UE_LOG(LogVault, Display, TEXT("Hello"));
	//			SelectedAsset->OnRenameRequested().ExecuteIfBound();
	//		}),
	//	FCanExecuteAction());

	static const FName FilterSelectionHook("AssetContextMenu");

	MenuBuilder.BeginSection(FilterSelectionHook, LOCTEXT("AssetContextMenuLabel", "Vault Asset"));
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_AddToProjectLabel", "Add To Project"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					LoadAssetPackIntoProject(SelectedAsset);

				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));

		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_UpdateVaultAssetLabel", "Update Asset"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					// On Asset Publisher prefilled to update an asset

					if (SelectedAsset->IsMetaValid())
					{
						FVaultModule::Get().VaultBasePanelWidget->SetActiveSubTab("Asset Publisher");

						FVaultMetadata AssetPublishMetadata;

						AssetPublishMetadata.Author = SelectedAsset->Author;
						AssetPublishMetadata.PackName = SelectedAsset->PackName;
						AssetPublishMetadata.FileId = SelectedAsset->FileId;
						AssetPublishMetadata.Description = SelectedAsset->Description;
						AssetPublishMetadata.CreationDate = SelectedAsset->CreationDate;
						AssetPublishMetadata.LastModified = FDateTime::UtcNow();
						AssetPublishMetadata.Tags = SelectedAsset->Tags;
						AssetPublishMetadata.Category = SelectedAsset->Category;
						AssetPublishMetadata.HierarchyBadness = SelectedAsset->HierarchyBadness;
						AssetPublishMetadata.ObjectsInPack = SelectedAsset->ObjectsInPack;

						FVaultModule::Get().OnAssetForUpdateChosen.ExecuteIfBound(AssetPublishMetadata);
					}



				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));

		// TODO understand why the fuck the command can't execute its function. Neither if its mapped in the VaultModule or in here above...
		//MenuBuilder.AddMenuEntry(FVaultCommands::Get().Rename, NAME_None, LOCTEXT("ACM_RenameAssetLabel", "Rename Asset"), FText::GetEmpty(), FSlateIcon());

		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_RenameAssetLabel", "Rename Asset"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					SelectedAsset->OnRenameRequested().ExecuteIfBound();
				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));

		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_EditVaultAssetDetailsLabel", "Edit Asset Metadata"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();
					const FString MetaFilePath = LibraryPath / SelectedAsset->FileId.ToString() + ".meta";

					const FText WarningMsg = LOCTEXT("EditAssetMetadataMsg", "Do you really want to manually edit this assets metadata?\nOnly continue if you know what you are doing.");
					const FText WarningTitle = LOCTEXT("EditAssetMetadataTitle", "Attempting to edit Metadata");

					const EAppReturnType::Type Confirmation = FMessageDialog::Open(
						EAppMsgType::YesNo, WarningMsg, &WarningTitle);

					if (Confirmation == EAppReturnType::Yes)
					{
						// Rather than provide a tonne of edit options in engine and a load of extra UI support, for the time being lets just open the file in a text editor
						FPlatformProcess::LaunchFileInDefaultExternalApplication(*MetaFilePath);
					}


				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));


		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_DeleteVaultAssetLabel", "Delete Asset"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					// Open a Msg dialog to confirm deletion
					const EAppReturnType::Type Confirmation = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("DeleteViaContextMsg", "Are you sure you want to delete this asset from the Vault Library \nThis option cannot be undone."));
					if (Confirmation == EAppReturnType::Yes)
					{
						// Delete Pack. Handles all the UI stuff from here as well as the file deletes.
						DeleteAssetPack(SelectedAsset);
					}
				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));

		if (SelectedAsset->InProjectVersion != 0)
		{
			MenuBuilder.AddMenuEntry(LOCTEXT("ACM_DeleteLocalMetadataLabel", "Remove Metadata from Project"), LOCTEXT("ACM_DeleteLocalMetadataTooltip", "Delete Local Metadata of this asset that is added when importing an asset.\nThis will remove the indicator shown when the asset has been imported.\nCan be used when an imported asset has been removed from the project."), FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
					{
						FMetadataOps::DeleteMetadata(*SelectedAsset);
						RefreshLibrary();
					}),
					FCanExecuteAction(),
						FGetActionCheckState(),
						FIsActionButtonVisible()));
		}

	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();

}

TSharedRef<SWidget> SLoaderWindow::OnSortingOptionsMenuOpened()
{
	FMenuBuilder MenuBuilder(true, nullptr, nullptr, true);

	static const FName SortingOptionsHook("SortingOptionsMenu");

	MenuBuilder.BeginSection(SortingOptionsHook, LOCTEXT("SortingOptionsMenuLabel", "Sorting Option"));
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("SOM_FilenameLabel", "Package Name"), FText::GetEmpty(), ActiveSortingType == SortingTypes::Filename ? FSlateIcon("VaultStyle", bSortingReversed ? "UpArrow" : "DownArrow") : FSlateIcon("VaultStyle", "Empty"),
			FUIAction(FExecuteAction::CreateLambda([this]()
				{
					bSortingReversed = ActiveSortingType == SortingTypes::Filename ? !bSortingReversed : false;
					ActiveSortingType = SortingTypes::Filename;
					RefreshLibrary();
				}),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible()));
		MenuBuilder.AddMenuEntry(LOCTEXT("SOM_CreationDateLabel", "Creation Date"), FText::GetEmpty(), ActiveSortingType == SortingTypes::CreationDate ? FSlateIcon("VaultStyle", bSortingReversed ? "UpArrow" : "DownArrow") : FSlateIcon("VaultStyle", "Empty"),
			FUIAction(FExecuteAction::CreateLambda([this]()
				{
					bSortingReversed = ActiveSortingType == SortingTypes::CreationDate ? !bSortingReversed : false;
					ActiveSortingType = SortingTypes::CreationDate;
					RefreshLibrary();
				}),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible()));
		MenuBuilder.AddMenuEntry(LOCTEXT("SOM_ModifiedDateLabel", "Modification Date"), FText::GetEmpty(), ActiveSortingType == SortingTypes::ModificationDate ? FSlateIcon("VaultStyle", bSortingReversed ? "UpArrow" : "DownArrow") : FSlateIcon("VaultStyle", "Empty"),
			FUIAction(FExecuteAction::CreateLambda([this]()
				{
					bSortingReversed = ActiveSortingType == SortingTypes::ModificationDate ? !bSortingReversed : false;
					ActiveSortingType = SortingTypes::ModificationDate;
					RefreshLibrary();
				}),
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible()));
	}
	MenuBuilder.EndSection();


	static const FName FilterOptionsHook("FilterOptionsMenu");

	MenuBuilder.BeginSection(FilterOptionsHook, LOCTEXT("FilterOptionsMenuLabel", "Filtering Options"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("FOM_HierarchyBadnessLabel", "Hide Assets with bad File Hierarchy"), 
			FText::GetEmpty(), 
			FSlateIcon("VaultStyle", "Empty"),
			FUIAction(
				FExecuteAction::CreateLambda([this]()
				{
					bHideBadHierarchyAssets = !bHideBadHierarchyAssets;
					RefreshLibrary();
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([this]()
				{
					return bHideBadHierarchyAssets;
				}),
				FIsActionButtonVisible()
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SLoaderWindow::OnSearchBoxChanged(const FText& inSearchText)
{
	//FilteredAssetItems.Empty();

	// If its now empty, it was probably cleared or backspaced through, so we need to reapply just the filter based results.
	if (inSearchText.IsEmpty())
	{
		UpdateFilteredAssets();
		SortFilteredAssets();
		return;
	}
	if (inSearchText.ToString().Len() < LastSearchTextLength)
	{
		UpdateFilteredAssets();
	}

	LastSearchTextLength = inSearchText.ToString().Len();

	// Store Strict Search - This controls if we only search pack name, or various data entries.
	const bool bStrictSearch = StrictSearchCheckBox->GetCheckedState() == ECheckBoxState::Checked;

	const FString SearchString = inSearchText.ToString();
	
	// Holder for the newly filtered Results:
	TArray<TSharedPtr<FVaultMetadata>> SearchMatchingEntries;

	// Instead of searching raw meta, we search the filtered results, so this respects the tag and dev filters first, and we search within that.
	for (auto Meta : FilteredAssetItems)
	{
		if (Meta->PackName.ToString().Contains(SearchString))
		{
			SearchMatchingEntries.Add(Meta);
			continue;
		}
		
		if (bStrictSearch == false)
		{
			if (Meta->Author.ToString().Contains(SearchString) || Meta->Description.Contains(SearchString))
			{
				SearchMatchingEntries.Add(Meta);
				continue;
			}
			for (FString tag : Meta->Tags)
			{
				if (tag.Contains(SearchString)) {
					SearchMatchingEntries.Add(Meta);
					break;
				}
			}
		}
	}

	FilteredAssetItems = SearchMatchingEntries;
	SortFilteredAssets();
	TileView->RebuildList();
	TileView->ScrollToTop();

}

void SLoaderWindow::OnSearchBoxCommitted(const FText& InFilterText, ETextCommit::Type CommitType)
{
	OnSearchBoxChanged(InFilterText);
}

void SLoaderWindow::ConstructMetadataWidget(TSharedPtr<FVaultMetadata> AssetMeta)
{

	// Safety Catches for Null Assets. Should never occur.
	if (!MetadataWidget.IsValid())
	{
		UE_LOG(LogVault, Error, TEXT("Error - Metadata Ptr is Null."));
		return;
	}

	if (!AssetMeta.IsValid() || !AssetMeta->IsMetaValid())
	{
		UE_LOG(LogVault, Error, TEXT("Error - Metadata Incoming Data is Null."));
		return;
	}

	// Padding between words
	const FMargin WordPadding = FMargin(0.f,12.f,0.f,0.f);

	MetadataWidget->ClearChildren();
	
	// Pack Name
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
	[
		SNew(STextBlock)
			.Text(FText::FromName(AssetMeta->PackName))
	];

	// Description
	MetadataWidget->AddSlot()
		.AutoHeight()

		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			//.Text(FText::Format(LOCTEXT("Meta_DescLbl", "Description: \n{0}"), FText::FromString(AssetMeta->Description)))
			.Text(FText::FromString(AssetMeta->Description))
		];

	// Author
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_AuthorLbl", "Author: {0}"), FText::FromName(AssetMeta->Author)))
		];



	// Creation Date
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_CreationLbl", "Created: {0}"), FText::FromString(AssetMeta->CreationDate.ToString())))
		];

	// Last Modified Date
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_LastModifiedLbl", "Last Modified: {0}"), FText::FromString(AssetMeta->LastModified.ToString())))
		];

	// Category
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::Format(LOCTEXT("MetaCategoryLbl", "Category: {0}"), FText::FromString(FVaultMetadata::CategoryToString(AssetMeta->Category))))
		];

	// Tags List - Header
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			//.Text(FText::Format(LOCTEXT("Meta_TagsLbl", "Tags: {0}"), FText::FromString(FString::Join(AssetMeta->Tags.Array(), TEXT(",")))))
			.Text(LOCTEXT("Meta_TagsLbl", "Tags:"))
		];

	// Tags, Per Tag. Instead of using Join, we add them as separate lines for ease of reading
	for (auto MyTag : AssetMeta->Tags)
	{
		MyTag.TrimStartAndEndInline();
		const FText TagName = FText::FromString(MyTag);

		//MyTag.RemoveSpacesInline();
		MetadataWidget->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("PerTagKeyFor{0}", "- {1}"), TagName, TagName))
			];
	}

	

	// Object List 
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::Format(LOCTEXT("Meta_FilesLbl", "Files: {0}"), FText::FromString(FString::Join(AssetMeta->ObjectsInPack.Array(), TEXT(",")))))
		];

}

void SLoaderWindow::LoadAssetPackIntoProject(TSharedPtr<FVaultMetadata> InPack)
{
	// Root Directory
	const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();

	// All files live in same directory, so we just do some string mods to get the pack file that matches the meta file.
	const FString AssetToImportTemp = LibraryPath / InPack->FileId.ToString() + ".upack";

//#pragma region ImportTask
//		// UPacks import natively with Unreal, so no need to try to use the PakUtilities, better to use the native importer and let Unreal handle the Pak concepts. 
//		// Appearantly not, as unreal crashes with some assets, but UnrealPak does not...
//		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
//
//		// Set up our Import Task
//		UAssetImportTask* Task = NewObject<UAssetImportTask>();
//		Task->AddToRoot();
//		Task->bAutomated = true;
//		Task->bReplaceExisting = true;
//		Task->bSave = true;
//		Task->Filename = AssetToImportTemp;
//		Task->DestinationPath = "/Game";
//
//		TArray<UAssetImportTask*> Tasks;
//		Tasks.Add(Task);
//
//		AssetToolsModule.Get().ImportAssetTasks(Tasks);
//
//		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
//
//		TArray<FAssetData> ImportedAssets;
//		for (FString ImportedPath : Task->ImportedObjectPaths)
//		{
//			FAssetData ImportedAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FName(ImportedPath), true);
//			ImportedAsset.GetAsset();
//			ImportedAssets.Add(ImportedAsset);
//		}
//
//		FMetadataOps::CopyMetadataToLocal(*InPack);
//		InPack->InProjectVersion = 1;
//
//		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
//		TArray<FString> ImportedAssetFolders;
//		ImportedAssetFolders.Add("");
//		FString FileName;
//		FString FileExtension;
//		FPaths::Split(Task->ImportedObjectPaths[0], ImportedAssetFolders[0], FileName, FileExtension);
//
//		ContentBrowserModule.Get().SyncBrowserToFolders(ImportedAssetFolders);
//		ContentBrowserModule.Get().SyncBrowserToAssets(ImportedAssets, true, true);
//
//
//		// Allow GC to collect
//		Task->RemoveFromRoot();
//#pragma endregion
//		UE_LOG(LogVault, Warning, TEXT("Failed to import using Editors import for .upack! Falling back to UnrealPak."));

#pragma region ExecuteUnrealPak
	FScopedSlowTask ImportTask(1.0F, LOCTEXT("ImportingAssetsText", "Importing Assets..."));
	ImportTask.MakeDialog();

	TArray<FString> TargetPathArray;

	for (FString path : InPack->ObjectsInPack)
	{
		if (TargetPathArray.Num() <= 0)
		{
			path.ParseIntoArray(TargetPathArray, TEXT("/"));
		}
		else
		{
			TArray<FString> CurrentPathArray;
			path.ParseIntoArray(CurrentPathArray, TEXT("/"));

			for (int i = 0; i < CurrentPathArray.Num(); i++)
			{
				if (TargetPathArray.Num() > i && !CurrentPathArray[i].Equals(TargetPathArray[i]))
				{
					for (int j = TargetPathArray.Num() - 1; j >= i; j--) {
						TargetPathArray.RemoveAt(j);
					}
					break;
				}
			}
		}
	}

	ImportTask.EnterProgressFrame(0.1f);

	TargetPathArray.RemoveAt(0);
	const FString TargetPath = FPaths::ProjectContentDir() + "/" + UKismetStringLibrary::JoinStringArray(TargetPathArray, TEXT("/"));
	UE_LOG(LogVault, Display, TEXT("%s"), *TargetPath);


	const FString cmd = "-Extract \"" + AssetToImportTemp + "\" \"" + TargetPath + "\"";

	UE_LOG(LogVault, Display, TEXT("UnrealPak Command: %s"), *cmd);

	bool bPakResult = ExecuteUnrealPak(*cmd);
		
	if (!bPakResult)
	{
		UE_LOG(LogVault, Error, TEXT("Failed to import Asset!"));
		return;
	}

	ImportTask.EnterProgressFrame(0.8f);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> ImportedAssets;
	for (FString ImportedPath : InPack->ObjectsInPack)
	{
		FAssetData ImportedAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FName(ImportedPath), true);
		ImportedAsset.GetAsset();
		ImportedAssets.Add(ImportedAsset);
	}

	FMetadataOps::CopyMetadataToLocal(*InPack);
	InPack->InProjectVersion = 1;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FString> ImportedAssetFolders;
	ImportedAssetFolders.Add("");
	FString FileName;
	FString FileExtension;
	FString ImportedFilePath = "";
	for (auto& Element : InPack->ObjectsInPack)
	{
		ImportedFilePath = Element;
		break;
	}
	FPaths::Split(ImportedFilePath, ImportedAssetFolders[0], FileName, FileExtension);


	ImportTask.EnterProgressFrame(0.1f);

	ContentBrowserModule.Get().SyncBrowserToFolders(ImportedAssetFolders);
	ContentBrowserModule.Get().SyncBrowserToAssets(ImportedAssets, true, true);

#pragma endregion

}

void SLoaderWindow::DeleteAssetPack(TSharedPtr<FVaultMetadata> InPack)
{
	// Confirmation will have occurred already for this operation (Might be changed in future to have confirmation here)
	UE_LOG(LogVault, Display, TEXT("Deleting File(s) from Vault: %s"), *InPack->PackName.ToString());

	const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString FilePathAbsNoExt = LibraryPath / InPack->FileId.ToString();
	const FString AbsThumbnailPath = FilePathAbsNoExt + ".png";
	const FString AbsMetaPath = FilePathAbsNoExt + ".meta";
	const FString AbsPackPath = FilePathAbsNoExt + ".upack";

	IFileManager::Get().Delete(*AbsThumbnailPath, true);
	IFileManager::Get().Delete(*AbsMetaPath, true);
	IFileManager::Get().Delete(*AbsPackPath, true);


	FVaultModule::Get().MetaFilesCache.Remove(*InPack);
	RefreshLibrary();
	//UpdateFilteredAssets();
	//TileView->RebuildList();

}

void SLoaderWindow::RefreshAvailableFiles()
{
	IsConnected = FVaultSettings::Get().CheckConnection();
	if (IsConnected) {
		FVaultModule::Get().UpdateMetaFilesCache();
		FVaultStyle::CacheThumbnailsLocally();
	}
	else
	{
		UE_LOG(LogVault, Error, TEXT("Couldn't find vault folder! Connection might be lost."))
	}
}

// Applies the List of filters all together.
void SLoaderWindow::UpdateFilteredAssets()
{
	FilteredAssetItems.Empty();

	// Special Condition to check if all boxes are cleared:
	if (ActiveCategoryFilters.Num() == 0 && ActiveTagFilters.Num() == 0 && ActiveDevFilters.Num() == 0)
	{
		if (bHideBadHierarchyAssets == false)
		{
			PopulateBaseAssetList();
			TileView->RebuildList();
			TileView->ScrollToTop();
			return;
		}
		else 
		{
			for (auto Asset : FVaultModule::Get().MetaFilesCache)
			{
				if (Asset.HierarchyBadness <= 0)
				{
					FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
				}
			}
			TileView->RebuildList();
			TileView->ScrollToTop();
			return;
		}
	}

	for (auto Asset : FVaultModule::Get().MetaFilesCache)
	{
		// Skip this asset if it has bad hierarchy and we are hiding those
		if (bHideBadHierarchyAssets && Asset.HierarchyBadness > 0)
		{
			continue;
		}

		// Apply all filtered Categories
		if (ActiveTagFilters.Num() == 0 && ActiveDevFilters.Num() == 0 && ActiveCategoryFilters.Contains(Asset.Category))
		{
			FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
			continue;
		}

		// Apply all filtered Tags
		for (auto UserTag : Asset.Tags)
		{
			if (ActiveTagFilters.Contains(UserTag) && (ActiveDevFilters.Contains(Asset.Author) || ActiveDevFilters.Num() == 0) && (ActiveCategoryFilters.Contains(Asset.Category) || ActiveCategoryFilters.Num() == 0))
			{
				FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
				break;
			}
		}

		// Apply All Developer Tags
		if (ActiveTagFilters.Num() == 0 && ActiveDevFilters.Contains(Asset.Author) && (ActiveCategoryFilters.Contains(Asset.Category) || ActiveCategoryFilters.Num() == 0))
		{
			FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
			continue;
		}
	}

	TileView->RebuildList();
	TileView->ScrollToTop();

}

void SLoaderWindow::SortFilteredAssets(TEnumAsByte<SortingTypes> SortingType, bool Reverse)
{
	bSortingReversed = Reverse;
	ActiveSortingType = SortingType;
	switch (SortingType) {
	case SortingTypes::Filename : 
		FilteredAssetItems.Sort([&](const TSharedPtr<FVaultMetadata>& a, const TSharedPtr<FVaultMetadata>& b) 
		{
			return Reverse ? b->PackName.LexicalLess(a->PackName) : a->PackName.LexicalLess(b->PackName);
		});
		break;
	case SortingTypes::CreationDate:
		FilteredAssetItems.Sort([&](const TSharedPtr<FVaultMetadata>& a, const TSharedPtr<FVaultMetadata>& b) 
			{
			return Reverse ? a->CreationDate < b->CreationDate : a->CreationDate > b->CreationDate;
		});
		break;
	case SortingTypes::ModificationDate:
		FilteredAssetItems.Sort([&](const TSharedPtr<FVaultMetadata>& a, const TSharedPtr<FVaultMetadata>& b) 
			{
			return Reverse ? a->LastModified < b->LastModified : a->LastModified > b->LastModified;
		});
		break;
	}
}

void SLoaderWindow::SortFilteredAssets()
{
	SortFilteredAssets(ActiveSortingType, bSortingReversed);
}

void SLoaderWindow::OnThumbnailSliderValueChanged(float Value)
{
	TileUserScale = Value;
	TileView->SetItemWidth(TILE_BASE_WIDTH * TileUserScale);
	TileView->SetItemHeight(TILE_BASE_HEIGHT * TileUserScale);
	TileView->RebuildList();
}

FText SLoaderWindow::DisplayTotalAssetsInLibrary() const
{
	int assetCount = FVaultModule::Get().MetaFilesCache.Num();

	FText Display = FText::Format(LOCTEXT("displayassetcountlabel", "Total Assets in library: {0}"),assetCount);
	return Display;
}

FReply SLoaderWindow::OnRefreshLibraryClicked()
{
	RefreshLibrary();
	return FReply::Handled();
}

void SLoaderWindow::RefreshLibrary()
{
	RefreshAvailableFiles();
	PopulateCategoryArray();
	PopulateTagArray();
	PopulateDeveloperNameArray();

	//SearchBox->AdvanceSearch//
	//TileView->RebuildList();
	UpdateFilteredAssets();
	if (!SearchBox->GetText().IsEmpty()) {
		OnSearchBoxChanged(SearchBox->GetText());
	}
	SortFilteredAssets(ActiveSortingType, bSortingReversed);
}

void SLoaderWindow::OnAssetUpdateHappened()
{
	RefreshLibrary();
}

void SLoaderWindow::ModifyActiveCategoryFilters(FVaultCategory CategoryModified, bool bFilterThis)
{
	UE_LOG(LogVault, Display, TEXT("Enabling CategoryFilter %s"), *FVaultMetadata::CategoryToString(CategoryModified));

	if (bFilterThis)
	{
		ActiveCategoryFilters.Add(CategoryModified);
		UpdateFilteredAssets();
		SortFilteredAssets();
		return;
	}

	ActiveCategoryFilters.Remove(CategoryModified);
	UpdateFilteredAssets();
	SortFilteredAssets();
}

void SLoaderWindow::ModifyActiveTagFilters(FString TagModified, bool bFilterThis)
{
	UE_LOG(LogVault, Display, TEXT("Enabling Tag Filter For %s"), *TagModified);
	
	if (bFilterThis)
	{
		// Push our Active Tag into our Set of Tags currently being searched
		ActiveTagFilters.Add(TagModified);
		UpdateFilteredAssets();
		SortFilteredAssets();
		return;
	}

	ActiveTagFilters.Remove(TagModified);
	UpdateFilteredAssets();
	SortFilteredAssets();
}

void SLoaderWindow::ModifyActiveDevFilters(FName DevModified, bool bFilterThis)
{
	UE_LOG(LogVault, Display, TEXT("Enabling Dev Filter %s"), *DevModified.ToString());

	if (bFilterThis)
	{
		ActiveDevFilters.Add(DevModified);
		UpdateFilteredAssets();
		SortFilteredAssets();
		return;
	}

	ActiveDevFilters.Remove(DevModified);
	UpdateFilteredAssets();
	SortFilteredAssets();
}

#undef LOCTEXT_NAMESPACE