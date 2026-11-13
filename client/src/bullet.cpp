#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "player.h"
#include "sound.h"
#include "unsorted.h"

void BulletManager::Initialize()
{
    int i;
    Bullet *bullet;

    memset(this, 0, sizeof(BulletManager));
    this->nextBullet = this->bullets;
    this->bullets[0xaf].state = 6;
    this->bullets2[0x168].state = 6;
    this->nextBullet2 = this->bullets2;
    this->clearItemType = 5;
    bullet = this->bullets;
    for (i = 0; i < 0xaf; i++, bullet++)
    {
        bullet->sprites.sprite0.scriptIndex = -1;
        bullet->sprites.sprite4.scriptIndex = -1;
        bullet->sprites.sprite1.scriptIndex = -1;
        bullet->sprites.sprite2.scriptIndex = -1;
        bullet->sprites.sprite3.scriptIndex = -1;
    }
    bullet = this->bullets2;
    for (i = 0; i < 0x168; i++, bullet++)
    {
        bullet->sprites.sprite0.scriptIndex = -1;
        bullet->sprites.sprite4.scriptIndex = -1;
        bullet->sprites.sprite1.scriptIndex = -1;
        bullet->sprites.sprite2.scriptIndex = -1;
        bullet->sprites.sprite3.scriptIndex = -1;
    }
}

void BulletManager::Reset()
{
    memset(this->bullets, 0, sizeof(this->bullets));
    memset(this->bullets2, 0, sizeof(this->bullets2));
    memset(this->lasers, 0, sizeof(this->lasers));
    this->nextBullet = this->bullets;
    this->nextBullet2 = this->bullets2;
    this->bullets[0xaf].state = 6;
    this->bullets2[0x168].state = 6;
    this->clearItemType = 5;
}


static void CopyBulletSprite(Sprite *dst, Sprite *src)
{
    *dst = *src;
}

void BulletManager::ShowSpawnSprite(Sprite *dst, Sprite *src, BulletTypeSprites *sprites, int color)
{
    if (dst->spriteIndex != src->spriteIndex + color)
    {
        if (sprites->sprite0.spriteData->height <= 16.0f)
        {
            this->etamaAnm->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets16[color]);
        }
        else if (sprites->sprite0.spriteData->height <= 32.0f)
        {
            this->etamaAnm->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets32[color]);
        }
        else
        {
            this->etamaAnm->SetSprite(dst, src->spriteIndex + color);
        }
    }
}

void BulletManager::ClearAll(int arg)
{
}

void BulletManager::ConvertBulletsToItems(D3DXVECTOR3 *pos, float arg)
{
}

int BulletManager::LoadAnm()
{
    return g_SpriteEngine->PreloadAnm(8, "etama.anm") == NULL;
}

void BulletManager::ReleaseAnm()
{
    g_SpriteEngine->ReleaseAnm(8);
}

BulletCommand *Bullet::SetCommand(int i, int concurrent, DWORD flag)
{
    BulletCommand *cmd = &this->commands[i];

    cmd->flag = flag;
    cmd->concurrentFlag = concurrent;
    this->flags |= flag;
    this->commandIndex = 0;
    return cmd;
}

BulletCommand *BulletShooter::SetCommand(int i, int concurrent, DWORD flag)
{
    BulletCommand *cmd = &this->commands[i];

    cmd->flag = flag;
    cmd->concurrentFlag = concurrent;
    this->flags |= flag;
    return cmd;
}

void Bullet::SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x10);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
}

void Bullet::SetWaitCommand(int i, int concurrent, int frames)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20000);

    cmd->frames = frames;
}

void Bullet::SetSpriteCommand(int i, int concurrent, int type, int color)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x4000);

    cmd->frames = type;
    cmd->maxTurns = color;
}

void BulletShooter::SetAngleAccelCommand(int i, int concurrent, int frames, float arg1, float arg0)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
}

void BulletShooter::SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x10);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
}

void BulletShooter::SetWaitCommand(int i, int concurrent, int frames)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20000);

    cmd->frames = frames;
}

void PlayerUnknown36c::AddTurned(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle,
                                 void *unk_2c)
{
    if (this->count < 0x80)
    {
        this->entries[this->count].pos = *pos;
        this->entries[this->count].unk_c = *origin;
        this->entries[this->count].size = *size;
        this->entries[this->count].size.x /= 2.0f;
        this->entries[this->count].size.y /= 2.0f;
        this->entries[this->count].radius = 0.0f;
        this->entries[this->count].angle = angle;
        this->entries[this->count].unk_2c = unk_2c;
        this->count++;
    }
}

void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer = 0;
    this->grazeDelayTimer = 0;
}

