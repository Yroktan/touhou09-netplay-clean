




#pragma once

#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"

inline Enemy *EnemyManager::SpawnPlayerEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score,
                                             int mirrored)
{
    return this->Spawn(sub, pos, life, itemDrop, score, mirrored, 1);
}

inline Enemy *EnemyManager::SpawnPlayerEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score,
                                                     EnemyScriptVars *vars)
{
    return this->SpawnWithVars(sub, pos, life, itemDrop, score, vars, 1);
}

inline GameSide *ExAttack::GetSide()
{
    return &g_GameManager.sides[this->side];
}
