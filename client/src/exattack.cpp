#include "exattack.h"
#include "gamemanager.h"
#include "sound.h"
#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#include "CoopGame.h"
#endif

ExAttack *ExAttackManager::AddAttack(int type, D3DXVECTOR3 *pos, int side, D3DXVECTOR3 *target)
{
#ifdef TH09_NATIVE_NETPLAY
    if ((coop::Active() && !coop::Spawning()) || thnet::RouteExAttack(type, pos, side)) return &this->attacks[0x100];
#endif
    ExAttack *attack = this->attacks;
    int count = 0;
    int i;

    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active != 0 && attack->side == side)
        {
            count++;
        }
    }
    if (count >= this->limits[side])
    {
        return &this->attacks[0x100];
    }
    g_SoundPlayer.QueueSound(0x2d, side != 0 ? 500 : -500);
    attack = this->attacks;
    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active == 0)
        {
            goto found;
        }
    }
    return attack;
found:
    memset(attack, 0, sizeof(ExAttack));
    attack->active = 1;
    attack->pos = *pos;
    attack->side = side;
    attack->targetSide = 1 - side;
    attack->target = target;
    attack->onUpdate = g_ExAttackTypes[type].onUpdate;
    attack->onDraw = g_ExAttackTypes[type].onDraw;
    attack->onFree = g_ExAttackTypes[type].onFree;
    attack->timer = 0;
#ifdef TH09_NATIVE_NETPLAY
    thnet::InitAttackRandom(attack,type);
    thnet::PatternRandom random(attack);
#endif
    if (g_ExAttackTypes[type].onInit(attack) != 0)
    {
        attack->active = 0;
    }
    return attack;
}

int ExAttackManager::OnUpdate(ExAttackManager *mgr)
{
#ifdef TH09_NATIVE_NETPLAY
    coop::NativeScope cooperativePattern;
    thnet::ActorScope networkPattern;
#endif
    ExAttack *attack = mgr->attacks;
    int i;
    int j;

    if (!(g_GameManager.stateFlags & 0x1800) && !g_GameManager.sides[0].flag0)
    {
        mgr->layerTails[0] = NULL;
        mgr->layerTails[1] = NULL;
        mgr->layerTails[2] = NULL;
        mgr->layerHeads[0] = NULL;
        mgr->layerHeads[1] = NULL;
        mgr->layerHeads[2] = NULL;
        mgr->counts[1] = 0;
        mgr->counts[0] = 0;
        for (i = 0x100; i > 0; i--, attack++)
        {
            if (attack->active == 0)
            {
                continue;
            }
#ifdef TH09_NATIVE_NETPLAY
            thnet::PatternRandom random(attack);
#endif
            if (attack->onUpdate(attack) != 0)
            {
                if (attack->onFree != NULL)
                {
                    attack->onFree(attack);
                }
                if (attack->sprites != NULL)
                {
                    g_MemoryTracker.Free(attack->sprites);
                    attack->sprites = NULL;
                }
                attack->active = 0;
                continue;
            }
            for (j = 0; j < attack->spriteCount; j++)
            {
                g_SpriteEngine->Step(&attack->sprites[j]);
            }
            if (mgr->layerTails[attack->layer] != NULL)
            {
                mgr->layerTails[attack->layer]->next = attack;
            }
            else
            {
                mgr->layerHeads[attack->layer] = attack;
            }
            mgr->counts[attack->side]++;
            mgr->layerTails[attack->layer] = attack;
            attack->next = NULL;
            attack->timer++;
        }
    }
    return 1;
}

int ExAttackManager::OnDraw(ExAttackManager *mgr)
{
    int i;
    int j;
    ExAttack *attack;

    for (i = 0; i < 2; i++)
    {
        g_App.SetView2D(i);
        for (attack = mgr->layerHeads[i]; attack != NULL; attack = attack->next)
        {
            for (j = 0; j < attack->spriteCount; j++)
            {
                if (attack->sprites[j].autoRotate != 0)
                {
                    attack->sprites[j].SetRotationZ(attack->angle);
                }
                attack->sprites[j].pos.x =
                    g_GameManager.ArenaToScreenX(attack->sprites[j].posOffset.x + attack->pos.x);
                attack->sprites[j].pos.y =
                    g_GameManager.ArenaToScreenY(attack->sprites[j].posOffset.y + attack->pos.y);
                attack->sprites[j].pos.z = 0.06f;
                g_SpriteEngine->Draw(&attack->sprites[j]);
            }
            if (attack->onDraw != NULL)
            {
                attack->onDraw(attack);
            }
        }
    }
    return 1;
}

int ExAttackManager::OnDraw2(ExAttackManager *mgr)
{
    int j;
    ExAttack *attack;

    g_App.SetView2D(2);
    for (attack = mgr->layerHeads[2]; attack != NULL; attack = attack->next)
    {
        for (j = 0; j < attack->spriteCount; j++)
        {
            if (attack->sprites[j].autoRotate != 0)
            {
                attack->sprites[j].SetRotationZ(attack->angle);
            }
            attack->sprites[j].pos.x = attack->sprites[j].posOffset.x + attack->pos.x;
            attack->sprites[j].pos.y = attack->sprites[j].posOffset.y + attack->pos.y;
            attack->sprites[j].pos.z = 0.06f;
            g_SpriteEngine->Draw(&attack->sprites[j]);
        }
        if (attack->onDraw != NULL)
        {
            attack->onDraw(attack);
        }
    }
    return 1;
}



static int FreeAttacks(ExAttackManager *mgr)
{
    int i;
    ExAttack *attack;

    attack = mgr->attacks;
    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active == 0)
        {
            continue;
        }
        if (attack->onFree != NULL)
        {
            attack->onFree(attack);
        }
        if (attack->sprites != NULL)
        {
            g_MemoryTracker.Free(attack->sprites);
            attack->sprites = NULL;
        }
    }
    return 0;
}

void ExAttackManager::RemoveChain(ExAttackManager *mgr)
{
    if (mgr != NULL)
    {
        FreeAttacks(mgr);
        g_Chain.Remove(mgr->drawElem2);
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}

void ExAttackManager::Reset()
{
    int i;
    ExAttack *attack;

    attack = this->attacks;
    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active == 0)
        {
            continue;
        }
        if (attack->onFree != NULL)
        {
            attack->onFree(attack);
        }
        if (attack->sprites != NULL)
        {
            g_MemoryTracker.Free(attack->sprites);
            attack->sprites = NULL;
        }
        memset(attack, 0, sizeof(ExAttack));
    }
}

int ExAttackManager::AddedCallback(ExAttackManager *mgr)
{
    mgr->limits[0] = g_Unknown4a117c[g_GameManager.sides[0].character] * g_GameManager.difficulty +
                     g_Unknown4a1128[g_GameManager.sides[0].character];
    mgr->limits[1] = g_Unknown4a117c[g_GameManager.sides[1].character] * g_GameManager.difficulty +
                     g_Unknown4a1128[g_GameManager.sides[1].character];
    mgr->Reset();
    return 0;
}

ExAttackManager *ExAttackManager::RegisterChain()
{
    ExAttackManager *mgr = (ExAttackManager *)g_MemoryTracker.Register(new ExAttackManager(), sizeof(ExAttackManager),
                                                                       "ExAttackCtrlInf");

    memset(mgr, 0, sizeof(ExAttackManager));
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)ExAttackManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, 0xe) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, 0x10);
    mgr->drawElem2 = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnDraw2);
    mgr->drawElem2->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem2, 0x1d);
    return mgr;
}
