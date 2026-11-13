#include <string.h>

#include "ascii.h"
#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"
#include "sound.h"
#include "sprite.h"
#include "unsorted.h"
#ifdef TH09_NATIVE_NETPLAY
#include "CoopGame.h"
#include "NetGame.h"
#endif

Enemy *EnemyManager::Spawn(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored,
                                int usePlayerEcl)
{
#ifdef TH09_NATIVE_NETPLAY
#endif
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        enemy->mirrored = mirrored;
        if (life >= 0)
        {
            enemy->life = life;
#ifdef TH09_NATIVE_NETPLAY
            if(coop::Active() && this->sideIdx==0 && usePlayerEcl && sub==2)enemy->life*=2;
#endif
        }
        enemy->pos = *pos;
#ifdef TH09_NATIVE_NETPLAY
        thnet::InitEnemyRandom(enemy,sub);
#endif
        if (mirrored)
        {
            enemy->pos.x = -enemy->pos.x;
        }
        if (!usePlayerEcl)
        {
            this->ecl.StartSub(&enemy->ctx, sub);
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->ctx, sub);
            if (this->playerEcl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        enemy->color = enemy->primarySprite.color;
        enemy->item = (char)itemDrop;
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->life;
        break;
    }
    this->spawnFailed = i == 0x80;
    return enemy;
}

Enemy *EnemyManager::SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score,
                                        EnemyScriptVars *vars, int usePlayerEcl)
{
#ifdef TH09_NATIVE_NETPLAY
#endif
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
#ifdef TH09_NATIVE_NETPLAY
        thnet::InitEnemyRandom(enemy,sub);
#endif
        if (!usePlayerEcl)
        {
            this->ecl.StartSub(&enemy->ctx, sub);
            enemy->ctx.vars = *vars;
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->ctx, sub);
            enemy->ctx.vars = *vars;
            if (this->playerEcl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        enemy->color = enemy->primarySprite.color;
        enemy->item = itemDrop;
        if (life >= 0)
        {
            enemy->life =
#ifdef TH09_NATIVE_NETPLAY
                coop::BossHealth(enemy,life);
#else
                life;
#endif
        }
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->life;
        break;
    }
    this->spawnFailed = i == 0x80;
    return enemy;
}



static void MoveEnemyEffects(Enemy *enemy)
{
    Effect *eff;
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        eff = enemy->effects[i];
        if (eff == NULL)
        {
            continue;
        }
        eff->unk_48 = enemy->pos;
        if (eff->unk_7c < enemy->effectRadius)
        {
            eff->unk_7c += 0.3f;
        }
        else
        {
            eff->unk_7c = enemy->effectRadius;
        }
        eff->unk_80 = AddNormalizeAngle(eff->unk_80, 0.031415928f);
    }
}

void Enemy::ReleaseEffects()
{
    int i;

    for (i = 0; i < this->effectCount; i++)
    {
        if (this->effects[i] == NULL)
        {
            continue;
        }
        this->effects[i]->fadeOut = 1;
        this->effects[i] = NULL;
    }
    this->effectCount = 0;
}

int EnemyManager::ClearEnemiesForBonus(int maxBonus, int start)
{
    Enemy *enemy = this->enemies;
    int total = start;
    int bonus = 2000;
    int k;
    int j;

    for (k = 0; k < 0x80; k++, enemy++)
    {
        if (!enemy->active)
        {
            continue;
        }
        if (enemy->isBoss)
        {
            continue;
        }
        if (enemy->undying)
        {
            continue;
        }
        enemy->life = 0;
        if (enemy->grazeable)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            g_AsciiText.AddPopup(this->sideIdx, &enemy->absPos, bonus, bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
            total += bonus;
            bonus += 30;
            if (bonus > maxBonus)
            {
                bonus = maxBonus;
            }
            if (enemy->trailFlags)
            {
                for (j = 0; j < enemy->trailSegmentCount; j += 6)
                {
                    g_AsciiText.AddPopup(this->sideIdx, &enemy->trailBoxes[j].pos, bonus,
                                         bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
                    total += bonus;
                    bonus += 30;
                    if (bonus > maxBonus)
                    {
                        bonus = maxBonus;
                    }
                }
            }
        }
        if (enemy->deathSub >= 0)
        {
            this->ecl.StartSub(&enemy->ctx, enemy->deathSub);
            enemy->deathSub = -1;
        }
    }
    return total;
}

float LerpAngleShort(float from, float to, float t)
{
    float diff;
    float wrapped;

    if (from < to)
    {
        diff = to - from;
        wrapped = from + 6.2831855f - to;
    }
    else
    {
        diff = from - to;
        wrapped = to + 6.2831855f - from;
        from = to;
    }
    if (!(diff < wrapped))
    {
        diff = wrapped;
    }
    return diff * t + from;
}

Enemy *EnemyManager::FindByUnknownType(int type)
{
    int i;
    Enemy *enemy = this->enemies;

    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->unknownType == type && enemy->active)
        {
            return enemy;
        }
    }
    return NULL;
}

void PlayerUnknown36c::Add(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, void *unk_2c)
{
    if (this->count < 0x80)
    {
        this->entries[this->count].pos = *pos;
        this->entries[this->count].size = *size;
        this->entries[this->count].size.x /= 2.0f;
        this->entries[this->count].size.y /= 2.0f;
        this->entries[this->count].radius = 0.0f;
        this->entries[this->count].angle = 0.0f;
        this->entries[this->count].unk_2c = unk_2c;
        this->count++;
    }
}

