

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "unsorted.h"




int UpdateEffectType37(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 8.0f)
    {
        effect->unk_7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}



int CheckFocusAreaCharacter15(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;

    if (diff.x * diff.x < player->unk_368->unk_7c * player->unk_368->unk_7c ||
        diff.y * diff.y < player->unk_368->unk_7c * player->unk_368->unk_7c)
    {
        return 1;
    }
    return 0;
}



int InitExAttackType26(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f) + 200.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}





int UpdateExAttackType26(ExAttack *attack)
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
            if (attack->timer.HasTickedEvery(3))
            {
                BulletShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 0x13;
                shooter.aimMode = 2;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = 0.0f;
                shooter.speed1 = 0.2f;
                shooter.SetAccelCommand(0, 0, 120, 0.006666667f, -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 0x10;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (attack->timer > 0x32)
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




int SendComboAttacksCharacter15(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 80 - g_GameManager.rank.unk_b4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(26, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= 80 - g_GameManager.rank.unk_b4 * 3;
    }
    return 0;
}




int InitEffectType37(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0xf0, "Effect");
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





int DrawEffectType37(Effect *effect)
{
    if (effect->unk_c9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        VertexDiffuseXyzrwh *strip;

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
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x + effect->unk_7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x + effect->unk_7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
        strip = vtx;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unk_7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unk_7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y + effect->unk_7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y + effect->unk_7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        g_SpriteEngine->DrawColoredStrip(effect->sprites, strip, 6);
    }
    return 1;
}
