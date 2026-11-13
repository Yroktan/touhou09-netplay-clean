

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sound.h"
#include "sprite.h"
#include "unsorted.h"





int UpdateEffectType26(Effect *effect)
{
    float target;
    float delta;

    effect->unk_c9 = 1;
    if (effect->unk_7c < 112.0f)
    {
        effect->unk_7c += 8.0f;
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
    effect->unk_80 = AddNormalizeAngle(effect->unk_80, delta * 0.125f);
    return 1;
}





int CheckFocusAreaCharacter4(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = *pos - player->pos;
    D3DXVECTOR3 center;
    float angle = AddNormalizeAngle(player->unk_368->unk_80, 1.5707964f);
    D3DXVECTOR3 offset;

    SetPolar((Vector2 *)&offset, angle, player->unk_368->unk_7c * 0.5f);
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



struct EffectData34
{

    int state;
    unsigned char pad_4[0x4c - 0x4];

    int unknown4c;

    float unknown50;

    VertexDiffuseXyzrwh fan[0x21];
    VertexDiffuseXyzrwh rings[4][0x20];

    float unknownCe8[4][0x21];
};
ASSERT_SIZE(EffectData34, 0xef8);



int UpdateFocusedEnemyCharacter4(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = g_Rng.GetRandomF32SignedInRange(0.1f);
    enemy->velocity2.y = enemy->unk_5424 > 0x78 ? -1.5f : enemy->unk_5424.AsFloat() * (-1.5f / 120.0f);
    enemy->velocity2.z = 0.0f;
    enemy->unk_5424++;
    return 0;
}



struct ExAttackData14
{
    ExAttackData00 base;

    int unknown4c;

    float radius;
    VertexDiffuseXyzrwh fan[0x21];
    VertexDiffuseXyzrwh rings[4][0x21];

    float offsets[4][0x21];
};
ASSERT_SIZE(ExAttackData14, 0xf48);




int InitExAttackType14(ExAttack *attack)
{
    ExAttackData14 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData14));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData14 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(0.19634955f) + 1.5707964f;
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
    data->base.targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->radius = 4.0f;
    return 0;
}







int UpdateExAttackTypes14And22(ExAttack *attack)
{
    ExAttackData14 *data = (ExAttackData14 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);
    D3DXVECTOR3 diff;
    float t;
    float angle;
    int i;
    int j;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        diff = g_GameManager.sides[attack->targetSide].player->pos - attack->pos;
        attack->pos += data->base.velocity;
        if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
        {
            return 1;
        }
        if (attack->pos.y > 480.0f)
        {
            return 1;
        }
        if (diff.x * diff.x + diff.y * diff.y < 9216.0f)
        {
            g_SoundPlayer.QueueSound(0x10, attack->targetSide != 0 ? 500 : -500);
            attack->sprites->shown = 0;
            attack->sprites->scriptPos = NULL;
            attack->sprites->color = 0xffffffff;
            data->base.state++;
            attack->timer = 0;
            return 0;
        }
        break;
    case 2:
        t = 1.0f - attack->timer.AsFloat() * 0.020833334f;
        data->radius = (1.0f - t * t) * 64.0f;
        if (attack->timer > 1)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->base.state++;
                attack->timer = 0;
            }
            else
            {
                g_GameManager.sides[attack->targetSide].player->unk_36c.AddCircle(&attack->pos,
                                                                                      data->radius * 0.8f, NULL);
            }
            if (attack->timer >= 30)
            {
                data->base.state++;
                attack->timer = 0;
                return 0;
            }
        }
        break;
    case 3:
        angle = -3.1415927f;
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 0x20; j++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                data->rings[i][0].pos.z = 0.0f;
                data->offsets[i][j] = g_Rng.GetRandomF32SignedInRange(attack->timer.AsFloat());
                angle += 0.2026834f;
            }
            angle += 0.05067085f;
        }
        if (attack->timer >= 30)
        {
            return 1;
        }
        break;
    }
    return 0;
}





int DrawExAttackTypes14And22(ExAttack *attack)
{
    ExAttackData14 *data = (ExAttackData14 *)attack->data;

    if (data->base.state >= 2)
    {
        float angle = -3.1415927f;
        VertexDiffuseXyzrwh *vtx;
        BYTE alpha;
        int i;
        int j;

        AddNormalizeAngle(angle, -1.5707964f);
        vtx = data->fan;
        if (data->base.state == 2)
        {
            alpha = 0xff;
        }
        else
        {
            alpha = 0xff - attack->timer.AsFrames() * 0xff / 30;
        }
        vtx->pos = attack->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = (alpha / 2 << 24) | 0xffffff;
        vtx++;
        for (i = 0; i < 0x20; i++)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, data->radius);
            vtx->pos += attack->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = (alpha << 24) | 0xff4040;
            vtx++;
            angle += 0.2026834f;
        }
        attack->sprites->color = 0xffffffff;
        g_SpriteEngine->DrawColoredFan(attack->sprites, data->fan, 0x21);
        g_SpriteEngine->FlushQuadBatch();
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        for (i = 0; i < 4; i++)
        {
            vtx = data->rings[i];
            for (j = 0; j < 0x20; j++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, data->offsets[i][j] + data->radius);
                vtx->pos += attack->pos;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->diffuse = (alpha << 24) | 0xffffff;
                vtx->w = 1.0f;
                vtx++;
                angle += 0.2026834f;
            }
            g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
            g_App.d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 0x1f, data->rings[i], sizeof(VertexDiffuseXyzrwh));
            if (data->base.state == 1)
            {
                break;
            }
            angle += 0.05067085f;
        }
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearSpriteData();
        g_SpriteEngine->ClearTexture();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearZWrite();
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    }
    data->unknown4c = 0;
    return 0;
}




