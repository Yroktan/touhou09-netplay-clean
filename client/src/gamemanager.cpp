
#include <time.h>

#include "gamemanager.h"
#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gui.h"
#include "mainmenu.h"
#include "player.h"
#include "replay.h"
#include "screeneffect.h"
#include "sound.h"
#include "spellcard.h"
#include "sprite.h"
#include "stage.h"

#ifdef TH09_EXE


GameManager g_GameManager;
ChainElem g_GameManagerCalcChainElem;
ChainElem g_GameManagerDrawChainElem;
#endif

BOOL GameManager::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < -144.0f)
    {
        return FALSE;
    }
    if (144.0f < x - width / 2.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (448.0f < y - height / 2.0f)
    {
        return FALSE;
    }
    return TRUE;
}

int GameManager::OnDraw(GameManager *mgr)
{
    if (mgr->isPaused)
    {
        mgr->isPaused = 2;
    }
    if (g_App.scene != 2)
    {
        return 3;
    }
    if ((mgr->stateFlags >> 5 & 3) == 1)
    {
        return 3;
    }
    if (mgr->loadingThreadState != 0)
    {
        return 3;
    }
    return 1;
}

void FormatTodayDate(char *buf)
{
    time_t t;

    time(&t);
    strftime(buf, 10, "%y/%m/%d", localtime(&t));
}

void GameManager::ResetState()
{
    int i;

    for (i = 0; i < 3; i++)
    {
        this->sides[i].flags &= ~1;
    }
    this->rank.unk_b0 = 0;
    this->rank.unk_b8 = 2;
    this->rank.unk_c0 = 0;
    this->rank.unk_d0 = 0;
    this->rank.unk_bc = 0;
    this->unk_110 = 0;
    this->unk_128 = 0;
    this->unk_12c = 0;
    this->unk_130 = 0;
    this->isPaused = 0;
    this->isInContinueMenu = 0;
    this->unk_13e = 0;
    this->unk_380 = 0;
}

void RemoveGameManagerChain()
{
    if (g_GameManager.sides[0].stats->score >= 1000000000)
    {
        g_GameManager.sides[0].stats->score = 999999999;
    }
    g_GameManager.sides[0].stats->guiScore = g_GameManager.sides[0].stats->score;
    if (g_GameManager.sides[1].stats->score >= 1000000000)
    {
        g_GameManager.sides[1].stats->score = 999999999;
    }
    g_GameManager.sides[1].stats->guiScore = g_GameManager.sides[1].stats->score;
    g_App.gameSpeed = 1.0f;
    g_Chain.Remove(&g_GameManagerCalcChainElem);
    g_Chain.Remove(&g_GameManagerDrawChainElem);
}

WORD Rng::GetSeed()
{
    return this->seed;
}

void GameConfig::SetDefaults()
{
    memset(this, 0, sizeof(GameConfig));
    this->version = 0x90003;
    this->padThresholdX = 600;
    this->padThresholdY = 600;
    this->musicMode = 1;
    this->playSounds = 1;
    this->difficulty = 1;
    this->unk_ac = 0;
    this->colorMode16Bits = 0;
    this->windowed = 0;
    this->frameSkip = 0;
    this->mappings[0][0] = g_PlayerInputs[0].mappings[0];
    this->mappings[0][1] = g_PlayerInputs[0].mappings[1];
    this->mappings[0][2] = g_PlayerInputs[0].mappings[2];
    this->mappings[1][0] = g_PlayerInputs[1].mappings[0];
    this->mappings[1][1] = g_PlayerInputs[1].mappings[1];
    this->mappings[1][2] = g_PlayerInputs[1].mappings[2];
    this->unk_b3 = 2;
    this->bgmVolume = 100;
    this->seVolume = 80;
    this->unk_b4[0] = 0;
    this->unk_b4[1] = 0;
    this->unk_b4[2] = 0;
    this->pads[0] = 0;
    this->pads[1] = 1;
    this->pads[2] = 2;
}

void App::ResetSubThread()
{
    this->subThread = NULL;
    this->closeSubThread = 0;
    this->subThreadRunning = 0;
}

int GameManager::IsDemo()
{
    return this->stateFlags >> 1 & 1;
}

