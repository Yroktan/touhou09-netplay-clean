

#include <string.h>

#include "player.h"
#include "enemy.h"
#include "gui.h"

void Player::GetMoveDelta(int direction, int focused, float *deltaX, float *deltaY)
{
    *deltaY = 0.0f;
    *deltaX = 0.0f;
    if (focused)
    {
        switch (direction)
        {
        case 4:
            *deltaX = this->shtFile->speedFocused;
            break;
        case 3:
            *deltaX = -this->shtFile->speedFocused;
            break;
        case 1:
            *deltaY = -this->shtFile->speedFocused;
            break;
        case 2:
            *deltaY = this->shtFile->speedFocused;
            break;
        case 5:
            *deltaY = *deltaX = -this->shtFile->speedDiagonalFocused;
            break;
        case 7:
            *deltaX = -(*deltaY = this->shtFile->speedDiagonalFocused);
            break;
        case 6:
            *deltaY = -(*deltaX = this->shtFile->speedDiagonalFocused);
            break;
        case 8:
            *deltaY = *deltaX = this->shtFile->speedDiagonalFocused;
            break;
        }
    }
    else
    {
        switch (direction)
        {
        case 4:
            *deltaX = this->shtFile->speed;
            break;
        case 3:
            *deltaX = -this->shtFile->speed;
            break;
        case 1:
            *deltaY = -this->shtFile->speed;
            break;
        case 2:
            *deltaY = this->shtFile->speed;
            break;
        case 5:
            *deltaY = *deltaX = -this->shtFile->speedDiagonal;
            break;
        case 7:
            *deltaX = -(*deltaY = this->shtFile->speedDiagonal);
            break;
        case 6:
            *deltaY = -(*deltaX = this->shtFile->speedDiagonal);
            break;
        case 8:
            *deltaY = *deltaX = this->shtFile->speedDiagonal;
            break;
        }
    }
    *deltaX *= this->moveScaleX * this->unk_1cdc;
    *deltaY *= this->moveScaleY * this->unk_1ce0;
}

