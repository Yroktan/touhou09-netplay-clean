

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int UpdateEffectType28(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 320.0f)
    {
        effect->unk_7c += 8.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->unk_80 = 0.0f;
    return 1;
}





int CheckFocusAreaCharacter8(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = *pos - player->pos;
    D3DXVECTOR3 center;
    float angle = AddNormalizeAngle(player->unk_368->unk_80, 1.5707964f);
    D3DXVECTOR3 offset;

    SetPolar((Vector2 *)&offset, angle, player->unk_368->unk_7c * 0.8660254f);
    if (diff.x * offset.x + diff.y * offset.y > 0.0f)
    {
        center = player->pos - offset;
    }
    else
    {
        center = player->pos + offset;
    }
    diff = center - *pos;
    if (diff.x * diff.x + diff.y * diff.y < player->unk_368->unk_7c * player->unk_368->unk_7c)
    {
        return 1;
    }
    return 0;
}



struct ExAttackData16
{
    int state;
    D3DXVECTOR3 velocity;
    float unknown10;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData16, 0x50);



int InitExAttackType16(ExAttack *attack)
{
    ExAttackData16 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData16));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData16 *)attack->data;
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
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 64.0f);
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}






int UpdateExAttackTypes16And23(ExAttack *attack)
{
    ExAttackData16 *data = (ExAttackData16 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            if (attack->pos.x < 0.0f)
            {
                g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 0xb);
                data->unknown10 = -0.03f;
            }
            else
            {
                g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 0xc);
                data->unknown10 = 0.03f;
            }
            data->velocity.y = 2.0f;
            data->velocity.x = 0.0f;
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
            data->velocity.x += data->unknown10;
            if (attack->pos.x < -144.0f || attack->pos.x > 144.0f)
            {
                data->velocity.x *= -1.0f;
            }
            if (attack->pos.y >= 464.0f)
            {
                return 1;
            }
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&attack->pos, 13.0f, NULL);
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




int SendComboAttacksCharacter8(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 100 - g_GameManager.rank.unk_b4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(16, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= 100 - g_GameManager.rank.unk_b4 * 3;
    }
    return 0;
}




int InitExAttackType23(ExAttack *attack)
{
    ExAttackData16 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData16));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData16 *)attack->data;
    attack->timer = 0x5a;
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
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}





int InitEffectTypes10And18And24And25And26And28And33(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0x294, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unk_48 = effect->pos;
    effect->unk_54.x = 0.0f;
    effect->unk_54.y = 0.0f;
    effect->unk_54.z = 1.0f;
    effect->unk_60.x = 0.0f;
    effect->unk_60.y = -1.0f;
    effect->unk_60.z = 0.0f;
    effect->unk_80 = effect->velocity.x;
    effect->unk_88 = effect->velocity.z;
    effect->unk_94 = 0.0f;
    effect->unk_98 = 0.0f;
    effect->unk_9c = 1.0f;
    effect->unk_8c = 0x20;
    effect->unk_7c = 16.0f;
    effect->unk_c9 = 1;
    return 0;
}




int DrawEffectType28(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = 1.0471976f / (effect->unk_8c / 2 - 1);
        float angle = AddNormalizeAngle(effect->unk_80, 1.0471976f);
        float across = AddNormalizeAngle(effect->unk_80, -1.5707964f);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        D3DXVECTOR3 offset;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        SetPolar((Vector2 *)&offset, across, effect->unk_7c * 0.8660254f);
        for (i = effect->unk_8c; i > 0; i -= 2)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unk_7c);
            vtx->pos += effect->pos + offset;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->pos.z = 0.0f;
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
        angle = AddNormalizeAngle(angle, 2.0943952f - delta);
        for (i = effect->unk_8c; i > 0; i -= 2)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unk_7c);
            vtx->pos += effect->pos - offset;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->pos.z = 0.0f;
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unk_8c + 1);
    return 1;
}




int UpdateEffectType29(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 48.0f)
    {
        effect->unk_7c += 4.0f;
    }
    effect->unk_80 = AddNormalizeAngle(effect->unk_80, 0.05235988f);
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}




int CheckFocusAreaCharacter9(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 v = player->pos - *pos;
    float dist = v.x * v.x + v.y * v.y;
    float angle;

    if (dist < player->unk_368->unk_7c * player->unk_368->unk_7c)
    {
        return 1;
    }
    if (dist > (player->unk_368->unk_7c + 24.0f) * (player->unk_368->unk_7c + 24.0f))
    {
        return 0;
    }
    SetPolar((Vector2 *)&v, player->unk_368->unk_80, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unk_368->unk_80, 1.0471976f);
    SetPolar((Vector2 *)&v, angle, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unk_368->unk_80, 2.0943952f);
    SetPolar((Vector2 *)&v, angle, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unk_368->unk_80, 3.1415927f);
    SetPolar((Vector2 *)&v, angle, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unk_368->unk_80, 4.1887903f);
    SetPolar((Vector2 *)&v, angle, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unk_368->unk_80, 5.2359877f);
    SetPolar((Vector2 *)&v, angle, player->unk_368->unk_7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    return 0;
}
