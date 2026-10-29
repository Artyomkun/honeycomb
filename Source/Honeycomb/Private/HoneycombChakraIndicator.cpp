#include "HoneycombChakraIndicator.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

void UHoneycombChakraIndicator::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree)
    {
        return;
    }

    USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ChakraBox"));
    Box->SetWidthOverride(28.0f);
    Box->SetHeightOverride(28.0f);

    UImage* Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ChakraIcon"));
    Icon->SetColorAndOpacity(FLinearColor::White);
    Box->AddChild(Icon);

    WidgetTree->RootWidget = Box;
}
