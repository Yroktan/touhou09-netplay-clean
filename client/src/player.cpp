#include "player.h"
#include "ascii.h"
#include "sound.h"
#include "sprite.h"
#include "bullet.h"
#include "effect.h"
#include "gui.h"
#include "enemy.h"
#include "spellcard.h"




static void ResetTargets(Player *player)
{
    player->orbHomingTarget = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->homingTarget = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->homingTargetSet = 0;
    player->moveScaleX = player->moveScaleY = 1.0f;
}

float Player::AngleFromPlayer(D3DXVECTOR3 *pos)
{
    float dx = pos->x - this->pos[0];
    float dy = pos->y - this->pos[1];

    if (dy == 0.0f && dx == 0.0f)
    {
        return 1.5707964f;
    }
    return Atan2(dy, dx);
}

float Player::AngleToPlayer(D3DXVECTOR3 *pos)
{
    float dx = this->pos[0] - pos->x;
    float dy = this->pos[1] - pos->y;

    if (dy == 0.0f && dx == 0.0f)
    {
        return 1.5707964f;
    }
    return Atan2(dy, dx);
}





static int LoadShotTypeFile(ShtFile **out, char *path)
{
    int i;
    ShotData *shot;
    float diagonal;

    *out = (ShtFile *)LoadFileData(path, NULL, 0);
    if (*out == NULL)
    {
        return -1;
    }
    diagonal = sinf(0.7853982f);
    (*out)->speedDiagonal = diagonal * (*out)->speed;
    (*out)->speedDiagonalFocused = diagonal * (*out)->speedFocused;
    for (i = 0; i < (*out)->tableCount; i++)
    {
        (*out)->tables[i].shots = (ShotData *)((int)(*out)->tables[i].shots + (int)*out);
        shot = (*out)->tables[i].shots;
        while (shot->frame >= 0)
        {
            shot->fire = g_ShotFireFuncs[(int)shot->fire];
            shot->update = g_ShotUpdateFuncs[(int)shot->update];
            shot->draw = g_ShotDrawFuncs[(int)shot->draw];
            shot->hit = g_ShotHitFuncs[(int)shot->hit];
            shot++;
        }
    }
    return 0;
}

void Player::AddUnknown30388(float amount)
{
    GameSide *side;
    int before;
    int after;

    before = (int)this->unk_30388 / 100;
    this->unk_30388 += amount;
    side = this->side;
    if (side->character == 1)
    {
        this->unk_30388 += amount * 0.15f;
    }
    if (this->unk_30388 >= 400.0f)
    {
        this->unk_30388 = 400.0f;
    }
    after = (int)this->unk_30388 / 100;
    if (before != after)
    {
        side->gui->StartUnknown66fc(after);
    }
}

void PlayerHitArea::Reset()
{
    memset(this, 0, sizeof(PlayerHitArea));
    this->damageInterval = 1;
}

void PlayerUnknown24::Reset()
{
    this->timer5c = 0;
    this->timer68 = 0;
    this->flags &= ~3;
}

void Player::SetRegisterChainArg(BYTE arg)
{
    this->registerChainArg = arg;
}

int Player::CheckHitBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    start.x = pos->x - size->x;
    start.y = pos->y - size->y;
    end.x = pos->x + size->x;
    end.y = size->y + pos->y;
    if (this->hitboxMin.x > end.x || this->hitboxMin.y > end.y || this->hitboxMax.x < start.x ||
        this->hitboxMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckHitCircle(D3DXVECTOR3 *pos, float radius)
{
    D3DXVECTOR3 d;
    float r;

    d = this->pos - *pos;
    r = radius + this->shtFile->hitboxSize;
    if (d.x * d.x + d.y * d.y >= r * r)
    {
        return 0;
    }
    return 1;
}

int Player::TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->playerState != 0 && this->playerState != 3)
    {
        return 0;
    }
    start = *pos - *size / 2.0f;
    end = *pos + *size / 2.0f;
    if (this->grazeTestMin.x > end.x || this->grazeTestMax.x < start.x || this->grazeTestMin.y > end.y ||
        this->grazeTestMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 boxMin;
    D3DXVECTOR3 boxMax;

    start = this->pos - *origin;
    Rotate2D(&end, &start, -angle);
    end.z = 0.0f;
    start = end + *origin;
    boxMin = start - this->hitboxSize;
    boxMax = start + this->hitboxSize;
    start = *pos - *size;
    end = *pos + *size;
    if (!(boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y))
    {
        return 1;
    }
    if (!canGraze)
    {
        return 0;
    }
    start.x -= 48.0f;
    start.y -= 48.0f;
    end.x += 48.0f;
    end.y += 48.0f;
    if (boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y)
    {
        return 0;
    }
    return 2;
}

