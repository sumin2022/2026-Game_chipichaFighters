// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_CodeSet/MySimpleWidget.h"

#include "Rendering/DrawElements.h"
#include "SlateOptMacros.h"
#include "widgets/layout/SBox.h"
#include "widgets/text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMySimpleWidget::Construct(FArguments const& InArgs) {
  /*
  ChildSlot
  [
          // Populate the widget
  ];
  */

  ChildSlot[SNew(SBox).WidthOverride(500).HeightOverride(
      500)[SNew(STextBlock).Text(FText::FromString(TEXT("안녕하세요!")))]];
}
int32 SMySimpleWidget::OnPaint(FPaintArgs const& Args,
                               FGeometry const& AllottedGeometry,
                               FSlateRect const& MyCullingRect,
                               FSlateWindowElementList& OutDrawElements,
                               int32 LayerId, FWidgetStyle const& InWidgetStyle,
                               bool bParentEnabled) const {
  // 부모 OnPaint 호출해 자식 위젯 그리기
  int32 NewLayer = SCompoundWidget::OnPaint(
      Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
      InWidgetStyle, bParentEnabled);

  // 빨간 반투명 사각형 그리기
  //FSlateDrawElement::MakeBox(
  //    OutDrawElements, NewLayer + 1, AllottedGeometry.ToPaintGeometry(),
  //    FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None,
  //    FLinearColor(1.f, 0.f, 0.f, 0.3f));

  return NewLayer + 1;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
