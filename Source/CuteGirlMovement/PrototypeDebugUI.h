#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PrototypeDebugUI.generated.h"

UCLASS()
class CUTEGIRLMOVEMENT_API UPrototypeDebugUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
};