Bullet *BulletManager::SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle, int set)
{
    float v;
    int i;
    Bullet *cur;
    float angle;

    if (set == 0)
    {
        cur = this->nextBullet;
        for (i = 0; i < 0xaf; i++)
        {
            if (cur->state == 0)
            {
                break;
            }
            cur++;
            if (cur->state == 6)
            {
                cur = this->bullets;
            }
        }
        if (i >= 0xaf)
        {
            return cur;
        }
    }
    else
    {
        cur = this->nextBullet2;
        for (i = 0; i < 0x168; i++)
        {
            if (cur->state == 0)
            {
                break;
            }
            cur++;
            if (cur->state == 6)
            {
                cur = this->bullets2;
            }
        }
        if (i >= 0x168)
        {
            return cur;
        }
    }
    angle = 0.0f;
    if (shooter->count2 > 1)
    {
        v = shooter->speed1 - (shooter->speed1 - shooter->speed2) * row / shooter->count2;
    }
    else
    {
        v = shooter->speed1;
    }
    switch (shooter->aimMode)
    {
    case 0:
    case 1:
        if (shooter->count1 & 1)
        {
            angle += (col + 1) / 2 * shooter->angle2;
        }
        else
        {
            angle += (col / 2 + 0.5f) * shooter->angle2;
        }
        if (col & 1)
        {
            angle *= -1.0f;
        }
        if (shooter->aimMode == 0)
        {
            angle += aimAngle;
        }
        angle += shooter->angle1;
        break;
    case 2:
        angle += aimAngle;
    case 3:
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 4:
        angle += aimAngle;
    case 5:
        angle += 3.1415927f / shooter->count1;
        angle += col * 6.2831855f / shooter->count1;
        angle += shooter->angle1;
        break;
    case 6:
        angle = g_Rng.GetRandomF32InRange(shooter->angle1 - shooter->angle2) + shooter->angle2;
        break;
    case 7:
        v = g_Rng.GetRandomF32InRange(shooter->speed1 - shooter->speed2) + shooter->speed2;
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 8:
        angle = g_Rng.GetRandomF32InRange(shooter->angle1 - shooter->angle2) + shooter->angle2;
        v = g_Rng.GetRandomF32InRange(shooter->speed1 - shooter->speed2) + shooter->speed2;
        break;
    }
    cur->mgr = this;
    cur->state = 1;
    cur->spawnedFlag = 1;
    cur->grazed = 0;
    cur->timer = 0;
    cur->harmless = 0;
    cur->grazeDelayTimer = 0;
    cur->unk_10be = 0;
    cur->speed = v;
    cur->angle = AddNormalizeAngle(angle, 0.0f);
    cur->unk_10bd = shooter->unk_20c;
    cur->pos = shooter->pos;
    cur->pos[2] = 0.1f;
    SetPolar((Vector2 *)&cur->velocity, angle, g_App.gameSpeed * v);
    cur->activeFlags = shooter->flags;
    cur->color = shooter->color;
    cur->scriptFuncState = 0;
    cur->spawnedInArea = 0;
    CopyBulletSprite(&cur->sprites.sprite0, &shooter->typeSprites->sprite0);
    CopyBulletSprite(&cur->sprites.sprite4, &shooter->typeSprites->sprite4);
    cur->sprite = shooter->sprite;
    cur->color = shooter->color;
    cur->sprites.unk_d44 = shooter->typeSprites->unk_d44;
    cur->sprites.hitboxSize = shooter->typeSprites->hitboxSize;
    cur->sprites.unk_d40 = shooter->typeSprites->unk_d40;
    cur->sprites.height = shooter->typeSprites->height;
    cur->sprites.sizeClass = shooter->typeSprites->sizeClass;
    cur->sound = shooter->bulletSound;
    cur->boundsCheckDelay = 0;
    if (cur->sprites.sprite0.spriteIndex != shooter->typeSprites->sprite0.spriteIndex + shooter->color)
    {
        this->etamaAnm->SetSprite(&cur->sprites.sprite0, shooter->typeSprites->sprite0.spriteIndex + shooter->color);
    }
    if (cur->sprites.sprite4.spriteIndex != shooter->typeSprites->sprite4.spriteIndex + shooter->color)
    {
        if (cur->sprites.sprite0.spriteData->height <= 16.0f)
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4,
                                      shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets16[shooter->color]);
        }
        else if (cur->sprites.sprite0.spriteData->height <= 32.0f)
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4,
                                      shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets32[shooter->color]);
        }
        else
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + shooter->color);
        }
    }


    if ((BYTE)shooter->flags & 2)
    {
        CopyBulletSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1);
        this->ShowSpawnSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1, &cur->sprites, shooter->color);
        cur->state = 2;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 4)
    {
        CopyBulletSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2);
        this->ShowSpawnSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2, &cur->sprites, shooter->color);
        cur->state = 3;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 8)
    {
        CopyBulletSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3);
        this->ShowSpawnSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3, &cur->sprites, shooter->color);
        cur->state = 4;
        cur->pos -= cur->velocity * 4.0f;
    }
    memcpy(cur->commands, shooter->commands, sizeof(cur->commands));
    cur->flags = shooter->flags;
    cur->activeFlags = 0;
    cur->commandIndex = shooter->commandIndex;
    RunBulletCommands(cur);
    if (this->clearAllFrames != 0 && !(cur->flags & 0x1000))
    {
        cur->state = 5;
    }
    return cur;
}

Bullet *BulletManager::FirePattern(BulletShooter *shooter)
{
    int col;
    int row;
    float aimAngle;
    Bullet *bullet;

    shooter->typeSprites = &this->typeSprites[shooter->sprite];
    aimAngle = this->side->player->AngleToPlayer(&shooter->pos);
    for (row = 0; row < shooter->count2; row++)
    {
        for (col = 0; col < shooter->count1; col++)
        {
            bullet = this->SpawnBullet(shooter, col, row, aimAngle, 0);
            if (bullet == &this->bullets[0xaf])
            {
                goto done;
            }
        }
    }
done:
    if (shooter->flags & 0x200)
    {
        g_SoundPlayer.QueueSoundPanned(shooter->sound, shooter->pos.x);
    }
    return bullet;
}