int Player::HandleInput()
{
    int side;
    PlayerInput *input;
    float deltaX;
    float deltaY;
    float *pos;
    int i;

    deltaX = 0.0f;
    deltaY = 0.0f;
    if (this->unk_a8 == 0)
    {
        return 0;
    }
    side = this->sideIdx;
    input = &g_PlayerInputs[side];
    if (input->IsHeld2(0x50) == 0x50)
    {
        this->direction = 5;
    }
    else if (input->IsHeld2(0x60) == 0x60)
    {
        this->direction = 7;
    }
    else if (input->IsHeld2(0x90) == 0x90)
    {
        this->direction = 6;
    }
    else if (input->IsHeld2(0xa0) == 0xa0)
    {
        this->direction = 8;
    }
    else if (input->IsHeld2(0x20))
    {
        this->direction = 2;
    }
    else if (input->IsHeld2(0x10))
    {
        this->direction = 1;
    }
    else if (input->IsHeld2(0x40))
    {
        this->direction = 3;
    }
    else if (input->IsHeld2(0x80))
    {
        this->direction = 4;
    }
    else
    {
        this->direction = 0;
    }
    this->focused = g_GameManager.config->unk_b4[this->sideIdx] == 1 ? g_PlayerInputs[side].unk_2a >= 8
                                                                         : input->IsHeld2(4) != 0;
    if (this->focused)
    {
        if (this->unk_364 == NULL)
        {
            this->unk_364 = this->side->effectManager->SpawnReservedEffect(7, &this->pos, this->sideIdx, 0xffffffff);
            this->unk_368 = this->side->effectManager->SpawnReservedEffect(
                g_CharacterValuesA10[this->side->character], &this->pos, this->sideIdx + 2, 0xffffffff);
        }
        switch (this->direction)
        {
        case 4:
            this->unk_1cd8 = 0.0f;
            deltaX = this->shtFile->speedFocused;
            break;
        case 3:
            this->unk_1cd8 = 3.1415927f;
            deltaX = -this->shtFile->speedFocused;
            break;
        case 1:
            this->unk_1cd8 = -1.5707964f;
            deltaY = -this->shtFile->speedFocused;
            break;
        case 2:
            this->unk_1cd8 = 1.5707964f;
            deltaY = this->shtFile->speedFocused;
            break;
        case 5:
            this->unk_1cd8 = -2.3561945f;
            deltaX = -this->shtFile->speedDiagonalFocused;
            deltaY = deltaX;
            break;
        case 7:
            this->unk_1cd8 = 2.3561945f;
            deltaY = this->shtFile->speedDiagonalFocused;
            deltaX = -deltaY;
            break;
        case 6:
            this->unk_1cd8 = -0.7853982f;
            deltaX = this->shtFile->speedDiagonalFocused;
            deltaY = -deltaX;
            break;
        case 8:
            this->unk_1cd8 = 0.7853982f;
            deltaX = this->shtFile->speedDiagonalFocused;
            deltaY = deltaX;
            break;
        }
    }
    else
    {
        if (this->unk_364 != NULL)
        {
            this->unk_364->sprites->SetPendingInterrupt(1);
            this->unk_364 = NULL;
            this->unk_368->active = 0;
            this->unk_368 = NULL;
        }
        switch (this->direction)
        {
        case 4:
            this->unk_1cd8 = 0.0f;
            deltaX = this->shtFile->speed;
            break;
        case 3:
            this->unk_1cd8 = 3.1415927f;
            deltaX = -this->shtFile->speed;
            break;
        case 1:
            this->unk_1cd8 = -1.5707964f;
            deltaY = -this->shtFile->speed;
            break;
        case 2:
            this->unk_1cd8 = 1.5707964f;
            deltaY = this->shtFile->speed;
            break;
        case 5:
            this->unk_1cd8 = -2.3561945f;
            deltaX = -this->shtFile->speedDiagonal;
            deltaY = deltaX;
            break;
        case 7:
            this->unk_1cd8 = 2.3561945f;
            deltaY = this->shtFile->speedDiagonal;
            deltaX = -deltaY;
            break;
        case 6:
            this->unk_1cd8 = -0.7853982f;
            deltaX = this->shtFile->speedDiagonal;
            deltaY = -deltaX;
            break;
        case 8:
            this->unk_1cd8 = 0.7853982f;
            deltaX = this->shtFile->speedDiagonal;
            deltaY = deltaX;
            break;
        }
    }
    deltaX *= this->moveScaleX * this->unk_1cdc;
    deltaY *= this->moveScaleY * this->unk_1ce0;
    if (deltaX < 0.0f && this->velocityX >= 0.0f)
    {
        this->anm->StartScript(&this->sprite, 1);
    }
    else if (deltaX == 0.0f && this->velocityX < 0.0f)
    {
        this->anm->StartScript(&this->sprite, 2);
    }
    if (deltaX > 0.0f && this->velocityX <= 0.0f)
    {
        this->anm->StartScript(&this->sprite, 3);
    }
    else if (deltaX == 0.0f && this->velocityX > 0.0f)
    {
        this->anm->StartScript(&this->sprite, 4);
    }
    this->velocityX = deltaX;
    this->velocityY = deltaY;
    this->unk_1ccc.x = deltaX * g_App.gameSpeed;
    this->unk_1ccc.y = deltaY * g_App.gameSpeed;
    pos = this->pos;
    pos[0] += this->unk_1ccc.x;
    pos[1] += this->unk_1ccc.y;
    if (pos[0] < g_GameManager.playerArea[0])
    {
        pos[0] = g_GameManager.playerArea[0];
    }
    else if (pos[0] > g_GameManager.playerArea[0] + g_GameManager.playerArea[2])
    {
        pos[0] = g_GameManager.playerArea[0] + g_GameManager.playerArea[2];
    }
    if (pos[1] < g_GameManager.playerArea[1])
    {
        pos[1] = g_GameManager.playerArea[1];
    }
    else if (pos[1] > g_GameManager.playerArea[1] + g_GameManager.playerArea[3])
    {
        pos[1] = g_GameManager.playerArea[1] + g_GameManager.playerArea[3];
    }
    this->hitboxMin = this->pos - this->hitboxSize;
    this->hitboxMax = this->pos + this->hitboxSize;
    this->grazeMin = this->pos - this->grazeSize;
    this->grazeMax = this->pos + this->grazeSize;
    this->grazeTestMin = this->pos - this->grazeTestHalfSize;
    this->grazeTestMax = this->pos + this->grazeTestHalfSize;
    for (i = 0; i < 4; i++)
    {
        if (this->options[i].update != NULL)
        {
            this->options[i].update(this, &this->options[i]);
            g_SpriteEngine->Step(&this->options[i].sprite);
            this->options[i].timer++;
        }
    }
    if (deltaY != 0.0f || deltaX != 0.0f)
    {
        for (i = 15; i > 0; i--)
        {
            this->posHistory[i] = this->posHistory[i - 1];
        }
        this->posHistory[0] = this->pos;
    }
    return 0;
}





static void UpdateHitAreas(Player *player)
{
    int i;
    PlayerHitArea **area;
    PlayerHitArea **next;
    PlayerHitArea *spare;

    area = player->activeHitAreas;
    for (i = 0; i < player->activeHitAreaCount; i++)
    {
        if ((*area)->unk_40 > 0)
        {
            (*area)->unk_40--;
        }
        else
        {
            (*area)->framesLeft--;
            (*area)->radius += (*area)->radiusGrowth;
            (*area)->width += (*area)->widthGrowth;
            (*area)->height += (*area)->heightGrowth;
            if ((*area)->framesLeft <= 0)
            {
                (*area)->Deactivate();
                player->freeHitAreas[player->freeHitAreaCount] = *area;
                player->freeHitAreaCount++;
                for (next = area; (*next = next[1]) != NULL; next++)
                {
                }
                player->activeHitAreaCount--;
                i--;
                continue;
            }
        }
        area++;
    }
    spare = &player->hitAreas[0x200];
    if (player->hitAreas[0x200].active)
    {
        if (spare->unk_40 > 0)
        {
            spare->unk_40--;
        }
        else
        {
            spare->framesLeft--;
            spare->radius += spare->radiusGrowth;
            spare->width += spare->widthGrowth;
            spare->height += spare->heightGrowth;
            if (spare->framesLeft <= 0)
            {
                spare->Deactivate();
            }
            else
            {
                player->activeHitAreas[player->activeHitAreaCount] = spare;
            }
        }
    }
    else
    {
        player->activeHitAreas[player->activeHitAreaCount] = NULL;
    }
}





static void UpdateSpellCardInput(Player *player)
{
    if (g_GameManager.gui->unk_1095c != 0)
    {
        return;
    }
    if (player->GetPlayerState() == 4)
    {
        return;
    }
    if (!g_PlayerInputs[player->sideIdx].IsHeld2(2))
    {
        return;
    }
    if (player->unk_30388 >= 400.0f)
    {
        if (player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            player->otherSide->spellCard->Start(2, 2, 0, player->shtFile->spellCardNames[2]);
            player->unk_30384 = 0.0f;
            player->unk_30388 = 0.0f;
            player->StartClearCircle3();
            return;
        }
        player->otherSide->spellCard->Start(1, 1, 0, player->shtFile->spellCardNames[1]);
        player->unk_30384 = 100.0f;
        player->unk_30388 = 100.0f;
        player->StartClearCircle2();
        return;
    }
    if (player->unk_30388 >= 300.0f)
    {
        player->otherSide->spellCard->Start(1, 1, 0, player->shtFile->spellCardNames[1]);
        player->unk_30384 = 0.0f;
        player->unk_30388 = 0.0f;
        player->StartClearCircle2();
        return;
    }
    if (player->unk_30388 >= 200.0f)
    {
        player->otherSide->spellCard->Start(0, 0, 0, player->shtFile->spellCardNames[0]);
        player->unk_30384 = 0.0f;
        player->unk_30388 = 0.0f;
        player->StartClearCircle1();
    }
}




static void UpdateRespawningState(Player *player)
{
    float t;

    player->unk_3031c.unk_18 = 60;
    t = 1.0f - player->stateTimer.AsFloat() * (1.0f / 30.0f);
    player->sprite.scaleY = t + t + 1.0f;
    player->sprite.scaleX = 1.0f - t;
    player->sprite.SetBlendModeAdd();
    player->unk_1cdc = player->unk_1ce0 = 1.0f;
    player->sprite.color = (player->stateTimer.AsFrames() * 255 / 30) << 24 | 0xffffff;
    player->unk_3031c.unknown10 = 0;
    if (player->sideIdx == 0)
    {
        player->pos.x = t * -176.0f;
    }
    else
    {
        player->pos.x = t * 176.0f;
    }
    player->pos.y = t * 64.0f + 384.0f;
    if (player->stateTimer.AsFrames() >= 30)
    {
        player->SetPlayerState(3);
        player->sprite.scaleY = player->sprite.scaleX = 1.0f;
        player->sprite.color = 0xffffffff;
        player->sprite.SetBlendModeNormal();
        player->pos.x = 0.0f;
        player->pos.y = 384.0f;
    }
}