void GameManager::AddExtend()
{
    if (g_GameManager.sides[0].stats->lives < 7.0f)
    {
        g_GameManager.sides[0].stats->lives += 1.0f;
        g_SoundPlayer.QueueSound(0x1c, 0);
    }
    g_GameManager.sides[0].stats->extendsGiven++;
    if (g_GameManager.mode == 1)
    {
        if (g_GameManager.sides[0].stats->extendsGiven >= 8)
        {
            g_GameManager.sides[0].stats->extendsGiven = 99999;
        }
    }
    else if (g_GameManager.sides[0].stats->extendsGiven >= 5)
    {
        g_GameManager.sides[0].stats->extendsGiven = 99999;
    }
}

#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#endif
int GameManager::OnUpdate(GameManager *mgr)
{
    int i;
    DWORD step;
    int item;

    g_GameManager.unk_12c++;
    if (g_GameManager.mode != 2 || g_GameManager.sides[0].unk_28 != 0 || g_GameManager.sides[1].unk_28 != 0)
    {
        g_PlayerInputs[0].unk_0 = g_PlayerInputs[2].unk_0;
        g_PlayerInputs[0].unk_2 = g_PlayerInputs[2].unk_2;
        g_PlayerInputs[0].unk_4 = g_PlayerInputs[2].unk_4;
        g_PlayerInputs[0].unk_6 = g_PlayerInputs[2].unk_6;
        g_PlayerInputs[0].unk_8 = g_PlayerInputs[2].unk_8;
        g_PlayerInputs[1].unk_0 = g_PlayerInputs[2].unk_0;
        g_PlayerInputs[1].unk_2 = g_PlayerInputs[2].unk_2;
        g_PlayerInputs[1].unk_4 = g_PlayerInputs[2].unk_4;
        g_PlayerInputs[1].unk_6 = g_PlayerInputs[2].unk_6;
        g_PlayerInputs[1].unk_8 = g_PlayerInputs[2].unk_8;
    }
    if (g_GameManager.loadingThreadState != 0)
    {
        if (g_GameManager.loadingThreadState == 2)
        {
            return 4;
        }
        mgr->unk_128++;
        return 3;
    }
#ifdef TH09_NATIVE_NETPLAY
    if (thnet::WaitForPeer()) return 3;
    thnet::BeginGameUpdate();
    if (thnet::Active() && thnet::NetworkPaused()) return 3;
#endif
    if (g_GameManager.unk_38c != 0)
    {
        g_SpriteEngine->ReleaseSurface(8);
        g_App.loadingState = 0;
        if (g_GameManager.unk_38c == 1)
        {
            g_App.PlayMusicById(0, 0);
        }
        g_GameManager.unk_38c = 0;
    }
    if (!mgr->isInContinueMenu && !mgr->unk_13e && !mgr->isPaused && !(mgr->stateFlags >> 1 & 1) &&
        (g_PlayerInputs[2].unk_6 & 8))
    {
        mgr->isPaused = 1;
        mgr->unk_130 = 1;
        g_SoundPlayer.QueueSoundCommand(6, 0, "Pause");
        g_SoundPlayer.QueueSound(0x22, 0);
        g_App.AddTotalPlayTime();
    }
    if (mgr->stateFlags >> 1 & 1)
    {
        if (g_PlayerInputs[2].unk_6 != 0)
        {
            g_App.scene = 1;
        }
        mgr->demoFrames++;
        if (mgr->demoFrames == 6000)
        {
            ScreenEffect::StartEffect(4, 120, 0, 0, 0, 0x23, 2);
            g_App.FadeOutMusic(3.0f);
        }
        if (mgr->demoFrames == 6120)
        {
            g_App.scene = 1;
            return 3;
        }
    }
    if (mgr->isPaused == 1 || mgr->isPaused == 2 || mgr->isInContinueMenu || mgr->unk_13e)
    {
        return 3;
    }
    if (!g_GameManager.gui->IsMsgRunning())
    {
        g_GameManager.rank.unk_d0++;
        g_GameManager.rank.unk_b0++;
        if (g_GameManager.rank.unk_b0 % g_GameManager.rank.unk_c4 == g_GameManager.rank.unk_c4 - 1)
        {
            g_GameManager.rank.unk_b4++;
            if (g_GameManager.rank.unk_b4 > g_GameManager.rank.unk_c8)
            {
                g_GameManager.rank.unk_b4 = g_GameManager.rank.unk_c8;
            }
        }
        g_GameManager.rank.unk_c0++;
        if (g_GameManager.rank.unk_b0 % 3600 == 3599)
        {
            if (g_GameManager.sides[0].player->unk_a0 < 16)
            {
                g_GameManager.sides[0].player->unk_a0++;
            }
            if (g_GameManager.sides[0].player->unk_a4 < 16)
            {
                g_GameManager.sides[0].player->unk_a4++;
            }
            if (g_GameManager.sides[1].player->unk_a0 < 16)
            {
                g_GameManager.sides[1].player->unk_a0++;
            }
            if (g_GameManager.sides[1].player->unk_a4 < 16)
            {
                g_GameManager.sides[1].player->unk_a4++;
            }
        }
        if (g_GameManager.rank.unk_b8 < 6 && g_GameManager.rank.unk_c0 % 720 == 719)
        {
            g_GameManager.rank.unk_b8++;
        }
        if (!g_GameManager.unknownBits134_11 && !g_GameManager.sides[0].flag0)
        {
            g_GameManager.rank.unk_cc += mgr->rank.unk_b0 < 1800   ? 3
                                            : mgr->rank.unk_b0 < 3600 ? 4
                                            : mgr->rank.unk_b0 < 5400 ? 6
                                                                         : 7;
        }
        if (g_GameManager.rank.unk_cc > 10000)
        {
            D3DXVECTOR3 pos;
            Enemy *enemy;

            item = (WORD)(g_Rng.GetRandomU16() % 4);
            if (item == g_GameManager.unk_374)
            {
                item++;
                if (item >= 4)
                {
                    item = 0;
                }
            }
            g_GameManager.rank.unk_cc -= 10000;
            pos.x = 0.0f;
            pos.z = 0.0f;
            g_GameManager.unk_374 = item;
            pos.y = -32.0f;
            enemy = g_GameManager.sides[0].enemyManager->Spawn(0x13, &pos, 0x33e, -2, 10000, 0, 0);
            enemy->item = item;
            enemy->unknownBit13 = 1;
            enemy = g_GameManager.sides[1].enemyManager->Spawn(0x13, &pos, 0x33e, -2, 10000, 0, 0);
            enemy->unknownBit13 = 1;
            enemy->item = item;
            g_SoundPlayer.QueueSound(0x33, 0);
            g_GameManager.sides[0].gui->StartUnknown6ee8(1);
            g_GameManager.sides[1].gui->StartUnknown6ee8(1);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (g_GameManager.sides[i].stats->score >= 1000000000)
        {
            g_GameManager.sides[i].stats->score = 999999999;
        }
        if (g_GameManager.sides[i].stats->guiScore != g_GameManager.sides[i].stats->score)
        {
            if (g_GameManager.sides[i].stats->score < g_GameManager.sides[i].stats->guiScore)
            {
                g_GameManager.sides[i].stats->score = g_GameManager.sides[i].stats->guiScore;
            }
            step = (g_GameManager.sides[i].stats->score - g_GameManager.sides[i].stats->guiScore) / 15;
            if (step >= 0x8d55e)
            {
                step = 0x8d55e;
            }
            else if (step == 0)
            {
                step = 1;
            }
            if (g_GameManager.sides[i].stats->scoreIncrement < step)
            {
                g_GameManager.sides[i].stats->scoreIncrement = step;
            }
            if (g_GameManager.sides[i].stats->guiScore + g_GameManager.sides[i].stats->scoreIncrement >
                g_GameManager.sides[i].stats->score)
            {
                g_GameManager.sides[i].stats->scoreIncrement =
                    g_GameManager.sides[i].stats->score - g_GameManager.sides[i].stats->guiScore;
            }
            g_GameManager.sides[i].stats->guiScore += g_GameManager.sides[i].stats->scoreIncrement;
            if (g_GameManager.sides[i].stats->guiScore >= g_GameManager.sides[i].stats->score)
            {
                g_GameManager.sides[i].stats->scoreIncrement = 0;
                g_GameManager.sides[i].stats->guiScore = g_GameManager.sides[i].stats->score;
            }
        }
    }
    if (g_GameManager.mode == 0 &&
        g_GameManager.sides[0].stats->guiScore >= g_GameManager.sides[0].stats->extendsGiven * 2000000 + 1000000)
    {
        g_GameManager.AddExtend();
    }
    if (g_GameManager.mode == 1 &&
        g_GameManager.sides[0].stats->guiScore >= (g_GameManager.sides[0].stats->extendsGiven + 1) * 500000)
    {
        g_GameManager.AddExtend();
    }
    mgr->frames++;
    return 1;
}

void GameManager::LoadingThread(GameManager *arg)
{




    GameManager *mgr = &g_GameManager;
    int i;

    g_GameManager.ResetState();
    FormatTodayDate(g_Hscr.date);
    mgr->stateFlags |= 4;
    mgr->unknownBits134_11 = 0;
    g_GameManager.unk_374 = -1;
    mgr->unknownBit134_14 = 0;
    g_GameManager.rank.unk_d0 = 0;
    g_GameManager.frames = 0;
    g_GameManager.unk_38c = 1;
    g_GameManager.difficultyMask = 0xff;
    g_GameManager.rank.unk_b8 = 2;
    g_GameManager.rank.unk_cc = 0;
    g_GameManager.rank.unk_c0 = 0;
    g_GameManager.demoFrames = 0;
    g_GameManager.unk_100 = 0;
    g_GameManager.unk_104 = g_GameManager.mode == 2 ? 2 : 1;
    g_GameManager.unk_108[0] = 0;
    g_GameManager.unk_108[1] = 0;
    if (!g_GameManager.IsReplay())
    {
        if (g_App.loadsResources)
        {
            memset(g_GameManager.unk_d8, 0, sizeof(g_GameManager.unk_d8));
        }
        else if (g_GameManager.mode == 0 || g_GameManager.mode == 1)
        {
            g_GameManager.unk_fc++;
        }
    }
    else
    {
        memset(g_GameManager.unk_d8, 0, sizeof(g_GameManager.unk_d8));
        if (!g_App.loadsResources && (g_GameManager.mode == 0 || g_GameManager.mode == 1))
        {
            g_GameManager.unk_fc++;
        }
    }
    if (g_GameManager.config != NULL)
    {
        delete g_GameManager.config;
        g_GameManager.config = NULL;
    }
    g_GameManager.config = new GameConfig;
    *g_GameManager.config = g_App.config;
    g_GameManager.timelineSpeed = 1.0f;
    if (g_App.loadsResources)
    {
        mgr->unknownBit134_13 = 0;
        for (i = 0; i < 2; i++)
        {
            if (g_GameManager.sides[i].stats != NULL)
            {
                delete g_GameManager.sides[i].stats;
                g_GameManager.sides[i].stats = NULL;
            }
            g_GameManager.sides[i].stats = new GameStats;
            memset(g_GameManager.sides[i].stats, 0, sizeof(GameStats));
        }
        if (g_GameManager.mode == 0)
        {
            g_GameManager.sides[0].stats->lives = g_GameManager.config->unk_ac + 2.0f;
        }
        else
        {
            g_GameManager.sides[0].stats->lives = 0.0f;
        }
        ReplayManager::Release(g_GameManager.replayManager);
        g_GameManager.replayManager = NULL;
        if (!g_GameManager.IsReplay())
        {
            g_GameManager.rngSeed = g_Rng.seed;
            g_GameManager.PickOpponent();
            g_GameManager.replayManager = ReplayManager::RegisterChain(0, NULL);
        }
        else
        {
            g_GameManager.replayManager = ReplayManager::RegisterChain(1, g_GameManager.replayFile);
            g_GameManager.PickOpponent();
        }
    }
    else
    {
        if (mgr->mode == 0 &&
            mgr->sides[0].stats->guiScore >= mgr->sides[0].stats->extendsGiven * 2000000 + 1000000)
        {
            mgr->AddExtend();
        }
        if (mgr->mode == 1 &&
            mgr->sides[0].stats->guiScore >= (mgr->sides[0].stats->extendsGiven + 1) * 500000)
        {
            g_GameManager.AddExtend();
        }
        if (!g_GameManager.IsReplay())
        {
            g_GameManager.rngSeed = g_Rng.seed;
            g_GameManager.PickOpponent();
            ReplayManager::RunAddedCallback(g_GameManager.replayManager, 0);
        }
        else
        {
            ReplayManager::RunAddedCallback(g_GameManager.replayManager, 1);
            g_GameManager.PickOpponent();
        }
        if (g_GameManager.mode == 0 || g_GameManager.mode == 1)
        {
            memset(g_GameManager.sides[1].stats, 0, sizeof(GameStats));
        }
    }
    g_App.InitCameras();
    g_GameManager.rank.unk_b0 = 0;
    if ((g_GameManager.gui = Gui::RegisterChain()) == NULL)
    {
        goto fail;
    }
    Player::LoadAnms();
    for (i = 0; i < 2; i++)
    {
        if (g_GameManager.sides[i].unk_28)
        {
            g_GameManager.config->unk_b4[i] = 0;
        }
        g_GameManager.sides[i].flags &= ~1;
        if ((g_GameManager.sides[i].stage = Stage::RegisterChain(i)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].player = Player::RegisterChain(0, i, g_GameManager.sides[i].unk_28)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].bulletManager = BulletManager::RegisterChain(i)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].effectManager = EffectManager::RegisterChain(i, 0x100, 7)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].enemyManager = EnemyManager::RegisterChain(i)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].spellCard = SpellCard::RegisterChain(i)) == NULL)
        {
            goto fail;
        }
        if ((g_GameManager.sides[i].gui = GuiSide::RegisterChain(i)) == NULL)
        {
            goto fail;
        }
    }
    if ((g_GameManager.sides[2].effectManager = EffectManager::RegisterChain(2, 0x320, 1)) == NULL)
    {
        goto fail;
    }
    if ((g_GameManager.exAttackManager = ExAttackManager::RegisterChain()) == NULL)
    {
        goto fail;
    }
    switch (g_GameManager.difficulty)
    {
    case 4:
        g_GameManager.rank.unk_b4 = 18;
        g_GameManager.sides[0].player->unk_a0 = 10;
        g_GameManager.sides[0].player->unk_a4 = 10;
        g_GameManager.sides[1].player->unk_a0 = 10;
        g_GameManager.sides[1].player->unk_a4 = 10;
        g_GameManager.rank.unk_c4 = 600;
        g_GameManager.rank.unk_c8 = 22;
        break;
    case 3:
        g_GameManager.rank.unk_b4 = 8;
        g_GameManager.sides[0].player->unk_a0 = 8;
        g_GameManager.sides[0].player->unk_a4 = 8;
        g_GameManager.sides[1].player->unk_a0 = 8;
        g_GameManager.sides[1].player->unk_a4 = 8;
        if (g_GameManager.mode == 2)
        {
            g_GameManager.rank.unk_c4 = 900;
            g_GameManager.rank.unk_c8 = 22;
        }
        else
        {
            g_GameManager.rank.unk_c4 = 720;
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 6;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 17;
        }
        break;
    case 2:
        g_GameManager.rank.unk_b4 = 6;
        g_GameManager.sides[0].player->unk_a0 = 4;
        g_GameManager.sides[0].player->unk_a4 = 4;
        g_GameManager.sides[1].player->unk_a0 = 4;
        g_GameManager.sides[1].player->unk_a4 = 4;
        if (g_GameManager.mode == 2)
        {
            g_GameManager.rank.unk_c4 = 720;
            g_GameManager.rank.unk_c8 = 22;
        }
        else
        {
            g_GameManager.rank.unk_c4 = 900;
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 3;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 15;
        }
        break;
    case 1:
        g_GameManager.rank.unk_b4 = 1;
        g_GameManager.sides[0].player->unk_a0 = 1;
        g_GameManager.sides[0].player->unk_a4 = 1;
        g_GameManager.sides[1].player->unk_a0 = 1;
        g_GameManager.sides[1].player->unk_a4 = 1;
        if (g_GameManager.mode == 2)
        {
            g_GameManager.rank.unk_c4 = 900;
            g_GameManager.rank.unk_c8 = 20;
        }
        else
        {
            g_GameManager.rank.unk_c4 = 1200;
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 1;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc + 10;
        }
        break;
    case 0:
        g_GameManager.rank.unk_b4 = 1;
        g_GameManager.sides[0].player->unk_a0 = 1;
        g_GameManager.sides[0].player->unk_a4 = 1;
        g_GameManager.sides[1].player->unk_a0 = 1;
        g_GameManager.sides[1].player->unk_a4 = 1;
        g_GameManager.rank.unk_c4 = 1800;
        if (g_GameManager.mode != 2)
        {
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc / 3 + 1;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 6;
        }
        else
        {
            g_GameManager.rank.unk_c8 = 16;
        }
        break;
    }
    DrawFps(0);
    g_App.FinishLoadingSprites();
    while (g_GameManager.stageEndState != 0)
    {
        Sleep(17);
    }
    g_GameManager.loadingThreadState = 0;
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
    g_App.redrawFrames = 60;
    g_App.unk_59c = 0;
    g_Unknown4ac884 = 2;
    g_App.lastTotalPlayTimeTick = timeGetTime();
    return;