Bullet *BulletManager::FirePattern2(BulletShooter *shooter)
{
    int col;
    int row;
    float aimAngle;
    Bullet *bullet;

    shooter->typeSprites = &this->typeSprites[shooter->sprite];
    aimAngle = this->side->player->AngleToPlayer(&shooter->pos);
    for (row = 0; row < shooter->count2; row++)
    {
        for (col = 0; col < shooter->count1; col++)
        {
            bullet = this->SpawnBullet(shooter, col, row, aimAngle, 1);
            if (bullet == &this->bullets2[0x168])
            {
                goto done;
            }
        }
    }
done:
    if (shooter->flags & 0x200)
    {
        g_SoundPlayer.QueueSoundPanned(shooter->sound, shooter->pos.x);
    }
    return bullet;
}



Laser *BulletManager::SpawnLaser(BulletShooter *shooter)
{
    int i;
    Laser *laser;

    laser = this->lasers;
    if (this->clearAllFrames != 0 && !(shooter->flags & 4))
    {
        return laser;
    }
    for (i = 0; i < 0x30; i++, laser++)
    {
        if (laser->active != 0)
        {
            continue;
        }
        this->etamaAnm->StartScript(&laser->sprite0, shooter->sprite + 10);
        this->etamaAnm->SetSprite(&laser->sprite0, laser->sprite0.spriteIndex + shooter->color);
        this->etamaAnm->InitSprite(&laser->sprite1, g_BulletColorOffsets16[shooter->color] + 0x92);
        laser->sprite1.blendMode = 1;
        laser->pos = shooter->pos;
        laser->color = shooter->color;
        laser->active = 1;
        laser->angle = shooter->angle1;
        if (shooter->aimMode == 0)
        {
            laser->angle += this->side->player->AngleToPlayer(&shooter->pos);
        }
        laser->flags = shooter->flags;
        laser->timer = 0;
        laser->startOffset = shooter->exFloats[0];
        laser->endOffset = shooter->exFloats[1];
        laser->maxLength = shooter->exFloats[2];
        laser->width = shooter->exFloats[3];
        laser->speed = shooter->speed1;
        laser->startFrames = shooter->exInts[0];
        laser->durationFrames = shooter->exInts[1];
        laser->endFrames = shooter->exInts[2];
        laser->hitStartFrame = shooter->exInts[3];
        laser->endHitFrames = shooter->laserEndHitFrames;
        laser->drawDelayFlag = 0;
        if (laser->startFrames == 0)
        {
            laser->state = 1;
        }
        else
        {
            laser->state = 0;
        }
        break;
    }
    return laser;
}

static void SlowBulletStart(Bullet *bullet)
{
    if (bullet->ex[0].timer <= 16)
    {
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle,
                 (5.0f - bullet->ex[0].timer.AsFloat() * 5.0f / 16.0f + bullet->speed) * g_App.gameSpeed);
    }
    else
    {
        bullet->activeFlags ^= 1;
    }
    bullet->ex[0].timer++;
}



static void AccelerateAlongVelocity(Bullet *bullet)
{
    if (bullet->ex[1].timer >= bullet->ex[1].duration)
    {
        bullet->activeFlags &= ~0x10;
    }
    else
    {
        bullet->velocity += bullet->ex[1].accel * g_App.gameSpeed;
        if (Fabs(bullet->velocity.x) > 0.0001f || Fabs(bullet->velocity.y) > 0.0001f)
        {
            bullet->angle = Atan2(bullet->velocity.y, bullet->velocity.x);
        }
    }
    bullet->ex[1].timer++;
}



static void TurnAndAccelerate(Bullet *bullet)
{
    if (bullet->ex[2].timer >= bullet->ex[2].duration)
    {
        bullet->activeFlags &= ~0x20;
    }
    else
    {
        bullet->angle = AddNormalizeAngle(bullet->angle, g_App.gameSpeed * bullet->ex[2].turnAngle);
        bullet->speed = g_App.gameSpeed * bullet->ex[2].turnSpeed + bullet->speed;
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * bullet->speed);
    }
    bullet->ex[2].timer++;
}



static void StopTurnAndRestart(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x40;
        }
        bullet->angle += bullet->ex[3].turnAngle;
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}


static void StopTurnToAngle(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x100;
        }
        bullet->angle = bullet->ex[3].turnAngle;
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}



static void StopTurnToPlayer(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x80;
        }
        bullet->angle =
            AddNormalizeAngle(bullet->mgr->side->player->AngleToPlayer(&bullet->pos), bullet->ex[3].turnAngle);
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}



