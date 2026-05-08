#pragma once

#include <Pages/BasePage.h>
#include <TheMachineApp.h>
#include <UIOverlord.h>
#include <daisy.h>

template<typename AppType>
class MainPage : public BasePage<true>
{
public:
  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  MainPage()
    : BasePage()
    , startPage_(nullptr)
  {
  }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param startPage
  void Init(daisy::UiPage& startPage) { startPage_ = &startPage; }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param buttonID
  /// @param numberOfPresses
  /// @param isRetriggering
  /// @return
  bool OnButton(uint16_t buttonID,
                uint8_t numberOfPresses,
                bool isRetriggering) override
  {
    if (buttonID == BUTTON_1) {
      if (startPage_ == nullptr || numberOfPresses == 0)
        return false;
      if (auto* ui = GetParentUI())
        ui->OpenPage(*startPage_);
    }
    return true;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param potID
  /// @param newPosition
  /// @return
  bool OnPotMoved(uint16_t potID, float newPosition) override
  {
    // Pot id corresponds to voice id
    AppType& theApp = AppType::getInstance();
    switch(potID)
    {
      case POT_1:
        theApp.kick_.config_.volume.SetFrom0to1(newPosition);
        break;
      case POT_2:
        theApp.snare_.config_.volume.SetFrom0to1(newPosition);
        break;
      case POT_3:
        theApp.hat_.config_.volume.SetFrom0to1(newPosition);
        break;
    }
    return true;
  }

protected:
  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param display
  /// @param nowMS
  void InternalDraw(daisy::OneBitGraphicsDisplay& display,
                    uint32_t nowMS) override
  {
    uint16_t h = display.GetBounds().GetHeight() / 2;
    daisy::Rectangle rect = display.GetBounds().WithHeight(h);
    display.SetCursor(26, 0);
    display.WriteString("THE", Font_11x18, true);
    display.SetCursor(30, 20);
    display.WriteString("MACHINE", Font_11x18, true);

    if ((nowMS % flashPeriodMS_) < flashOnMS_) {
      display.SetCursor(12, 40);
      display.WriteString("PRESS OK BUTTON", Font_7x10, true);
    }

    daisy::FixedCapStr<20> text = "TIME:";
    text.AppendInt(nowMS);
    display.SetCursor(0, 53);
    display.WriteString(text.Cstr(), Font_6x8, true);
  }

private:
  daisy::UiPage* startPage_;
  const uint32_t flashPeriodMS_ = 1000;
  const uint32_t flashOnMS_ = flashPeriodMS_ / 2;
};