void PlayerUnknown24::Run()
{
    int dir = 0;
    int focus = 0;
    D3DXVECTOR3 sizes[3];
    float margins[3];
    D3DXVECTOR3 pos;
    int region;
    int k;
    int j;
    int limit;
    int alt;
    int newDir;
    float dx;
    float dy;
    Player *player;
    PlayerCpuLevel *level;
    EnemyManager *em;
    Enemy *enemy;

    margins[0] = 48.0f;
    margins[1] = 16.0f;
    margins[2] = 0.0f;
    memset(&g_PlayerInputs[this->player->sideIdx], 0, 0x58);
    k = 0;
    if (g_GameManager.gui->unk_1095c != 0 || g_GameManager.gui->IsMsgRunning())
    {
        return;
    }
    player = this->player;
    level = &g_PlayerCpuLevels[player->side->unk_2c];
    if (g_GameManager.IsMode1())
    {
        player->otherSide->gui->SetUnknown76d4Time(level->seconds5c[2] * 60 - this->timer5c.AsFrames());
    }
    if (!(this->flags & 1))
    {
        if (!this->player->side->flag0 && this->player->otherSide->player->unk_30410.unk_3c <= 360)
        {
            this->timer5c++;
        }
        player = this->player;
        if (player->unk_a8 >= 10)
        {
            limit = level->seconds5c[0];
        }
        else if (player->unk_a8 >= 2)
        {
            limit = level->seconds5c[1];
        }
        else
        {
            limit = level->seconds5c[2];
        }
        if (g_GameManager.IsMode1() && player->GetPlayerState() != 1)
        {
            player->SetPlayerState(3);
            this->player->stateTimer = 2;
        }
        if (this->timer5c.AsFrames() / 60 >= limit)
        {
            this->flags |= 1;
            if (g_GameManager.IsMode1())
            {
                this->player->otherSide->gui->StartUnknown7430();
                this->player->otherSide->gui->SetUnknown76d4Time(-1);
            }
        }
    }
    else if (!(this->flags & 2))
    {
        this->timer68++;
        if (this->player->unk_a8 >= 10)
        {
            limit = level->seconds68[0];
        }
        else if (this->player->unk_a8 >= 2)
        {
            limit = level->seconds68[1];
        }
        else
        {
            limit = level->seconds68[2];
        }
        if (this->timer68.AsFrames() / 60 >= limit)
        {
            this->flags |= 2;
        }
    }
    g_GameManager.rank.unk_bc = level->rank;
    if (this->player->unk_30388 >= this->unk_58)
    {
        this->unk_c = 1;
    }
    else if (this->unk_c)
    {
        g_PlayerInputs[this->player->sideIdx].unk_34 |= 1;
        this->unk_c = 0;
    }
    player = this->player;
    if (player->pos.x < -96.0f)
    {
        region = 0;
    }
    else if (player->pos.x < 0.0f)
    {
        region = 1;
    }
    else if (player->pos.x < 96.0f)
    {
        region = 2;
    }
    else
    {
        region = 3;
    }
    if (player->pos.y < 288.0f)
    {
        region += 4;
    }
    if (player->pos.y < 192.0f)
    {
        region += 4;
    }
    sizes[0].x = 28.0f;
    sizes[0].y = 28.0f;
    sizes[1].x = 10.0f;
    sizes[1].y = 10.0f;
    sizes[2] = player->hitboxSize;
    if (player->side->enemyManager->unk_2ac3b8 >= 4)
    {
        g_PlayerInputs[player->sideIdx].unk_2c |= 4;
        focus = 1;
    }
    g_PlayerCpuDirOrder[region][1] = this->lastDirection;
    if (!(this->flags & 2) || this->player->otherSide->player->unk_30410.unk_3c > 300)
    {
        if (this->counter != 0)
        {
            this->counter--;
            for (k = 0; k < 2; k++)
            {
                dir = this->lastDirection;
                this->player->GetMoveDelta(dir, focus, &dx, &dy);
                pos = this->player->pos;
                pos.x += dx;
                pos.y += dy;
                if (pos.x < g_GameManager.playerArea[0])
                {
                    pos.x = g_GameManager.playerArea[0];
                }
                else if (pos.x > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
                {
                    pos.x = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
                }
                if (pos.y < g_GameManager.playerArea[1])
                {
                    pos.y = g_GameManager.playerArea[1];
                }
                else if (pos.y > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
                {
                    pos.y = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
                }
                if (this->player->unk_36c.CheckHitsAt(&pos, &sizes[k], margins[k]) == NULL)
                {
                    goto found;
                }
            }
        }
        for (;;)
        {
            for (k = 0; k < 3; k++)
            {
                for (j = k; j < 10; j++)
                {
                    dir = g_PlayerCpuDirOrder[region][j];
                    this->player->GetMoveDelta(dir, focus, &dx, &dy);
                    pos = this->player->pos;
                    pos.x += dx;
                    pos.y += dy;
                    if (pos.x < g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[0];
                    }
                    else if (pos.x > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
                    }
                    if (pos.y < g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[1];
                    }
                    else if (pos.y > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
                    }
                    if (this->player->unk_36c.CheckHitsAt(&pos, &sizes[k], margins[k]) == NULL)
                    {
                        goto found;
                    }
                }
            }
            if (!(g_PlayerInputs[this->player->sideIdx].unk_2c & 4))
            {
                break;
            }
            g_PlayerInputs[this->player->sideIdx].unk_2c &= ~4;
            focus = 0;
        }
        if (!(this->flags & 1))
        {
            player = this->player;
            if (player->GetPlayerState() == 0 && player->unk_1b74 == 0)
            {
                if (this->unk_c && player->unk_30384 >= 100.0f)
                {
                    g_PlayerInputs[player->sideIdx].unk_34 |= 1;
                    this->unk_c = 0;
                    return;
                }
                g_PlayerInputs[player->sideIdx].unk_32 |= 2;
                g_PlayerInputs[this->player->sideIdx].unk_2c |= 2;
            }
        }
        this->lastDirection = dir;
        return;
    }
found:
    if (this->player->unk_30384 >= this->unk_58)
    {
        g_PlayerInputs[this->player->sideIdx].unk_34 |= 1;
        this->unk_c = 0;
        this->unk_58 = ((int)g_Rng.GetRandomU32InRange(4) + 1.0f) * 100.0f;
        if (this->unk_58 >= 400.0f)
        {
            this->unk_58 = 400.0f;
        }
    }
    else if (this->unk_c)
    {
        g_PlayerInputs[this->player->sideIdx].unk_2c |= 1;
    }
    if (g_PlayerCpuDirButtons[dir] != 0 && !(this->flags & 2))
    {
        if (this->counter == 0 && this->history2[0] == this->history2[1] && this->history2[1] == this->history2[2] &&
            this->history2[2] == this->history2[3] && this->history2[3] == this->history2[4] &&
            this->history2[4] == this->history2[5])
        {
            alt = g_PlayerCpuAltDirs[g_Rng.GetRandomU16InRange(2)][dir];
            this->player->GetMoveDelta(alt, focus, &dx, &dy);
            pos = this->player->pos;
            pos.x += dx;
            pos.y += dy;
            if (pos.x < g_GameManager.playerArea[0])
            {
                pos.x = g_GameManager.playerArea[0];
            }
            else if (pos.x > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
            {
                pos.x = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
            }
            if (pos.y < g_GameManager.playerArea[1])
            {
                pos.y = g_GameManager.playerArea[1];
            }
            else if (pos.y > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
            {
                pos.y = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
            }
            if (k < 2)
            {
                k++;
            }
            if (this->player->unk_36c.CheckHitsAt(&pos, &sizes[k], margins[k]) == NULL)
            {
                dir = alt;
                this->counter = 8;
            }
        }
        g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
        if (this->counter == 0)
        {
            for (j = 7; j > 0; j--)
            {
                this->history2[j] = this->history2[j - 1];
            }
        }
        this->history2[0] = g_PlayerCpuDirButtons[dir];
    }
    else
    {
        D3DXVECTOR3 target;

        target.x = -1000.0f;
        if (this->player->unk_30f64 > -999.0f)
        {
            target = *(D3DXVECTOR3 *)&this->player->unk_30f64;
        }
        else
        {
            em = this->player->side->enemyManager;
            if (em->unk_2ac444 != NULL)
            {
                enemy = em->unk_2ac444;
                target = enemy->pos;
                target.y += 128.0f;
                if (enemy->unknownType == 3 || enemy->unknownBit13)
                {
                    target.y = 400.0f;
                }
            }
            else if (em->unk_2ac448 != NULL)
            {
                target = em->unk_2ac448->pos;
                target.y += 128.0f;
            }
        }
        if (this->history[0] == this->history[2] && this->history[1] == this->history[3] &&
            this->history[0] != this->history[1])
        {
            target.x = -target.x;
            target.y = g_Rng.GetRandomF32InRange(448.0f);
        }
        if (target.x > -999.0f)
        {
            if (target.y < 448.0f)
            {
                pos = this->player->pos;
                if (target.x + 6.0f < this->player->pos.x)
                {
                    newDir = (target.y + 6.0f < pos.y && pos.y > 48.0f) ? 5 : 7;
                    this->player->GetMoveDelta(newDir, focus, &dx, &dy);
                    pos.x += dx;
                    pos.y += dy;
                    if (pos.x < g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[0];
                    }
                    else if (pos.x > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
                    }
                    if (pos.y < g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[1];
                    }
                    else if (pos.y > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
                    }
                    if (this->player->unk_36c.CheckHitsAt(&pos, &sizes[1], 0.0f) == NULL)
                    {
                        g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[newDir];
                    }
                    else
                    {
                        g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
                    }
                }
                else if (target.x - 6.0f > this->player->pos.x)
                {
                    newDir = (target.y + 6.0f < pos.y && pos.y > 48.0f) ? 6 : 8;
                    this->player->GetMoveDelta(dir, focus, &dx, &dy);
                    pos.x += dx;
                    pos.y += dy;
                    if (pos.x < g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[0];
                    }
                    else if (pos.x > g_GameManager.playerArea[2] + g_GameManager.playerArea[0])
                    {
                        pos.x = g_GameManager.playerArea[2] + g_GameManager.playerArea[0];
                    }
                    if (pos.y < g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[1];
                    }
                    else if (pos.y > g_GameManager.playerArea[3] + g_GameManager.playerArea[1])
                    {
                        pos.y = g_GameManager.playerArea[3] + g_GameManager.playerArea[1];
                    }
                    if (this->player->unk_36c.CheckHitsAt(&pos, &sizes[1], 0.0f) == NULL)
                    {
                        g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[newDir];
                    }
                    else
                    {
                        g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
                    }
                }
                else
                {
                    g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
                }
            }
            else
            {
                g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
            }
        }
        else
        {
            g_PlayerInputs[this->player->sideIdx].unk_2c |= g_PlayerCpuDirButtons[dir];
        }
    }
    for (j = 7; j > 0; j--)
    {
        this->history[j] = this->history[j - 1];
    }
    this->history[0] = g_PlayerCpuDirButtons[dir];
    player = this->player;
    em = player->side->enemyManager;
    if ((em->unk_2ac444 != NULL && (em->unk_2ac444->unknownType == 3 || em->unk_2ac444->unknownBit13)) ||
        player->activeHitAreaCount == 0)
    {
        if (em->enemyCount ? this->timer0.HasTickedEvery(10) : this->timer0.HasTickedEvery(30))
        {
            g_PlayerInputs[player->sideIdx].unk_32 |= 1;
        }
    }
    if (g_GameManager.IsMode1() && (this->flags & 2) &&
        this->player->otherSide->player->unk_30410.unk_3c <= 300)
    {
        g_PlayerInputs[this->player->sideIdx].unk_32 &= ~1;
    }
    this->lastDirection = dir;
    this->timer0++;
}