fail:
    g_GameManager.loadingThreadState = 2;
    g_App.FinishLoadingSprites();
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
    g_App.unk_59c = 0;
    g_Unknown4ac884 = 2;
}

void GameManager::ResetRound()
{
    int i;

#ifdef TH09_NATIVE_NETPLAY
    thnet::RoundReset();
#endif
    g_GameManager.ResetState();
    g_AsciiText.Initialize();
    g_GameManager.gui->Reset();
    g_GameManager.exAttackManager->Reset();
    for (i = 0; i < 2; i++)
    {
        g_GameManager.sides[i].flags &= ~1;
        g_GameManager.sides[i].stage->Reset();
        g_GameManager.sides[i].player->Reset();
        g_GameManager.sides[i].bulletManager->Reset();
        g_GameManager.sides[i].effectManager->Reset();
        g_GameManager.sides[i].enemyManager->Reset();
        g_GameManager.sides[i].spellCard->Reset();
        g_GameManager.sides[i].gui->Reset();
    }
    g_GameManager.sides[2].effectManager->Reset();
    if (g_GameManager.mode == 2)
    {
        g_GameManager.rank.unk_b4 -= 8;
        if (g_GameManager.rank.unk_b4 < g_GameManager.unk_100)
        {
            g_GameManager.rank.unk_b4 = g_GameManager.unk_100;
        }
    }
    else
    {
        switch (g_GameManager.difficulty)
        {
        case 0:
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc / 3 + 1;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 6;
            g_GameManager.rank.unk_c4 = (g_GameManager.unk_100 + 3) * 600;
            break;
        case 1:
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 1;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc + 10;
            g_GameManager.rank.unk_c4 = (g_GameManager.unk_100 + 2) * 600;
            break;
        case 2:
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 3;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 15;
            g_GameManager.rank.unk_c4 = g_GameManager.unk_100 * 600 + 900;
            break;
        case 3:
            g_GameManager.rank.unk_b4 = g_GameManager.unk_fc + 6;
            g_GameManager.rank.unk_c8 = g_GameManager.unk_fc / 2 + 17;
            g_GameManager.rank.unk_c4 = g_GameManager.unk_100 * 600 + 900;
            break;
        case 4:
            g_GameManager.rank.unk_b4 = 18;
            g_GameManager.rank.unk_c8 = 22;
            g_GameManager.rank.unk_c4 = (g_GameManager.unk_100 + 1) * 600;
            break;
        }
        g_GameManager.rank.unk_b4 -= g_GameManager.unk_100 * 2;
        if (g_GameManager.rank.unk_b4 < 1)
        {
            g_GameManager.rank.unk_b4 = 1;
        }
    }
    g_GameManager.rank.unk_b0 = 0;
    g_GameManager.frames = 0;
    g_GameManager.rank.unk_cc -= 1000;
    if (g_GameManager.rank.unk_cc < 0)
    {
        g_GameManager.rank.unk_cc = 0;
    }
}

