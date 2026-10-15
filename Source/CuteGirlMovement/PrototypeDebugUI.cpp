#include "PrototypeDebugUI.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UPrototypeDebugUI::NativeConstruct()
{
	Super::NativeConstruct();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Canvas"));
	WidgetTree->RootWidget = Canvas;

	/* Basic host/client display for HUD. Debug only.
	*/

	const ENetMode NetMode = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
	const bool bIsHost = NetMode == NM_Standalone || NetMode == NM_ListenServer;
	const bool bIsClient = NetMode == NM_Client;
	const TCHAR* RoleText = bIsHost ? TEXT("HOST") : bIsClient ? TEXT("CLIENT") : TEXT("SERVER");
	UTextBlock* RoleLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoleLabel"));
	RoleLabel->SetText(FText::FromString(RoleText));
	RoleLabel->SetColorAndOpacity(FSlateColor(bIsHost
		? FLinearColor(0.35f, 1.0f, 0.55f)
		: bIsClient ? FLinearColor(0.35f, 0.8f, 1.0f) : FLinearColor::White));

	if (UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(RoleLabel))
	{
		Slot->SetAnchors(bIsHost ? FAnchors(1.0f, 0.0f, 1.0f, 0.0f) : FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		Slot->SetAlignment(bIsHost ? FVector2D(1.0f, 0.0f) : FVector2D::ZeroVector);
		Slot->SetPosition(bIsHost ? FVector2D(-24.0f, 24.0f) : FVector2D(24.0f, 24.0f));
		Slot->SetAutoSize(true);
	}
}