static void UpdateInvulnerability(Player *player)
{
    if (player->unk_3031c.unk_18 != 0)
    {
        player->unk_3031c.unk_18--;
    }
    if (player->GetPlayerState() == 3)
    {
        if (player->unk_303e0 != NULL)
        {
            player->unk_303e0->pos = player->pos;
        }
        player->stateTimer--;
        if (player->stateTimer.AsFrames() <= 0)
        {
            if (player->unk_303e0 != NULL)
            {
                player->unk_303e0->active = 0;
                player->unk_303e0 = NULL;
            }
            player->SetPlayerState(0);
            player->stateTimer = 0;
            player->sprite.color = 0xffffffff;
        }
        else
        {
            player->sprite.color = player->stateTimer.AsFrames() % 8 < 2 ? 0xfff02020 : 0xffffffff;
        }
    }
    else
    {
        player->stateTimer++;
    }
    if (player->unk_1b74 > 0)
    {
        if (player->stateTimer.AsFrames() % 4 < 2)
        {
            player->sprite.color = 0xfff02020;
        }
        else
        {
            player->sprite.color = 0xffffffff;
        }
        player->unk_1b74--;
        if (player->unk_1b74 <= 0)
        {
            player->unk_1b74 = 0;
            player->sprite.color = 0xffffffff;
        }
    }
}

int Player::OnDraw2(Player *player)
{
    g_App.SetView(player->sideIdx);
    player->DrawHitBullets();
    return 1;
}

int Player::LoadAnms()
{
    int i;
    SpriteEngine *se = g_SpriteEngine;

    for (i = 0; i < 2; i++)
    {
        if (se->GetAnmFile(i + 5) != NULL &&
            g_GameManager.sides[i].character == g_GameManager.sides[i].unk_24)
        {
            continue;
        }
        if (se->PreloadAnm(i + 5, g_CharacterFiles[g_GameManager.sides[i].character]
                                                  [g_GameManager.sides[i].unk_30 != 0 ? 5 : 1]) == NULL)
        {
            return -1;
        }
        se = g_SpriteEngine;
    }
    return 0;
}



static int FreeResources(Player *player)
{
    if (FreesResources())
    {
        g_SpriteEngine->ReleaseAnm(player->sideIdx + 5);
        if (player->shtFile != NULL)
        {
            g_MemoryTracker.Free(player->shtFile);
            player->shtFile = NULL;
            g_ShtFiles[player->sideIdx] = NULL;
        }
    }
    return 0;
}

void Player::RemoveChain(Player *player)
{
    if (player != NULL)
    {
        FreeResources(player);
        g_Chain.Remove(player->drawElem);
        player->drawElem = NULL;
        g_Chain.Remove(player->drawElem2);
        player->drawElem2 = NULL;
        g_Chain.Remove(player->calcElem);
        player->calcElem = NULL;
        delete player;
    }
}

PlayerHitArea *Player::AllocHitArea(D3DXVECTOR3 *pos, int framesLeft, int unk_40)
{
    PlayerHitArea *area;

    if (this->freeHitAreaCount <= 1)
    {
        area = &this->hitAreas[0x200];
    }
    else
    {
        this->freeHitAreaCount--;
        area = this->freeHitAreas[this->freeHitAreaCount];
        this->freeHitAreas[this->freeHitAreaCount] = NULL;
        this->activeHitAreas[this->activeHitAreaCount] = area;
        this->activeHitAreaCount++;
    }
    area->Reset();
    area->active = 1;
    area->x = pos->x;
    area->y = pos->y;
    area->framesLeft = framesLeft;
    area->unk_40 = unk_40;
    return area;
}

PlayerHitArea *Player::AddHitBox(D3DXVECTOR3 *pos, float width, float height, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->kind = 1;
    return area;
}

PlayerHitArea *Player::AddHitCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->kind = 1;
    return area;
}

PlayerHitArea *Player::AddDamageBox(D3DXVECTOR3 *pos, float width, float height, int unk_28, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->damage = unk_28;
    area->kind = 0;
    return area;
}

PlayerHitArea *Player::AddDamageCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->damage = unk_28;
    area->kind = 0;
    return area;
}

PlayerHitArea *Player::AddKind4Box(D3DXVECTOR3 *pos, float width, float height, int unk_28, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->damage = unk_28;
    area->kind = 4;
    return area;
}

PlayerHitArea *Player::AddKind4Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->damage = unk_28;
    area->kind = 4;
    return area;
}

PlayerHitArea *Player::AddKind2Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unk_40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->damage = unk_28;
    area->kind = 2;
    return area;
}

void PlayerUnknown30410::AddCombo(D3DXVECTOR3 *pos, int a, int b, int c, int d)
{
    int prev = this->unk_c;
    int value;
    D3DXVECTOR3 effectPos;
    D3DXVECTOR3 effectVelocity;
    Effect *effect;

    if (g_GameManager.gui->GetUnknown11ea8())
    {
        return;
    }
    this->unk_3c = 0;
    this->count++;
    if (this->maxCount < this->count)
    {
        this->maxCount = this->count;
    }
    this->unk_c += g_GameManager.rank.unk_b4 * d / 10 + d;
    if (this->unk_c % 10 != 0)
    {
        this->unk_c = this->unk_c - this->unk_c % 10 + 10;
    }
    if (this->unk_c > 999990)
    {
        this->unk_c = 999990;
        this->player->side->AddScore(5000);
    }
    if (this->unk_c >= 500000 && prev < 500000)
    {
        this->player->otherSide->spellCard->Start(1, 1, 0, this->player->shtFile->spellCardNames[1]);
        if (this->player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
        }
    }
    else if (this->unk_c >= 300000 && prev < 300000)
    {
        if (this->player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
        }
    }
    else if (this->unk_c >= 100000 && prev < 100000)
    {
        this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
    }
    value = this->unk_c >= 999990 ? -1 : this->unk_c;
    if (this->count < 10)
    {
        if (this->count % 2 == 0)
        {
            g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                                 this->unk_c >= 300000   ? 0xffffff00
                                 : this->unk_c >= 100000 ? 0xffffffc0
                                                            : 0xffffffff);
        }
    }
    else if (this->count < 30)
    {
        if (this->count % 5 == 0)
        {
            g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                                 this->unk_c >= 300000   ? 0xffffff00
                                 : this->unk_c >= 100000 ? 0xffffffc0
                                                            : 0xffffffff);
        }
    }
    else if (this->count % 10 == 0)
    {
        g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                             this->unk_c >= 300000   ? 0xffffff00
                             : this->unk_c >= 100000 ? 0xffffffc0
                                                        : 0xffffffff);
    }
    if (a != 0)
    {
        if (g_GameManager.difficulty == 2)
        {
            a = a * 5 / 4;
        }
        else if (g_GameManager.difficulty == 3)
        {
            a = a * 4 / 3;
        }
        this->unk_2c += a;
        this->unk_30 += b;
        this->unk_34 += c;
        this->func(this, pos);
        while (this->unk_30 >= 60 - g_GameManager.rank.unk_b4 * 3 / 2)
        {
            if (this->player->otherSide->enemyManager->unk_2ac3b8 < 25)
            {
                effectPos.x = g_GameManager.ArenaToScreenX(pos->x) + g_Rng.GetRandomF32SignedInRange(32.0f);
                effectPos.y = g_GameManager.ArenaToScreenY(pos->y) + g_Rng.GetRandomF32SignedInRange(32.0f);
                effectPos.z = 0.0f;
                g_App.SetCamera(1 - this->player->sideIdx);
                effectVelocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
                effectVelocity.y = g_Rng.GetRandomF32InRange(128.0f);
                effectVelocity.z = 0.0f;
                effect = g_GameManager.sides[2].effectManager->SpawnEffectWithVelocity(
                    this->player->sideIdx + 3, &effectPos, &effectVelocity, 1, 0xffffffff);
                g_App.SetCamera(this->player->sideIdx);
                effect->unk_a4 = 0;
                effect->unk_a6 = 1;
                effect->unk_a0 =
                    g_Rng.GetRandomF32InRange(0.5f) * 0.016666668f + g_GameManager.rank.unk_b4 * 0.0016666667f;
                effect->unk_a8 = this->player->sideIdx;
            }
            this->unk_30 += g_GameManager.rank.unk_b4 * 2 - 60;
            this->unk_2c -= 10;
            if (this->unk_2c < 0)
            {
                this->unk_2c = 0;
            }
        }
        while (this->unk_2c >= (30 - g_GameManager.rank.unk_b4) * 4)
        {
            effectPos.x = g_GameManager.ArenaToScreenX(pos->x) + g_Rng.GetRandomF32SignedInRange(32.0f);
            effectPos.y = g_GameManager.ArenaToScreenY(pos->y) + g_Rng.GetRandomF32SignedInRange(32.0f);
            effectPos.z = 0.0f;
            g_App.SetCamera(1 - this->player->sideIdx);
            effectVelocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
            effectVelocity.y = g_Rng.GetRandomF32InRange(128.0f);
            effectVelocity.z = 0.0f;
            effect = g_GameManager.sides[2].effectManager->SpawnEffectWithVelocity(
                this->player->sideIdx + 1, &effectPos, &effectVelocity, 1, 0xffffffff);
            g_App.SetCamera(this->player->sideIdx);
            effect->unk_a4 = 0;
            effect->unk_a6 = 4;
            switch (g_GameManager.difficulty)
            {
            case 0:
                effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.04f + 0.9f;
                break;
            case 1:
                effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.05f + 1.1f;
                break;
            case 2:
                effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.08f + 1.3f;
                break;
            case 3:
                effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.11f + 1.3f;
                break;
            case 4:
                effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.04f + 1.6f;
                break;
            }
            effect->unk_a8 = this->player->sideIdx;
            this->unk_2c += g_GameManager.rank.unk_b4 * 4 - 120;
        }
    }
    if (this->timer20 < 30)
    {
        this->timer20 += 30;
    }
    else if (this->timer20 < 50)
    {
        this->timer20 += 5;
    }
    else if (this->timer20 < 60)
    {
        this->timer20 += 2;
    }
    if (this->bonus <= this->unk_c)
    {
        this->bonus = this->unk_c;
        this->timer14 = this->timer20.AsFrames() + 45;
    }
}

