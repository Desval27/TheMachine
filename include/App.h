#pragma once

#include <Singleton.h>
#include <BasicApp.h>

class App : public BasicApp<4>, public Singleton<App>
{
    friend class Singleton<App>;

  private:
    App() : BasicApp() {}

  public:
};