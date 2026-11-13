

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int UpdateEffectType30(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_84 < 1.5707964f)
    {
        effect->unk_84 += 0.031415928f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->unk_80 = -1.5707964f;
    return 1;
}



struct ExAttackData18
{
    ExAttackData00 base;
    float angle;
    float speed;

    int bounced;
    D3DXVECTOR3 positions[16];
    float angles[16];
};
ASSERT_SIZE(ExAttackData18, 0x158);



int InitExAttackType18(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(4, sizeof(ExAttackData18));
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







int UpdateExAttackTypes18And24(ExAttack *attack)
{
    ExAttackData18 *data = (ExAttackData18 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);
    int i;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[0], 9);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[1], 10);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[2], 10);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[3], 10);
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            data->angle = g_GameManager.sides[attack->targetSide].player->AngleToPlayer(&attack->pos);
            data->speed = 0.2f;
            data->bounced = 0;
            for (i = 0; i < 16; i++)
            {
                data->positions[i] = attack->pos;
                data->angles[i] = attack->angle;
            }
        }
        break;
    case 1:
        for (i = 15; i > 0; i--)
        {
            data->positions[i] = data->positions[i - 1];
            data->angles[i] = data->angles[i - 1];
        }
        data->positions[0] = attack->pos;
        data->angles[0] = data->angle;
        attack->sprites[0].SetRotationZ(data->angle);
        attack->sprites[1].posOffset = data->positions[1] - attack->pos;
        attack->sprites[1].SetRotationZ(data->angles[1]);
        attack->sprites[2].posOffset = data->positions[3] - attack->pos;
        attack->sprites[2].SetRotationZ(data->angles[3]);
        attack->sprites[3].posOffset = data->positions[5] - attack->pos;
        attack->sprites[3].SetRotationZ(data->angles[5]);
        if (attack->timer > 0x14)
        {
            D3DXVECTOR3 ahead;

            SetPolar((Vector2 *)&data->base.velocity, data->angle, data->speed);
            data->speed += 0.1f;
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->base.state = 2;
                attack->timer = 0;
                attack->sprites[0].SetPendingInterrupt(1);
                attack->sprites[1].SetPendingInterrupt(1);
                attack->sprites[2].SetPendingInterrupt(1);
                attack->sprites[3].SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->base.velocity;
            if (!data->bounced)
            {
                if (attack->pos.x < -144.0f)
                {
                    data->base.velocity.x = -data->base.velocity.x;
                    data->angle = 3.1415927f - data->angle;
                    data->angle = AddNormalizeAngle(data->angle, 0.0f);
                    data->bounced = 1;
                }
                else if (attack->pos.x > 144.0f)
                {
                    data->base.velocity.x = -data->base.velocity.x;
                    data->angle = 3.1415927f - data->angle;
                    data->angle = AddNormalizeAngle(data->angle, 0.0f);
                    data->bounced = 1;
                }
            }
            SetPolar((Vector2 *)&ahead, data->angle, data->speed);
            ahead += attack->pos;
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&ahead, 12.0f, NULL);
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




int SendComboAttacksCharacter10(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (25 - g_GameManager.rank.unk_b4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(18, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (25 - g_GameManager.rank.unk_b4) * 4;
    }
    return 0;
}




int InitExAttackType24(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(4, 0x158);
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




int InitEffectType30(Effect *effect)
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
    effect->unk_80 = AddNormalizeAngle(effect->mgr->side->player->unk_1cd8, 3.1415927f);
    effect->unk_94 = 0.0f;
    effect->unk_98 = 0.0f;
    effect->unk_84 = 0.0f;
    effect->unk_88 = effect->velocity.z;
    effect->unk_8c = 0x10;
    effect->unk_9c = 1.0f;
    effect->unk_7c = 128.0f;
    effect->unk_c9 = 1;
    effect->unk_80 = -1.5707964f;
    return 0;
}



int DrawEffectType30(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = effect->unk_84 / (effect->unk_8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = effect->unk_80 - effect->unk_84 * 0.5f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        for (i = effect->unk_8c; i > 0; i--)
        {
            vtx->pos.z = 0.0f;
            angle = AddNormalizeAngle(angle, 0.0f);
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unk_7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x50600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unk_8c + 1);
    return 1;
}




static int LineSide(D3DXVECTOR3 *p, D3DXVECTOR3 *a, D3DXVECTOR3 *b)
{
    float cross = (p->y - a->y) * b->x + (a->y - b->y) * p->x + (b->y - p->y) * a->x;

    if (cross > 0.0f)
    {
        return 1;
    }
    if (cross < 0.0f)
    {
        return -1;
    }
    return 0;
}





int UpdateEffectType31(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 128.0f)
    {
        effect->unk_7c += 6.0f;
    }
    if (g_GameManager.sides[effect->mgr->sideIdx].player->unk_1ccc.x < 0.0f)
    {
        effect->unk_80 += (-2.0943952f - effect->unk_80) * 0.02f;
    }
    else if (g_GameManager.sides[effect->mgr->sideIdx].player->unk_1ccc.x > 0.0f)
    {
        effect->unk_80 += (-1.0471976f - effect->unk_80) * 0.02f;
    }
    else
    {
        effect->unk_80 += (-1.5707964f - effect->unk_80) * 0.05f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}



int UpdatePlayerShotType6(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, 0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}



int UpdatePlayerShotType7(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, -0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}





int CheckFocusAreaCharacter11(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = *pos - player->pos;
    D3DXVECTOR3 pts[10];
    Effect *effect = player->unk_368;
    float angle = effect->unk_80;

    SetPolar((Vector2 *)&pts[0], angle, effect->unk_7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[2], angle, effect->unk_7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[4], angle, effect->unk_7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[6], angle, effect->unk_7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[8], angle, effect->unk_7c);
    pts[1] = (pts[0] - pts[4]) / 1.618f + pts[4];
    pts[3] = (pts[2] - pts[6]) / 1.618f + pts[6];
    pts[5] = (pts[4] - pts[8]) / 1.618f + pts[8];
    pts[7] = (pts[6] - pts[0]) / 1.618f + pts[0];
    pts[9] = (pts[8] - pts[2]) / 1.618f + pts[2];
    if (LineSide(&diff, &pts[0], &pts[4]) >= 0 && LineSide(&diff, &pts[4], &pts[7]) >= 0 &&
        LineSide(&diff, &pts[7], &pts[0]) >= 0)
    {
        return 1;
    }
    if (LineSide(&diff, &pts[8], &pts[2]) >= 0 && LineSide(&diff, &pts[2], &pts[5]) >= 0 &&
        LineSide(&diff, &pts[5], &pts[8]) >= 0)
    {
        return 1;
    }
    if (LineSide(&diff, &pts[6], &pts[0]) >= 0 && LineSide(&diff, &pts[0], &pts[3]) >= 0 &&
        LineSide(&diff, &pts[3], &pts[6]) >= 0)
    {
        return 1;
    }
    return 0;
}