void Enemy::Move()
{
    this->posDelta = this->pos - this->prevPos;
    this->prevPos = this->pos;
    if (this->useVelocity2)
    {
        this->pos.x += g_App.gameSpeed * this->velocity2.x;
        this->pos.y += g_App.gameSpeed * this->velocity2.y;
        return;
    }
    if (!this->mirrored)
    {
        this->pos.x += g_App.gameSpeed * this->velocity.x;
    }
    else
    {
        this->pos.x -= g_App.gameSpeed * this->velocity.x;
    }
    this->pos.y += g_App.gameSpeed * this->velocity.y;
}

void EnemyManager::Initialize()
{
    int i;
    Enemy *enemy;

    memset(this, 0, sizeof(EnemyManager));
    for (i = 0; i < ARRAY_SIZE(this->timelineSignals); i++)
    {
        this->timelineSignals[i] = -1;
    }
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    enemy->manager = this;
    for (i = 0; i < 2; i++)
    {
        enemy->sprites[i].scriptIndex = -1;
    }
    for (i = 0; i < 0x60; i++)
    {
        enemy->trailBoxes[i].pos.x = -999.0f;
    }
    enemy->active = 1;
    enemy->callbackTimer = 0;
    enemy->onScreen = 0;
    enemy->hitboxSize = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->moveMode = 0;
    enemy->holdFire = 0;
    enemy->mirrored = 0;
    enemy->isBoss = 0;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathEffect = 0;
    enemy->deathEffect2 = 0;
    enemy->deathEffect3 = 0;
    enemy->shootInterval = 0;
    enemy->shootTimer = 0;
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->turnLeft = -1;
    enemy->turnRight = -1;
    enemy->turnStill = -1;
    enemy->shootable = 1;
    enemy->contactDamage = 1;
    enemy->hidden = 0;
    enemy->hitByShots = 1;
    enemy->grazeable = 0;
    enemy->deathMode = 0;
    enemy->deathSub = -1;
    enemy->clampPos = 0;
    enemy->effectCount = 0;
    enemy->pendingInterrupt = -1;
    for (i = 0; i < 4; i++)
    {
        enemy->lifeCallbackLife[i] = -1;
    }
    enemy->timerCallbackTime = -1;
    enemy->laserIndex = 0;
    enemy->hitFlashTimer = 0;
    enemy->rotateSprite = 0;
    enemy->rankSpeedLow = -0.15f;
    enemy->rankSpeedHigh = 0.15f;
    enemy->shooter.sound = 7;
    enemy->shooter.bulletSound = 0x19;
    enemy->minShootDistSq = 1024.0f;
}