static void BounceOffWalls(Bullet *bullet)
{
    float speed;

    if (!g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1], bullet->sprites.sprite0.spriteData->width,
                                      bullet->sprites.sprite0.spriteData->height))
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.QueueSound(bullet->sound, 0);
        }
        if (bullet->pos.x < -144.0f || bullet->pos.x >= 144.0f)
        {
            bullet->angle = -bullet->angle - 3.1415927f;
            bullet->angle = AddNormalizeAngle(bullet->angle, 0.0f);
        }
        if (bullet->pos.y < 0.0f || (bullet->pos.y >= 448.0f && (bullet->activeFlags & 0x400)))
        {
            bullet->angle = -bullet->angle;
        }
        bullet->speed = bullet->ex[4].turnSpeed;
        speed = bullet->speed;
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle, speed * g_App.gameSpeed);
        bullet->ex[4].duration++;
        if (bullet->ex[4].duration >= bullet->ex[4].maxTurns)
        {
            bullet->activeFlags &= ~0xc00;
        }
    }
}



static void WrapAroundX(Bullet *bullet)
{
    if (bullet->pos.x < 0.0)
    {
        bullet->pos.x += 384.0f;
    }
    else if (bullet->pos.x > 384.0)
    {
        bullet->pos.x -= 384.0f;
    }
    if (bullet->ex[6].timer <= 0)
    {
        bullet->activeFlags ^= 0x400000;
    }
    else
    {
        bullet->ex[6].timer--;
    }
}



static void WrapAroundY(Bullet *bullet)
{
    if (bullet->pos.y < 0.0)
    {
        bullet->pos.y += 448.0f;
    }
    else if (bullet->pos.y > 448.0)
    {
        bullet->pos.y -= 448.0f;
    }
    if (bullet->ex[6].timer <= 0)
    {
        bullet->activeFlags ^= 0x800000;
    }
    else
    {
        bullet->ex[6].timer--;
    }
}



static void DrawBullet(Bullet *bullet)
{
    Sprite *sprite;

    switch (bullet->state)
    {
    case 2:
        sprite = &bullet->sprites.sprite1;
        break;
    case 3:
        sprite = &bullet->sprites.sprite2;
        break;
    case 4:
        sprite = &bullet->sprites.sprite3;
        break;
    case 5:
        sprite = &bullet->sprites.sprite4;
        break;
    default:
        sprite = &bullet->sprites.sprite0;
    }
    sprite->pos[0] = g_GameManager.ArenaToScreenX(bullet->pos[0]);
    sprite->pos[1] = g_GameManager.ArenaToScreenY(bullet->pos[1]);
    sprite->pos[2] = 0.05f;
    sprite->color = sprite->color & 0xff000000 | 0xffffff;
    if (sprite->autoRotate != 0)
    {
        sprite->SetRotationZ(AddNormalizeAngle(bullet->angle + 1.5707964f, 0.0f));
    }
    g_SpriteEngine->Draw(sprite);
}

int BulletManager::OnDraw(BulletManager *mgr)
{
    int i;
    float s;
    float ca;
    Laser *laser;
    float len;
    float scale;
    Bullet *bullet;

    g_App.SetView(mgr->sideIdx);
    laser = mgr->lasers;
    for (i = 0x30; i != 0; i--, laser++)
    {
        if (!laser->active)
        {
            continue;
        }
        SinCos(&s, &ca, laser->angle);
        len = (laser->endOffset - laser->startOffset) / 2.0f + laser->startOffset;
        laser->sprite0.pos[0] = g_GameManager.ArenaToScreenX(len * ca + laser->pos[0]);
        laser->sprite0.pos[1] = g_GameManager.ArenaToScreenY(len * s + laser->pos[1]);
        laser->sprite0.pos[2] = 0.07f;
        laser->color = laser->color & 0xff000000 | 0xffffff;
        g_SpriteEngine->Draw(&laser->sprite0);
        if ((laser->startOffset < 16.0f || laser->speed == 0.0f) && (laser->drawDelayFlag == 0 || laser->state != 0))
        {
            laser->sprite1.pos[0] = g_GameManager.ArenaToScreenX(ca * laser->startOffset + laser->pos[0]);
            laser->sprite1.pos[1] = g_GameManager.ArenaToScreenY(s * laser->startOffset + laser->pos[1]);
            laser->sprite1.pos[2] = 0.05f;
            laser->sprite1.color = laser->sprite0.color;
            laser->sprite1.unk_bit6 = 1;
            laser->sprite1.color = laser->sprite1.color & 0xffffff | 0xff000000;
            scale = laser->width / 10.0f;
            laser->sprite1.scaleY = laser->sprite1.scaleX = scale * ((16.0f - laser->startOffset) / 16.0f);
            if (laser->sprite1.scaleY <= 0.0f)
            {
                laser->sprite1.scaleY = laser->sprite1.scaleX = scale;
            }
            g_SpriteEngine->Draw(&laser->sprite1);
        }
    }
    for (i = 0; i < 6; i++)
    {
        bullet = mgr->drawLists[i];
        while (bullet != NULL)
        {
            DrawBullet(bullet);
            bullet = bullet->next;
        }
    }
    EffectManager::DrawUnknown114(mgr->side->effectManager);
    return 1;
}