void PlayerUnknown30410::Update()
{
    if (this->timer20 > 0)
    {
        this->unk_3c = 0;
        this->timer20--;
        if (this->timer20 <= 0)
        {
            this->timer20 = 0;
            this->count = 0;
            this->unk_38 = 0;
        }
    }
    if (this->timer14 > 0)
    {
        this->timer14--;
        if (this->timer14 <= 0)
        {
            this->player->side->AddScore(this->bonus);
            this->bonus = 0;
            this->unk_c = 0;
            this->timer14 = 0;
        }
    }
}

void PlayerUnknown30410::EndCombo()
{
    this->ResetCombo();
    this->player->side->AddScore(this->bonus);
    this->bonus = 0;
    this->unk_c = 0;
    this->timer14 = 0;
}

PlayerUnknown36cEntry *PlayerUnknown36c::CheckHitsAt(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, float margin)
{
    Player *player = this->player;
    PlayerUnknown36cEntry *entry;
    int i;
    D3DXVECTOR3 savedPos;
    D3DXVECTOR3 savedSize;

    if (player->GetPlayerState() != 0)
    {
        return NULL;
    }
    if (player->unk_1b74 > 0)
    {
        return NULL;
    }
    entry = this->entries;
    savedPos = player->pos;
    savedSize = player->hitboxSize;
    player->pos = *pos;
    this->player->hitboxMin = *pos - *size;
    this->player->hitboxMax = *pos + *size;
    this->player->hitboxSize = *size;
    for (i = 0; i < this->count; i++, entry++)
    {
        if (entry->radius == 0.0f)
        {
            if (entry->angle == 0.0f)
            {
                if (this->player->CheckHitBox(&entry->pos, &entry->size))
                {
                    this->player->pos = savedPos;
                    this->player->hitboxSize = savedSize;
                    return entry;
                }
            }
            else if (this->player->CheckLaserHit(&entry->pos, &entry->size, &entry->unk_c, entry->angle, 0))
            {
                this->player->pos = savedPos;
                this->player->hitboxSize = savedSize;
                return entry;
            }
        }
        else if (this->player->CheckHitCircle(&entry->pos, margin + entry->radius))
        {
            this->player->pos = savedPos;
            this->player->hitboxSize = savedSize;
            return entry;
        }
    }
    this->player->pos = savedPos;
    this->player->hitboxSize = savedSize;
    return NULL;
}

PlayerUnknown36cEntry *PlayerUnknown36c::CheckHits()
{
    Player *player = this->player;
    PlayerUnknown36cEntry *entry;
    int i;
    int hit;

    if (player->GetPlayerState() != 0)
    {
        return NULL;
    }
    if (player->unk_1b74 > 0)
    {
        return NULL;
    }
    entry = this->entries;
    player->hitboxMin = player->pos - player->hitboxSize;
    this->player->hitboxMax = this->player->pos + this->player->hitboxSize;
    for (i = 0; i < this->count; i++, entry++)
    {
        if (entry->radius == 0.0f)
        {
            if (entry->angle == 0.0f)
            {
                hit = this->player->CheckHitBox(&entry->pos, &entry->size);
            }
            else
            {
                hit = this->player->CheckLaserHit(&entry->pos, &entry->size, &entry->unk_c, entry->angle, 0);
            }
        }
        else
        {
            hit = this->player->CheckHitCircle(&entry->pos, entry->radius);
        }
        if (hit)
        {
            return entry;
        }
    }
    return NULL;
}

void Player::AddUnknown30454(int script, D3DXVECTOR3 *pos)
{
    PlayerUnknown30454 *entry = this->unk_30454;
    int i;

    if (g_GameManager.gui->unk_1095c != 0)
    {
        return;
    }
    for (i = 0; i < 4; i++, entry++)
    {
        if (entry->unk_1c == 0)
        {
            entry->script = script;
            entry->pos = *pos;
            entry->velocity.x = 0.0f;
            entry->velocity.y = -1.5f;
            entry->velocity.z = 0.0f;
            entry->unk_1c = 1;
            this->side->bulletManager->etamaAnm->StartScriptAtOrigin(&entry->sprite, script + 0x46);
            return;
        }
    }
}

void Player::UpdateUnknown30454()
{
    PlayerUnknown30454 *entry = this->unk_30454;
    int i;
    D3DXVECTOR3 size(this->shtFile->unknown10, this->shtFile->unknown10, 16.0f);
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    Effect *effect;

    this->unk_30f64 = -1000.0f;
    for (i = 0; i < 4; i++, entry++)
    {
        if (entry->unk_1c == 0)
        {
            continue;
        }
        entry->pos += entry->velocity;
        if (entry->pos.y >= 464.0f)
        {
            entry->unk_1c = 0;
            continue;
        }
        if (entry->velocity.y < 3.0f)
        {
            entry->velocity.y += 0.03f;
        }
        else
        {
            entry->velocity.y = 3.0f;
        }
        if (this->TouchesGrazeBox(&entry->pos, &size))
        {
            switch (entry->script)
            {
            case 0:
                this->AddUnknown30388(400.0f);
                break;
            case 1:


                for (i = 0; i < g_GameManager.rank.unk_b4 + 10 + g_GameManager.difficulty * 2; i++)
                {
                    g_App.SetCamera(this->sideIdx);
                    pos.x = g_GameManager.ArenaToScreenX(this->pos.x) + g_Rng.GetRandomF32SignedInRange(64.0f);
                    pos.y = g_GameManager.ArenaToScreenY(this->pos.y) + g_Rng.GetRandomF32SignedInRange(64.0f);
                    pos.z = 0.0f;
                    g_App.SetCamera(1 - this->sideIdx);
                    velocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
                    velocity.y = g_Rng.GetRandomF32InRange(128.0f);
                    velocity.z = 0.0f;
                    effect = g_GameManager.sides[2].effectManager->SpawnEffectWithVelocity(this->sideIdx + 1, &pos,
                                                                                              &velocity, 1, 0xffffffff);
                    effect->unk_a4 = 3;
                    effect->unk_a6 = 4;
                    switch (g_GameManager.difficulty)
                    {
                    case 0:
                        effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.04f + 0.9f;
                        break;
                    case 1:
                        effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.05f + 1.1f;
                        break;
                    case 2:
                        effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.08f + 1.3f;
                        break;
                    case 3:
                        effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.1f + 1.7f;
                        break;
                    case 4:
                        effect->unk_a0 = g_GameManager.rank.unk_b4 * 0.08f + 1.5f;
                        break;
                    }
                    effect->unk_a8 = this->sideIdx;
                    effect->unk_98 = i * 2.0f;
                }
                g_App.SetCamera(this->sideIdx);
                break;
            case 2:
                this->unk_30410.AddCombo(&this->pos, 1, 0, 400, 0);
                break;
            case 3:
                this->unk_30410.AddCombo(&this->pos, 0, 0, 0, 70000);
                break;
        }
        entry->unk_1c = 0;
        g_SoundPlayer.QueueSound(0x1f, this->sideIdx != 0 ? 500 : -500);
        }
        else
        {
            this->unk_30f64 = entry->pos.x;
            this->unk_30f68 = 400.0f - (400.0f - entry->pos.y) * 0.5f;
        }
    }
}

