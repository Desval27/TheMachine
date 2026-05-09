#pragma once

#include <BasicApp.h>
#include <DrumVoice.h>
#include <Monkey.h>
#include <Music/Music.h>
#include <Singleton.h>

template<std::size_t MAX_DEGREES = Music::DEF_MAX_DEGREES,
         std::size_t SCALE_DEGREES = Music::DEF_SCALE_DEGREES,
         std::size_t MAX_EVENTS = 16>
class App
  : public BasicApp<MAX_DEGREES, SCALE_DEGREES>
  , public Singleton<App<MAX_DEGREES, SCALE_DEGREES>>
{
  using BaseApp = BasicApp<MAX_DEGREES, SCALE_DEGREES>;
  using SingletonApp = Singleton<App<MAX_DEGREES, SCALE_DEGREES>>;
  using DrumPattern = Music::PatternEventSet<MAX_EVENTS>;

private:
  App()
    : BaseApp()
  {
    kickPattern_.Clear();
    snarePattern_.Clear();
    hatPattern_.Clear();
  }

  friend SingletonApp;

public:
  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param sample_rate
  void Init(float sample_rate) override
  {
    BaseApp::Init(sample_rate);
    kick_.Init(sample_rate);
    snare_.Init(sample_rate);
    hat_.Init(sample_rate);

    kick_.t_.SetFreq(56.0F);
    kick_.t_.SetTone(.7f * random() / (float)RAND_MAX);
    kick_.t_.SetDecay(random() / (float)RAND_MAX);
    kick_.t_.SetSelfFmAmount(random() / (float)RAND_MAX);

    snare_.t_.SetDecay(random() / (float)RAND_MAX);
    // snare_.t_.SetSnappy(random() / (float)RAND_MAX);
    snare_.t_.SetTone(.8f * random() / (float)RAND_MAX);

    hat_.t_.SetDecay(random() / (float)RAND_MAX);
    hat_.t_.SetSustain((random() / (float)RAND_MAX) > .8f);
    hat_.t_.SetTone(random() / (float)RAND_MAX);
    hat_.t_.SetNoisiness(random() / (float)RAND_MAX);

    Music::BuildEuclid<MAX_EVENTS>(5, MAX_EVENTS, 1, kickPattern_);
    Music::BuildEuclid<MAX_EVENTS>(7, MAX_EVENTS, 2, snarePattern_);
    Music::BuildEuclid<MAX_EVENTS>(13, MAX_EVENTS, 1, hatPattern_);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief Processes the audio stream for all voices mixed into separate left
  /// & right values.
  /// @return left & right floating point values.
  std::tuple<float, float> Process(bool trigger = false) override
  {
    const std::size_t patternLength = kickPattern_.Count();
    const bool isPatternStep = trigger && patternIndex_ < patternLength;
    bool kickTrigger = isPatternStep && kickPattern_[patternIndex_];
    bool snareTrigger = isPatternStep && snarePattern_[patternIndex_];
    bool hatTrigger = isPatternStep && hatPattern_[patternIndex_];

    auto [kickL, kickR] = kick_.Process(kickTrigger);
    // auto [snareL, snareR] = snare_.Process(snareTrigger);
    auto [hatL, hatR] = hat_.Process(hatTrigger);
    float snareL = 0.0F, snareR = 0.0F;
    // float hatL = 0.0F, hatR = 0.0F;

    if (trigger) {
      patternIndex_ =
        patternLength > 0 ? (patternIndex_ + 1) % patternLength : 0;
    }

    const float evenMix = 1.0 / 2;
    float mixL = (kickL * evenMix) + (snareL * evenMix) + (hatL * evenMix);
    float mixR = (kickR * evenMix) + (snareR * evenMix) + (hatR * evenMix);
    return { mixL, mixR };
  }

protected:
  void InternalUpdate(uint32_t nowMS) override
  {
    kick_.Update(nowMS);
    snare_.Update(nowMS);
    hat_.Update(nowMS);
  }

public:
  DrumVoice<daisysp::AnalogBassDrum> kick_;
  DrumVoice<daisysp::HiHat<>> snare_;
  DrumVoice<daisysp::HiHat<daisysp::RingModNoise, daisysp::SwingVCA>> hat_;

  std::size_t patternIndex_{ 0 };
  DrumPattern kickPattern_;
  DrumPattern snarePattern_;
  DrumPattern hatPattern_;
};