int GameManager::AddedCallback(GameManager *mgr)
{
    if (g_App.scene != 3 && g_App.scene != 12 && g_App.scene != 10)
    {
        g_App.loadsResources = 1;
    }
    else
    {
        g_App.loadsResources = 0;
    }
    g_GameManager.loadingThreadState = 1;
    if (g_App.prevScene == 1)
    {
        D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else
    {
        D3DXVECTOR3 pos(280.0f, 430.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
    }
    if (mgr->stageEndState >= 2)
    {
        mgr->stageEndState = 1;
    }
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)GameManager::LoadingThread, NULL);
    g_App.AddTotalPlayTime();
    g_App.AddPlayTime();
    return 0;
}

int GameManager::DeletedCallback(GameManager *mgr)
{
    int i;

    g_GameManager.sides[0].unk_24 = g_GameManager.sides[0].character;
    g_GameManager.sides[1].unk_24 = g_GameManager.sides[1].character;
    g_GameManager.unk_f8 = g_GameManager.stage;
    mgr->stateFlags &= ~4;
    if (g_App.scene != 3 && g_App.scene != 12 && g_App.scene != 10)
    {
        g_App.freesResources = 1;
    }
    else
    {
        g_App.freesResources = 0;
    }
    memset(g_Unknown4d9698, 0, sizeof(g_Unknown4d9698));
    if (g_GameManager.sides[0].stats != NULL)
    {
        g_Hscr.score = g_GameManager.sides[0].stats->score;
        g_Hscr.character = g_GameManager.sides[0].character;
        g_Hscr.difficulty = g_GameManager.difficulty;
        g_Hscr.unk_2b = g_GameManager.sides[0].stats->unk_14;
        g_Hscr.slowdown = 0.0f;
        g_Hscr.magic = 0x52435348;
        g_Hscr.unk_17 = 0;
        g_Hscr.size2 = 0x2c;
        g_Hscr.size = 0x2c;
        g_Hscr.version = 2;
        g_Hscr.isNew = 0;
    }
    if ((g_GameManager.stateFlags >> 3 & 1) && g_App.freesResources)
    {
        ReplayManager::RemoveChain(mgr->replayManager);
        mgr->replayManager = NULL;
    }
    for (i = 0; i < 2; i++)
    {
        Stage::RemoveChain(mgr->sides[i].stage);
        mgr->sides[i].stage = NULL;
        Player::RemoveChain(mgr->sides[i].player);
        mgr->sides[i].player = NULL;
        BulletManager::RemoveChain(mgr->sides[i].bulletManager);
        mgr->sides[i].bulletManager = NULL;
        EffectManager::RemoveChain(mgr->sides[i].effectManager);
        mgr->sides[i].effectManager = NULL;
        EnemyManager::RemoveChain(mgr->sides[i].enemyManager);
        mgr->sides[i].enemyManager = NULL;
        SpellCard::RemoveChain(mgr->sides[i].spellCard);
        mgr->sides[i].spellCard = NULL;
        GuiSide::RemoveChain(mgr->sides[i].gui);
        mgr->sides[i].gui = NULL;
    }
    Gui::RemoveChain(mgr->gui);
    mgr->gui = NULL;
    ExAttackManager::RemoveChain(mgr->exAttackManager);
    mgr->exAttackManager = NULL;
    EffectManager::RemoveChain(mgr->sides[2].effectManager);
    mgr->sides[2].effectManager = NULL;
    if (mgr->config != NULL)
    {
        delete mgr->config;
        mgr->config = NULL;
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    g_GameManager.frames = 0;
    return 0;
}

int GameManager::RegisterChain()
{
    GameManager *mgr = &g_GameManager;

    mgr->frames = 0;
    g_GameManagerCalcChainElem.SetCallback((ChainCallback)GameManager::OnUpdate);
    g_GameManagerCalcChainElem.addedCallback = (ChainCallback)GameManager::AddedCallback;
    g_GameManagerCalcChainElem.deletedCallback = (ChainCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChainElem.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChainElem, 2) != 0)
    {
        return -1;
    }
    g_GameManagerDrawChainElem.SetCallback((ChainCallback)GameManager::OnDraw);
    g_GameManagerDrawChainElem.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChainElem, 5);
    return 0;
}

void GameManager::Init()
{
    memset(this, 0, sizeof(GameManager));
    this->SetPlayArea();
    this->demoIndex = 0;
    this->initialLives[0] = 10;
    this->initialLives[1] = 10;
}
