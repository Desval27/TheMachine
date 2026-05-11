#pragma once

#include <BasicApp.h>
#include <DrumVoice.h>
#include <monkey.hpp>
#include <music/music.hpp>
#include <singleton.hpp>

template<std::size_t MAX_DEGREES = music::DEF_MAX_DEGREES,
         std::size_t SCALE_DEGREES = music::DEF_SCALE_DEGREES,
         std::size_t MAX_EVENTS = 16>
class TheMachine
  : public BasicApp<MAX_DEGREES, SCALE_DEGREES>
  , public Singleton<TheMachine<MAX_DEGREES, SCALE_DEGREES>>
{
  using TBasicApp = BasicApp<MAX_DEGREES, SCALE_DEGREES>;
  using TSingletonApp = Singleton<TheMachine<MAX_DEGREES, SCALE_DEGREES>>;
  using TDrumPattern = music::PatternEventSet<MAX_EVENTS>;

private:
  TheMachine()
    : TBasicApp()
  {
    kickPattern_.clear();
    snarePattern_.clear();
    hatPattern_.clear();
  }

  friend TSingletonApp;

public:
  /////////////////////////////////////////////////////////////////////////////
  /// @brief
  /// @param sample_rate
  void init(float sample_rate) override
  {
    TBasicApp::init(sample_rate);
    kick_.init(TBasicApp::setup, 0, sample_rate);
    snare_.init(TBasicApp::setup, 0, sample_rate);
    hat_.init(TBasicApp::setup, 0, sample_rate);

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

    music::build_euclid<MAX_EVENTS>(5, MAX_EVENTS, 1, kickPattern_);
    music::build_euclid<MAX_EVENTS>(7, MAX_EVENTS, 2, snarePattern_);
    music::build_euclid<MAX_EVENTS>(13, MAX_EVENTS, 1, hatPattern_);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// @brief Processes the audio stream for all voices mixed into separate left
  /// & right values.
  /// @return left & right floating point values.
  std::tuple<float, float> process(bool trigger = false) override
  {
    const std::size_t patternLength = kickPattern_.size();
    const bool isPatternStep = trigger && patternIndex_ < patternLength;
    bool kickTrigger = isPatternStep && kickPattern_[patternIndex_];
    bool snareTrigger = isPatternStep && snarePattern_[patternIndex_];
    bool hatTrigger = isPatternStep && hatPattern_[patternIndex_];

    auto [kickL, kickR] = kick_.process(kickTrigger);
    // auto [snareL, snareR] = snare_.process(snareTrigger);
    auto [hatL, hatR] = hat_.process(hatTrigger);
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
  void internal_update(uint32_t nowMS) override
  {
    kick_.update(nowMS);
    snare_.update(nowMS);
    hat_.update(nowMS);
  }

public:
  DrumVoice<daisysp::AnalogBassDrum> kick_;
  DrumVoice<daisysp::HiHat<>> snare_;
  DrumVoice<daisysp::HiHat<daisysp::RingModNoise, daisysp::SwingVCA>> hat_;

  std::size_t patternIndex_{ 0 };
  TDrumPattern kickPattern_;
  TDrumPattern snarePattern_;
  TDrumPattern hatPattern_;
};