int SendComboAttacksCharacter4(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= (28 - g_GameManager.rank.unk_b4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(14, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= (28 - g_GameManager.rank.unk_b4) * 5;
    }
    return 0;
}




int InitExAttackType22(ExAttack *attack)
{
    ExAttackData14 *data;

    attack->AllocSprites(1, sizeof(ExAttackData14));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    D3DXVECTOR3 target = *attack->target;
    data = (ExAttackData14 *)attack->data;
    data->base.velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->base.velocity, target.z, 2.0f);
    data->base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->base.start.z = 0.0f;
    data->base.targetPos.x = target.x;
    data->base.targetPos.y = target.y;
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    data->radius = 4.0f;
    return 0;
}




int UpdatePlayerShotType5(Player *player, PlayerBullet *bullet)
{
    if (bullet->unk_470 == 1)
    {
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        if (bullet->timer >= 90)
        {
            bullet->state = 0;
        }
    }
    bullet->unk_472 = 1;
    return 0;
}





int DrawEffectType26(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = 2.0943952f / (effect->unk_8c / 2 - 1);
        float angle = AddNormalizeAngle(effect->unk_80, 0.5235988f);
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
        SetPolar((Vector2 *)&offset, across, effect->unk_7c * 0.5f);
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
        angle = AddNormalizeAngle(angle, 1.0471976f - delta);
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




int InitEffectType34(Effect *effect)
{
    EffectData34 *data;

    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(sizeof(EffectData34), "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    memset(effect->vertices, 0, sizeof(EffectData34));
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
    data = (EffectData34 *)effect->vertices;
    data->unknown50 = 0.0f;
    data->state = 0;
    return 0;
}





int UpdateEffectType34(Effect *effect)
{
    EffectData34 *data = (EffectData34 *)effect->vertices;
    float t;
    int i;
    int j;

    if (data->state == 0)
    {
        t = 1.0f - effect->timer.AsFloat() * 0.02f;
        data->unknown50 = (1.0f - t * t) * 64.0f;
        if (effect->timer >= 50)
        {
            data->state = 1;
            effect->timer = 0;
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 0x20; j++)
            {
                data->unknownCe8[i][j] = g_Rng.GetRandomF32SignedInRange(effect->timer.AsFloat());
            }
        }
        if (effect->timer >= 30)
        {
            return 0;
        }
    }
    return 1;
}




int DrawEffectType34(Effect *effect)
{
    EffectData34 *data = (EffectData34 *)effect->vertices;
    float angle = -3.1415927f;
    VertexDiffuseXyzrwh *vtx;
    BYTE alpha;
    int i;
    int j;

    AddNormalizeAngle(angle, -1.5707964f);
    vtx = data->fan;
    if (data->state == 0)
    {
        alpha = 0xff;
    }
    else
    {
        alpha = 0xff - effect->timer.AsFrames() * 0xff / 50;
    }
    vtx->pos = effect->pos;
    vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
    vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
    vtx->diffuse = (alpha / 4 << 24) | 0xff0000;
    vtx->w = 1.0f;
    vtx++;
    for (i = 0; i < 0x20; i++)
    {
        if (angle >= 3.1415927f)
        {
            angle -= 6.2831855f;
        }
        vtx->pos.z = 0.0f;
        SetPolar((Vector2 *)&vtx->pos, angle, data->unknown50);
        vtx->pos += effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->diffuse = (alpha << 24) | 0xff0000;
        vtx->w = 1.0f;
        vtx++;
        angle += 0.2026834f;
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, data->fan, 0x21);
    g_SpriteEngine->FlushQuadBatch();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    for (i = 0; i < 4; i++)
    {
        vtx = data->rings[i];
        for (j = 0; j < 0x20; j++)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, data->unknownCe8[i][j] + data->unknown50);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->diffuse = (alpha << 24) | 0xffffff;
            vtx->w = 1.0f;
            vtx++;
            angle += 0.2026834f;
        }
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        g_App.d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 0x1f, data->rings[i], sizeof(VertexDiffuseXyzrwh));
        if (data->state == 0)
        {
            break;
        }
        angle += 0.05067085f;
    }
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    data->unknown4c = 0;
    return 1;
}
