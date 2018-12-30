#include "../Sinboha/Sinboha.h"
#include <iostream>
#include <thread>

using namespace std;
using namespace SINBOHA_NSP;
using namespace std::chrono_literals;

class CallBack : public SinbohaCallbackIf
{
    virtual void OnStatusChange(SinbohaStatus Status) override
    {
        if (SinbohaStatus::SINBOHA_STATUS_PENDING == Status)
        {
            cout << "PENDING" << endl;
            return;
        }

        if (SinbohaStatus::SINBOHA_STATUS_ACTIVE == Status)
        {
            cout << "ACTIVE" << endl;
            return;
        }

        if (SinbohaStatus::SINBOHA_STATUS_STANDBY == Status)
        {
            cout << "STANDBY" << endl;
            return;
        }
    }

    virtual void OnReceiveData(const std::string & Data) override
    {
        cout << Data << endl;
    }
};

int main(int argc, char** argv)
{
    string Peer = argv[1];
    int PeerPort = atoi(argv[2]);
    int Port = atoi(argv[3]);
    int Timeout = atoi(argv[4]);
    int Heartbeat = atoi(argv[5]);
    int SwitchTimeout = atoi(argv[6]);

    auto ha = GetSinbohaIf();

    auto rc = ha->Initialize(Peer, "", PeerPort, Port, chrono::milliseconds(Timeout), chrono::milliseconds(Heartbeat), chrono::milliseconds(SwitchTimeout));

    ha->RegisterCallback(make_shared<CallBack>());
    
    for (int i=0; i<1000; i++)
    {
        this_thread::sleep_for(5s);
        ha->SyncData("hello, I am " + std::to_string((int)ha->GetHaStatus()));
    }

    ha->Release();
}