void Player::DrawUnknown30454()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->unk_30454[i].unk_1c != 0)
        {
            this->unk_30454[i].sprite.pos.x = g_GameManager.ArenaToScreenX(this->unk_30454[i].pos.x);
            this->unk_30454[i].sprite.pos.y = g_GameManager.ArenaToScreenY(this->unk_30454[i].pos.y);
            this->unk_30454[i].sprite.pos.z = 0.15f;
            g_SpriteEngine->DrawUnrotated(&this->unk_30454[i].sprite);
        }
    }
}

int Player::CheckHitAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet)
{
    PlayerHitArea **area;


    D3DXVECTOR3 d;
    D3DXVECTOR3 rot;

    D3DXVECTOR3 half;
    D3DXVECTOR3 max;
    float dx;
    float dy;
    Effect *effect;

    for (area = this->activeHitAreas;; area++)
    {
        if (*area == NULL)
        {
            return 0;
        }
        if ((*area)->kind != 1 && (*area)->kind != 2 && (*area)->kind != 3)
        {
            continue;
        }
        if ((*area)->unk_40 > 0)
        {
            continue;
        }
        if ((*area)->radius != 0.0)
        {
            dx = pos->x - (*area)->x;
            dy = pos->y - (*area)->y;
            if (dx * dx + dy * dy < (*area)->radius * (*area)->radius)
            {
            hit:
                if ((*area)->kind != 2 && (*area)->kind != 3)
                {
                    (*area)->hits++;
                    return 2;
                }
                if (bullet != NULL && bullet->sprite == 0)
                {
                    break;
                }
            }
        }
        else if ((*area)->angle != 0.0f)
        {
            d.x = pos->x - (*area)->x;
            d.y = pos->y - (*area)->y;
            Rotate2D(&rot, &d, -(*area)->angle);
            half.x = (*area)->width;
            half.y = (*area)->height;
            if (-half.x <= rot.x && rot.x <= half.x && -half.y <= rot.y && rot.y <= half.y)
            {
                goto hit;
            }
        }
        else
        {
            half.x = (*area)->x - (*area)->width;
            half.y = (*area)->y - (*area)->height;
            max.x = (*area)->width + (*area)->x;
            max.y = (*area)->height + (*area)->y;
            if (!(half.x > pos->x || max.x < pos->x || half.y > pos->y || max.y < pos->y))
            {
                goto hit;
            }
        }
    }
    d.x = g_GameManager.ArenaToScreenX(pos->x);
    d.y = g_GameManager.ArenaToScreenY(pos->y);
    d.z = 0.0f;
    g_App.SetCamera(1 - this->sideIdx);
    rot.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    rot.y = g_Rng.GetRandomF32InRange(128.0f);
    rot.z = 0.0f;
    effect = g_GameManager.sides[2].effectManager->SpawnEffectWithVelocity(this->sideIdx + 1, &d, &rot, 1,
                                                                              0xffffffff);
    g_App.SetCamera(this->sideIdx);
    effect->unk_a4 = bullet->sprite;
    effect->unk_a6 = bullet->color;
    effect->unk_a0 = bullet->speed;
    effect->unk_a8 = bullet->mgr->sideIdx;
    effect->unk_aa = (BYTE)bullet->unk_10bd;
    this->AddUnknown30388(0.2f);
    g_SoundPlayer.QueueSound(0x2e, this->sideIdx != 0 ? 500 : -500);
    this->unk_30410.AddCombo(pos, (g_GameManager.difficulty >= 3) + 2, 2, 0,
                                this->unk_30410.unk_38 > 100 ? 1000 : this->unk_30410.unk_38 * 10);
    this->side->AddScore(50);
    (*area)->hits++;
    return 2;
}

