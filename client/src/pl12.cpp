

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int UpdateEffectType32(Effect *effect)
{
    if (effect->unk_7c > 0.0f)
    {
        effect->unk_c9 = 1;
        effect->unk_7c -= 32.0f;
    }
    else
    {
        effect->unk_c9 = 0;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}




int FirePlayerShotType7(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;

    if (frame == sd->frame)
    {
        angle = player->AngleFromPlayer(&player->orbHomingTarget) + sd->angle;
        if (player->orbHomingTarget.x < -144.0f)
        {
            angle = -1.5707964f;
        }
        FireShot(player, bullet, sd);
        bullet->velocity.x = cosf(angle) * sd->speed;
        bullet->velocity.y = sinf(angle) * sd->speed;
        bullet->angle = angle;
        return 1;
    }
    return 0;
}



struct ExAttackData20
{
    ExAttackData00 base;


    float speed;
    float angle;
    float angleStep;

    int count;
};
ASSERT_SIZE(ExAttackData20, 0x5c);




int InitExAttackType20(ExAttack *attack)
{
    ExAttackData20 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData20));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData20 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->base.velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->base.velocity, angle, 2.0f);
    data->base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->base.start.z = 0.0f;
    data->base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->base.targetPos.y = g_Rng.GetRandomF32InRange(144.0f) + 32.0f;
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->speed = 1.0f;
    data->angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    data->angleStep = g_Rng.GetRandomU16InRange(2) ? 0.10471976f : -0.10471976f;
    data->count = 0;
    return 0;
}






int UpdateExAttackType20(ExAttack *attack)
{
    ExAttackData20 *data = (ExAttackData20 *)attack->data;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            attack->sprites->visible = 0;
        }
        break;
    case 1:
        if (attack->timer < 30)
        {
            if (attack->timer.HasTickedEvery(2))
            {
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 0x15;
                shooter.aimMode = 5;
                shooter.count1 = 8;
                shooter.count2 = 1;
                shooter.color = data->count % 3;
                shooter.angle1 = data->angle;
                shooter.speed1 = data->speed;
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern2(&shooter);
                data->count++;
                data->angle += data->angleStep;
                data->speed += 0.2f;
            }
            break;
        }
        return 1;
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}




int SendComboAttacksCharacter12(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (40 - g_GameManager.rank.unk_b4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(20, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (40 - g_GameManager.rank.unk_b4) * 4;
    }
    return 0;
}





int InitEffectType32(Effect *effect)
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
    effect->unk_7c = 640.0f;
    effect->unk_c9 = 1;
    return 0;
}



int DrawEffectType32(Effect *effect)
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
        g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unk_8c + 1);
    }
    return 1;
}




int UpdateEffectType33(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 96.0f)
    {
        effect->unk_7c += 16.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}



struct ExAttackData21
{
    ExAttackData19 disc;

    float unknown688;
};
ASSERT_SIZE(ExAttackData21, 0x68c);




int InitExAttackType21(ExAttack *attack)
{
    ExAttackData21 *data;
    VertexTex1DiffuseXyzrwh *vtx;
    D3DXVECTOR3 uv;
    float angle;
    float speed;
    int i;

    attack->AllocSprites(1, sizeof(ExAttackData21));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData21 *)attack->data;
    {
        float dir = g_Rng.GetRandomF32InRange(-3.1415927f);
        data->disc.base.velocity.z = 0.0f;
        SetPolar((Vector2 *)&data->disc.base.velocity, dir, 2.0f);
    }
    data->disc.base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->disc.base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->disc.base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->disc.base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->disc.base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->disc.base.start.z = 0.0f;
    data->disc.base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 24.0f);
    data->disc.base.targetPos.y = g_Rng.GetRandomF32InRange(128.0f) + 128.0f;
    data->disc.base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->disc.base.end.x = g_GameManager.ArenaToScreenX(data->disc.base.targetPos.x);
    data->disc.base.end.y = g_GameManager.ArenaToScreenY(data->disc.base.targetPos.y);
    data->disc.base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->disc.points[0] = data->disc.base.targetPos;
    data->unknown688 = -3.1415927f;
    vtx = data->disc.vertices;
    vtx->w = 1.0f;
    vtx->uv.x = 0.5f;
    vtx->uv.y = 0.5f;
    angle = -3.1415927f;
    vtx++;
    speed = g_Rng.GetRandomF32SignedInRange(0.06666667f);
    for (i = 1; i < 0x21; i++)
    {
        if (angle >= 3.1415927f)
        {
            angle -= 6.2831855f;
        }
        vtx->w = 1.0f;
        SetPolar((Vector2 *)&uv, angle, 0.5f);
        vtx->uv.x = uv.x + 0.5f;
        vtx->uv.y = uv.y + 0.5f;
        vtx++;
        data->disc.points[i].z = 0.0f;
        data->disc.radii[i] = g_Rng.GetRandomF32SignedInRange(8.0f) + 80.0f;
        data->disc.radiusSpeeds[i] = speed;
        speed += g_Rng.GetRandomF32SignedInRange(0.033333335f);
        if (speed < -0.06666667f)
        {
            speed = -0.06666667f;
        }
        else if (speed > 0.06666667f)
        {
            speed = 0.06666667f;
        }
        SetPolar((Vector2 *)&data->disc.points[i], angle, data->disc.radii[i]);
        data->disc.points[i] += data->disc.base.targetPos;
        angle += 0.2026834f;
    }
    return 0;
}