int BulletManager::AddedCallback(BulletManager *mgr)
{
    DWORD i;
    SpriteEngine *se = g_SpriteEngine;
    AnmLoaded *anm = se->GetAnm(8);

    if (anm->file == NULL)
    {
        anm = se->PreloadAnm(8, "etama.anm");
    }
    mgr->etamaAnm = anm;
    if (mgr->etamaAnm == NULL)
    {
        return -1;
    }
    for (i = 0; i < ARRAY_SIZE(g_BulletTypeScripts); i++)
    {
        mgr->etamaAnm->StartScript(&mgr->typeSprites[i].sprite0, g_BulletTypeScripts[i][0]);
        mgr->etamaAnm->StartScript(&mgr->typeSprites[i].sprite1, g_BulletTypeScripts[i][1]);
        mgr->etamaAnm->StartScript(&mgr->typeSprites[i].sprite2, g_BulletTypeScripts[i][2]);
        mgr->etamaAnm->StartScript(&mgr->typeSprites[i].sprite3, g_BulletTypeScripts[i][3]);
        mgr->etamaAnm->StartScript(&mgr->typeSprites[i].sprite4, g_BulletTypeScripts[i][4]);
        mgr->typeSprites[i].sprite0.flags |= 0x2000;
        mgr->typeSprites[i].sprite1.flags |= 0x2000;
        mgr->typeSprites[i].sprite2.flags |= 0x2000;
        mgr->typeSprites[i].sprite3.flags |= 0x2000;
        mgr->typeSprites[i].sprite4.flags |= 0x2000;
        mgr->typeSprites[i].unk_d44 = mgr->typeSprites[i].sprite0.spriteIndex;
        mgr->typeSprites[i].sprite0.baseSpriteIndex = mgr->typeSprites[i].sprite0.spriteIndex;
        mgr->typeSprites[i].height = mgr->typeSprites[i].sprite0.spriteData->height;
        if (mgr->typeSprites[i].sprite0.spriteData->height <= 8.0f)
        {
            mgr->typeSprites[i].hitboxSize.x = 4.0f;
            mgr->typeSprites[i].hitboxSize.y = 4.0f;
            mgr->typeSprites[i].sizeClass = 5;
        }
        else if (mgr->typeSprites[i].sprite0.spriteData->height <= 16.0f)
        {
            switch (g_BulletTypeScripts[i][0])
            {
            case 2:
            case 0x65:
            case 0x66:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 4;
                break;
            case 4:
            case 6:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 4;
                break;
            case 5:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 3;
            case 0x60:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 4;
                break;
            case 0x61:
            case 0x62:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 4;
                break;
            case 0x6a:
                mgr->typeSprites[i].hitboxSize.x = 4.0f;
                mgr->typeSprites[i].hitboxSize.y = 4.0f;
                mgr->typeSprites[i].sizeClass = 4;
                break;
            default:
                mgr->typeSprites[i].hitboxSize.x = 6.0f;
                mgr->typeSprites[i].hitboxSize.y = 6.0f;
                mgr->typeSprites[i].sizeClass = 3;
            }
        }
        else if (mgr->typeSprites[i].sprite0.spriteData->height <= 32.0f)
        {
            switch (g_BulletTypeScripts[i][0])
            {
            case 8:
            case 0x67:
            case 0x68:
            case 0x69:
            case 0x6b:
                mgr->typeSprites[i].hitboxSize.x = 5.0f;
                mgr->typeSprites[i].hitboxSize.y = 5.0f;
                mgr->typeSprites[i].sizeClass = 2;
                break;
            case 9:
            case 0x63:
            case 0x64:
                mgr->typeSprites[i].hitboxSize.x = 8.0f;
                mgr->typeSprites[i].hitboxSize.y = 8.0f;
                mgr->typeSprites[i].sizeClass = 1;
                break;
            default:
                mgr->typeSprites[i].hitboxSize.x = 10.0f;
                mgr->typeSprites[i].hitboxSize.y = 10.0f;
                mgr->typeSprites[i].sizeClass = 1;
            }
        }
        else
        {
            mgr->typeSprites[i].sizeClass = 0;
            mgr->typeSprites[i].hitboxSize.x = 24.0f;
            mgr->typeSprites[i].hitboxSize.y = 24.0f;
        }
    }
    return 0;
}

