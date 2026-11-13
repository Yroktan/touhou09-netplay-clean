

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "spellcard.h"
#include "unsorted.h"




int CheckFocusAreaCharacter1(Player *player, D3DXVECTOR3 *pos)
{
    float top = player->pos.y + 32.0f;
    float dx;
    float width;

    if (top <= pos->y)
    {
        return 0;
    }
    top = pos->y - (top - 448.0f);
    dx = pos->x - player->pos.x;
    width = (200704.0f - top * top) * player->unk_368->unk_84 * player->unk_368->unk_84;
    if (width < 196.0f)
    {
        width = 196.0f;
    }
    if (dx * dx <= width)
    {
        return 1;
    }
    return 0;
}



int HitPlayerShotType1(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    player->unk_30354++;
    return 0;
}




int UpdateFocusedEnemyDrift(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = g_Rng.GetRandomF32SignedInRange(0.1f);
    enemy->velocity2.y = enemy->unk_5424 > 0x78 ? -1.0f : enemy->unk_5424.AsFloat() * (-1.0f / 120.0f);
    enemy->velocity2.z = 0.0f;
    enemy->unk_5424++;
    return 0;
}



int InitExAttackType2(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData00));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData00 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->targetPos.y = 448.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}





int UpdateExAttackType2(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
    EnemyScriptVars vars;

    if (data->state == 0)
    {
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            vars.floatVars[0] = -1.5707964f;
            g_GameManager.sides[attack->targetSide].enemyManager->SpawnPlayerEnemyWithVars(8, &data->targetPos, 0x14,
                                                                                            -2, 1000, &vars);
            return 1;
        }
    }
    return 0;
}




int SendComboAttacksCharacter1(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (40 - g_GameManager.rank.unk_b4) * 3)
    {
        g_GameManager.exAttackManager->AddAttack(2, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (30 - g_GameManager.rank.unk_b4) * 4;
    }
    return 0;
}



int InitSpellCardTypes3And6(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}



int IsSpellCardDurationOver(SpellCard *card)
{
    if (card->unk_98 > 0xc0)
    {
        return 1;
    }
    return 0;
}

int InitSpellCardTypes4And7(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}


int InitSpellCardTypes5And8(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1364 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}
