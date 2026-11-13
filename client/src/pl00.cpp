

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "spellcard.h"
#include "sprite.h"
#include "unsorted.h"



int IsSpellCardEnemyDefeated(SpellCard *card)
{
    return g_GameManager.sides[card->sideIdx].enemyManager->FindByUnknownType(3) == NULL;
}




int UpdateEffectType10(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 96.0f)
    {
        effect->unk_7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}



int IsWithinCircularFocusArea(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unk_368->unk_7c;

    if (diff.x * diff.x + diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}




int InitExAttackType0(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}




int InitExAttackType1(ExAttack *attack)
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
    attack->timer = 0x5a;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(64.0f) + attack->pos.x;
    data->targetPos.y = g_Rng.GetRandomF32SignedInRange(64.0f) + attack->pos.x;
    data->targetPos.z = 0.0f;
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    return 0;
}






int UpdateExAttackTypes0And1(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->velocity;
            if ((data->velocity.x < 0.0f && attack->pos.x < -144.0f) ||
                (data->velocity.x > 0.0f && attack->pos.x > 144.0f))
            {
                data->velocity.x = -data->velocity.x;
            }
            data->velocity.y += 0.04f;
            if (data->velocity.y > 3.0f)
            {
                data->velocity.y = 3.0f;
            }
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&attack->pos, 18.0f, NULL);
            if (attack->pos.y >= 480.0f)
            {
                return 1;
            }
        }
        break;
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}




int SendComboAttacksCharacter0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (28 - g_GameManager.rank.unk_b4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(0, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (28 - g_GameManager.rank.unk_b4) * 5;
    }
    return 0;
}




int InitSpellCardType0(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}



int FreePrimarySpellCardData(SpellCard *card)
{
    if (card->unk_1360 != NULL)
    {
        g_MemoryTracker.Free(card->unk_1360);
    }
    card->unk_1360 = NULL;
    return 0;
}

int InitSpellCardType1(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}




int InitSpellCardType2(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unk_1364 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}



int FreeSecondarySpellCardData(SpellCard *card)
{
    if (card->unk_1364 != NULL)
    {
        g_MemoryTracker.Free(card->unk_1364);
    }
    card->unk_1364 = NULL;
    return 0;
}




int DrawEffectType10(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = 6.2831855f / (effect->unk_8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        for (i = effect->unk_8c; i > 0; i--)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unk_7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unk_8c + 1);
    return 1;
}






int UpdatePlayerShotType1(Player *player, PlayerBullet *bullet)
{
    float x;
    float y;
    float len;

    if (bullet->state == 1 && bullet->timer.AsFrames() >= 40 && bullet->timer.HasTicked())
    {
        if (player->orbHomingTarget.x > -900.0f)
        {
            x = player->orbHomingTarget.x - bullet->pos[0];
            y = player->orbHomingTarget.y - bullet->pos[1];
            len = sqrtf(x * x + y * y) / (bullet->speed * 0.25f);
            if (len < 1.0f)
            {
                len = 1.0f;
            }
            x = x / len + bullet->velocity.x;
            y = y / len + bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->speed = len > 10.0f ? 10.0f : len;
            if (bullet->speed < 1.0f)
            {
                bullet->speed = 1.0f;
            }
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
        else if (bullet->speed < 10.0f)
        {
            bullet->speed += 0.33333334f;
            x = bullet->velocity.x;
            y = bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
    }
    bullet->angle = Atan2(bullet->velocity.y, bullet->velocity.x);
    return 0;
}
