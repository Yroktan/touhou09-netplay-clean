

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int UpdateEffectType18(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 144.0f)
    {
        effect->unk_7c += 0.7f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}




int FirePlayerShotType3(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = -24.0f;
        player->unk_1cdc = player->unk_1ce0 = 0.5f;
        return 1;
    }
    return 0;
}





int DrawPlayerShotType2(Player *player, PlayerBullet *bullet)
{
    int red;
    int green;
    int blue;
    int alpha;
    int i;

    blue = bullet->sprite.colorRgb[2];
    green = bullet->sprite.colorRgb[1];
    red = bullet->sprite.colorRgb[0];
    alpha = bullet->sprite.alpha;

    for (i = 0; i < 4; i++)
    {

        if (*(DWORD *)&bullet->trail[i].x == 0xc479c000)
        {
            break;
        }
        bullet->sprite.scaleX = 3.0f - i * 0.33333334f;
        bullet->sprite.scaleY = 4.0f - i * 0.33333334f;
        bullet->sprite.colorRgb[2] = blue * i / 4 + 0x3f;
        bullet->sprite.colorRgb[1] = green * i / 4 + 0x3f;
        bullet->sprite.alpha = alpha * i / 4 + 0x3f;
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.colorRgb[2] = blue;
    bullet->sprite.colorRgb[1] = green;
    bullet->sprite.colorRgb[0] = red;
    bullet->sprite.alpha = alpha;
    return 0;
}



int HitPlayerShotType2(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    return 1;
}





int InitExAttackType13(ExAttack *attack)
{
    ExAttackData00 *data;

    attack->AllocSprites(1, sizeof(ExAttackData00));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData00 *)attack->data;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x =
        g_Rng.GetRandomF32SignedInRange(128.0f) + g_GameManager.sides[attack->targetSide].player->pos.x;
    data->targetPos.y =
        g_Rng.GetRandomF32SignedInRange(128.0f) + g_GameManager.sides[attack->targetSide].player->pos.y;
    if (data->targetPos.y > 432.0f)
    {
        data->targetPos.y = 432.0f;
    }
    if (data->targetPos.x < -128.0f)
    {
        data->targetPos.x = -128.0f;
    }
    else if (data->targetPos.x > 128.0f)
    {
        data->targetPos.x = 128.0f;
    }
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}






int UpdateExAttackType13(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 180.0f);
        if (attack->timer > 0xb4)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer.HasTickedEvery(2))
        {
            g_GameManager.sides[attack->targetSide].effectManager->SpawnEffect(9, &attack->pos, 1, 0xffffffff);
        }
        if (attack->timer > 0x14)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&attack->pos, 14.0f, NULL);
            }
            if (attack->timer > 300)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
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




int SendComboAttacksCharacter3(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 70 - g_GameManager.rank.unk_b4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(13, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= 70;
    }
    return 0;
}



int DrawEffectType18(Effect *effect)
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






int UpdatePlayerShotType4(Player *player, PlayerBullet *bullet)
{
    bullet->pos = player->pos;
    bullet->pos.y += bullet->velocity.y;
    bullet->velocity.y -= 1.0f;
    if (bullet->timer >= 15)
    {
        player->unk_1cdc = player->unk_1ce0 = 1.0f;
    }
    if (bullet->state == 1)
    {
        float width = bullet->timer.AsFloat() + 80.0f;
        D3DXVECTOR3 pos = bullet->pos;

        pos.y -= 16.0f;
        pos.x -= (width - 96.0f) * 0.5f;
        player->AddDamageBox(&pos, width, 40.0f, 15, 0, 0);
        player->AddHitBox(&pos, width, 30.0f, 0, 0);
    }
    return 0;
}