int EnemyTimeline::Run()
{
    if (g_GameManager.gui->IsMsgRunning())
    {
        return 0;
    }
    if (this->delay.AsFloat() > 0.0f)
    {
        this->delay--;
        return 0;
    }
    while (this->pos->time >= 0)
    {
        if (this->timer == this->pos->time)
        {
            if (!(this->pos->difficulties & g_GameManager.difficultyMask))
            {
                goto next;
            }
            switch (this->pos->opcode)
            {
            case 16:
                g_GameManager.isInContinueMenu = 1;
                break;
            case 0:
            case 1:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->Spawn(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 17:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;
                Enemy *enemy;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                enemy = this->mgr->Spawn(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
                enemy->unknownBit9 = 1;
            }
                break;
            case 15:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->Spawn(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 12:
                this->mirror = 1;
            case 11:
            {
                EnemyTimelineSpawnWithArgs *args = (EnemyTimelineSpawnWithArgs *)this->pos->args;
                D3DXVECTOR3 pos;
                Enemy *enemy;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                enemy = this->mgr->Spawn(args->sub, &pos, args->life, -1, args->score, this->mirror, 0);
                enemy->pointItems = args->pointItems;
                enemy->powerItems = args->powerItems;
            }
                break;
            case 4:
                this->mirror = 1;
            case 2:
            {
                EnemyTimelineSpawnRandomX *args = (EnemyTimelineSpawnRandomX *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = g_Rng.GetRandomF32InRange(args->xMax - args->xMin) + args->xMin;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->Spawn(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 5:
                this->mirror = 1;
            case 3:
            {
                EnemyTimelineSpawnAnyX *args = (EnemyTimelineSpawnAnyX *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = g_Rng.GetRandomF32InRange(384.0f);
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->Spawn(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 8:
                this->mgr->bosses[this->pos->args[0]]->pendingInterrupt = this->pos->args[1];
                break;
            case 10:
                if (this->mgr->bosses[this->pos->args[0]] != NULL && this->mgr->bosses[this->pos->args[0]]->active)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            case 13:
            {
                int count;
                int j;

                count = 0;
                for (j = 0; j < ARRAY_SIZE(this->mgr->timelineSignals); j++)
                {
                    if (this->mgr->timelineSignals[j] == this->pos->args[0])
                    {
                        count++;
                        this->mgr->timelineSignals[j] = -1;
                    }
                }
                if (count == 0)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            }
            case 14:
            {
                int k;

                for (k = 0; k < ARRAY_SIZE(this->mgr->timelineSignals); k++)
                {
                    if (this->mgr->timelineSignals[k] >= 0)
                    {
                        continue;
                    }
                    this->mgr->timelineSignals[k] = this->pos->args[0];
                }
                break;
            }
            }
        }
        else if (this->timer < this->pos->time)
        {
            break;
        }
    next:
        this->pos = (EnemyTimelineEntry *)((char *)this->pos + this->pos->size);
    }
tick:
    if (this->pos->time < 0)
    {
        this->pos = NULL;
        this->timer = 0;
        return 1;
    }
    this->timer += g_GameManager.timelineSpeed;
    return 0;
}

void Enemy::FreeThreads()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->threads[i] != NULL)
        {
            g_MemoryTracker.Free(this->threads[i]);
            this->threads[i] = NULL;
        }
    }
}



void Enemy::Despawn()
{
    int i;

    if (this->deathMode == 0)
    {
        this->active = 0;
    }
    if (this->isBoss && this->bossIndex < 4)
    {
        this->manager->bosses[this->bossIndex] = NULL;
        this->isBoss = 0;
    }
    if (this->effectCount != 0)
    {
        this->ReleaseEffects();
    }
    if (this->isBoss)
    {
        this->manager->bosses[this->bossIndex] = NULL;
    }
    for (i = 0; i < 4; i++)
    {
        this->lifeCallbackLife[i] = -1;
    }
    this->timerCallbackTime = -1;
    this->FreeThreads();
    if (this->manager->side->player->homingEnemy == this)
    {
        this->manager->side->player->homingEnemy = NULL;
    }
}

void Enemy::ClampPos()
{
    float *pos;

    if (this->clampPos)
    {
        pos = this->pos;
        if (pos[0] < this->lowerBound[0])
        {
            pos[0] = this->lowerBound[0];
        }
        else if (pos[0] > this->upperBound[0])
        {
            pos[0] = this->upperBound[0];
        }
        if (pos[1] < this->lowerBound[1])
        {
            pos[1] = this->lowerBound[1];
        }
        else if (pos[1] > this->upperBound[1])
        {
            pos[1] = this->upperBound[1];
        }
    }
}

void Enemy::CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 box;

    box = *size / 1.5f;
    this->manager->side->player->unk_36c.Add(pos, &box, NULL);
}

void Enemy::DropItems(int kind)
{
    D3DXVECTOR3 effectPos;
    D3DXVECTOR3 effectVelocity;
    Effect *effect;
    EnemyManager *mgr;
    float radius;
    int count;
    int a;
    int b;
    int c;
    int d;
    char deathEffect;

    if (kind)
    {
        this->manager->side->player->unk_30410.ResetCombo();
    }
    if (this->unknownBit13)
    {
        g_SoundPlayer.QueueSoundPanned(0x12, this->absPos.x);
        this->manager->side->player->unk_30410.AddComboCounted(&this->absPos, 400, 0, 300, 1000);
        this->manager->side->player->AddUnknown30454(this->item, &this->pos);
        return;
    }
    if (this->unknownType)
    {
        g_SoundPlayer.QueueSoundPanned(0x12, this->absPos.x);
        this->manager->side->player->AddUnknown30454(g_Rng.GetRandomU16InRange(4), &this->pos);
    }
    g_SoundPlayer.QueueSoundPanned(this->index % 2 + 2, this->absPos.x);
    if (this->unk_3380 && kind && !this->useVelocity2)
    {
        this->manager->side->effectManager->SpawnEffect(5, &this->absPos, 1, 0xffffffff);
    }
    else
    {
        this->manager->side->effectManager->SpawnEffect(this->deathEffect + 0x14, &this->absPos, 1, 0xffffffff);
        deathEffect = this->deathEffect;
        if (!this->unk_3380)
        {
            if (deathEffect == 0)
            {
                radius = 5.0f;
            }
            else if (deathEffect == 1)
            {
                radius = 5.5f;
            }
            else if (deathEffect == 2)
            {
                radius = 6.0f;
            }
            else
            {
                radius = 6.25f;
            }
            this->manager->side->player->AddKind2Circle(&this->absPos, 32.0f, radius, 3, deathEffect + 8, 4);
        }
        else
        {
            if (deathEffect == 0)
            {
                radius = 5.0f;
            }
            else if (deathEffect == 1)
            {
                radius = 5.5f;
            }
            else if (deathEffect == 2)
            {
                radius = 6.0f;
            }
            else
            {
                radius = 6.25f;
            }
            this->manager->side->player->AddKind2Circle(&this->absPos, 32.0f, radius, 2, deathEffect + 8, 4);
        }
    }
    if (this->unk_3380 == 0)
    {
        a = 4;
    }
    else if (this->unk_3380 == 1)
    {
        a = 10;
    }
    else if (this->unk_3380 == 2)
    {
        a = 20;
    }
    else
    {
        a = this->unk_3380 == 3 ? 30 : 50;
    }
    count = this->manager->side->player->unk_30410.unk_38;
    b = count > 10 ? 60 : count * 3 + 30;
    if (this->unk_3380)
    {
        c = 0;
    }
    else
    {
        c = count > 24 ? 28 : count + 4;
    }
    if (!this->unk_3380)
    {
        d = count > 100 ? 3000 : count * 30 + 10;
    }
    else
    {
        d = count > 50 ? 11000 : (count + 5) * 200;
    }
    if (kind)
    {
        d *= 2;
    }
    this->manager->side->player->unk_30410.AddComboCounted(&this->absPos, b, c, a, d);
    count = this->manager->side->player->unk_30410.unk_38;
    if (count > 128)
    {
        radius = 2.0f;
    }
    else
    {
        radius = count * 0.0078125f + 1.0f;
    }
    this->manager->side->player->AddUnknown30388(radius);
    if (this->unk_3380 && this->unk_3380 < 3 && kind < 2)
    {
        mgr = this->manager;
        if (mgr->otherSide->enemyManager->unk_2ac3b8 < 25)
        {
            effectPos.x = g_GameManager.ArenaToScreenX(this->pos.x);
            effectPos.y = g_GameManager.ArenaToScreenY(this->pos.y);
            effectPos.z = 0.0f;
            g_App.SetCamera(1 - mgr->sideIdx);
            effectVelocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
            effectVelocity.y = g_Rng.GetRandomF32InRange(128.0f);
            effectVelocity.z = 0.0f;
            effect = g_GameManager.sides[2].effectManager->SpawnEffectWithVelocity(
                this->manager->sideIdx + 3, &effectPos, &effectVelocity, 1, 0xffffffff);
            g_App.SetCamera(this->manager->sideIdx);
            effect->unk_a4 = this->unk_3380 > 2 ? 2 : this->unk_3380;
            effect->unk_a6 = this->unk_3380 + 1 > 3 ? 3 : this->unk_3380 + 1;
            effect->unk_a0 = g_Rng.GetRandomF32InRange(0.5f) * 0.016666668f;
            effect->unk_a8 = this->manager->sideIdx;
        }
    }
}

int EnemyManager::OnUpdate(EnemyManager *mgr)
{
    int bomb = 0;
    int i;
    D3DXVECTOR3 boxSize;
    int hurt;
    int hit;
    int seg;
    int kind;
    Enemy *enemy;
    Player *player;
    Enemy **head;

    if (g_GameManager.unknownBits134_11)
    {
        return 1;
    }
    g_GameManager.gui->UpdateEnemyMarker(mgr->sideIdx, &D3DXVECTOR3(-999.0f, 0.0f, 0.0f));
    g_App.SetCamera(mgr->sideIdx);
    mgr->drawListHeads[3] = NULL;
    mgr->drawListHeads[2] = NULL;
    mgr->drawListHeads[1] = NULL;
    mgr->drawListHeads[0] = NULL;
    if (!g_GameManager.gui->unk_1095c
#ifdef TH09_NATIVE_NETPLAY
#endif
        )
    {
        if (mgr->timeline.pos != NULL)
        {
#ifdef TH09_NATIVE_NETPLAY
            thnet::WaveRandom random(mgr->sideIdx,mgr->unk_2ac3d4,mgr->timeline.timer.AsFrames());
#endif
            mgr->timeline.Run();
        }
        else if (!mgr->unk_2ac3b0 || mgr->unk_2ac3c8 > 0)
        {
            mgr->timeline.mirror = g_TimelineOrder[mgr->unk_2ac3d4] >> 7;
            mgr->timeline.pos =
                (EnemyTimelineEntry *)mgr->ecl.GetTimeline(g_TimelineOrder[mgr->unk_2ac3d4++] & 0x7f);
            mgr->timeline.timer = 0;
        }
    }
    mgr->enemyCount = 0;
    mgr->unk_2ac3b0 = 0;
    mgr->unk_2ac444 = NULL;
    mgr->unk_2ac448 = NULL;
    mgr->unk_2ac3b4 = 0;
    mgr->unk_2ac3b8 = 0;
    enemy = mgr->enemies;
    if (mgr->side->player->unk_364 != NULL)
    {
        mgr->unk_2ac3c8 = 0;
    }
    else
    {
        mgr->unk_2ac3c8++;
    }
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (!enemy->active)
        {
            if (mgr->side->player->homingEnemy == enemy)
            {
                mgr->side->player->homingEnemy = NULL;
            }
            continue;
        }
        if (mgr->side->flag0 && enemy->unknownType != 1 && enemy->unknownType != 2)
        {
            if (enemy->hidden)
            {
                continue;
            }
            head = &mgr->drawListHeads[enemy->drawListIndex];
            enemy->nextInDrawList = *head;
            *head = enemy;
            continue;
        }
        hit = 0;
        if (enemy->unknownFlag8)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
            goto die;
        }
        mgr->enemyCount++;
        if (!(enemy->flags2 & 0x4dc0))
        {
            mgr->unk_2ac3b0++;
        }
        if (enemy->unknownType)
        {
            mgr->unk_2ac3b4++;
        }
        if (enemy->unk_3380)
        {
            mgr->unk_2ac3b8++;
        }
        if (enemy->useVelocity2)
        {
            if (enemy->unk_5424.HasTickedEvery(2))
            {
                mgr->side->effectManager->SpawnEffect(9, &enemy->pos, 1, 0xffffffff);
            }
            if (enemy->unk_5424 > mgr->unk_2ac44c)
            {
                BulletShooter shooter;

                shooter.pos = enemy->pos;
                shooter.sprite = 1;
                shooter.aimMode = 0;
                shooter.speed1 = g_GameManager.rank.unk_b4 * 0.1f + 2.0f;
                shooter.count2 = 1;
                shooter.angle1 = 0.0f;
                shooter.angle2 = 0.15707964f;
                shooter.speed2 = 0.0f;
                shooter.unk_1fa = 0;
                shooter.flags = 4;
                shooter.count1 = (mgr->side->character != 13) * 2 + 1;
                shooter.color = 2;
                mgr->side->bulletManager->FirePattern(&shooter);
                enemy->active = 0;
                enemy->Despawn();
                continue;
            }
        }
        if ((enemy->haltedByBomb && mgr->side->player->GetPlayerState()) || enemy->halted)
        {
            enemy->callbackTimer--;
            goto flash;
        }
    again:
        if (mgr->ecl.RunEnemy(enemy) == -1)
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (!enemy->isWarpingToBoss)
        {
            enemy->ClampPos();
            enemy->Move();
            enemy->ClampPos();
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        else
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        if (enemy->trailFlags)
        {
            for (seg = enemy->trailSegmentCount - 1; seg > 0; seg--)
            {
                enemy->trailBoxes[seg].pos = enemy->trailBoxes[seg - 1].pos;
                enemy->trailBoxes[seg].velocity = enemy->trailBoxes[seg - 1].velocity;
                enemy->trailBoxes[seg].angle = enemy->trailBoxes[seg - 1].angle;
            }
            enemy->trailBoxes[0].pos = enemy->absPos;
            enemy->trailBoxes[0].velocity = enemy->velocity;
            enemy->trailBoxes[0].angle = enemy->angle;
        }
        if (enemy->primarySprite.spriteData == NULL)
        {
            enemy->hidden = 1;
        }
        if (!enemy->hidden && !enemy->onScreen &&
            g_GameManager.IsInPlayArea(enemy->absPos[0], enemy->absPos[1], enemy->primarySprite.spriteData->width,
                                       enemy->primarySprite.spriteData->height))
        {
            enemy->onScreen = 1;
        }
        else if (enemy->onScreen == 1 && !enemy->staysOffscreen &&
                 ((!enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height)) ||
                  (enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height) &&
                   !g_GameManager.IsInPlayArea(enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.x,
                                               enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height))))
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (enemy->CheckLifeCallback())
        {
            goto again;
        }
        if (enemy->timerCallbackTime >= 0 && Enemy::CheckTimerCallback())
        {
            goto again;
        }
        enemy->primarySprite.color = enemy->color;
        g_SpriteEngine->Step(&enemy->primarySprite);
        enemy->color = enemy->primarySprite.color;
        for (seg = 0; seg < 2; seg++)
        {
            if (enemy->sprites[seg].scriptIndex >= 0 && g_SpriteEngine->Step(&enemy->sprites[seg]))
            {
                enemy->sprites[seg].scriptIndex = -1;
            }
        }
        kind = 0;
        if (!enemy->hidden && !enemy->hiddenByBomb)
        {
            if (enemy->shootable && (!enemy->unk_3380 || mgr->side->character != 11))
            {
                enemy->CheckPlayerContact(&enemy->absPos, &enemy->hitboxSize);
                if (enemy->trailFlags)
                {
                    boxSize = enemy->hitboxSize;
                    for (seg = 1; seg < enemy->trailContactSegments; seg += 6)
                    {
                        if (enemy->trailFlags & 2)
                        {
                            boxSize = enemy->hitboxSize -
                                      enemy->hitboxSize * (float)seg / (float)enemy->trailContactSegments;
                        }
                        enemy->CheckPlayerContact(&enemy->trailBoxes[seg].pos, &boxSize);
                    }
                }
            }
            enemy->lastDamage = 0;
            if (enemy->hitByShots)
            {
                hurt = mgr->side->player->CalcDamage(&enemy->absPos, &enemy->hitboxSize, &enemy->unk_2e5c,
                                                     &bomb, &enemy->unk_2e60);
                if (enemy->extraHitboxSize.x > 0.0f)
                {
                    mgr->side->player->CalcDamage(&enemy->absPos, &enemy->extraHitboxSize, &enemy->unk_2e5c,
                                                  &bomb, &enemy->unk_2e60);
                }
                if (hurt > 0)
                {
                    if (enemy->contactDamage)
                    {
                        if (enemy->invulnTimer > 0)
                        {
                            if (enemy->isBoss)
                            {
                                hurt /= 9;
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        if (hurt == enemy->unk_2e5c)
                        {
                            kind = 1;
                        }
                        if (hurt == enemy->unk_2e60)
                        {
                            kind = 2;
                        }
                        if (enemy->unk_3380)
                        {
                            if (!enemy->useVelocity2)
                            {
                                hurt = (hurt - enemy->unk_2e5c) * 4 + enemy->unk_2e5c / 4;
                            }
                            else
                            {
                                hurt = (hurt - enemy->unk_2e5c) * 4 + enemy->unk_2e5c / 2;
                            }
                        }
#ifdef TH09_NATIVE_NETPLAY
                        if (!coop::DamageBoss(enemy, hurt))
#endif
                            enemy->life -= hurt;
                        enemy->lastDamage = hurt;
                    }
                    hit = 1;
                }
                D3DXVECTOR3 targetDiff = mgr->side->player->orbHomingTarget - mgr->side->player->pos;
                D3DXVECTOR3 toward = enemy->absPos - mgr->side->player->pos;
                if (targetDiff.x * targetDiff.x + targetDiff.y * targetDiff.y > toward.x * toward.x + toward.y * toward.y)
                {
                    mgr->side->player->orbHomingTarget = enemy->absPos;
                }
                player = mgr->side->player;
                if (Fabs(enemy->absPos.x - player->pos.x) < 64.0f &&
                    (player->homingEnemy == NULL || player->homingEnemy->pos.y > enemy->absPos.y))
                {
                    player->homingEnemy = enemy;
                }
            }
        }
        if (enemy->hasDied && enemy->life > 0)
        {
            enemy->hasDied = 0;
        }
        if (enemy->life <= 0 && !enemy->hasDied && !enemy->undying)
        {
            int k;

        die:
            enemy->hasDied = 1;
            enemy->phaseEndSecondsLeft = (enemy->timerCallbackTime - enemy->callbackTimer.AsFrames()) / 60;
            enemy->timerCallbackTime = -1;
            for (k = 0; k < 4; k++)
            {
                enemy->lifeCallbackLife[k] = -1;
            }
            for (k = 0; k < 4; k++)
            {
                if (enemy->threads[k] != NULL)
                {
                    g_MemoryTracker.Free(enemy->threads[k]);
                    enemy->threads[k] = NULL;
                }
            }
            switch (enemy->deathMode)
            {
            case 3:
                enemy->contactDamage = 0;
                enemy->deathMode = 0;
                enemy->life = 1;
                if (enemy->deathEffect >= 0)
                {
                    mgr->side->effectManager->SpawnEffect(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                    mgr->side->effectManager->SpawnEffect(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                    mgr->side->effectManager->SpawnEffect(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                }
                if (mgr->side->player->GetPlayerState() == 0)
                {
                    mgr->side->player->stateTimer = 90;
                    mgr->side->player->SetPlayerState(3);
                }
                enemy->haltedByBomb = 0;
                enemy->intangibleDuringBomb = 0;
                break;
            case 1:
                mgr->side->AddScore(enemy->score);
                enemy->unk_bit20 = 1;
                enemy->shootable = 0;
                enemy->contactDamage = 0;
                enemy->hitByShots = 0;
                goto dead;
            case 0:
                mgr->side->AddScore(enemy->score);
                enemy->active = 0;
            dead:
                if (enemy->isBoss)
                {
                    enemy->ReleaseEffects();
                }
            case 2:
                enemy->life = 0;
                break;
            }
            if (!enemy->unknownFlag8)
            {
                enemy->DropItems(kind);
            }
            if (enemy->deathSub >= 0)
            {
                int l;

                enemy->ResetRankSpeeds();
                enemy->stackDepth = 0;
                for (l = 0; l < 4; l++)
                {
                    enemy->lifeCallbackLife[l] = -1;
                }
                enemy->timerCallbackTime = -1;
                enemy->FreeThreads();
                enemy->shooter = enemy->manager->enemyTemplate.shooter;
                enemy->shootInterval = 0;
                mgr->ecl.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
    flash:
        if (enemy->hitFlashTimer)
        {
            enemy->hitFlashTimer--;
            enemy->primarySprite.useColor2 = 0;
        }
        else if (hit)
        {
            if (enemy->lifeCallbackProximity < 2)
            {
                g_SoundPlayer.QueueSoundPanned(0x14, enemy->absPos.x);
            }
            else
            {
                g_SoundPlayer.QueueSoundPanned(0x25, enemy->absPos.x);
            }
            enemy->primarySprite.color2Rgb[2] = 0xff;
            enemy->primarySprite.color2Rgb[1] = 0x60;
            enemy->primarySprite.color2Rgb[0] = 0x80;
            enemy->primarySprite.color2Alpha = enemy->primarySprite.alpha;
            enemy->primarySprite.useColor2 = 1;
            enemy->hitFlashTimer = 1;
        }
        else
        {
            enemy->primarySprite.useColor2 = 0;
        }
        if (enemy->isBoss)
        {
            D3DXVECTOR3 markerPos;

            if (enemy->isBoss < 4)
            {
                if (!enemy->hidden)
                {
                    markerPos.x = enemy->absPos.x + 32.0f;
                }
                else
                {
                    markerPos.x = -999.0f;
                }
                markerPos.y = 472.0f;
                markerPos.z = 0.0f;
                g_AsciiText.SetBossMarkerPos(enemy->bossIndex, &markerPos);
                if (enemy->lifeCallbackProximity == 0)
                {
                    g_AsciiText.SetBossMarkerState(enemy->bossIndex, enemy->primarySprite.useColor2);
                }
                else
                {
                    g_AsciiText.SetBossMarkerState(enemy->bossIndex, enemy->lifeCallbackProximity + 1);
                }
            }
        }
        if ((enemy->unknownBit9 && mgr->unk_2ac444 == NULL) || enemy->useVelocity2)
        {
            mgr->unk_2ac444 = enemy;
        }
        if (mgr->unk_2ac448 == NULL)
        {
            mgr->unk_2ac448 = enemy;
        }
        if (enemy->unknownType == 3)
        {
            g_GameManager.gui->UpdateEnemyMarker(mgr->sideIdx, &enemy->pos);
            mgr->unk_2ac444 = enemy;
        }
        else if (enemy->unknownBit13)
        {
            mgr->unk_2ac444 = enemy;
        }
        MoveEnemyEffects(enemy);
        enemy->callbackTimer++;
        if (enemy->invulnTimer > 0)
        {
            enemy->invulnTimer--;
        }
        if (!enemy->hidden && enemy->active)
        {
            head = &mgr->drawListHeads[enemy->drawListIndex];
            enemy->nextInDrawList = *head;
            *head = enemy;
        }
    }
    mgr->frameTimer++;
    return 1;
}



struct EnemySpriteScale
{
    float x;
    float y;
};




static int DrawEnemies(EnemyManager *mgr, int from, int to)
{
    float uSpan2;
    float cs2;
    float theta;
    float uDecr;
    VertexTex1DiffuseXyzrwh *out;
    float lastTheta;
    float tu;
    float sinA;
    int vertices;
    float width;
    float height;
    Enemy *enemy;
    int b;
    Sprite *sprite;
    D3DCOLOR savedColor;
    int i;
    EnemySpriteScale savedScale;
    BYTE taper;
    int step;
    int next;

    for (i = from; i < to; i++)
    {
        enemy = mgr->drawListHeads[i];
        while (enemy != NULL)
        {
            for (sprite = enemy->sprites, b = 0; b < 1; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(enemy->angle);
                    }
                    if (!enemy->useMainSpriteOffset)
                    {
                        sprite->pos = enemy->absPos + sprite->posOffset;
                    }
                    else
                    {
                        sprite->pos = enemy->absPos + enemy->primarySprite.posOffset;
                    }
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_GameManager.ArenaToScreenX(sprite->pos.x);
                    sprite->pos.y = g_GameManager.ArenaToScreenY(sprite->pos.y);
                    g_SpriteEngine->Draw(sprite);
                }
            }
            if (enemy->rotateSprite)
            {
                enemy->primarySprite.SetRotationZ(enemy->angle);
            }
            enemy->primarySprite.pos = enemy->absPos + enemy->primarySprite.posOffset;
            enemy->primarySprite.pos.x = g_GameManager.ArenaToScreenX(enemy->primarySprite.pos.x);
            enemy->primarySprite.pos.y = g_GameManager.ArenaToScreenY(enemy->primarySprite.pos.y);
            enemy->primarySprite.pos.z = 0.25f;
            if (enemy->trailFlags)
            {
                savedScale = *(EnemySpriteScale *)&enemy->primarySprite.scaleX;
                savedColor = enemy->primarySprite.color;
                if (!(enemy->trailFlags & 8))
                {
                    for (b = enemy->trailSegmentCount - 1; b > 0; b -= enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            continue;
                        }
                        if (enemy->rotateSprite)
                        {
                            enemy->primarySprite.SetRotationZ(enemy->trailBoxes[b].angle);
                        }
                        if (enemy->trailFlags & 2)
                        {
                            enemy->primarySprite.scaleX = savedScale.x - b * savedScale.x / enemy->trailSegmentCount;
                        }
                        if (enemy->trailFlags & 4)
                        {
                            enemy->primarySprite.alpha =
                                ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                        }
                        enemy->primarySprite.pos = enemy->trailBoxes[b].pos + enemy->primarySprite.posOffset;
                        enemy->primarySprite.pos.z = 0.3f;
                        enemy->primarySprite.pos.x = g_GameManager.ArenaToScreenX(enemy->primarySprite.pos.x);
                        enemy->primarySprite.pos.y = g_GameManager.ArenaToScreenY(enemy->primarySprite.pos.y);
                        g_SpriteEngine->Draw(&enemy->primarySprite);
                    }
                }
                else
                {
                    vertices = 0;
                    for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            break;
                        }
                        vertices += 2;
                    }
                    if (vertices > 2)
                    {
                        uSpan2 = enemy->primarySprite.spriteData->uvEndX - enemy->primarySprite.spriteData->uvStartX;
                        uDecr = uSpan2 / ((vertices + 1) / 2 - 1);
                        tu = enemy->primarySprite.spriteData->uvEndX + enemy->primarySprite.uvScrollX;
                        out = (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices;
                        for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep, tu -= uDecr)
                        {
                            if (enemy->trailBoxes[b].pos.x < -990.0f)
                            {
                                break;
                            }
                            if (b == 0)
                            {
                                theta = enemy->trailBoxes[0].angle;
                            }
                            else
                            {
                                theta = LerpAngleShort(enemy->trailBoxes[b - 1].angle, enemy->trailBoxes[b].angle, 0.5f);
                            }
                            taper = enemy->trailFlags & 2;
                            if (taper && b > 0)
                            {
                                step = enemy->trailSegmentStep;
                                next = b + step;
                                if (next < enemy->trailSegmentCount && Fabs(lastTheta - theta) < 0.00001f)
                                {
                                    float interpolated = LerpAngleShort(enemy->trailBoxes[next - 1].angle,
                                                                       enemy->trailBoxes[step].angle, 0.5f);
                                    long double difference = theta - interpolated;
                                    if (Fabs((float)difference) < 0.00001f)
                                    {
                                        vertices -= 2;
                                        continue;
                                    }
                                }
                            }
                            lastTheta = theta;
                            sinA = sinf(theta);
                            cs2 = cosf(theta);
                            width = 0.0f;
                            height = savedScale.y * enemy->primarySprite.spriteData->height / 2.0f;
                            if (taper)
                            {
                                theta = 1.0f - (float)b / enemy->trailSegmentCount;
                                width *= theta;
                                height *= theta;
                            }
                            out[1].diffuse = enemy->primarySprite.color;
                            out[0].diffuse = out[1].diffuse;
                            if (enemy->trailFlags & 4)
                            {
                                ((BYTE *)&out[1].diffuse)[3] =
                                    ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                                ((BYTE *)&out[0].diffuse)[3] = ((BYTE *)&out[1].diffuse)[3];
                            }
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width - sinA * height + out->pos.x + 32.0f;
                            out->pos.y = sinA * width + cs2 * height + out->pos.y + 16.0f;
                            out->uv.x = tu;
                            out->uv.y = enemy->primarySprite.spriteData->uvStartY + enemy->primarySprite.uvScrollY;
                            out++;
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width + sinA * height + out->pos.x + 32.0f;
                            out->pos.y = sinA * width - cs2 * height + out->pos.y + 16.0f;
                            out->uv.x = tu;
                            out->uv.y = enemy->primarySprite.spriteData->uvEndY + enemy->primarySprite.uvScrollY;
                            out++;
                        }
                        if (vertices > 2)
                        {
                            g_SpriteEngine->DrawVertexStrip(&enemy->primarySprite,
                                                            (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices, vertices);
                        }
                    }
                }
                *(EnemySpriteScale *)&enemy->primarySprite.scaleX = savedScale;
                enemy->primarySprite.color = savedColor;
            }
            if (!(enemy->trailFlags & 0x10) && !enemy->hiddenByBomb)
            {
                g_SpriteEngine->Draw(&enemy->primarySprite);
            }
            for (b = 1; b < 2; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(-enemy->angle);
                    }
                    if (!enemy->useMainSpriteOffset)
                    {
                        sprite->pos = enemy->absPos + sprite->posOffset;
                    }
                    else
                    {
                        sprite->pos = enemy->absPos + enemy->primarySprite.posOffset;
                    }
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_GameManager.ArenaToScreenX(sprite->pos.x);
                    sprite->pos.y = g_GameManager.ArenaToScreenY(sprite->pos.y);
                    g_SpriteEngine->Draw(sprite);
                }
            }
            enemy = enemy->nextInDrawList;
        }
    }
    return 1;
}

int EnemyManager::OnDraw(EnemyManager *mgr)
{
    g_App.SetView2D(mgr->sideIdx);
    return DrawEnemies(mgr, 0, 2);
}

int EnemyManager::OnDraw2(EnemyManager *mgr)
{
    g_App.SetView2D(mgr->sideIdx);
    return DrawEnemies(mgr, 2, 4);
}

int EnemyManager::AddedCallback(EnemyManager *mgr)
{
    int i;
    SpriteEngine *se = g_SpriteEngine;
    AnmLoaded *anm = se->GetAnm(9);

    if (anm->file == NULL)
    {
        if (g_GameManager.stage == 0 || g_GameManager.stage == 3 || g_GameManager.stage == 7)
        {
            mgr->anm = se->PreloadAnm(9, "enemy1.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
        else if (g_GameManager.stage == 12 || g_GameManager.stage == 15)
        {
            mgr->anm = se->PreloadAnm(9, "enemy13.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
        else
        {
            mgr->anm = se->PreloadAnm(9, "enemy.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
    }
    else
    {
        mgr->anm = anm;
    }
    mgr->otherPlayerAnm = g_SpriteEngine->GetAnm(6 - mgr->sideIdx);
    memset(&mgr->ecl, 0, sizeof(EnemyScripts));
    if (mgr->ecl.Load("enemy.ecl") != 0)
    {
        return -1;
    }
    if (mgr->playerEcl.Load(g_CharacterFiles[mgr->otherSide->character][2]) != 0)
    {
        return -1;
    }
    if (mgr->sideIdx == 0)
    {
        for (i = 0; i < 0x100; i++)
        {
            g_TimelineOrder[i] = g_Rng.GetRandomU32InRange(mgr->ecl.GetTimelineCount());
            g_TimelineOrder[i] |= g_Rng.GetRandomU32InRange(2) << 7;
        }
    }
    mgr->timeline.mgr = mgr;
    mgr->timeline.pos = (EnemyTimelineEntry *)mgr->ecl.GetTimeline(g_TimelineOrder[mgr->unk_2ac3d4++] & 0x7f);
    mgr->timeline.delay = 200;
    mgr->unk_2ac44c = mgr->side->character == 5 ? 200 : 160;
    return 0;
}

static int FreeResources(EnemyManager *mgr)
{
    int i;
    Enemy *enemy;

    enemy = mgr->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        enemy->FreeThreads();
    }
    g_SpriteEngine->ReleaseAnm(9);
    mgr->ecl.Unload();
    mgr->playerEcl.Unload();
    return 0;
}

void EnemyManager::RemoveChain(EnemyManager *mgr)
{
    if (mgr != NULL)
    {
        FreeResources(mgr);
        g_Chain.Remove(mgr->calcElem);
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->drawElem2);
        delete mgr;
    }
}

void EnemyManager::Reset()
{
    int i;
    Enemy *enemy;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        enemy->FreeThreads();
        enemy->active = 0;
    }
    if (this->sideIdx == 0)
    {
        for (i = 0; i < 0x100; i++)
        {
            g_TimelineOrder[i] = g_Rng.GetRandomU32InRange(this->ecl.GetTimelineCount());
            g_TimelineOrder[i] |= g_Rng.GetRandomU32InRange(2) << 7;
        }
    }
    this->unk_2ac3d4 = 0;
    this->timeline.pos = (EnemyTimelineEntry *)this->ecl.GetTimeline(g_TimelineOrder[0] & 0x7f);
    this->timeline.delay = 200;
    this->timeline.timer = 0;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

EnemyManager *EnemyManager::RegisterChain(int sideIdx)
{
    EnemyManager *mgr =
        (EnemyManager *)g_MemoryTracker.Register(new EnemyManager(), sizeof(EnemyManager), "EnemyCtrlInf");

    mgr->Initialize();
    mgr->sideIdx = sideIdx;
    mgr->side = &g_GameManager.sides[sideIdx];
    mgr->otherSide = &g_GameManager.sides[1 - sideIdx];
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)EnemyManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)EnemyManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, sideIdx + 0xa) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)EnemyManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, sideIdx + 0xa);
    mgr->drawElem2 = g_Chain.CreateElem((ChainCallback)EnemyManager::OnDraw2);
    mgr->drawElem2->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem2, sideIdx + 0x11);
    return mgr;
}
