#include "network.h"
#include "sprite.h"
#include "unsorted.h"


extern NetPacket g_NetInputPacket;
extern NetPacket g_NetSetupPacket;

int App::UpdateNetPlay()
{
    int result = 0;
    int status;
    DWORD now;
    WORD seeds[2];
    int predicted;
    WORD inputs[2];
    Network *network;

    this->EnterCriticalSection(4);
    if (g_Network->unk_b8 == 0)
    {
        if (g_Network->side != 0)
        {
            g_Network->unk_ac = 1;
            g_Network->CreateDeviceAddress();
            g_Network->CreateHostAddress();
            g_Network->Connect();
            this->connectStartTime = timeGetTime();
            g_Network->unk_b8 = 1;
            this->unk_478 = 1;
            g_SpriteEngine->RenderTextCentered(this->unk_584, 0xffffff, 0, "接続可能な妖怪を探しています");
        }
        else
        {
        host:
            Sleep(100);
            g_Network->unk_ac = 0;
            g_Network->CreateDeviceAddress();
            g_Network->Host();
            g_SpriteEngine->RenderTextCentered(this->unk_584, 0xffffff, 0, "妖怪の接続を待っています");
            g_Network->unk_b8 = 2;
            this->unk_474 = timeGetTime();
        }
    }
    if (g_Network->unk_b8 == 1)
    {
        now = timeGetTime();
        this->unk_478 = 1;
        if (now < this->connectStartTime)
        {
            this->connectStartTime = now;
        }
        if (now - this->connectStartTime >= g_Rng.GetRandomU32InRange(4000) + 4000 && g_Network->side != 1)
        {
            g_Network->Close();
            goto host;
        }
        goto wait;
    }
    if (g_Network->unk_b8 == 2 || g_Network->unk_b8 == 3)
    {
        if (g_Network->GetOtherUnknown1c() == 0)
        {
            if (this->unk_458 != 0)
            {
                status = 3;
                goto end;
            }
            this->unk_478 = 1;
            now = timeGetTime();
            if (now < this->unk_474)
            {
                this->unk_474 = now;
            }
            if (now - this->unk_474 > g_Rng.GetRandomU32InRange(4000) + 6000)
            {
                g_Network->Close();
                g_Network->unk_b8 = 0;
                this->unk_474 = timeGetTime();
            }
            goto wait;
        }
        this->unk_478 = 0;
        this->unk_474 = timeGetTime();
    }
    network = g_Network;
    if (this->unk_460 == 0 || this->unk_460 == 2)
    {
        if (this->unk_458 == 0 || (unsigned int)this->unk_458 % (60 / g_Network->syncFrames) == 0)
        {
            if (this->unk_460 == 0)
            {
                if (network->initialFramesQueued == 0)
                {
                    if (network->unk_ac == 0 && 60 / network->syncFrames - 1 > 0)
                    {
                        for (int frame = -9998; frame + 9998 < 60 / network->syncFrames - 1; frame++)
                        {
                            g_App.AddRecord(0, frame, 0, g_Rng.GetSeed());
                            g_App.AddRecord(1, frame, 0, g_Rng.GetSeed());
                        }
                    }
                    network->unk_b0 = 0;
                    Network *sender = g_Network;
                    g_NetSetupPacket.type = 2;
                    g_NetSetupPacket.side = (char)g_Network->unk_ac;
                    g_NetSetupPacket.syncFrames = g_Network->syncFrames;
                    g_NetSetupPacket.unk_8 = g_Network->isLan;
                    g_NetSetupPacket.seed = g_Rng.GetSeed();
                    sender->Send(&g_NetSetupPacket, sizeof(g_NetSetupPacket));
                    if (g_Network->isLan != 0)
                    {
                        this->PopRecord(0, &seeds[0], &predicted);
                        this->PopRecord(1, &seeds[1], &predicted);
                    }
                    g_Network->initialFramesQueued = 1;
                }
                if (g_Network->unk_b0 == 0)
                {
                waitForFrame:
                    result = 1;
                    this->LeaveCriticalSection(4);
                    return result;
                }
                g_NetInputPacket.input = ReadNetInput();
                this->unk_458++;
                this->unk_464 = timeGetTime();
            }
            this->unk_468 = timeGetTime();
            g_Network->unk_b0 = 0;
            Network *sender = g_Network;
            g_NetInputPacket.type = 0;
            char side = (char)g_Network->unk_ac;
            g_NetInputPacket.side = side;
            g_NetInputPacket.value = 60 / g_Network->syncFrames - 1 + this->unk_458;
            g_NetInputPacket.seed = g_Rng.GetSeed();
            g_NetInputPacket.checksum = side + g_NetInputPacket.value + g_NetInputPacket.seed + g_NetInputPacket.unk_8;
            sender->Send(&g_NetInputPacket, sizeof(g_NetInputPacket));
            this->unk_460 = 1;
            this->AddRecord(g_Network->unk_ac, g_NetInputPacket.value, g_NetInputPacket.input, g_NetInputPacket.seed);
        }
        else
        {
            this->InsertRecord(network->unk_ac, this->unk_458, g_Rng.GetSeed());
            this->InsertRecord(1 - g_Network->unk_ac, this->unk_458, g_Rng.GetSeed());
            this->unk_458++;
        }
    }
    if (this->RecordsMeet(1 - g_Network->unk_ac))
    {
        this->unk_45c = this->unk_458;
        this->unk_460 = 0;
        inputs[0] = this->PopRecord(0, &seeds[0], &predicted);
        inputs[1] = this->PopRecord(1, &seeds[1], &predicted);
        UpdatePlayerInput(0, inputs[0]);
        UpdatePlayerInput(1, inputs[1]);
        UpdatePlayerInput(2, inputs[0] | inputs[1]);
        this->LeaveCriticalSection(4);
        return result;
    }
    now = timeGetTime();
    this->unk_46c = now;
    if (now < this->unk_464)
    {
        this->unk_464 = now;
    }
    if (now - this->unk_464 <= 20000)
    {
        goto waitForFrame;
    }
    if (now - this->unk_464 > 60000)
    {
        status = 3;
        goto end;
    }
    this->unk_478 = 1;
    g_SpriteEngine->RenderTextCentered(this->unk_584, 0xffffff, 0, "妖怪からの通信が途絶えています");
wait:
    status = 2;
end:
    this->LeaveCriticalSection(4);
    if (ReadNetInput() & 2)
    {
        return 3;
    }
    if ((this->stateFlags & 0x80) && !this->HasSubThread())
    {
        return 4;
    }
    return status;
}