int Player::CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->CheckHitAreas(pos, size, bullet))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f - 60.0f;
    start.y = pos->y - size->y / 2.0f - 60.0f;
    end.x = size->x / 2.0f + pos->x + 60.0f;
    end.y = size->y / 2.0f + pos->y + 60.0f;
    if (this->grazeMin.x > end.x || this->grazeMax.x < start.x || this->grazeMin.y > end.y ||
        this->grazeMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#include "CoopGame.h"
#endif
void Player::Die()
{
#ifdef TH09_NATIVE_NETPLAY
    if (!coop::AllowDeath(this) || !thnet::BeforeDeath(this)) return;
    thnet::DeathScope netDeath(this);
#endif
    int lifeLoss;
    float angle;

    if (g_GameManager.gui->GetUnknown11ea8() != 0)
    {
        return;
    }
    g_SoundPlayer.QueueSoundPanned(4, this->pos.x);
    lifeLoss = g_GameManager.rank.unk_b8;
    if (this->unk_20 != 0)
    {
        lifeLoss += g_GameManager.rank.unk_bc;
    }
    if (this->unk_a8 <= 1)
    {
        if (this->side->character == 8 && this->unk_30388 >= 200.0f)
        {
            if (this->unk_30388 >= 400.0f)
            {
                if (this->otherSide->enemyManager->FindByUnknownType(3) == NULL)
                {
                    this->otherSide->spellCard->Start(2, 2, 0, this->shtFile->spellCardNames[2]);
                    this->unk_30384 = 0.0f;
                    this->unk_30388 = 0.0f;
                    this->StartClearCircle3();
                    return;
                }
                this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
                this->unk_30384 = 100.0f;
                this->unk_30388 = 100.0f;
                this->StartClearCircle2();
                return;
            }
            if (this->unk_30388 >= 300.0f)
            {
                this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
                this->unk_30384 = 0.0f;
                this->unk_30388 = 0.0f;
                this->StartClearCircle2();
                return;
            }
            if (this->unk_30388 >= 200.0f)
            {
                this->otherSide->spellCard->Start(0, 0, 0, this->shtFile->spellCardNames[0]);
                this->unk_30384 = 0.0f;
                this->unk_30388 = 0.0f;
                this->StartClearCircle1();
            }
            return;
        }
        this->unk_a8 = 0;
        g_GameManager.gui->CountRoundWin(1 - this->sideIdx);
        this->side->effectManager->SpawnReservedEffect(0x13, &this->pos, 4, 0xff4040ff);
    }
    else if (this->unk_a8 - lifeLoss > 1)
    {
        this->unk_a8 -= lifeLoss;
    }
    else
    {
        g_SoundPlayer.QueueSound(0x30, this->sideIdx != 0 ? 500 : -500);
        this->side->gui->StartUnknown6ee8(0);
        this->unk_a8 = 1;
    }
    this->playerState = 4;
    this->stateTimer = 0;
    angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    this->unk_1cd8 = angle;
    SetPolar((Vector2 *)&this->unk_1ccc, angle, 1.0f);
    g_GameManager.gui->unk_e93c[this->sideIdx] = 0x3c;
    g_GameManager.gui->unk_e934[this->sideIdx] = 0x80ff0000;
    if (this->unk_364 != NULL)
    {
        this->unk_364->sprites->SetPendingInterrupt(1);
        this->unk_364 = NULL;
        this->unk_368->active = 0;
        this->unk_368 = NULL;
    }
    g_GameManager.rank.LowerUnknownCc(1500);
    if (g_GameManager.rank.unk_b8 > 4)
    {
        g_GameManager.rank.unk_b8 -= 2;
    }
    else if (g_GameManager.rank.unk_b8 > 2)
    {
        g_GameManager.rank.unk_b8--;
    }
    this->unk_30410.EndCombo();
    this->unk_24.Reset();
}





static int UpdateDyingState(Player *player)
{
    float *pos;
    float amount;

    if (player->stateTimer.AsFloat() < 60.0f)
    {
        player->pos += player->unk_1ccc * (60.0f - player->stateTimer.AsFloat()) / 20.0f;
        pos = player->pos;
        if (pos[0] < g_GameManager.playerArea[0])
        {
            pos[0] = g_GameManager.playerArea[0];
        }
        else if (pos[0] > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
        {
            pos[0] = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
        }
        if (pos[1] < g_GameManager.playerArea[1])
        {
            pos[1] = g_GameManager.playerArea[1];
        }
        else if (pos[1] > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
        {
            pos[1] = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
        }
        player->sprite.color = (player->stateTimer.AsFrames() * 255 / 10) << 24 | 0xffffff;
        return 0;
    }
    if (player->unk_a8 == 1)
    {
        amount = 400.0f;
    }
    else
    {
        amount = 130.0f - player->unk_a8 * 10.0f;
    }
    player->AddUnknown30388(amount);
    player->SetPlayerState(3);
    player->stateTimer = 60;
    player->AddHitCircle(&player->pos, 16.0f, 20.0f, 8, 0);
    player->AddKind4Circle(&player->pos, 16.0f, 20.0f, 10, 8, 0);
    return 0;
}




static int CheckDeath(Player *player)
{
#ifdef TH09_NATIVE_NETPLAY
    if (thnet::RemotePlayer(player->sideIdx)) return 0;
#endif
    PlayerUnknown36cEntry *entry;

    if (player->GetPlayerState() != 0 || player->unk_1b74 > 0)
    {
        return 0;
    }
    entry = player->unk_36c.CheckHits();
    if (entry == NULL)
    {
        return 0;
    }
    if (entry->unk_2c != NULL)
    {
        ((Bullet *)entry->unk_2c)->state = 5;
    }
    player->Die();
    return 1;
}

int Player::OnUpdate(Player *player)
{
    GameSide *otherSide;

    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    if (player->side->flag0 && player->side->character != 2)
    {
        return 1;
    }
    if (!player->side->flag0)
    {
        player->UpdateUnknown30454();
    }
    if (player->GetPlayerState() == 5)
    {
        return 1;
    }
    g_App.SetCamera(player->sideIdx);
    if (player->GetPlayerState() == 4)
    {
        if (UpdateDyingState(player))
        {
            goto respawn;
        }
    }
    else if (player->GetPlayerState() == 2)
    {
        if (Player::UpdateUnknown0Is2())
        {
            goto respawn;
        }
    }
    else if (player->GetPlayerState() == 1)
    {
    respawn:
        UpdateRespawningState(player);
    }
    UpdateInvulnerability(player);
    if (player->unk_20 > 0)
    {
        player->unk_24.Run();
    }
    UpdateHitAreas(player);
    if (!g_GameManager.gui->IsMsgRunning())
    {
        UpdateSpellCardInput(player);
    }
    if (player->GetPlayerState() != 2 && player->GetPlayerState() != 4 && player->GetPlayerState() != 1)
    {
        player->HandleInput();
    }
#ifdef TH09_NATIVE_NETPLAY
    thnet::BeforePlayer(player);
#endif
    g_SpriteEngine->Step(&player->sprite);
    player->UpdateBullets();
    if (!g_GameManager.gui->IsMsgRunning())
    {
        player->unk_30410.unk_3c++;
        if (player->unknownFlag3)
        {
            g_SoundPlayer.QueueSound(0x30, player->sideIdx != 0 ? 500 : -500);
            player->side->gui->StartUnknown6ee8(0);
            player->unknownFlag3 = 0;
            if (g_GameManager.IsMode1() && player->sideIdx == 0)
            {
                otherSide = player->otherSide;
                player->side->gui->StartUnknown76d4(
                    g_PlayerCpuLevels[otherSide->unk_2c].seconds5c[2] * 60 -
                    otherSide->player->unk_24.timer5c.AsFrames());
            }
        }
        if (g_GameManager.gui->unk_1095c == 0)
        {
            player->UpdateShotInput();
        }
    }
    if (!player->side->flag0 && g_GameManager.gui->unk_1095c == 0)
    {
        CheckDeath(player);
    }
    ResetTargets(player);
    if (!player->side->flag0)
    {
        player->unk_30410.Update();
    }
    player->unk_36c.count = 0;
#ifdef TH09_NATIVE_NETPLAY
    thnet::AfterPlayer(player);
#endif
    return 1;
}

int Player::OnDraw(Player *player)
{
    g_App.SetView2D(player->sideIdx);
    if (player->GetPlayerState() != 5)
    {
        player->DrawBullets();
        if (player->unk_a8 == 0 &&
            (g_GameManager.gui->unk_1095c >= 2 || g_GameManager.gui->unk_10960 > 30))
        {
            return 1;
        }
        if (!g_GameManager.isInContinueMenu)
        {
            player->sprite.pos.x = g_GameManager.ArenaToScreenX(player->pos.x);
            player->sprite.pos.y = g_GameManager.ArenaToScreenY(player->pos.y);
            player->sprite.pos.z = 0.1f;
            g_SpriteEngine->DrawUnrotated(&player->sprite);
        }
    }
    player->DrawUnknown30454();
    return 1;
}

void Player::Reset()
{
    int i;
    int level;
    float *pos;

    this->anm->StartScript(&this->sprite, 0);
    pos = this->pos;
    i = 0;
    pos[i++] = -160.0f;
    pos[i++] = g_GameManager.arena[3] - 64.0f;
    pos[i] = 0.49f;
    for (i = 0; i < 0x200; i++)
    {
        this->hitAreas[i].Reset();
        this->freeHitAreas[i] = &this->hitAreas[i];
        this->activeHitAreas[i] = NULL;
    }
    this->activeHitAreaCount = 0;
    this->freeHitAreaCount = 0x200;
    this->direction = 0;
    this->unk_30410.unk_3c = 0;
    this->SetPlayerState(5);
    this->stateTimer = 120;
    this->unk_1b84 = 1;
    for (i = 0; i < 0x80; i++)
    {
        this->bullets[i].state = 0;
    }
    this->unk_1cdc = this->unk_1ce0 = 1.0f;
    this->moveScaleX = this->moveScaleY = 1.0f;
    for (i = 0; i < 0x10; i++)
    {
        this->posHistory[i] = this->pos;
    }
    this->unk_303e8 = 0;
    this->focused = 2;
    this->unk_303b0 = 6;
    this->unk_24.unk_58 = g_Rng.GetRandomF32InRange(400.0f) + 100.0f;
    if (this->unk_24.unk_58 >= 400.0f)
    {
        this->unk_24.unk_58 = 400.0f;
    }
    this->unk_30410.EndCombo();
    this->unk_1b74 = 0;
    this->unk_30384 = 0.0f;
    this->unk_a8 = g_GameManager.initialLives[this->sideIdx];
    level = g_GameManager.unk_100;
    if (level > 2)
    {
        level = 2;
    }
    switch (g_GameManager.difficulty)
    {
    case 0:
    case 1:
        this->unk_a4 = this->unk_a0 = level + 1;
        break;
    case 2:
        this->unk_a4 = this->unk_a0 = level + 4;
        break;
    case 3:
        this->unk_a4 = this->unk_a0 = level + 8;
        break;
    case 4:
        this->unk_a4 = this->unk_a0 = level + 12;
        this->unk_a8 = 1;
        this->unk_30388 = 400.0f;
        break;
    }
    if (this->unk_a8 == 1)
    {
        this->unknownFlag3 = 1;
    }
    this->unk_303a4 = -1;
    this->unk_1cd8 = -1.5707964f;
    this->unk_364 = NULL;
    this->unk_24.Reset();
    memset(this->unk_30454, 0, sizeof(this->unk_30454));
}

int Player::AddedCallback(Player *player)
{
    GameSide *side;

    if (g_ShtFiles[player->sideIdx] != NULL && player->side->character == player->side->unk_24)
    {
        player->shtFile = g_ShtFiles[player->sideIdx];
    }
    else
    {
        if (g_ShtFiles[player->sideIdx] != NULL)
        {
            g_MemoryTracker.Free(g_ShtFiles[player->sideIdx]);
        }
        if (LoadShotTypeFile(&player->shtFile, g_CharacterFiles[player->side->character][0]))
        {
            return -1;
        }
        g_ShtFiles[player->sideIdx] = player->shtFile;
    }
    side = player->side;
    player->unk_303f0 = g_CharacterValuesBd0[side->character];
    player->unk_303f4 = g_CharacterValuesB90[side->character];
    player->unk_303f8 = g_CharacterValuesB50[side->character];
    player->unk_303fc = g_CharacterValuesB10[side->character];
    player->unk_30400 = g_CharacterValuesAd0[side->character];
    player->anm = g_SpriteEngine->GetAnm(player->sideIdx + 5);
    player->hitboxSize.x = player->hitboxSize.y = player->shtFile->hitboxSize / 2.0f;
    player->hitboxSize.z = 5.0f;
    player->grazeSize.x = player->grazeSize.y = player->shtFile->grazeSize / 2.0f;
    player->grazeSize.z = 5.0f;
    player->grazeTestHalfSize.x = player->grazeTestHalfSize.y = player->shtFile->unknown10 / 2.0f;
    player->grazeTestHalfSize.z = 5.0f;
    player->unk_30388 = 100.0f;
    player->unk_30410.player = player;
    player->unk_30410.func = g_CharacterFuncs9d0[side->character];
    player->unk_a4 = player->unk_a0 = 1;
    player->unk_24.player = player;
    player->unk_36c.player = player;
    player->unk_ac = 2;
    player->unk_b8 = player->unk_b4 = player->unk_b0 = 0;
    player->unk_30404 = g_CharacterFuncsA50[side->character];
    player->unk_30408 = g_CharacterFuncsA90[side->character];
    player->Reset();
    return 0;
}

Player::Player()
{
}

Player *Player::RegisterChain(int arg, int sideIdx, int unk_20)
{
    Player *player = (Player *)g_MemoryTracker.Register(new Player(), sizeof(Player), "PlayerInf");

    memset(player, 0, sizeof(Player));
    player->stateTimer = 0;
    player->SetRegisterChainArg(arg);
    player->side = &g_GameManager.sides[sideIdx];
    player->sideIdx = sideIdx;
    player->otherSide = &g_GameManager.sides[1 - sideIdx];
    player->unk_20 = unk_20;
    player->calcElem = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcElem->arg = player;
    player->calcElem->addedCallback = (ChainCallback)Player::AddedCallback;
    if (g_Chain.AddToCalcChain(player->calcElem, sideIdx + 0xf) == 0)
    {
        player->drawElem = g_Chain.CreateElem((ChainCallback)Player::OnDraw);
        player->drawElem2 = g_Chain.CreateElem((ChainCallback)Player::OnDraw2);
        player->drawElem->arg = player;
        player->drawElem2->arg = player;
        g_Chain.AddToDrawChain(player->drawElem, sideIdx + 0xc);
        g_Chain.AddToDrawChain(player->drawElem2, sideIdx + 0xe);
    }
    return player;
}


static void GetBox(D3DXVECTOR3 &min, D3DXVECTOR3 &max, D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    min.x = pos->x - size->x / 2.0f;
    min.y = pos->y - size->y / 2.0f;
    max.x = size->x / 2.0f + pos->x;
    max.y = size->y / 2.0f + pos->y;
}

void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd)
{
    if (sd->option == 0)
    {
        bullet->pos = player->pos;
    }
    else
    {
        bullet->pos = player->options[sd->option - 1].pos;
    }
    bullet->pos[0] += sd->offsetX;
    bullet->pos[1] += sd->offsetY;
    bullet->pos[2] = 0.495f;
    bullet->size.x = sd->sizeX;
    bullet->size.y = sd->sizeY;
    bullet->size.z = 1.0f;
    bullet->angle = sd->angle;
    bullet->speed = sd->speed;
    bullet->velocity.x = cosf(sd->angle) * sd->speed;
    bullet->velocity.y = sinf(sd->angle) * sd->speed;
    bullet->timer = 0;
    bullet->unk_470 = 0;
    bullet->type = sd->type;
    bullet->damage = sd->damage;
    bullet->script = sd->script;
    if (sd->sound >= 0)
    {
        g_SoundPlayer.QueueSoundPanned(sd->sound, player->pos.x);
    }
    player->anm->StartScript(&bullet->sprite, sd->script + 5);
    bullet->unk_46e = 0;
}

int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

void FireShots(Player *player, int frame, int table)
{
    ShotData *shot;
    int ret;
    PlayerBullet *curBullet;
    int i;

    shot = player->shtFile->tables[table].shots;
    curBullet = player->bullets;
    for (i = 0; i < 0x80; i++, curBullet++)
    {
        if (curBullet->state != 0)
        {
            continue;
        }
    again:
        if (shot->fire != NULL)
        {
            ret = shot->fire(player, curBullet, frame, shot);
        }
        else
        {
            ret = TryFireShot(player, curBullet, frame, shot);
        }
        if (ret == 1)
        {
            curBullet->sprite.flags |= 0x2000;
            curBullet->state = 1;
            curBullet->shot = shot;
            curBullet->update = curBullet->shot->update;
            curBullet->draw = curBullet->shot->draw;
            curBullet->hit = curBullet->shot->hit;
        }
        shot++;
        if (shot->frame < 0)
        {
            return;
        }
        if (ret == 0)
        {
            goto again;
        }
    }
}

void Player::UpdateBullets()
{
    int i;
    PlayerBullet *bullet;

    if (this->side->flags & 1)
    {
        return;
    }
    bullet = this->bullets;
    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->update != NULL && bullet->update(this, bullet))
        {
            bullet->state = 0;
            continue;
        }
        bullet->pos[0] += g_App.gameSpeed * bullet->velocity.x;
        bullet->pos[1] += g_App.gameSpeed * bullet->velocity.y;
        if (bullet->type != 2 &&
            !g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1], bullet->sprite.spriteData->width,
                                        bullet->sprite.spriteData->height))
        {
            bullet->state = 0;
        }
        if (g_SpriteEngine->Step(&bullet->sprite))
        {
            bullet->state = 0;
        }
        bullet->timer++;
    }
}

