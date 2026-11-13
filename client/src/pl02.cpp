

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"




int InitEffectType16(Effect *effect)
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
    effect->unk_84 = 0.01f;
    return 0;
}




int UpdateEffectType16(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->timer >= 0x1e)
    {
        if (effect->unk_84 < 0.08f)
        {
            effect->unk_84 += 0.001f;
        }
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}




int DrawEffectType16(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = 3.1415927f / (effect->unk_8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = 0.0f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.y -= 432.0f;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        for (i = effect->unk_8c; i > 0; i--)
        {
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, 448.0f);
            vtx->pos.x *= effect->unk_84;
            vtx->pos += effect->pos;
            vtx->pos.y -= 432.0f;
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






int FirePlayerShotType1(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int i;

    if (frame != sd->frame)
    {
        return 0;
    }
    player->unk_3031c.timer = 30;
    player->unk_3031c.bullet = bullet;
    bullet->unk_468 = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->unk_448 = sd->offsetY;
    bullet->unk_46a = 0xf;
    FireShot(player, bullet, sd);
    bullet->type = 2;
    for (i = 0x1f; i >= 0; i--)
    {
        bullet->trail[i].x = -999.0f;
    }
    bullet->pos.x = -999.0f;
    return 1;
}






int UpdatePlayerShotType2(Player *player, PlayerBullet *bullet)
{
    int i;

    if (player->unk_3031c.timer <= 0)
    {
        player->unk_3031c.timer = 0;
        player->unk_3031c.bullet = NULL;
        bullet->state = 0;
        return 1;
    }
    bullet->pos = player->pos;
    bullet->pos.x += bullet->velocity.z;
    bullet->pos.z = 0.44f;
    bullet->size.y = bullet->pos.y;
    bullet->sprite.scaleY = bullet->pos.y * 0.071428575f;
    bullet->pos.y *= 0.5f;
    for (i = 0x1f; i > 0; i--)
    {
        bullet->trail[i] = bullet->trail[i - 1];
        bullet->trail[i].y -= 1.0f;
    }
    for (i = 0; i < bullet->unk_46a; i++)
    {
        if (bullet->trail[i * 2].x >= -900.0f)
        {
            player->AddKind4Box(&bullet->trail[i * 2], 4.0f, 448.0f, 1, 1, 0);
        }
    }
    bullet->trail[0] = bullet->pos;
    player->unk_3031c.timer--;
    return 0;
}




int DrawPlayerShotType1(Player *player, PlayerBullet *bullet)
{
    int alpha = bullet->sprite.alpha;
    int i;

    for (i = 0; i < bullet->unk_46a * 2; i++)
    {

        if (*(DWORD *)&bullet->trail[i].x == 0xc479c000)
        {
            break;
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->trail[i].x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->trail[i].y);
        bullet->sprite.pos.z = 0.0f;
        bullet->sprite.alpha = alpha - alpha * i / (bullet->unk_46a * 2);
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.alpha = alpha;
    return 0;
}





int UpdateEffectType17(Effect *effect)
{
    float target;
    float delta;

    effect->unk_c9 = 1;
    if (effect->unk_84 < 1.5707964f)
    {
        effect->unk_84 += 0.05235988f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    target = AddNormalizeAngle(effect->mgr->side->player->unk_1cd8, 3.1415927f);
    delta = target - effect->unk_80;
    if (target > effect->unk_80)
    {
        if (delta > 3.1415927f)
        {
            delta -= 6.2831855f;
        }
    }
    else
    {
        if (delta < -3.1415927f)
        {
            delta += 6.2831855f;
        }
    }
    effect->unk_80 = AddNormalizeAngle(effect->unk_80, delta * 0.25f);
    return 1;
}



int FirePlayerShotType2(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->angle = sd->angle;
        return 1;
    }
    return 0;
}




int CheckFocusAreaCharacters2And10(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unk_368;
    float angle;

    if (diff.x * diff.x + diff.y * diff.y < circle->unk_7c * circle->unk_7c)
    {
        angle = player->AngleFromPlayer(pos) - circle->unk_80;
        if (Fabs(angle) > 3.1415927f)
        {
            angle = angle > 0.0f ? angle - 6.2831855f : angle + 6.2831855f;
        }
        if (Fabs(angle) <= circle->unk_84 * 0.5f)
        {
            return 1;
        }
    }
    return 0;
}



int UpdateFocusedEnemyCharacter2(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = 0.0f;
    enemy->velocity2.y = 0.0f;
    enemy->velocity2.z = 0.0f;
    enemy->unk_5424++;
    return 0;
}


struct ExAttackData03
{
    int state;
    D3DXVECTOR3 unknown04;

    D3DXVECTOR3 trail[0x20];
};
ASSERT_SIZE(ExAttackData03, 0x190);





int InitExAttackType3(ExAttack *attack)
{
    ExAttackData03 *data;
    D3DXVECTOR3 targetPos;
    D3DXVECTOR3 pos;
    float angle;
    int i;

    attack->AllocSprites(5, sizeof(ExAttackData03));
    data = (ExAttackData03 *)attack->data;
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[0], 9);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[1], 10);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[2], 11);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[3], 12);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[4], 13);
    g_App.SetCamera(attack->targetSide);
    targetPos.x = g_GameManager.ArenaToScreenX(g_GameManager.sides[attack->targetSide].player->pos.x);
    targetPos.y = g_GameManager.ArenaToScreenY(g_GameManager.sides[attack->targetSide].player->pos.y);
    g_App.SetCamera(attack->side);
    pos.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    pos.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    pos.z = 0.0f;
    attack->pos = pos;
    attack->layer = 2;
    angle = Atan2(targetPos.y - pos.y, targetPos.x - pos.x);
    attack->angle = angle;
    SetPolar((Vector2 *)&data->unknown04, angle, 3.5f);
    data->state = 0;
    g_GameManager.exAttackManager->AddAttack(4, &pos, attack->side, (D3DXVECTOR3 *)attack);
    for (i = 0; i < 0x20; i++)
    {
        data->trail[i] = attack->pos;
    }
    return 0;
}







