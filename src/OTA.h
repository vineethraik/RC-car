
#include <ArduinoOTA.h>

class OTA
{
public:
    void init();
    void handle();
} ota;

void OTA::init()
{

   ArduinoOTA.begin();

 

    
   
}

void OTA::handle()
{
    ArduinoOTA.handle();
}