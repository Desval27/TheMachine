#pragma once

#include <BasicApp.h>
#include <DrumVoice.h>
#include <Singleton.h>

template<std::size_t VOICE_COUNT,
         std::size_t MAX_DEGREES = Music::DEF_MAX_DEGREES,
         std::size_t SCALE_DEGREES = Music::DEF_SCALE_DEGREES>
class App
  : public BasicApp<MAX_DEGREES, SCALE_DEGREES>
  , public Singleton<App<VOICE_COUNT, MAX_DEGREES, SCALE_DEGREES>>
{
  using BaseApp = BasicApp<MAX_DEGREES, SCALE_DEGREES>;
  using SingletonApp = Singleton<App<VOICE_COUNT, MAX_DEGREES, SCALE_DEGREES>>;

private:
  static_assert(VOICE_COUNT > 0, "App needs at least one voice.");

  App()
    : BaseApp()
  {
  }

  friend SingletonApp;

public:
  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param sample_rate
  void Init(float sample_rate) override { BaseApp::Init(sample_rate); }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief Processes the audio stream for all voices mixed into separate left
  /// & right values.
  /// @return left & right floating point values.
  std::tuple<float, float> Process(bool trigger = false) override
  {
    const float evenMix = 1.0 / 3;
    auto [kickL, kickR] = kick_.Process(trigger);
    // float s = snare_.Process(trigger);
    // float h = hat_.Process(trigger);

    float mixL = kickL;
    float mixR = kickR;
    return { mixL, mixR };
  }

protected:
  void InternalUpdate(uint32_t nowMS) override {}

public:
  DrumVoice<daisysp::AnalogBassDrum> kick_;
  DrumVoice<daisysp::AnalogSnareDrum> snare_;
  DrumVoice<daisysp::HiHat<>> hat_;
  // DrumVoice hat_;
};