void Player::DrawBullets()
{
    PlayerBullet *bullet = this->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 1)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->pos.x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->pos.y);
        bullet->sprite.pos.z = 0.4f;
        if (bullet->unk_46e)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        if (bullet->draw != NULL)
        {
            bullet->draw(this, bullet);
        }
        else
        {
            g_SpriteEngine->Draw(&bullet->sprite);
        }
    }
}

void Player::DrawHitBullets()
{
    PlayerBullet *bullet = this->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 2)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->pos.x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->pos.y);
        bullet->sprite.pos.z = 0.2f;
        if (bullet->unk_46e)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        g_SpriteEngine->DrawSprite(&bullet->sprite);
    }
}

int Player::UpdateShotInput()
{
    int side;
    BYTE chargeType;
    int before;

    side = this->sideIdx;
    if ((g_PlayerInputs[side].IsJustPressed2(1) && !(g_GameManager.config->unk_b4[side] == 1 && g_PlayerInputs[side].IsHeld2(4))) ||
        ((chargeType = g_GameManager.config->unk_b4[side]) == 1 && g_PlayerInputs[side].IsHeld2(1) &&
         !g_PlayerInputs[side].IsHeld2(4)))
    {
        if (this->unk_303b0 == 0)
        {
            if (this->unk_303a4 < 0)
            {
                this->unk_303a4 = 0;
            }
            else if (this->unk_303a4 >= 10)
            {
                this->unk_303a4 -= 10;
            }
            else if (this->unk_303a4 >= 5)
            {
                this->unk_303a4 -= 5;
            }
            this->unk_303b0 = 3;
            this->unk_303bc = 1;
        }
        else if (this->unk_303b0 > 0)
        {
            this->unk_303b0--;
        }
        if (g_GameManager.config->unk_b4[this->sideIdx] != 1)
        {
            goto fire;
        }
    }
    else if ((chargeType == 1 && g_PlayerInputs[side].IsHeld2(4)) || (chargeType == 0 && g_PlayerInputs[side].IsHeld2(1)))
    {
        this->unk_303bc++;
        if (this->unk_303bc >= 10 && !this->charging)
        {
            if (this->unk_30384 == 0.0f)
            {
                g_SoundPlayer.QueueSound(0x2b, this->sideIdx != 0 ? 500 : -500);
                this->side->gui->StartUnknown5c6c();
            }
            before = (int)this->unk_30384 / 100;
            this->unk_30384 += g_App.gameSpeed * this->shtFile->unk_24;
            if (this->unk_30384 >= this->unk_30388)
            {
                this->unk_30384 = this->unk_30388;
                this->unk_3038c++;
                if (this->unk_3038c >= 300)
                {
                    goto release;
                }
            }
            else
            {
                this->unk_3038c = 0;
            }
            if (before < (int)this->unk_30384 / 100)
            {
                g_SoundPlayer.QueueSound(0x32, this->sideIdx != 0 ? 500 : -500);
            }
        }
        goto fire;
    }
release:
    if (this->unk_30384 >= 100.0f)
    {
        this->charging = 1;
        this->unk_30398 = 0;
        this->unk_1b74 = 8;
        if (this->unk_30384 >= 400.0f && this->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->otherSide->spellCard->Start(2, 2, 0, this->shtFile->spellCardNames[2]);
            this->SubUnknown30388(300.0f);
            this->unk_1b74 = 8;
            this->StartClearCircle3();
        }
        else if (this->unk_30384 >= 300.0f)
        {
            this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
            this->SubUnknown30388(200.0f);
            this->unk_1b74 = 8;
            this->StartClearCircle2();
        }
        else if (this->unk_30384 >= 200.0f)
        {
            this->otherSide->spellCard->Start(0, 0, 0, this->shtFile->spellCardNames[0]);
            this->unk_1b74 = 8;
            this->SubUnknown30388(100.0f);
            this->StartClearCircle1();
        }
    }
    this->side->gui->StopUnknown5c6c();
    this->unk_303bc = -1;
    this->unk_30384 = 0.0f;
    if (this->unk_303b0 > 0)
    {
        this->unk_303b0--;
    }
fire:
    if (this->charging)
    {
        if (this->unk_30398.HasTicked())
        {
            FireShots(this, this->unk_30398.AsFrames(), 1);
        }
        this->unk_30398++;
        if (this->unk_30398.AsFloat() >= this->shtFile->unk_28)
        {
            this->unk_30398 = 0;
            this->charging = 0;
        }
    }
    if (this->unk_303a4 >= 0)
    {
        if (this->unk_303a4.HasTicked())
        {
            FireShots(this, this->unk_303a4.AsFrames(), 0);
        }
        this->unk_303a4++;
        if (this->unk_303a4 >= 15)
        {
            this->unk_303a4 = -1;
        }
    }
    return 0;
}

