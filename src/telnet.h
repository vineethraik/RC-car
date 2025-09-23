
#include <WiFiClient.h>
#include <WiFiServer.h>

WiFiServer Server(23);
WiFiClient client;

class telnet
{
private:
public:
    telnet(/* args */);
    ~telnet();
    void init();
    void handle();
    void print(String);
    void println(String str){
        print(str);
        print("\n\r");
    }
};

telnet::telnet(/* args */)
{

}

telnet::~telnet()
{
}

void telnet::init()
{
    Server.begin();
    Server.setNoDelay(true);
}

void telnet::print(String str)
{
    
    {
        if (Server.hasClient())
        {
            if (!client || !client.connected())
            {
                if (client)
                    client.stop();
                client = Server.available();
                client.write(str.c_str());
            }
            else
            {
                client.stop();
                client = Server.available();
                client.write(str.c_str());
            }
        }
        else
        {
            if (client && client.connected())
            {
                client.write(str.c_str());
            }
        }
    }
}

void telnet::handle()
{
    if (Server.hasClient())
    {
        if (!client || !client.connected())
        {
            if (client)
                client.stop();
            client = Server.available();
            client.write("Connected\n\r");
        }
    }
}