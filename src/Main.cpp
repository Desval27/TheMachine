#include <daisy_seed.h>
#include <daisysp.h>
#include <dev/oled_ssd130x.h>

#include <monkey.hpp>

#include <MainPage.h>
#include <Pages/DrumVoicePage.h>
#include <TheMachine.h>
#include <ui_overlord.hpp>

using namespace daisysp;
using namespace daisy;

#define BPM 120.0F
#define ENCODER_COUNT 1
#define BUTTON_COUNT 5
#define POT_COUNT 4

using MyApp = TheMachine<>;
using MyMainPage = MainPage<MyApp>;
using MyOverlord = UIOverlord<SSD130xI2c128x64Driver,
                              ENCODER_COUNT,
                              BUTTON_COUNT,
                              POT_COUNT,
                              ENCODER_1, // MenuEncoder
                              BUTTON_1,  // OK Button
                              BUTTON_2,  // Cancel Button
                              true>;

DaisySeed hw;
Metro clock;

MyApp& theApp = MyApp::get_instance();

MyOverlord uiOverlord;
MyMainPage mainPage;
FullScreenItemMenu voiceMenu;
DrumVoicePage voicePage;

struct OpenDrumVoicePageContext
{
  DrumVoicePage* page;
  UI* ui;
  DrumVoiceConfig* config;
};

void
open_drum_voice_page(void* rawContext)
{
  auto* context = static_cast<OpenDrumVoicePageContext*>(rawContext);
  context->page->Bind(*context->config);
  context->ui->OpenPage(*context->page);
}

////////////////////////////////////////////////////////////////////////////////
// UI & Menu Structure
////////////////////////////////////////////////////////////////////////////////
OpenDrumVoicePageContext voicePageContexts[] = {
  { &voicePage, &uiOverlord.GetUi(), &theApp.kick_.config_ },
  { &voicePage, &uiOverlord.GetUi(), &theApp.snare_.config_ },
  { &voicePage, &uiOverlord.GetUi(), &theApp.hat_.config_ },
};

AbstractMenu::ItemConfig voiceMenuItems[] = {
  { .type = AbstractMenu::ItemType::callbackFunctionItem,
    .text = "KICK",
    .asCallbackFunctionItem{ open_drum_voice_page, &voicePageContexts[0] } },
  { .type = AbstractMenu::ItemType::callbackFunctionItem,
    .text = "SNARE",
    .asCallbackFunctionItem{ open_drum_voice_page, &voicePageContexts[1] } },
  { .type = AbstractMenu::ItemType::callbackFunctionItem,
    .text = "HAT",
    .asCallbackFunctionItem{ open_drum_voice_page, &voicePageContexts[2] } },
};

const MyOverlord::EncoderConfig encoderConfig[ENCODER_COUNT] = {
  { seed::D20, seed::D16 },
};
const MyOverlord::ButtonConfig buttonConfig[BUTTON_COUNT] = {
  { seed::D19 }, // Encoder
  { seed::D17 }, { seed::D18 }, { seed::D15 }, { seed::D21 },
};
const MyOverlord::PotConfig potConfig[POT_COUNT] = {
  { seed::A7 },
  { seed::A8 },
  { seed::A9 },
  { seed::A10 },
};

////////////////////////////////////////////////////////////////////////////////
// Main Audio Loop
////////////////////////////////////////////////////////////////////////////////
void
AudioCallback(AudioHandle::InterleavingInputBuffer in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t size)
{
  uiOverlord.ProcessControls();

  // Prepare the audio block
  for (size_t i = 0; i < size; i += 2) {
    bool t = clock.Process();
    auto [sigL, sigR] = theApp.process(t);
    out[i] = sigL;
    out[i + 1] = sigR;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief
/// @param sample_rate
void
init_components(float sample_rate)
{
  clock.Init(BPM / 60.0F, sample_rate);
  theApp.init(sample_rate);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief
/// @param sample_rate
void
init_ui(float sample_rate)
{
  voiceMenu.Init(voiceMenuItems,
                 ArrayLen(voiceMenuItems),
                 AbstractMenu::Orientation::leftRightSelectUpDownModify,
                 true);

  mainPage.Init(voiceMenu);

  uiOverlord.Init(
    sample_rate, mainPage, &hw.adc, encoderConfig, buttonConfig, potConfig);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief
/// @param
/// @return
int
main(void)
{
  hw.Configure();
  hw.Init();

  hw.SetAudioBlockSize(4);
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  float sample_rate = hw.AudioSampleRate();
  init_components(sample_rate);
  init_ui(sample_rate);

  hw.StartAudio(AudioCallback);
  while (1) {
    uiOverlord.ProcessUi();
    theApp.update(System::GetNow());
  }
}