int UpdateExAttackType21(ExAttack *attack)
{
    ExAttackData21 *data = (ExAttackData21 *)attack->data;

    D3DXVECTOR3 v;
    int i;

    switch (data->disc.base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->disc.base.end, &data->disc.base.start,
                           &data->disc.base.tangent0, &data->disc.base.tangent1, attack->timer.AsFloat(),
                           90.0f);
        if (attack->timer > 0x5a)
        {
            data->disc.base.state++;
            attack->pos = data->disc.base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 0xb);
            attack->sprites->visible = 0;
        }
        break;
    case 1:
    {
        VertexTex1DiffuseXyzrwh *vtx = &data->disc.vertices[1];
        float angle = -3.1415927f;
        float texAngle = data->unknown688;
        Bullet *bullet;

        for (i = 1; i < 0x21; i++)
        {
            SetPolar((Vector2 *)&v, texAngle, 0.5f);
            vtx->uv.x = v.x + 0.5f;
            vtx->uv.y = v.y + 0.5f;
            texAngle = AddNormalizeAngle(texAngle, 0.2026834f);
            data->disc.radii[i] += data->disc.radiusSpeeds[i];
            SetPolar((Vector2 *)&data->disc.points[i], angle, data->disc.radii[i]);
            data->disc.points[i] += data->disc.base.targetPos;
            angle += 0.2026834f;
            vtx++;
        }
        data->unknown688 = AddNormalizeAngle(data->unknown688, 0.05235988f);
        bullet = g_GameManager.sides[attack->targetSide].bulletManager->bullets;
        for (i = 0; i < 0xaf; i++, bullet++)
        {
            if (bullet->state != 0 && !bullet->unk_10be && bullet->sprites.unk_d44 != 0x14a)
            {
                v = bullet->pos - attack->pos;
                if (v.x * v.x + v.y * v.y <= 1024.0f)
                {
                    bullet->unk_10be = 1;
                    BulletShooter shooter;
                    shooter.pos = bullet->pos;
                    shooter.sprite = 0x16;
                    shooter.aimMode = 1;
                    shooter.count1 = 1;
                    shooter.count2 = 1;
                    shooter.color = 0;
                    shooter.angle1 = bullet->angle;
                    shooter.speed1 = bullet->speed + 1.0f;
                    shooter.angle2 = 0.0f;
                    shooter.speed2 = 0.0f;
                    shooter.unk_1fa = 0;
                    shooter.flags = 0;
                    g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter)->unk_10be = 1;
                }
            }
        }
        bullet = g_GameManager.sides[attack->targetSide].bulletManager->bullets2;
        for (i = 0; i < 0x168; i++, bullet++)
        {
            if (bullet->state != 0 && !bullet->unk_10be && bullet->sprites.unk_d44 != 0x14a)
            {
                v = bullet->pos - attack->pos;
                if (v.x * v.x + v.y * v.y <= 1024.0f)
                {
                    bullet->unk_10be = 1;
                    BulletShooter shooter;
                    shooter.pos = bullet->pos;
                    shooter.sprite = 0x16;
                    shooter.aimMode = 1;
                    shooter.count1 = 1;
                    shooter.count2 = 1;
                    shooter.color = 0;
                    shooter.angle1 = bullet->angle;
                    shooter.speed1 = bullet->speed + 0.7f;
                    shooter.angle2 = 0.0f;
                    shooter.speed2 = 0.0f;
                    shooter.unk_1fa = 0;
                    shooter.flags = 0;
                    g_GameManager.sides[attack->targetSide].bulletManager->FirePattern2(&shooter)->unk_10be = 1;
                }
            }
        }
        if (!attack->sprites->shown)
        {
            return 1;
        }
        break;
    }
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}





int DrawExAttackTypes19And21(ExAttack *attack)
{
    ExAttackData19 *data = (ExAttackData19 *)attack->data;

    if (data->base.state >= 1)
    {
        BYTE alpha = attack->sprites->alpha;
        VertexTex1DiffuseXyzrwh *vtx = data->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = data->points[0];
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->diffuse = (alpha << 24) | 0xffffff;
        vtx++;
        for (i = 1; i < 0x20; i++)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            vtx->pos = data->points[i];
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->diffuse = 0x8080ff;
            vtx++;
            angle += 0.2026834f;
        }
        *vtx = data->vertices[1];
        g_SpriteEngine->DrawVertexFan(attack->sprites, data->vertices, 0x21);
    }
    data->unk_684 = 0;
    return 0;
}