void BulletManager::RemoveChain(BulletManager *mgr)
{
    if (mgr != NULL)
    {
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}



void RunBulletCommands(Bullet *bullet)
{
    BulletCommand *cmd;

    int stop;

    while (bullet->commandIndex < 18)
    {
        cmd = &bullet->commands[bullet->commandIndex];
        if (cmd->flag == 0)
        {
            return;
        }
        if (cmd->concurrentFlag == 0 && bullet->activeFlags != 0)
        {
            return;
        }
        if ((bullet->flags & cmd->flag) == 0)
        {
            bullet->commandIndex++;
            continue;
        }
        switch (cmd->flag)
        {
        case 1:
            bullet->activeFlags |= 1;
            bullet->ex[0].timer = 0;
            bullet->ex[0].accel.z = 0.0f;
            break;
        case 0x10:
            bullet->activeFlags |= 0x10;
            bullet->ex[1].turnSpeed = cmd->arg0;
            bullet->ex[1].turnAngle = cmd->arg1 > -990.0f ? cmd->arg1 : bullet->angle;
            bullet->ex[1].timer = 0;
            bullet->ex[1].duration = cmd->frames;
            SetPolar((Vector2 *)&bullet->ex[1].accel, bullet->ex[1].turnAngle, g_App.gameSpeed * bullet->ex[1].turnSpeed);
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x20:
            bullet->activeFlags |= 0x20;
            bullet->ex[2].turnSpeed = cmd->arg0;
            bullet->ex[2].turnAngle = cmd->arg1;
            bullet->ex[2].timer = 0;
            bullet->ex[2].duration = cmd->frames;
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x40:
        case 0x80:
        case 0x100:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[3].turnAngle = cmd->arg0;
            bullet->ex[3].turnSpeed = cmd->arg1 > -999.0f ? cmd->arg1 : bullet->speed;
            bullet->ex[3].timer = 0;
            bullet->ex[3].duration = cmd->frames;
            bullet->ex[3].maxTurns = cmd->maxTurns;
            bullet->ex[3].turnCount = 0;
            break;
        case 0x400:
        case 0x800:
            bullet->activeFlags |= cmd->flag;
            if (cmd->arg0 >= 0.0f)
            {
                bullet->ex[4].turnSpeed = cmd->arg0;
            }
            else
            {
                bullet->ex[4].turnSpeed = bullet->speed;
            }
            bullet->ex[4].maxTurns = cmd->frames;
            bullet->ex[4].duration = 0;
            break;
        case 0x400000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[6].timer = cmd->frames;
            break;
        case 0x800000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[6].timer = cmd->frames;
            break;
        case 0x20000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[5].timer = cmd->frames;
            break;
        case 0x2000:
            bullet->boundsCheckDelay = cmd->frames;
            bullet->commandIndex++;
            continue;
        case 0x4000:
            bullet->sprites = bullet->mgr->typeSprites[cmd->frames];
            bullet->mgr->etamaAnm->SetSprite(&bullet->sprites.sprite0,
                                             bullet->sprites.sprite0.spriteIndex + cmd->maxTurns);
            bullet->commandIndex++;
            continue;
        case 0x40000:
            bullet->state = 5;
            break;
        case 0x80000:
            g_SoundPlayer.QueueSoundPanned(cmd->frames, bullet->pos.x);
            bullet->commandIndex++;
            continue;
        case 0x1000000:
        {
            BulletShooter shooter;



            int frames;

            shooter.pos = bullet->pos;
            frames = cmd->frames;
            stop = cmd->frames & 0x80000000;
            shooter.aimMode = ((DWORD)frames & 0x7f000000) >> 24;
            shooter.sprite = ((BYTE *)&cmd->frames)[2];
            shooter.color = ((BYTE *)&cmd->frames)[1];
            shooter.commandIndex = frames & 0xff;
            shooter.count1 = cmd->maxTurns;
            shooter.speed1 = cmd->arg0;
            shooter.speed2 = cmd->arg1;
            cmd++;
            bullet->commandIndex++;
            shooter.count2 = cmd->frames;
            shooter.flags = cmd->maxTurns;
            shooter.angle1 = cmd->arg0;
            shooter.angle2 = cmd->arg1;
            memcpy(shooter.commands, bullet->commands, sizeof(shooter.commands));
            bullet->mgr->FirePattern(&shooter);
            bullet->commandIndex++;
            if (stop)
            {
                bullet->state = 5;
                break;
            }
            continue;
        }
        }
        bullet->commandIndex++;
        return;
    }
}

#ifdef TH09_NATIVE_NETPLAY
void BulletManager::SnapshotProjectiles(bool *occupied) const {
    for(int i=0;i<0x218;++i) occupied[i]=bullets[i].state!=0;
    for(int i=0;i<0x30;++i) occupied[0x218+i]=lasers[i].active!=0;
}
void BulletManager::AdvanceNewProjectiles(const bool *occupied) {
    for(int i=0;i<0x218;++i) {
        Bullet *b=&bullets[i];
        if(occupied[i] || !b->state || b->state==6)continue;
        if(b->state==1) {
            RunBulletCommands(b);
            if(b->activeFlags&1) SlowBulletStart(b);
            if(b->activeFlags&0x10) AccelerateAlongVelocity(b);
            if(b->activeFlags&0x20) TurnAndAccelerate(b);
            if(b->activeFlags&0x40) StopTurnAndRestart(b);
            if(b->activeFlags&0x100) StopTurnToAngle(b);
            if(b->activeFlags&0x80) StopTurnToPlayer(b);
            if(b->activeFlags&0xc00) BounceOffWalls(b);
            if(b->activeFlags&0x400000) WrapAroundX(b);
            if(b->activeFlags&0x800000) WrapAroundY(b);
            if(b->activeFlags&0x20000) {
                if(b->ex[5].timer<=0)b->activeFlags^=0x20000;else b->ex[5].timer--;
            }
            if(b->boundsCheckDelay)b->boundsCheckDelay--;
            b->pos+=b->velocity;
            if(!b->boundsCheckDelay && !g_GameManager.IsInPlayArea(b->pos.x,b->pos.y,
                b->sprites.sprite0.spriteData->width,b->sprites.sprite0.spriteData->height)) {
                if(b->activeFlags&0xdc0) {
                    if(++b->outOfBoundsFrames>=128){b->FreeBullet();continue;}
                } else if(!b->outOfBoundsFrames){b->FreeBullet();continue;}
                else --b->outOfBoundsFrames;
            } else b->outOfBoundsFrames=0;
            if(b->sprites.sprite0.scriptPos)g_SpriteEngine->Step(&b->sprites.sprite0);
        } else if(b->state>=2 && b->state<=4) {
            int state=b->state;
            b->grazeDelayTimer--;
            b->pos+=b->velocity/(state==2?2.0f:state==3?2.5f:3.0f);
            Sprite *sprite=state==2?&b->sprites.sprite1:state==3?&b->sprites.sprite2:&b->sprites.sprite3;
            if(g_SpriteEngine->Step(sprite)) {b->state=1;b->timer=0;}
        } else if(b->state==5) {
            b->pos+=b->velocity/2.0f;
            if(g_SpriteEngine->Step(&b->sprites.sprite4)){b->FreeBullet();continue;}
        }
        b->timer++;b->grazeDelayTimer++;
    }
    for(int i=0;i<0x30;++i) {
        Laser &laser=lasers[i];if(occupied[0x218+i] || !laser.active)continue;
        laser.endOffset+=g_App.gameSpeed*laser.speed;
        if(laser.endOffset-laser.startOffset>laser.maxLength)laser.startOffset=laser.endOffset-laser.maxLength;
        if(laser.startOffset<0)laser.startOffset=0;
        if(laser.state==0 && laser.timer>=laser.startFrames){laser.state=1;laser.timer=0;laser.baseWidth=laser.width;}
        if(laser.state==1 && laser.timer>=laser.durationFrames){laser.state=2;laser.timer=0;}
        if((laser.state==2 && laser.timer>=laser.endFrames) || laser.startOffset>=640){laser.active=0;continue;}
        laser.timer++;g_SpriteEngine->Step(&laser.sprite0);
    }
}
#endif
int BulletManager::OnUpdate(BulletManager *mgr)
{
    Bullet *bullet;
    int i;
    int j;
    int fadeTime;
    int res;
    Laser *las;
    int alpha;
    float scale;

    bullet = mgr->bullets;
    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    g_App.SetCamera(mgr->sideIdx);
    mgr->bulletCount = 0;
    mgr->bulletCount1 = 0;
    mgr->bulletCount2 = 0;
    mgr->ClearDrawLists();
    for (i = 0; i < 0x218; bullet++, i++)
    {
        if (bullet->state == 0 || bullet->state == 6)
        {
            continue;
        }
        if (mgr->side->flag0)
        {
            bullet->next = mgr->drawLists[bullet->sprites.sizeClass];
            mgr->drawLists[bullet->sprites.sizeClass] = bullet;
            continue;
        }
        mgr->bulletCount++;
        if (i < 0xaf)
        {
            mgr->bulletCount1++;
        }
        else
        {
            mgr->bulletCount2++;
        }
        switch (bullet->state)
        {
        spawned:
            bullet->state = 1;
            bullet->timer = 0;
        case 1:
            RunBulletCommands(bullet);
            if (bullet->activeFlags != 0)
            {
                if (bullet->activeFlags & 1)
                {
                    SlowBulletStart(bullet);
                }
                if (bullet->activeFlags & 0x10)
                {
                    AccelerateAlongVelocity(bullet);
                }
                if (bullet->activeFlags & 0x20)
                {
                    TurnAndAccelerate(bullet);
                }
                if (bullet->activeFlags & 0x40)
                {
                    StopTurnAndRestart(bullet);
                }
                if (bullet->activeFlags & 0x100)
                {
                    StopTurnToAngle(bullet);
                }
                if (bullet->activeFlags & 0x80)
                {
                    StopTurnToPlayer(bullet);
                }
                if (bullet->activeFlags & 0xc00)
                {
                    BounceOffWalls(bullet);
                }
                if (bullet->activeFlags & 0x400000)
                {
                    WrapAroundX(bullet);
                }
                if (bullet->activeFlags & 0x800000)
                {
                    WrapAroundY(bullet);
                }
                DWORD flags = bullet->activeFlags;
                if (flags & 0x20000)
                {
                    if (bullet->ex[5].timer <= 0)
                    {
                        bullet->activeFlags = flags ^ 0x20000;
                    }
                    else
                    {
                        bullet->ex[5].timer--;
                    }
                }
            }
            if (bullet->boundsCheckDelay != 0)
            {
                bullet->boundsCheckDelay--;
            }
            bullet->pos += bullet->velocity;
            if (bullet->boundsCheckDelay == 0)
            {
                if (!g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1],
                                                  bullet->sprites.sprite0.spriteData->width,
                                                  bullet->sprites.sprite0.spriteData->height))
                {
                    if (bullet->activeFlags & 0xdc0)
                    {
                        bullet->outOfBoundsFrames++;
                        if (bullet->outOfBoundsFrames >= 0x80)
                        {
                            bullet->FreeBullet();
                            continue;
                        }
                    }
                    else
                    {
                        if (bullet->outOfBoundsFrames == 0)
                        {
                            bullet->FreeBullet();
                            continue;
                        }
                        bullet->outOfBoundsFrames--;
                    }
                }
                else
                {
                    bullet->outOfBoundsFrames = 0;
                }
            }
            if (!bullet->harmless)
            {
                if (bullet->grazed == 0)
                {
                    res = mgr->side->player->CheckGraze(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    if (res == 1)
                    {
                        bullet->grazed = 1;
                        goto checkHit;
                    }
                    if (res == 2 && !(bullet->flags & 0x1000))
                    {
                        bullet->state = 5;
                    }
                }
                else
                {
                checkHit:
                    res = mgr->side->player->CheckHitAreas(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    if (res != 0 && (res != 2 || !(bullet->flags & 0x1000)))
                    {
                        bullet->state = 5;
                    }
                    else
                    {
                        mgr->side->player->unk_36c.Add(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    }
                }
            }
            if (bullet->sprites.sprite0.scriptPos != NULL)
            {
                g_SpriteEngine->Step(&bullet->sprites.sprite0);
            }
            break;
        case 2:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite1) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 3:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 2.5f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite2) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 4:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 3.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite3) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 5:
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite4) != 0)
            {
                bullet->FreeBullet();
                continue;
            }
        }
        bullet->timer++;
        bullet->grazeDelayTimer++;
        bullet->next = mgr->drawLists[bullet->sprites.sizeClass];
        mgr->drawLists[bullet->sprites.sizeClass] = bullet;
    }
    if (!mgr->side->flag0)
    {
        las = mgr->lasers;
        D3DXVECTOR3 center;
        D3DXVECTOR3 extent;
        for (j = 0x30; j != 0; j--, las++)
        {
            if (!las->active)
            {
                continue;
            }
            las->endOffset = g_App.gameSpeed * las->speed + las->endOffset;
            if (las->endOffset - las->startOffset > las->maxLength)
            {
                las->startOffset = las->endOffset - las->maxLength;
            }
            if (las->startOffset < 0.0f)
            {
                las->startOffset = 0.0f;
            }
            extent.y = las->width / 2.0f;
            center.y = las->pos.y;
            scale = las->endOffset - las->startOffset;
            extent.x = scale;
            center.x = scale / 2.0f + las->startOffset + las->pos.x;
            las->sprite0.scaleX = las->width / las->sprite0.spriteData->width;
            las->sprite0.scaleY = scale / las->sprite0.spriteData->height;
            las->sprite0.SetRotationZ(AddNormalizeAngle(las->angle + 1.5707964f, 0.0f));
            switch (las->state)
            {
            case 0:
                if (las->flags & 1)
                {
                    alpha = las->timer.AsFloat() * 255.0f / las->startFrames;
                    if (alpha > 0xff)
                    {
                        alpha = 0xff;
                    }
                    las->sprite0.color = alpha << 24;
                }
                else
                {
                    int frames = las->startFrames;

                    fadeTime = frames > 30 ? 30 : frames;
                    if (frames - fadeTime < las->timer.AsFrames())
                    {
                        scale = las->timer.AsFloat() * las->width / frames;
                    }
                    else
                    {
                        scale = 1.2f;
                    }
                    las->baseWidth = scale;
                    las->sprite0.scaleX = scale / 16.0f;
                    extent.x = scale / 2.0f;
                }
                if (las->timer >= las->hitStartFrame)
                {
                    mgr->side->player->unk_36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                }
                if (las->timer < las->startFrames)
                {
                    break;
                }
                las->timer = 0;
                las->state++;
                las->baseWidth = las->width;
            case 1:
                mgr->side->player->unk_36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                if (las->timer < las->durationFrames)
                {
                    break;
                }
                las->timer = 0;
                las->state++;
                if (las->endFrames == 0)
                {
                    las->active = 0;
                    continue;
                }
            case 2:
                if (las->flags & 1)
                {
                    alpha = las->timer.AsFloat() * 255.0f / las->startFrames;
                    if (alpha > 0xff)
                    {
                        alpha = 0xff;
                    }
                    las->sprite0.color = alpha << 24;
                }
                else
                {
                    int frames = las->endFrames;

                    if (frames > 0)
                    {
                        scale = las->width - las->timer.AsFloat() * las->width / frames;
                        las->sprite0.scaleX = scale / 16.0f;
                        extent.x = scale / 2.0f;
                    }
                }
                if (las->timer < las->endHitFrames)
                {
                    mgr->side->player->unk_36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                }
                if (las->timer < las->endFrames)
                {
                    break;
                }
                las->active = 0;
                continue;
            }
            if (las->startOffset >= 640.0f)
            {
                las->active = 0;
            }
            las->timer++;
            g_SpriteEngine->Step(&las->sprite0);
        }
        if (mgr->clearAllFrames != 0)
        {
            mgr->clearAllFrames--;
        }
        mgr->updateTimer++;
        mgr->updateFrameCount++;
    }
    return 1;
}

BulletManager::BulletManager()
{
    this->Initialize();
}

BulletManager *BulletManager::RegisterChain(int sideIdx)
{
    BulletManager *mgr =
        (BulletManager *)g_MemoryTracker.Register(new BulletManager(), sizeof(BulletManager), "EtamaCtrlInf");

    mgr->Initialize();
    mgr->sideIdx = sideIdx;
    mgr->side = &g_GameManager.sides[sideIdx];
    mgr->otherSide = &g_GameManager.sides[1 - sideIdx];
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)BulletManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)BulletManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, sideIdx + 0xc) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)BulletManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, sideIdx + 0x15);
    return mgr;
}
