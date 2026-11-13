

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int SendComboAttacksCharacter7(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 200 - g_GameManager.rank.unk_b4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(6, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= 200 - g_GameManager.rank.unk_b4 * 3;
        combo->unk_30 -= 5;
        if (combo->unk_30 < 0)
        {
            combo->unk_30 = 0;
        }
    }
    return 0;
}




int UpdateEffectType25(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 56.0f)
    {
        effect->unk_7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->pos.y -= effect->unk_7c * 1.5f;
    return 1;
}




int CheckFocusAreaCharacter7(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unk_368;
    float dy = diff.y - circle->unk_7c * 1.5f;

    if (diff.x * diff.x + dy * dy < circle->unk_7c * circle->unk_7c)
    {
        return 1;
    }
    return 0;
}




struct ExAttackData08
{
    int state;

    int alternate;
    float heading;
    float spin;

    float turn;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData08, 0x5c);





int InitExAttackType6(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
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
    if (g_Rng.GetRandomU32InRange(2))
    {
        data->targetPos.x = -144.0f;
        data->heading = g_Rng.GetRandomF32SignedInRange(0.5235988f);
    data->turn = g_Rng.GetRandomF32SignedInRange(0.01308997f);
    }
    else
    {
        data->targetPos.x = 144.0f;
        data->heading = AddNormalizeAngle(3.1415927f, g_Rng.GetRandomF32SignedInRange(0.5235988f));
    data->turn = g_Rng.GetRandomF32SignedInRange(0.01308997f);
    }
    data->targetPos.y = g_Rng.GetRandomF32InRange(320.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}




int UpdateExAttackType6(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
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
            if (attack->timer.HasTickedEvery(20 - g_GameManager.rank.unk_b4 / 8))
            {
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = 2;
                shooter.count2 = 1;
                shooter.color = 6;
                shooter.angle1 = data->heading;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(1, 0, 120, g_GameManager.rank.unk_b4 * 0.00037878787f + 0.015833333f,
                                        -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                data->heading = AddNormalizeAngle(data->heading, data->turn);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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



int InitExAttackType7(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    if (g_Rng.GetRandomU32InRange(2))
    {
        data->targetPos.x = -144.0f;
        data->heading = 0.0f;
    }
    else
    {
        data->targetPos.x = 144.0f;
        data->heading = 3.1415927f;
    }
    data->targetPos.y = 224.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}




int UpdateExAttackType7(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
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
            if (attack->timer.HasTickedEvery(10 - g_GameManager.difficulty))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = g_GameManager.sides[1 - attack->targetSide].player->unk_a0 / 5 + 2;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unk_a0 * 0.1f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                shooter.sprite = 0;
                shooter.aimMode = 3;
                shooter.angle1 = AddNormalizeAngle(3.1415927f, -data->spin);
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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



int InitExAttackType9(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->heading = angle;
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = -144.0f;
    data->heading = 0.0f;
    data->targetPos.y = 192.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}







int UpdateExAttackTypes8And9(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
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
            if (attack->timer.HasTickedEvery(10 - g_GameManager.difficulty))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = g_GameManager.sides[1 - attack->targetSide].player->unk_a0 / 5 + 2;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unk_a0 * 0.1f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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



int InitExAttackType8(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->heading = angle;
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = 144.0f;
    data->heading = 3.1415927f;
    data->targetPos.y = 192.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}




int InitExAttackTypes11And12(ExAttack *attack)
{
    ExAttackData08 *data;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, attack->pos.z, 2.0f);
    data->heading = attack->pos.z;
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    attack->pos.z = 0.0f;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    data->end.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->end.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}





int UpdateExAttackType11(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
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
            if (attack->timer.HasTickedEvery(10))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = data->alternate != 0 ? 6 : 0;
                shooter.aimMode = 5;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unk_a4 * 0.05f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                data->alternate = 1 - data->alternate;
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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




int UpdateExAttackType12(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
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
            if (attack->timer.HasTickedEvery(10))
            {
                data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = data->alternate != 0 ? 6 : 0;
                shooter.aimMode = 5;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = data->heading;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unk_a4 * 0.05f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                data->alternate = 1 - data->alternate;
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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



int DrawEffectType25(Effect *effect)
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