int Player::CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *damageTaken, int *hitByBomb, int *kind4Damage)
{
    PlayerBullet *cur;
    D3DXVECTOR3 enemyMin;
    D3DXVECTOR3 enemyMax;
    int idx;
    D3DXVECTOR3 targetLo;
    D3DXVECTOR3 targetHi;
    float rotZ;
    int damage;
    PlayerHitArea **area;

    damage = 0;
    if (!this->stateTimer.HasTicked())
    {
        return 0;
    }
    GetBox(enemyMin, enemyMax, pos, size);
    cur = this->bullets;
    if (hitByBomb != NULL)
    {
        *hitByBomb = 0;
    }
    for (idx = 0; idx < 0x80; idx++, cur++)
    {
        if (cur->state == 0 || cur->state != 1)
        {
            continue;
        }
        GetBox(targetLo, targetHi, &cur->pos, &cur->size);
        if (targetLo.y > enemyMax.y || targetLo.x > enemyMax.x || targetHi.y < enemyMin.y || targetHi.x < enemyMin.x)
        {
            continue;
        }
        if (cur->type == 2 && cur->timer % 2 != 0)
        {
            continue;
        }
        if (cur->hit != NULL && cur->hit(this, cur, pos))
        {
            continue;
        }
        damage += cur->damage;
        if (cur->type != 2 && cur->type != 3)
        {
            if (cur->state == 1)
            {
                rotZ = cur->sprite.rotation.z;
                this->anm->StartScript(&cur->sprite, cur->script + 6);
                cur->sprite.rotation.z = rotZ;
                cur->pos[2] = 0.1f;
            }
            cur->state = 2;
            cur->velocity.x /= 8.0f;
            cur->velocity.y /= 8.0f;
            if (cur->type == 4)
            {
                this->side->effectManager->SpawnReservedEffect(0x22, &this->bullets->pos, 6, 0xffffffff);
                this->AddDamageCircle(&this->bullets->pos, 0.0f, 1.6f, 6, 0x46, 0);
                g_SoundPlayer.QueueSound(0x10, this->sideIdx != 0 ? 500 : -500);
            }
        }
    }
    *damageTaken = damage;
    for (area = this->activeHitAreas; *area != NULL; area++)
    {
        if ((*area)->kind != 0 && (*area)->kind != 2 && (*area)->kind != 4)
        {
            continue;
        }
        if ((*area)->unk_40 > 0)
        {
            continue;
        }
        if ((*area)->framesLeft % (*area)->damageInterval != 0)
        {
            continue;
        }
        if ((*area)->radius == 0.0f)
        {
            if ((*area)->angle == 0.0f)
            {
                if ((*area)->x - (*area)->width > enemyMax.x || (*area)->width + (*area)->x < enemyMin.x ||
                    (*area)->y - (*area)->height > enemyMax.y || (*area)->height + (*area)->y < enemyMin.y)
                {
                    continue;
                }
            }
            else
            {
                targetLo.x = pos->x - (*area)->x;
                targetLo.y = pos->y - (*area)->y;
                Rotate2D(&targetHi, &targetLo, -(*area)->angle);
                if (-(*area)->width > size->x / 2.0f + targetHi.x || (*area)->width < targetHi.x - size->x / 2.0f ||
                    -(*area)->height > size->y / 2.0f + targetHi.y || (*area)->height < targetHi.y - size->y / 2.0f)
                {
                    continue;
                }
            }
        }
        else if ((*area)->radius * (*area)->radius <
                 ((*area)->x - pos->x) * ((*area)->x - pos->x) + ((*area)->y - pos->y) * ((*area)->y - pos->y))
        {
            continue;
        }
        damage += (*area)->damage;
        (*area)->hits += (*area)->damage;
        if ((*area)->maxDamage > 0 && (*area)->maxDamage <= (*area)->hits)
        {
            (*area)->damage = 0;
            damage -= (*area)->hits - (*area)->maxDamage;
        }
        if ((*area)->kind == 4)
        {
            *kind4Damage += (*area)->damage;
        }
    }
    return damage;
}
