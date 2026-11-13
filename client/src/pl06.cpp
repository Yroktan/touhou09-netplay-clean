

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int UpdateEffectType27(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 16.0f)
    {
        effect->unk_7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}



int FirePlayerShotType4(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        player->side->effectManager->SpawnReservedEffect(0x23, &player->pos, 5, 0xffffffff);
    }
    return 0;
}



int CheckFocusAreaCharacter6(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unk_368->unk_7c;

    if (diff.x * diff.x < radius * radius)
    {
        return 1;
    }
    return 0;
}



int InitExAttackType15(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(200.0f) + 128.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}





int UpdateExAttackType15(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 8);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            BulletShooter shooter;

            shooter.pos = attack->pos;
            shooter.sprite = 0x13;
            shooter.aimMode = 5;
            shooter.count1 = 8;
            shooter.count2 = 1;
            shooter.color = 0;
            shooter.angle1 = g_Rng.GetRandomF32SignedInRange(3.1415927f);
            shooter.speed1 = 0.2f;
            shooter.SetAccelCommand(0, 0, 120, 0.016666668f, -999.0f);
            shooter.angle2 = 0.0f;
            shooter.speed2 = 0.0f;
            shooter.unk_1fa = 0;
            shooter.flags = 0x10;
            g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            shooter.aimMode = 3;
            shooter.color = 4;
            shooter.SetAccelCommand(0, 0, 100, 0.016f, -999.0f);
            g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            return 1;
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




int SendComboAttacksCharacter6(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 100 - g_GameManager.rank.unk_b4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(15, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (28 - g_GameManager.rank.unk_b4) * 5;
    }
    return 0;
}




int InitEffectTypes27And36(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0x78, "Effect");
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
    effect->unk_7c = 0.0f;
    effect->unk_c9 = 1;
    return 0;
}




int DrawEffectType27(Effect *effect)
{
    if (effect->unk_c9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x - effect->unk_7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x - effect->unk_7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(effect->unk_7c + vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(effect->unk_7c + vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
    }
    g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
    return 1;
}




int InitEffectType35(Effect *effect)
{
    effect->mgr->side->player->anm->StartScript(effect->sprites, 7);
    effect->sprites->zWriteDisabled = 1;
    effect->sprites->posOffset.x = 0.0f;
    effect->sprites->posOffset.y = 0.0f;
    effect->sprites->posOffset.z = 0.0f;
    return 0;
}



int UpdateEffectType35(Effect *effect)
{
    FireShots(effect->mgr->side->player, effect->timer.AsFrames(), 2);
    return 1;
}




int FirePlayerShotType5(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (!player->side->effectManager->GetReservedEffect(5)->active && frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}




int FirePlayerShotType6(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->pos = player->side->effectManager->GetReservedEffect(5)->pos;
        bullet->angle = -1.5707964f;
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = -sd->speed;
        return 1;
    }
    return 0;
}