int UpdateExAttackType3(ExAttack *attack)
{
    ExAttackData03 *data = (ExAttackData03 *)attack->data;
    D3DXVECTOR3 pos0;
    D3DXVECTOR3 pos1;
    D3DXVECTOR3 pos2;
    int i;

    switch (data->state)
    {
    case 0:
    {
        for (i = 0x1f; i > 0; i--)
        {
            data->trail[i] = data->trail[i - 1];
        }
        data->trail[0] = attack->pos;
        attack->sprites[1].posOffset = data->trail[6] - attack->pos;
        attack->sprites[2].posOffset = data->trail[12] - attack->pos;
        attack->sprites[3].posOffset = data->trail[18] - attack->pos;
        attack->sprites[4].posOffset = data->trail[24] - attack->pos;
        D3DXVECTOR3 size;
        float *s = size;
        s[0] = 12.0f;
        s[1] = 12.0f;
        s[2] = 0.0f;
        g_App.SetCamera(attack->targetSide);
        pos0.x = g_GameManager.ScreenToArenaX(attack->pos.x);
        pos0.y = g_GameManager.ScreenToArenaY(attack->pos.y);
        pos0.z = 0.0f;
        pos1.x = g_GameManager.ScreenToArenaX(data->trail[12].x);
        pos1.y = g_GameManager.ScreenToArenaY(data->trail[12].y);
        pos1.z = 0.0f;
        pos2.x = g_GameManager.ScreenToArenaX(data->trail[24].x);
        pos2.y = g_GameManager.ScreenToArenaY(data->trail[24].y);
        pos2.z = 0.0f;
        if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos0, &size, NULL) == 2 ||
            g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos1, &size, NULL) == 2 ||
            g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos2, &size, NULL) == 2)
        {
            attack->timer = 0;
            data->state = 1;
            attack->sprites[0].SetPendingInterrupt(1);
            attack->sprites[1].SetPendingInterrupt(1);
            attack->sprites[2].SetPendingInterrupt(1);
            attack->sprites[3].SetPendingInterrupt(1);
            attack->sprites[4].SetPendingInterrupt(1);
            break;
        }
        if (attack->timer >= 30)
        {
            attack->pos += data->unknown04;
            if (data->trail[0x1f].x < -32.0f || data->trail[0x1f].x > 672.0f || data->trail[0x1f].y < -32.0f ||
                data->trail[0x1f].y > 512.0f)
            {
                return 1;
            }
        }
        break;
    }
    case 1:
        if (attack->timer >= 15)
        {
            return 1;
        }
        break;
    }
    return 0;
}





int InitExAttackType4(ExAttack *attack)
{
    ExAttackData03 *data;
    ExAttack *parent;
    int i;

    attack->spriteCount = 5;
    attack->sprites = (Sprite *)g_MemoryTracker.Alloc(sizeof(Sprite) * 5, "./system\\global.h");
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[0], 0xe);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[1], 0xf);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[2], 0x10);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[3], 0x11);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[4], 0x12);
    data = (ExAttackData03 *)g_MemoryTracker.Alloc(sizeof(ExAttackData03), "./system\\global.h");
    attack->data = data;
    g_App.SetCamera(attack->targetSide);
    attack->pos.x = g_GameManager.ScreenToArenaX(attack->pos.x);
    attack->pos.y = g_GameManager.ScreenToArenaY(attack->pos.y);
    attack->layer = attack->targetSide;
    parent = (ExAttack *)attack->target;
    attack->angle = parent->angle;
    data->unknown04 = ((ExAttackData03 *)parent->data)->unknown04;
    data->state = 0;
    for (i = 0; i < 0x20; i++)
    {
        data->trail[i] = attack->pos;
    }
    return 0;
}







int UpdateExAttackType4(ExAttack *attack)
{
    ExAttackData03 *data = (ExAttackData03 *)attack->data;
    int i;

    switch (data->state)
    {
    case 0:
    {
        for (i = 0x1f; i > 0; i--)
        {
            data->trail[i] = data->trail[i - 1];
        }
        data->trail[0] = attack->pos;
        attack->sprites[1].posOffset = data->trail[6] - attack->pos;
        attack->sprites[2].posOffset = data->trail[12] - attack->pos;
        attack->sprites[3].posOffset = data->trail[18] - attack->pos;
        attack->sprites[4].posOffset = data->trail[24] - attack->pos;
        ExAttackData03 *parentData = (ExAttackData03 *)((ExAttack *)attack->target)->data;
        D3DXVECTOR3 size;
        float *s = size;
        s[0] = 12.0f;
        s[1] = 12.0f;
        s[2] = 0.0f;
        if (parentData == NULL || parentData->state != 0)
        {
            attack->timer = 0;
            data->state = 1;
            attack->sprites[0].SetPendingInterrupt(1);
            attack->sprites[1].SetPendingInterrupt(1);
            attack->sprites[2].SetPendingInterrupt(1);
            attack->sprites[3].SetPendingInterrupt(1);
            attack->sprites[4].SetPendingInterrupt(1);
            break;
        }
        if (attack->timer >= 30)
        {
            attack->pos += data->unknown04;
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&attack->pos, 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&data->trail[6], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&data->trail[12], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&data->trail[18], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&data->trail[24], 12.0f, NULL);
            if (parentData->trail[24].x < -32.0f || parentData->trail[24].x > 672.0f ||
                parentData->trail[24].y < -32.0f || parentData->trail[24].y > 512.0f)
            {
                return 1;
            }
        }
        break;
    }
    case 1:
        if (attack->timer >= 15)
        {
            return 1;
        }
        break;
    }
    return 0;
}



int FreeExAttackType4(ExAttack *attack)
{
    if (attack->data != NULL)
    {
        g_MemoryTracker.Free(attack->data);
    }
    attack->data = NULL;
    return 0;
}




int SendComboAttacksCharacter2(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (45 - g_GameManager.rank.unk_b4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(3, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (45 - g_GameManager.rank.unk_b4) * 2;
    }
    return 0;
}





int InitEffectType17(Effect *effect)
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
    effect->unk_7c = 160.0f;
    effect->unk_c9 = 1;
    return 0;
}




int DrawEffectType17(Effect *effect)
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





int UpdatePlayerShotType3(Player *player, PlayerBullet *bullet)
{
    float angle;
    float dx;
    float dy;

    if (bullet->state == 1 && bullet->timer.AsFrames() == 30)
    {
        if (player->orbHomingTarget.x >= -144.0f)
        {
            dx = player->orbHomingTarget.x - bullet->pos.x;
            dy = player->orbHomingTarget.y - bullet->pos.y;
            if (dy == 0.0f && dx == 0.0f)
            {
                angle = bullet->angle + 1.5707964f;
            }
            else
            {
                angle = Atan2(dy, dx) + bullet->angle;
            }
        }
        else
        {
            angle = -1.5707964f;
        }
        angle = AddNormalizeAngle(angle, 0.0f);
        bullet->velocity.x = cosf(angle) * bullet->speed;
        bullet->velocity.y = sinf(angle) * bullet->speed;
        bullet->angle = angle;
    }
    return 0;
}
