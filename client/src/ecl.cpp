#include "bullet.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "ascii.h"
#include "effect.h"
#include "sound.h"
#include "spellcard.h"
#include "stage.h"
#include "unsorted.h"
#ifdef TH09_NATIVE_NETPLAY
#include "CoopGame.h"
#include "NetGame.h"
#endif

void BulletManager::ClearAllToItems()
{
    this->ClearAll(1);
}

int EnemyScripts::Load(char *path)
{
    int i;

    this->file = (EnemyScriptHeader *)LoadFileData(path, NULL, 0);
    if (this->file == NULL)
    {
        LogPrintf(&g_Logger, "敵データの読み込みに失敗しました、データが壊れてるか失われています\r\n");
        return -1;
    }
    if (this->file->version != 0x900)
    {
        LogPrintf(&g_Logger, "敵データのバージョンが違います\r\n");
        return -1;
    }
    for (i = 0; i < this->file->timelineCount; i++)
    {
        this->file->timelines[i] = (char *)this->file->timelines[i] + (int)this->file;
    }
    this->subs = &this->file->timelines[this->file->timelineCount];
    for (i = 0; i < this->file->subCount; i++)
    {
        this->subs[i] = (char *)this->subs[i] + (int)this->file;
    }
    return 0;
}

int GetIntOperand(Enemy *enemy, int id)
{
    switch (id)
    {
    case 10000:
        return enemy->currentCtx->intVars[0];
    case 10001:
        return enemy->currentCtx->intVars[1];
    case 10002:
        return enemy->currentCtx->intVars[2];
    case 10003:
        return enemy->currentCtx->intVars[3];
    case 10004:
        return enemy->currentCtx->intVars[4];
    case 10005:
        return enemy->currentCtx->intVars[5];
    case 10006:
        return enemy->currentCtx->intVars[6];
    case 10007:
        return enemy->currentCtx->intVars[7];
    case 10008:
        return enemy->enemyInts[0];
    case 10009:
        return enemy->enemyInts[1];
    case 10010:
        return enemy->enemyInts[2];
    case 10011:
        return enemy->enemyInts[3];
    case 10012:
        return enemy->enemyInts[4];
    case 10013:
        return enemy->enemyInts[5];
    case 10014:
        return enemy->enemyInts[6];
    case 10015:
        return enemy->enemyInts[7];
    case 10053:
        return enemy->currentCtx->args.ints[0];
    case 10054:
        return enemy->currentCtx->args.ints[1];
    case 10055:
        return enemy->currentCtx->args.ints[2];
    case 10056:
        return enemy->currentCtx->args.ints[3];
    case 10036:
        return enemy->currentCtx->intVars2[0];
    case 10037:
        return enemy->currentCtx->intVars2[1];
    case 10038:
        return enemy->currentCtx->intVars2[2];
    case 10039:
        return enemy->currentCtx->intVars2[3];
    case 10032:
        return g_Rng.GetRandomU32() & 0x7fffffff;
    case 10033:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10034:
        return g_Rng.GetRandomU32();
    case 10035:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10040:
        return g_GameManager.difficulty;
    case 10041:
        return g_GameManager.rank.unk_b4;
    case 10049:
        return enemy->callbackTimer.current;
    case 10051:
        return enemy->life;
    case 10052:
        return enemy->manager->side->character;
    case 10094:
        return enemy->currentCtx->floatVars2[0];
    case 10095:
        return enemy->currentCtx->floatVars2[1];
    case 10016:
        return enemy->currentCtx->floatVars[0];
    case 10017:
        return enemy->currentCtx->floatVars[1];
    case 10018:
        return enemy->currentCtx->floatVars[2];
    case 10019:
        return enemy->currentCtx->floatVars[3];
    case 10020:
        return enemy->currentCtx->floatVars[4];
    case 10021:
        return enemy->currentCtx->floatVars[5];
    case 10022:
        return enemy->currentCtx->floatVars[6];
    case 10023:
        return enemy->currentCtx->floatVars[7];
    case 10024:
        return enemy->enemyFloats[0];
    case 10025:
        return enemy->enemyFloats[1];
    case 10026:
        return enemy->enemyFloats[2];
    case 10027:
        return enemy->enemyFloats[3];
    case 10028:
        return enemy->enemyFloats[4];
    case 10029:
        return enemy->enemyFloats[5];
    case 10030:
        return enemy->enemyFloats[6];
    case 10031:
        return enemy->enemyFloats[7];
    case 10057:
        return enemy->currentCtx->args.floats[0];
    case 10058:
        return enemy->currentCtx->args.floats[1];
    case 10059:
        return enemy->currentCtx->args.floats[2];
    case 10060:
        return enemy->currentCtx->args.floats[3];
    case 10061:
        return enemy->manager->ecl.callArgs.ints[0];
    case 10062:
        return enemy->manager->ecl.callArgs.ints[1];
    case 10063:
        return enemy->manager->ecl.callArgs.ints[2];
    case 10064:
        return enemy->manager->ecl.callArgs.ints[3];
    case 10065:
        return enemy->manager->ecl.callArgs.floats[0];
    case 10066:
        return enemy->manager->ecl.callArgs.floats[1];
    case 10067:
        return enemy->manager->ecl.callArgs.floats[2];
    case 10068:
        return enemy->manager->ecl.callArgs.floats[3];
    case 10042:
        return enemy->absPos.x;
    case 10043:
        return enemy->absPos.y;
    case 10044:
        return enemy->absPos.z;
    case 10045:
        return enemy->manager->side->player->pos.x;
    case 10046:
        return enemy->manager->side->player->pos.y;
    case 10047:
        return enemy->manager->side->player->pos.z;
    case 10074:
        return enemy->moveStart.x;
    case 10075:
        return enemy->moveStart.y;
    case 10076:
        return enemy->moveStart.z;
    case 10085:
        return enemy->posDelta.x;
    case 10086:
        return enemy->posDelta.y;
    case 10087:
        return enemy->posDelta.z;
    case 10088:
        return enemy->lifeCallbackLife[0];
    case 10089:
        return enemy->lifeCallbackLife[1];
    case 10090:
        return enemy->lifeCallbackLife[2];
    case 10091:
        return enemy->lifeCallbackLife[3];
    case 10069:
        return enemy->angle;
    case 10070:
        return enemy->angularVelocity;
    case 10071:
        return enemy->speed;
    case 10072:
        return enemy->acceleration;
    case 10073:
        return enemy->orbitRadius;
    case 10077:
        return enemy->orbitAngle;
    case 10078:
        return enemy->orbitAngularVelocity;
    case 10083:
        return enemy->lastDamage;
    case 10084:
        return enemy->bossIndex;
    case 10092:
        return enemy->item;
    case 10093:
        return enemy->score;
    case 10101:
        return enemy->unknownType < 3 ? enemy->manager->otherSide->player->unk_a0
                                     : enemy->manager->otherSide->player->unk_a4;
    case 10048:
        return enemy->manager->side->player->AngleToPlayer(&enemy->absPos);
    case 10050:
    {
        D3DXVECTOR3 diff = enemy->manager->side->player->pos - enemy->absPos;

        return D3DXVec3Length(&diff);
    }
    }
    return id;
}

int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch (*operand)
    {
    case 10000:
        return &enemy->currentCtx->intVars[0];
    case 10001:
        return &enemy->currentCtx->intVars[1];
    case 10002:
        return &enemy->currentCtx->intVars[2];
    case 10003:
        return &enemy->currentCtx->intVars[3];
    case 10004:
        return &enemy->currentCtx->intVars[4];
    case 10005:
        return &enemy->currentCtx->intVars[5];
    case 10006:
        return &enemy->currentCtx->intVars[6];
    case 10007:
        return &enemy->currentCtx->intVars[7];
    case 10008:
        return &enemy->enemyInts[0];
    case 10009:
        return &enemy->enemyInts[1];
    case 10010:
        return &enemy->enemyInts[2];
    case 10011:
        return &enemy->enemyInts[3];
    case 10012:
        return &enemy->enemyInts[4];
    case 10013:
        return &enemy->enemyInts[5];
    case 10014:
        return &enemy->enemyInts[6];
    case 10015:
        return &enemy->enemyInts[7];
    case 10053:
        return &enemy->currentCtx->args.ints[0];
    case 10054:
        return &enemy->currentCtx->args.ints[1];
    case 10055:
        return &enemy->currentCtx->args.ints[2];
    case 10056:
        return &enemy->currentCtx->args.ints[3];
    case 10036:
        return &enemy->currentCtx->intVars2[0];
    case 10037:
        return &enemy->currentCtx->intVars2[1];
    case 10038:
        return &enemy->currentCtx->intVars2[2];
    case 10039:
        return &enemy->currentCtx->intVars2[3];
    case 10040:
        return &g_GameManager.difficulty;
    case 10041:
        return &g_GameManager.rank.unk_b4;
    case 10049:
        return &enemy->callbackTimer.current;
    case 10051:
        return &enemy->life;
    case 10092:
        return &enemy->item;
    case 10093:
        return &enemy->score;
    case 10061:
        return &enemy->manager->ecl.callArgs.ints[0];
    case 10062:
        return &enemy->manager->ecl.callArgs.ints[1];
    case 10063:
        return &enemy->manager->ecl.callArgs.ints[2];
    case 10064:
        return &enemy->manager->ecl.callArgs.ints[3];
    }
    return operand;
}

float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch ((int)*operand)
    {
    case 10016:
        return &enemy->currentCtx->floatVars[0];
    case 10017:
        return &enemy->currentCtx->floatVars[1];
    case 10018:
        return &enemy->currentCtx->floatVars[2];
    case 10019:
        return &enemy->currentCtx->floatVars[3];
    case 10020:
        return &enemy->currentCtx->floatVars[4];
    case 10021:
        return &enemy->currentCtx->floatVars[5];
    case 10022:
        return &enemy->currentCtx->floatVars[6];
    case 10023:
        return &enemy->currentCtx->floatVars[7];
    case 10024:
        return &enemy->enemyFloats[0];
    case 10025:
        return &enemy->enemyFloats[1];
    case 10026:
        return &enemy->enemyFloats[2];
    case 10027:
        return &enemy->enemyFloats[3];
    case 10028:
        return &enemy->enemyFloats[4];
    case 10029:
        return &enemy->enemyFloats[5];
    case 10030:
        return &enemy->enemyFloats[6];
    case 10031:
        return &enemy->enemyFloats[7];
    case 10057:
        return &enemy->currentCtx->args.floats[0];
    case 10058:
        return &enemy->currentCtx->args.floats[1];
    case 10059:
        return &enemy->currentCtx->args.floats[2];
    case 10060:
        return &enemy->currentCtx->args.floats[3];
    case 10042:
        return &enemy->pos.x;
    case 10043:
        return &enemy->pos.y;
    case 10044:
        return &enemy->pos.z;
    case 10045:
        return &enemy->manager->side->player->pos.x;
    case 10046:
        return &enemy->manager->side->player->pos.y;
    case 10047:
        return &enemy->manager->side->player->pos.z;
    case 10094:
        return &enemy->currentCtx->floatVars2[0];
    case 10095:
        return &enemy->currentCtx->floatVars2[1];
    case 10065:
        return &enemy->manager->ecl.callArgs.floats[0];
    case 10066:
        return &enemy->manager->ecl.callArgs.floats[1];
    case 10067:
        return &enemy->manager->ecl.callArgs.floats[2];
    case 10068:
        return &enemy->manager->ecl.callArgs.floats[3];
    case 10074:
        return &enemy->moveStart.x;
    case 10075:
        return &enemy->moveStart.y;
    case 10076:
        return &enemy->moveStart.z;
    case 10079:
        return &enemy->moveDelta.x;
    case 10080:
        return &enemy->moveDelta.y;
    case 10081:
        return &enemy->moveDelta.z;
    case 10069:
        return &enemy->angle;
    case 10070:
        return &enemy->angularVelocity;
    case 10071:
        return &enemy->speed;
    case 10072:
        return &enemy->acceleration;
    case 10073:
        return &enemy->orbitRadius;
    case 10077:
        return &enemy->orbitAngle;
    case 10078:
        return &enemy->orbitAngularVelocity;
    }
    return operand;
}




static int ScriptReturn(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;

    if (enemy->noCallStack)
    {


    }
    enemy->stackDepth--;
    if (enemy->stackDepth < 0)
    {
        i = enemy->currentCtx->timedCallActive - 1;
        if (enemy->threads[i] != NULL)
        {
            g_MemoryTracker.Free(enemy->threads[i]);
        }
        enemy->threads[i] = NULL;


        enemy->callStack = enemy->savedCtx;
        enemy->currentCtx = &enemy->ctx;
        enemy->stackDepth = enemy->savedStackDepth;
        return 1;
    }
    *enemy->currentCtx = enemy->callStack[enemy->stackDepth];
    return 0;
}




static void ScriptSetTurnSprites(Enemy *enemy, EnemyScriptInstr *instr, int still, int left, int right,
                                 int stillFromLeft, int stillFromRight, int extra)
{
    enemy->turnStill = still;
    enemy->turnLeft = left;
    enemy->turnRight = right;
    enemy->turnStillFromLeft = stillFromLeft;
    enemy->turnStillFromRight = stillFromRight;
    enemy->turnExtra = extra;
    enemy->spriteDirection = 0xff;
}

void EnemyScripts::Unload()
{
    if (this->file != NULL)
    {
        g_MemoryTracker.Free(this->file);
    }
    this->file = NULL;
}

int EnemyScripts::StartSub(EnemyScriptState *ctx, short sub)
{
    if (sub < 0)
    {
        return 0;
    }
    ctx->code = this->subs[sub];
    ctx->timer = 0;
    ctx->waitTimer = 0;
    ctx->sub = sub;
    ctx->ecl = this;
    return 0;
}

float GetFloatOperand(Enemy *enemy, float id)
{
    switch ((int)id)
    {
    case 10000:
        return enemy->currentCtx->intVars[0];
    case 10001:
        return enemy->currentCtx->intVars[1];
    case 10002:
        return enemy->currentCtx->intVars[2];
    case 10003:
        return enemy->currentCtx->intVars[3];
    case 10004:
        return enemy->currentCtx->intVars[4];
    case 10005:
        return enemy->currentCtx->intVars[5];
    case 10006:
        return enemy->currentCtx->intVars[6];
    case 10007:
        return enemy->currentCtx->intVars[7];
    case 10008:
        return enemy->enemyInts[0];
    case 10009:
        return enemy->enemyInts[1];
    case 10010:
        return enemy->enemyInts[2];
    case 10011:
        return enemy->enemyInts[3];
    case 10012:
        return enemy->enemyInts[4];
    case 10013:
        return enemy->enemyInts[5];
    case 10014:
        return enemy->enemyInts[6];
    case 10015:
        return enemy->enemyInts[7];
    case 10053:
        return enemy->currentCtx->args.ints[0];
    case 10054:
        return enemy->currentCtx->args.ints[1];
    case 10055:
        return enemy->currentCtx->args.ints[2];
    case 10056:
        return enemy->currentCtx->args.ints[3];
    case 10036:
        return enemy->currentCtx->intVars2[0];
    case 10037:
        return enemy->currentCtx->intVars2[1];
    case 10038:
        return enemy->currentCtx->intVars2[2];
    case 10039:
        return enemy->currentCtx->intVars2[3];
    case 10032:
        return g_Rng.GetRandomU32() & 0x7fffffff;
    case 10033:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10034:
        return (int)g_Rng.GetRandomU32();
    case 10035:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10082:
        return g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    case 10040:
        return g_GameManager.difficulty;
    case 10041:
        return g_GameManager.rank.unk_b4;
    case 10049:
        return enemy->callbackTimer.current;
    case 10051:
        return enemy->life;
    case 10052:
        return enemy->manager->side->character;
    case 10092:
        return enemy->item;
    case 10093:
        return enemy->score;
    case 10061:
        return enemy->manager->ecl.callArgs.ints[0];
    case 10062:
        return enemy->manager->ecl.callArgs.ints[1];
    case 10063:
        return enemy->manager->ecl.callArgs.ints[2];
    case 10064:
        return enemy->manager->ecl.callArgs.ints[3];
    case 10065:
        return enemy->manager->ecl.callArgs.floats[0];
    case 10066:
        return enemy->manager->ecl.callArgs.floats[1];
    case 10067:
        return enemy->manager->ecl.callArgs.floats[2];
    case 10068:
        return enemy->manager->ecl.callArgs.floats[3];
    case 10016:
        return enemy->currentCtx->floatVars[0];
    case 10017:
        return enemy->currentCtx->floatVars[1];
    case 10018:
        return enemy->currentCtx->floatVars[2];
    case 10019:
        return enemy->currentCtx->floatVars[3];
    case 10020:
        return enemy->currentCtx->floatVars[4];
    case 10021:
        return enemy->currentCtx->floatVars[5];
    case 10022:
        return enemy->currentCtx->floatVars[6];
    case 10023:
        return enemy->currentCtx->floatVars[7];
    case 10024:
        return enemy->enemyFloats[0];
    case 10025:
        return enemy->enemyFloats[1];
    case 10026:
        return enemy->enemyFloats[2];
    case 10027:
        return enemy->enemyFloats[3];
    case 10028:
        return enemy->enemyFloats[4];
    case 10029:
        return enemy->enemyFloats[5];
    case 10030:
        return enemy->enemyFloats[6];
    case 10031:
        return enemy->enemyFloats[7];
    case 10057:
        return enemy->currentCtx->args.floats[0];
    case 10058:
        return enemy->currentCtx->args.floats[1];
    case 10059:
        return enemy->currentCtx->args.floats[2];
    case 10060:
        return enemy->currentCtx->args.floats[3];
    case 10042:
        return enemy->absPos.x;
    case 10043:
        return enemy->absPos.y;
    case 10044:
        return enemy->absPos.z;
    case 10045:
        return enemy->manager->side->player->pos.x;
    case 10046:
        return enemy->manager->side->player->pos.y;
    case 10047:
        return enemy->manager->side->player->pos.z;
    case 10094:
        return enemy->currentCtx->floatVars2[0];
    case 10095:
        return enemy->currentCtx->floatVars2[1];
    case 10074:
        return enemy->moveStart.x;
    case 10075:
        return enemy->moveStart.y;
    case 10076:
        return enemy->moveStart.z;
    case 10079:
        return enemy->moveDelta.x;
    case 10080:
        return enemy->moveDelta.y;
    case 10081:
        return enemy->moveDelta.z;
    case 10085:
        return enemy->posDelta.x;
    case 10086:
        return enemy->posDelta.y;
    case 10087:
        return enemy->posDelta.z;
    case 10088:
        return enemy->lifeCallbackLife[0];
    case 10089:
        return enemy->lifeCallbackLife[1];
    case 10090:
        return enemy->lifeCallbackLife[2];
    case 10091:
        return enemy->lifeCallbackLife[3];
    case 10048:
        return enemy->manager->side->player->AngleToPlayer(&enemy->absPos);
    case 10069:
        return enemy->angle;
    case 10070:
        return enemy->angularVelocity;
    case 10071:
        return enemy->speed;
    case 10072:
        return enemy->acceleration;
    case 10073:
        return enemy->orbitRadius;
    case 10077:
        return enemy->orbitAngle;
    case 10078:
        return enemy->orbitAngularVelocity;
    case 10101:
        if (enemy->unknownType < 3)
        {
            return enemy->manager->otherSide->player->unk_a0;
        }
        return enemy->manager->otherSide->player->unk_a4;
    case 10084:
        return enemy->bossIndex;
    case 10083:
        return enemy->lastDamage;
    case 10050:
    {
        D3DXVECTOR3 diff = enemy->manager->side->player->pos - enemy->absPos;

        return D3DXVec3Length(&diff);
    }
    }
    return id;
}


#define INT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetIntOperand(enemy, instr->args[n].i) : instr->args[n].i)
#define FLOAT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetFloatOperand(enemy, instr->args[n].f) : instr->args[n].f)
#define INT_OP(v, bit) ((instr->paramMask & (bit)) ? GetIntOperand(enemy, v) : v)
#define FLOAT_OP(v, bit) ((instr->paramMask & (bit)) ? GetFloatOperand(enemy, v) : v)
#define INT_VAR(n) (*ResolveOperand(enemy, &instr->args[n].i, instr->paramMask, (n)))
#define FLOAT_VAR(n) (*ResolveFloatOperand(enemy, &instr->args[n].f, instr->paramMask, (n)))








static void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr)
{
    float angle;

    angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);


    enemy->moveDelta.x = cosf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.y = sinf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.z = 0.0f;
    enemy->moveStart = enemy->absPos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
    if (enemy->mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}




static void ScriptMoveAngleTimed(Enemy *enemy, EnemyScriptInstr *instr, float angle)
{

    enemy->moveDelta.x = cosf(angle) * FLOAT_ARG(2) * INT_ARG(0);
    enemy->moveDelta.y = sinf(angle) * FLOAT_ARG(2) * INT_ARG(0);
    enemy->moveDelta.z = 0.0f;
    enemy->moveStart = enemy->absPos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
}



static void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 target;

    target.x = FLOAT_ARG(2);
    target.y = FLOAT_ARG(3);
    target.z = 0.0f;
    enemy->moveDelta = target - enemy->absPos;
    enemy->moveStart = enemy->pos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (enemy->mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}

void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float end;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) = (end - start) * t + start;
}

void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float hStart;
    float weightIn;
    float end;
    float slopeIn;
    float wEnd;
    float tangentOut;
    float outWeight;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    slopeIn = GetFloatOperand(enemy, instr->args[6].f);
    tangentOut = GetFloatOperand(enemy, instr->args[7].f);
    hStart = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    wEnd = t * t * (3.0f - 2.0f * t);
    weightIn = (1.0f - t) * (1.0f - t) * t;
    outWeight = (t - 1.0f) * t * t;
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) =
        hStart * start + wEnd * end + weightIn * slopeIn + outWeight * tangentOut;
}




static void ScriptLerp(Enemy *enemy, EnemyScriptInstr *instr)
{
    float diff;

    diff = FLOAT_ARG(1) - FLOAT_ARG(2);
    *ResolveFloatOperand(enemy, &instr->args[0].f, instr->paramMask, 0) = diff * FLOAT_ARG(3) + FLOAT_ARG(2);
}




static void ScriptStartInterp(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;
    EnemyScriptLoop *loop;

    for (loop = enemy->currentCtx->loops, i = 0; i < 8; i++, loop++)
    {
        if (loop->func != NULL && loop->target != instr->args[0].f)
        {
            continue;
        }
        loop->timer = 0;
        loop->target = instr->args[0].f;
        loop->duration = INT_ARG(1);
        loop->type = INT_ARG(2);
        loop->mode = INT_ARG(3);
        loop->func = g_EnemyScriptInterps[loop->type];
        loop->args[0] = FLOAT_ARG(4);
        loop->args[1] = FLOAT_ARG(5);
        loop->args[2] = FLOAT_ARG(6);
        loop->args[3] = FLOAT_ARG(7);
        break;
    }
}





static EnemyScriptInstr *ScriptJumpIf(Enemy *enemy, EnemyScriptInstr *instr)
{
    switch (instr->opcode)
    {
    case 40:
        if (INT_ARG(0) == INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 41:
        if (FLOAT_ARG(0) == FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 42:
        if (INT_ARG(0) != INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 43:
        if (FLOAT_ARG(0) != FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 44:
        if (INT_ARG(0) < INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 45:
        if (FLOAT_ARG(0) < FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 46:
        if (INT_ARG(0) <= INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 47:
        if (FLOAT_ARG(0) <= FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 48:
        if (INT_ARG(0) > INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 49:

        if (FLOAT_ARG(0) > FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 50:
        if (INT_ARG(0) >= INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 51:
        if (FLOAT_ARG(0) >= FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    jump:
        enemy->currentCtx->timer.current = instr->args[2].i;
        return (EnemyScriptInstr *)((char *)instr + instr->args[3].i);
    }
    return NULL;
}




static void ScriptCallSub(Enemy *enemy, EnemyScriptInstr *instr, int sub)
{
    enemy->currentCtx->code = (char *)instr + instr->size;
    if (!enemy->noCallStack)
    {
        enemy->callStack[enemy->stackDepth] = *enemy->currentCtx;
    }

    enemy->currentCtx->ecl->StartSub(enemy->currentCtx, sub);
    enemy->currentCtx->args = enemy->currentCtx->ecl->callArgs;
    if (!enemy->noCallStack && enemy->stackDepth < 15)
    {
        enemy->stackDepth++;
    }
}




static void ScriptSetSubSprite(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (INT_ARG(0) >= 2)
    {


    }
    if (INT_ARG(1) >= 0)
    {
        if (enemy->useStageAnm)
        {
            enemy->manager->otherPlayerAnm->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1));
        }
        else
        {
            enemy->manager->anm->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1));
        }
    }
    else
    {
        enemy->sprites[INT_ARG(0)].scriptIndex = -1;
    }
}




static void ScriptMoveRandom(Enemy *enemy, EnemyScriptInstr *instr)
{
    float angle;

    if (enemy->manager->side->player->pos.x < enemy->pos.x)
    {
        angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
    }


    if (enemy->pos[0] < enemy->lowerBound[0] + 96.0f)
    {
        if (angle > 1.5707964f)
        {
            angle = 3.1415927f - angle;
        }
        else if (angle < -1.5707964f)
        {
            angle = -3.1415927f - angle;
        }
    }
    if (enemy->pos[0] > enemy->upperBound[0] - 96.0f)
    {
        if (angle < 1.5707964f && angle >= 0.0f)
        {

            angle = 3.1415927f - enemy->angle;
        }
        else if (angle > -1.5707964f && angle <= 0.0f)
        {
            angle = -3.1415927f - angle;
        }
    }
    if (enemy->pos[1] < enemy->lowerBound[1] + 48.0f && angle < 0.0f)
    {
        angle = -angle;
    }
    if (enemy->pos[1] > enemy->upperBound[1] - 48.0f && angle > 0.0f)
    {
        angle = -angle;
    }
    if (INT_ARG(0) <= 0)
    {
        enemy->angle = angle;
        enemy->speed = FLOAT_ARG(2);
        enemy->moveMode = 1;
        enemy->moveDuration = 0;
        enemy->moveTimer = 0;
    }
    else
    {
        ScriptMoveAngleTimed(enemy, instr, angle);
    }
}




static void ScriptMoveRandom2(Enemy *enemy, EnemyScriptInstr *instr)
{
    float bound;
    float angle;

    if (g_Rng.GetRandomU32InRange(4))
    {
        if (enemy->manager->side->player->pos.x < enemy->pos.x)
        {
            bound = enemy->manager->side->player->pos.x + 384.0f;
            if (enemy->pos.x - enemy->manager->side->player->pos.x < bound - enemy->pos.x)
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f, 0.0f);
            }
        }
        else
        {
            bound = enemy->manager->side->player->pos.x - 384.0f;
            if (enemy->manager->side->player->pos.x - enemy->pos.x < enemy->pos.x - bound)
            {
                angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            else
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
        }
    }
    else
    {
        angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    }
    if (enemy->pos[1] < enemy->lowerBound[1] + 48.0f && angle < 0.0f)
    {
        angle = -angle;
    }
    if (enemy->pos[1] > enemy->upperBound[1] - 48.0f && angle > 0.0f)
    {
        angle = -angle;
    }
    if (INT_ARG(0) <= 0)
    {
        enemy->angle = angle;
        enemy->speed = FLOAT_ARG(2);
        enemy->moveMode = 1;
        enemy->moveDuration = 0;
        enemy->moveTimer = 0;
    }
    else
    {
        ScriptMoveAngleTimed(enemy, instr, angle);
    }
}





static void ScriptFireBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    int shortArg;
    BulletShooter *bs;

    bs = &enemy->shooter;
    bs->pos = enemy->absPos + enemy->shootOffset;
    shortArg = *(short *)&instr->args[0];
    bs->sprite = INT_OP(shortArg, 1);
    bs->aimMode = instr->opcode - 96;
    bs->count1 = INT_OP(instr->args[1].i, 4);
    bs->count2 = INT_OP(instr->args[2].i, 8);
    bs->angle1 = FLOAT_OP(instr->args[5].f, 0x40);
    bs->speed1 = FLOAT_OP(instr->args[3].f, 0x10);
    bs->angle2 = FLOAT_OP(instr->args[6].f, 0x80);
    bs->speed2 = FLOAT_OP(instr->args[4].f, 0x20);
    bs->unk_1fa = 0;
    bs->flags = instr->args[7].i;
    shortArg = ((short *)instr->args)[1];
    bs->color = INT_OP(shortArg, 2);
    enemy->manager->side->bulletManager->FirePattern2(bs);
}




static void ScriptUpdateMovement(Enemy *enemy)
{



    switch (enemy->moveMode)
    {
    case 3:
    {
        D3DXVECTOR3 polar;

        enemy->orbitAngle = AddNormalizeAngle(enemy->orbitAngle, g_App.gameSpeed * enemy->orbitAngularVelocity);

        enemy->orbitRadius =
            g_GameManager.timelineSpeed * enemy->orbitRadiusVelocity * g_App.gameSpeed + enemy->orbitRadius;
        SetPolar((Vector2 *)&polar, enemy->orbitAngle, enemy->orbitRadius);
        enemy->velocity.x = polar.x + enemy->moveStart.x - enemy->pos.x;
        enemy->velocity.y = polar.y + enemy->moveStart.y - enemy->pos.y;
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveDuration > 0)
        {

            enemy->moveTimer -= g_GameManager.timelineSpeed;
            if (enemy->moveTimer <= 0)
            {
                enemy->moveMode = 0;
            }
        }
        break;
    }
    case 1:
        enemy->angle = AddNormalizeAngle(enemy->angle, g_App.gameSpeed * enemy->angularVelocity);
        enemy->speed = g_App.gameSpeed * enemy->acceleration + enemy->speed;
        SetPolar((Vector2 *)&enemy->velocity, enemy->angle, enemy->speed);


        enemy->velocity[2] = 0.0f;
        if (enemy->moveDuration > 0)
        {
            enemy->moveTimer--;
            if (enemy->moveTimer <= 0)
            {
                enemy->moveMode = 0;
            }
        }
        break;
    case 2:
    {
        float t;
        DWORD flags;

        enemy->moveTimer--;
        t = 1.0f - enemy->moveTimer.AsFloat() / enemy->moveDuration;
        if (t < 0.0f)
        {
            t = 0.0f;
        }


        flags = enemy->flags;
        switch ((flags >> 11) & 7)
        {
        case 1:
            t = t * t;
            break;
        case 2:
            t = t * t * t;
            break;
        case 3:
            t = t * t * t * t;
            break;
        case 4:
            t = (t = 1.0f - t) * t;
            t = 1.0f - t;
            break;
        case 5:
            t = (t = 1.0f - t) * t * t;
            t = 1.0f - t;
            break;
        case 6:
            t = (t = 1.0f - t) * t * t * t;
            t = 1.0f - t;
            break;
        }
        enemy->velocity = enemy->moveStart + enemy->moveDelta * t - enemy->pos;
        if (flags & 0x8000)
        {
            enemy->velocity.x = -enemy->velocity.x;
        }
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveTimer <= 0)
        {
            enemy->flags = flags & ~0x600;
            enemy->pos = enemy->moveStart + enemy->moveDelta;
            enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }

    if (enemy->unk_3380)
    {
        if (!enemy->useVelocity2 && enemy->manager->side->player->unk_364 != NULL &&
            enemy->manager->side->player->CallUnknown30404(&enemy->pos))
        {
            enemy->useVelocity2 = 1;
            enemy->manager->anm->StartScript(&enemy->primarySprite, enemy->unk_3380 + 0x16);
            enemy->turnLeft = -1;
            g_SoundPlayer.QueueSound(0x2f, enemy->manager->sideIdx != 0 ? 500 : -500);
        }
        if (enemy->useVelocity2)
        {
            enemy->manager->side->player->unk_30408(enemy->manager->side->player, enemy);
        }
    }
}




static void ScriptUpdateShooting(Enemy *enemy)
{
    if (enemy->life > 0)
    {
        if (enemy->shootInterval > 0)
        {
            enemy->shootTimer++;
            if (enemy->shootTimer >= enemy->shootInterval)
            {
                ScriptFireBullets(enemy, (EnemyScriptInstr *)enemy->shootInstr);
                enemy->shootTimer = 0;
            }
        }
        if (enemy->turnLeft >= 0)
        {
            int dir;
            float vx;



            dir = 0;
            vx = enemy->velocity.x;
            if (enemy->mirrored)
            {
                vx = -vx;
            }
            if (vx < -0.01f)
            {
                dir = 1;
            }
            else if (vx > 0.01f)
            {
                dir = 2;
            }
            if (enemy->spriteDirection != dir)
            {
                AnmLoaded *anm = enemy->useStageAnm ? enemy->manager->otherPlayerAnm : enemy->manager->anm;

                switch (dir)
                {
                case 0:
                    if (enemy->spriteDirection == 0xff)
                    {
                        anm->StartScript(&enemy->primarySprite, enemy->turnStill);
                    }
                    else if (enemy->spriteDirection == 1)
                    {
                        anm->StartScript(&enemy->primarySprite, enemy->turnStillFromLeft);
                    }
                    else
                    {
                        anm->StartScript(&enemy->primarySprite, enemy->turnStillFromRight);
                    }
                    break;
                case 1:
                    anm->StartScript(&enemy->primarySprite, enemy->turnLeft);
                    break;
                case 2:
                    anm->StartScript(&enemy->primarySprite, enemy->turnRight);
                    break;
                }
                enemy->spriteDirection = dir;
            }
        }
    }
}

int EnemyScripts::RunEnemy(Enemy *enemy)
{
#ifdef TH09_NATIVE_NETPLAY
    coop::NativeScope cooperativePattern;
    thnet::ActorScope networkPattern;
    thnet::PatternRandom random(enemy);
#endif
    int value;
    int ctxIndex;
    EnemyScriptInstr *instr;

    ctxIndex = -1;
    enemy->callStack = enemy->savedCtx;
    enemy->currentCtx = &enemy->ctx;
    enemy->stackDepth = enemy->savedStackDepth;
restart:
    instr = (EnemyScriptInstr *)enemy->currentCtx->code;
    if (enemy->pendingInterrupt >= 0)
    {
        goto interrupt;
    }
loop:
    enemy->absPos = enemy->pos + enemy->posOrigin;
    if (enemy->currentCtx->waitTimer.AsFrames() > 0)
    {
        enemy->currentCtx->waitTimer--;
        enemy->currentCtx->timer--;
        goto done;
    }
    if (enemy->currentCtx->timer == instr->time)
    {
        if ((instr->difficultyMask & (enemy->difficultyMaskExtra | g_GameManager.difficultyMask)) !=
            (enemy->difficultyMaskExtra | g_GameManager.difficultyMask))
        {
            goto next;
        }
        switch (instr->opcode)
        {











        case 2:




            enemy->currentCtx->waitTimer = INT_ARG(0);
            break;

        case 5:
            INT_VAR(2)--;
            if (INT_ARG(2) <= 0)
            {
                break;
            }


        case 4:
            enemy->currentCtx->timer = instr->args[0].i;
            instr = (EnemyScriptInstr *)((char *)instr + instr->args[1].i);
            goto loop;

        case 6:



            INT_VAR(0) = INT_ARG(1);
            break;

        case 7:
            FLOAT_VAR(0) = FLOAT_ARG(1);
            break;

        case 8:
            INT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1 : -1) * INT_ARG(1);
            break;

        case 9:
            FLOAT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1.0f : -1.0f) * FLOAT_ARG(1);
            break;

        case 10:
            INT_VAR(0) += INT_ARG(1);
            break;

        case 15:
            FLOAT_VAR(0) += FLOAT_ARG(1);
            break;

        case 11:
            INT_VAR(0) -= INT_ARG(1);
            break;

        case 16:
            FLOAT_VAR(0) -= FLOAT_ARG(1);
            break;

        case 12:
            INT_VAR(0) *= INT_ARG(1);
            break;

        case 17:
            FLOAT_VAR(0) *= FLOAT_ARG(1);
            break;

        case 13:
            INT_VAR(0) /= INT_ARG(1);
            break;

        case 18:
            FLOAT_VAR(0) /= FLOAT_ARG(1);
            break;

        case 14:
            INT_VAR(0) %= INT_ARG(1);
            break;

        case 19:

            FLOAT_VAR(0) = fmodf(FLOAT_ARG(0), FLOAT_ARG(1));
            break;

        case 20:
            INT_VAR(0) = INT_ARG(1) + INT_ARG(2);
            break;

        case 25:
            FLOAT_VAR(0) = FLOAT_ARG(1) + FLOAT_ARG(2);
            break;

        case 21:
            INT_VAR(0) = INT_ARG(1) - INT_ARG(2);
            break;

        case 26:
            FLOAT_VAR(0) = FLOAT_ARG(1) - FLOAT_ARG(2);
            break;

        case 22:
            INT_VAR(0) = INT_ARG(1) * INT_ARG(2);
            break;

        case 27:
            FLOAT_VAR(0) = FLOAT_ARG(1) * FLOAT_ARG(2);
            break;

        case 23:
            INT_VAR(0) = INT_ARG(1) / INT_ARG(2);
            break;

        case 28:
            FLOAT_VAR(0) = FLOAT_ARG(1) / FLOAT_ARG(2);
            break;

        case 24:
            INT_VAR(0) = INT_ARG(1) % INT_ARG(2);
            break;

        case 29:
            FLOAT_VAR(0) = fmodf(FLOAT_ARG(1), FLOAT_ARG(2));
            break;

        case 30:
            INT_VAR(0)++;
            break;

        case 31:
            INT_VAR(0)--;
            break;

        case 32:
            FLOAT_VAR(0) = sinf(FLOAT_ARG(1));
            break;

        case 33:
            FLOAT_VAR(0) = cosf(FLOAT_ARG(1));
            break;

        case 34:

            FLOAT_VAR(0) = atan2f(FLOAT_ARG(4) - FLOAT_ARG(2), FLOAT_ARG(3) - FLOAT_ARG(1));
            break;

        case 37:
            FLOAT_VAR(0) = AddNormalizeAngle(FLOAT_ARG(0), 0.0f);
            break;

        case 35:
            ScriptLerp(enemy, instr);
            break;

        case 36:
            ScriptStartInterp(enemy, instr);
            break;

        case 38:
        {
            float radius;
            float angle;

            angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
            radius = FLOAT_ARG(3);
            FLOAT_VAR(0) = cosf(angle) * radius;
            FLOAT_VAR(1) = sinf(angle) * radius;
            break;
        }

        case 39:
        {
            float y;
            float x;

            x = FLOAT_ARG(1) - FLOAT_ARG(3);
            y = FLOAT_ARG(2) - FLOAT_ARG(4);

            FLOAT_VAR(0) = sqrtf(x * x + y * y);
            break;
        }

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        {
            EnemyScriptInstr *target;

            target = ScriptJumpIf(enemy, instr);
            if (target != NULL)
            {

                instr = target;
                goto loop;
            }


            break;
        }

        case 53:


            if (!ScriptReturn(enemy, instr))
            {
                goto restart;
            }
            goto nextThread;

        case 54:
            enemy->manager->anm->StartScript(&enemy->primarySprite, INT_ARG(0));
            enemy->useStageAnm = 0;
            break;




        case 1:
            return -1;






        case 55:
            value = INT_ARG(0);
            ScriptSetTurnSprites(enemy, instr, value, value + 1, value + 2, value + 3, value + 4, value + 5);
            enemy->useStageAnm = 0;
            break;



        case 56:
            ScriptSetTurnSprites(enemy, instr, INT_ARG(0), INT_ARG(1), INT_ARG(2), INT_ARG(3), INT_ARG(4),
                                 INT_ARG(5));
            enemy->useStageAnm = 0;
            break;

        case 57:
            ScriptSetSubSprite(enemy, instr);
            enemy->useStageAnm = 0;
            break;

        case 58:
            enemy->manager->otherPlayerAnm->StartScript(&enemy->primarySprite, INT_ARG(0));
            enemy->useStageAnm = 1;
            break;

        case 59:
            value = INT_ARG(0);
            ScriptSetTurnSprites(enemy, instr, value, value + 1, value + 2, value + 3, value + 4, value + 5);
            enemy->useStageAnm = 1;
            break;

        case 60:
            ScriptSetTurnSprites(enemy, instr, INT_ARG(0), INT_ARG(1), INT_ARG(2), INT_ARG(3), INT_ARG(4),
                                 INT_ARG(5));
            enemy->useStageAnm = 1;
            break;

        case 61:
            enemy->useStageAnm = 1;
            ScriptSetSubSprite(enemy, instr);
            break;

        case 62:
            if (!enemy->useStageAnm)
            {
                enemy->manager->anm->StartScript(&enemy->primarySprite, enemy->turnExtra);
            }
            else
            {
                enemy->manager->otherPlayerAnm->StartScript(&enemy->primarySprite, enemy->turnExtra);
            }
            break;

        case 63:
            enemy->pos.x = FLOAT_ARG(0);
            enemy->pos.y = FLOAT_ARG(1);
            enemy->pos.z = 0.0f;
            enemy->ClampPos();
            break;

        case 64:
            ScriptMovePos(enemy, instr);
            break;


        case 65:
            enemy->angle = AddNormalizeAngle(FLOAT_ARG(0), 0.0f);
            enemy->speed = FLOAT_ARG(1);
            enemy->moveMode = 1;
            enemy->moveTimer = enemy->moveDuration = 0;
            break;


        case 66:
            if (INT_ARG(0) <= 0)
            {
                enemy->angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
                enemy->speed = FLOAT_ARG(3);
                enemy->moveMode = 1;
                enemy->moveTimer = enemy->moveDuration = 0;
            }
            else
            {
                ScriptMoveAngle(enemy, instr);
            }
            break;

        case 67:
            ScriptMoveRandom(enemy, instr);
            break;

        case 178:
            ScriptMoveRandom2(enemy, instr);
            break;

        case 68:
            enemy->angle = AddNormalizeAngle(FLOAT_ARG(0), enemy->manager->side->player->AngleToPlayer(&enemy->pos));
            enemy->speed = FLOAT_ARG(1);
            break;

        case 69:
            if (INT_ARG(0) <= 0)
            {
                enemy->angle =
                    AddNormalizeAngle(FLOAT_ARG(2), enemy->manager->side->player->AngleToPlayer(&enemy->pos));
                enemy->speed = FLOAT_ARG(3);
                enemy->moveMode = 1;
                enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            }
            else
            {
                ScriptMoveAngle(enemy, instr);
            }
            break;

        case 70:
            enemy->angularVelocity = FLOAT_ARG(0);
            enemy->moveMode = 1;
            break;

        case 71:
            enemy->acceleration = FLOAT_ARG(0);
            enemy->moveMode = 1;
            break;



        case 72:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->moveStart.x = FLOAT_ARG(1);
            enemy->moveStart.y = FLOAT_ARG(2);
            enemy->orbitAngle = FLOAT_ARG(3);
            enemy->orbitAngularVelocity = FLOAT_ARG(4);
            enemy->orbitRadius = FLOAT_ARG(5);
            enemy->orbitRadiusVelocity = FLOAT_ARG(6);
            enemy->moveMode = 3;
            break;

        case 73:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->moveStart = enemy->pos;
            enemy->orbitAngle = FLOAT_ARG(1);
            enemy->orbitAngularVelocity = FLOAT_ARG(2);
            enemy->orbitRadius = 0.0f;
            enemy->orbitRadiusVelocity = FLOAT_ARG(3);
            enemy->moveMode = 3;
            break;

        case 74:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->orbitAngularVelocity = FLOAT_ARG(1);
            enemy->orbitRadiusVelocity = FLOAT_ARG(2);
            enemy->moveMode = 3;
            break;

        case 75:
            enemy->lowerBound[0] = FLOAT_ARG(0);
            enemy->lowerBound[1] = FLOAT_ARG(1);
            enemy->upperBound[0] = FLOAT_ARG(2);
            enemy->upperBound[1] = FLOAT_ARG(3);
            enemy->clampPos = 1;
            break;

        case 76:
            enemy->clampPos = 0;
            break;

        case 77:
            enemy->hitboxSize.x = FLOAT_ARG(0);
            enemy->hitboxSize.y = FLOAT_ARG(1);
            break;

        case 78:
            enemy->extraHitboxSize.x = FLOAT_ARG(0);
            enemy->extraHitboxSize.y = FLOAT_ARG(1);
            break;

        case 79:
            value = INT_ARG(0);
            enemy->hitByShots = !(value & 1);
            enemy->shootable = (value & 2) ? 0 : 1;
            enemy->contactDamage = !(value & 4);
            enemy->hidden = (value & 8) != 0;
            enemy->staysOffscreen = (value & 0x10) != 0;
            enemy->undying = (value & 0x20) != 0;
            break;

        case 80:
            value = INT_ARG(0);
            if (value & 1)
            {
                enemy->hitByShots = 0;
            }
            if (value & 2)
            {
                enemy->shootable = 0;
            }
            if (value & 4)
            {
                enemy->contactDamage = 0;
            }
            if (value & 8)
            {
                enemy->hidden = 1;
            }
            if (value & 0x10)
            {
                enemy->staysOffscreen = 1;
            }
            if (value & 0x20)
            {
                enemy->undying = 1;
            }
            break;

        case 81:
            value = INT_ARG(0);
            if (value & 1)
            {
                enemy->hitByShots = 1;
            }
            if (value & 2)
            {
                enemy->shootable = 1;
            }
            if (value & 4)
            {
                enemy->contactDamage = 1;
            }
            if (value & 8)
            {
                enemy->hidden = 0;
            }
            if (value & 0x10)
            {
                enemy->staysOffscreen = 0;
            }
            if (value & 0x20)
            {
                enemy->undying = 0;
            }
            break;


        case 86:
            INT_VAR(0) = (instr->paramMask & 2) ? GetIntOperand(enemy->manager->bosses[INT_ARG(2)], instr->args[1].i)
                                                : instr->args[1].i;
            break;

        case 87:
            if (enemy->manager->bosses[INT_ARG(2)] != NULL)
            {
                FLOAT_VAR(0) = (instr->paramMask & 2)
                                   ? GetFloatOperand(enemy->manager->bosses[INT_ARG(2)], instr->args[1].f)
                                   : instr->args[1].f;
            }
            break;

        case 88:

            value = INT_ARG(0);
            ScriptCallSub(enemy->manager->bosses[value],
                          (EnemyScriptInstr *)enemy->manager->bosses[value]->currentCtx->code, instr->args[1].i);
            break;

        case 89:
            if (enemy->manager->bosses[INT_ARG(0)] != NULL)
            {
                enemy->manager->bosses[INT_ARG(0)]->pendingInterrupt = INT_ARG(1);
            }
            break;

        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
            if (enemy->life <= 0)
            {
                break;
            }


            if (enemy->holdFire == 1)
            {
                memcpy(enemy->shootInstr, instr, sizeof(enemy->shootInstr));
                break;
            }
            ScriptFireBullets(enemy, instr);
            break;








        case 111:
        {
            BulletCommand *cmd;

            cmd = &enemy->shooter.commands[INT_ARG(0)];
            cmd->flag = INT_ARG(1);
            cmd->concurrentFlag = INT_ARG(2);
            cmd->frames = INT_ARG(3);
            cmd->maxTurns = INT_ARG(4);
            cmd->arg0 = FLOAT_ARG(5);
            cmd->arg1 = FLOAT_ARG(6);
            break;
        }

        case 138:
            enemy->deathEffect = *(char *)instr->args;
            enemy->deathEffect2 = ((BYTE *)instr->args)[1];
            enemy->deathEffect3 = ((BYTE *)instr->args)[2];
            break;

        case 105:

            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootTimer = 0;
            }
            break;

        case 106:
            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootTimer = g_Rng.GetRandomU32InRange(enemy->shootInterval);
            }
            break;

        case 107:
            enemy->holdFire = 1;
            break;

        case 108:
            enemy->holdFire = 0;
            break;

        case 109:
            enemy->shooter.pos = enemy->pos + enemy->shootOffset;
            enemy->manager->side->bulletManager->FirePattern2(&enemy->shooter);
            break;

        case 110:
            enemy->shootOffset.x = FLOAT_ARG(0);
            enemy->shootOffset.y = FLOAT_ARG(1);
            enemy->shootOffset.z = 0.0f;
            break;

        case 114:
        case 115:
        {
            EnemyScriptArg *laserArgs;
            BulletShooter *shooter;

            laserArgs = instr->args;

            shooter = &enemy->laserShooter;
            shooter->pos = enemy->absPos + enemy->shootOffset;
            shooter->sprite = *(short *)&laserArgs[0];
            shooter->color = INT_OP(((short *)laserArgs)[1], 2);
            shooter->angle1 = FLOAT_OP(laserArgs[1].f, 4);
            shooter->speed1 = FLOAT_OP(laserArgs[2].f, 8);
            shooter->exFloats[0] = FLOAT_OP(laserArgs[3].f, 0x10);
            shooter->exFloats[1] = FLOAT_OP(laserArgs[4].f, 0x20);
            shooter->exFloats[2] = FLOAT_OP(laserArgs[5].f, 0x40);
            shooter->exFloats[3] = FLOAT_OP(laserArgs[6].f, 0x80);
            shooter->exInts[0] = INT_OP(laserArgs[7].i, 0x100);
            shooter->exInts[1] = INT_OP(laserArgs[8].i, 0x200);
            shooter->exInts[2] = INT_OP(laserArgs[9].i, 0x400);
            shooter->exInts[3] = laserArgs[10].i;
            shooter->laserEndHitFrames = laserArgs[11].i;
            shooter->flags = laserArgs[12].i;

            if (instr->opcode == 115)
            {
                shooter->aimMode = 0;
            }
            else
            {
                shooter->aimMode = 1;
            }
            enemy->lasers[enemy->laserIndex] = enemy->manager->side->bulletManager->SpawnLaser(shooter);
            break;
        }

        case 116:
            enemy->laserIndex = INT_ARG(0);
            break;

        case 117:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = AddNormalizeAngle(enemy->lasers[value]->angle, FLOAT_ARG(1));
            }
            break;

        case 167:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = FLOAT_ARG(1);
            }
            break;

        case 118:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle =
                    enemy->manager->side->player->AngleToPlayer(&enemy->lasers[value]->pos) + FLOAT_ARG(1);
            }
            break;

        case 119:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->pos.x = FLOAT_ARG(1) + enemy->absPos.x;
                enemy->lasers[value]->pos.y = FLOAT_ARG(2) + enemy->absPos.y;
                enemy->lasers[value]->pos.z = FLOAT_ARG(3) + enemy->absPos.z;
            }
            break;

        case 170:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->drawDelayFlag = INT_ARG(1);
            }
            break;

        case 120:

            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active)
            {
                enemy->currentCtx->intVars2[2] = 1;
            }
            else
            {
                enemy->currentCtx->intVars2[2] = 0;
            }
            break;

        case 121:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active && enemy->lasers[value]->state < 2)
            {
                enemy->lasers[value]->state = 2;
                enemy->lasers[value]->timer = 0;
                enemy->lasers[value]->width = enemy->lasers[value]->baseWidth;
            }
            break;

        case 154:
        {
            int i;


            for (i = 0; i < 0x20; i++)
            {
                enemy->lasers[i] = NULL;
            }
            break;
        }

        case 171:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->maxLength = FLOAT_ARG(1);
            }
            break;

        case 172:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->startOffset = FLOAT_ARG(1);
                enemy->lasers[value]->endOffset = FLOAT_ARG(2);
            }
            break;

        case 163:
            enemy->manager->scriptArg163 = INT_ARG(0);
            break;

        case 127:


            if (INT_ARG(0) >= 0)
            {
                enemy->manager->bosses[INT_ARG(0)] = enemy;
                enemy->isBoss = 1;
                enemy->bossIndex = INT_ARG(0);
                g_AsciiText.SetBossMarkerInterrupt(enemy->bossIndex, 1);
                enemy->minShootDistSq = 0.0f;
            }
            else
            {
                enemy->manager->bosses[enemy->bossIndex] = NULL;
                enemy->isBoss = 0;
                g_AsciiText.SetBossMarkerInterrupt(enemy->bossIndex, 2);
                enemy->ReleaseEffects();
                g_AsciiText.SetBossMarkerPos(enemy->bossIndex, &D3DXVECTOR3(-999.0f, -999.0f, 0.0f));
            }
            break;

        case 128:

            enemy->effectRadius = instr->args[4].f;
            enemy->effectCount++;
            break;

        case 159:
            enemy->drawListIndex = INT_ARG(0);
            break;

        case 124:
            g_SoundPlayer.QueueSoundPanned(INT_ARG(0), enemy->pos.x);
            break;

        case 129:

            enemy->deathMode = *(BYTE *)instr->args;
            break;

        case 130:

            enemy->deathSub = *(short *)instr->args;
            break;

        case 126:
            enemy->interrupts[INT_ARG(1)] = INT_ARG(0);
            break;

        case 131:

            enemy->phaseStartLife = enemy->life = enemy->maxLife =
#ifdef TH09_NATIVE_NETPLAY
                coop::BossHealth(enemy,INT_ARG(0));
#else
                INT_ARG(0);
#endif
            break;

        case 132:
            enemy->callbackTimer = INT_ARG(0);
            break;

        case 133:
            enemy->lifeCallbackLife[INT_ARG(0)] =
#ifdef TH09_NATIVE_NETPLAY
                coop::BossHealth(enemy,INT_ARG(1));
#else
                INT_ARG(1);
#endif
            enemy->lifeCallbackSub[INT_ARG(0)] = INT_ARG(2);
            break;

        case 134:


            enemy->timerCallbackTime = INT_ARG(0);
            enemy->timerCallbackSub = INT_ARG(1);
            enemy->callbackTimer = 0;
            break;

        case 135:
            value = INT_ARG(0);
            if (enemy->threads[value] != NULL)
            {
                g_MemoryTracker.Free(enemy->threads[value]);
            }
            enemy->threads[value] = NULL;
            if (INT_ARG(1) >= 0)
            {
                enemy->threads[value] =
                    (EnemyScriptThread *)g_MemoryTracker.Alloc(sizeof(EnemyScriptThread), "ECLInt");
                if (enemy->threads[value] != NULL)
                {
                    memset(enemy->threads[value], 0, sizeof(EnemyScriptThread));
                    enemy->threads[value]->sub = INT_ARG(1);
                    enemy->manager->ecl.StartSub(&enemy->threads[value]->ctx, enemy->threads[value]->sub);
                    enemy->threads[value]->ctx.vars = enemy->currentCtx->vars;
                }
            }
            break;

        case 139:
            enemy->manager->side->effectManager->SpawnEffect(INT_ARG(0), &enemy->pos, INT_ARG(1), INT_VAR(2));
            break;




















        case 140:
        {
            D3DXVECTOR3 velocity;

            velocity.x = FLOAT_ARG(3);
            velocity.y = FLOAT_ARG(4);
            velocity.z = FLOAT_ARG(5);
            enemy->manager->side->effectManager->SpawnEffectWithVelocity(INT_ARG(0), &enemy->pos, &velocity,
                                                                            INT_ARG(1), INT_VAR(2));
            break;
        }

        case 143:
            enemy->item = INT_ARG(0);
            break;

        case 144:
            enemy->pointItems = INT_ARG(0);
            enemy->powerItems = INT_ARG(1);
            break;

        case 145:
            enemy->rotateSprite = *(BYTE *)instr->args;
            break;

        case 136:
            g_EnemyScriptFuncs[INT_ARG(0)](enemy, instr);
            break;

        case 137:
            if (INT_ARG(0) >= 0)
            {
                enemy->currentCtx->func = g_EnemyScriptFuncs[INT_ARG(0)];
                enemy->currentCtx->funcInstr = instr;
            }
            else
            {
                enemy->currentCtx->func = NULL;
            }
            break;

        case 146:
            enemy->currentCtx->timer += INT_ARG(0);
            break;

        case 147:
            enemy->manager->side->stage->waitLabel = INT_ARG(0);
            break;

        case 148:


            g_GameManager.unk_380 += 1800;
            break;

        case 93:
        {
            Enemy *child;

            if (enemy->life > 0)
            {
                EnemySpawnArgs spawn;
                D3DXVECTOR3 pos;

                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                child = enemy->manager->SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                           &enemy->currentCtx->vars, 1);
            }


            break;
        }

        case 94:
        {
            Enemy *child;

            if (enemy->life > 0)
            {
                EnemySpawnArgs spawn;
                D3DXVECTOR3 pos;

                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                pos += enemy->pos;
                child = enemy->manager->SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                           &enemy->currentCtx->vars, 1);
            }


            break;
        }

        case 95:
            enemy->manager->ClearEnemiesForBonus(8000, 0);
            break;

        case 149:
            enemy->primarySprite.pendingInterrupt = INT_ARG(0);
            break;

        case 150:
            enemy->sprites[instr->args[0].i].pendingInterrupt = instr->args[1].i;
            break;

        case 112:
            enemy->manager->side->bulletManager->ClearAllToItems();
            break;

        case 113:
            if (INT_ARG(0) >= 0)
            {
                enemy->shooter.sound = INT_ARG(0);
                enemy->shooter.flags |= 0x200;
            }
            else
            {
                enemy->shooter.flags &= ~0x200;
            }
            enemy->shooter.bulletSound = INT_ARG(1);
            break;

        case 151:
            enemy->noCallStack = *(BYTE *)instr->args;
            break;

        case 152:
            enemy->rankSpeedLow = FLOAT_ARG(0);
            enemy->rankSpeedHigh = FLOAT_ARG(1);
            enemy->rankCount1Low = INT_ARG(2);
            enemy->rankCount1High = INT_ARG(3);
            enemy->rankCount2Low = INT_ARG(4);
            enemy->rankCount2High = INT_ARG(5);
            break;

        case 153:
            enemy->timerCallbackSub = enemy->deathSub;
            enemy->callbackTimer = 0;
            break;

        case 155:

            enemy->keepSpellCard = *(BYTE *)instr->args;
            break;

        case 156:
            enemy->grazeable = *(BYTE *)instr->args;
            enemy->drawListIndex = 2;
            break;

        case 157:
            enemy->trailFlags = instr->args[0].i;
            enemy->trailSegmentCount = INT_ARG(1);
            enemy->trailContactSegments = INT_ARG(2);
            enemy->trailSegmentStep = INT_ARG(3);

            if (enemy->trailFlags & 8)
            {
                g_SpriteEngine->FillSpriteStripVertices(&enemy->primarySprite, (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices,
                                                        enemy->trailSegmentCount / enemy->trailSegmentStep * 2);
            }
            break;

        case 160:

            enemy->invulnTimer = INT_ARG(0);
            break;

        case 161:

            enemy->manager->side->bulletManager->ConvertBulletsToItems(&enemy->absPos, FLOAT_ARG(0));
            break;

        case 162:

            enemy->manager->side->bulletManager->ClearAll(4);
            break;

        case 165:
            enemy->primarySprite.rotation.z = FLOAT_ARG(0);
            break;

        case 166:
            *ResolveFloatOperand(enemy, &instr->args[1].f, instr->paramMask, 1) = sinf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            FLOAT_VAR(0) = cosf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            break;

        case 169:
            if ((enemy->manager->side->player->pos.x < enemy->pos.x && enemy->pos.x > 96.0f) || enemy->pos.x > 288.0f)
            {
                FLOAT_VAR(0) = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            break;

        case 173:
            enemy->haltedByBomb = INT_ARG(0);
            break;

        case 183:
            enemy->intangibleDuringBomb = INT_ARG(0);
            break;

        case 82:
            enemy->minShootDistSq = FLOAT_ARG(0);
            enemy->minShootDistSq *= enemy->minShootDistSq;
            break;

        case 175:
            enemy->manager->holdTimelineSpawns = INT_ARG(0);
            break;

        case 177:
            enemy->phaseStartLife =
#ifdef TH09_NATIVE_NETPLAY
                coop::BossHealth(enemy,INT_ARG(0));
#else
                INT_ARG(0);
#endif
            break;

        case 182:
            enemy->useMainSpriteOffset = INT_ARG(0);
            break;

        case 185:
            enemy->unknownType = INT_ARG(0);
            break;

        case 186:

            if (enemy->unknownType < 3)
            {
                if (enemy->manager->side->spellCard->unk_a4 != 0)
                {
                    enemy->manager->side->spellCard->unk_a4 = 2;
                }
            }
            else if (enemy->unknownType == 3)
            {
                if (enemy->manager->side->spellCard->unk_a8 != 0)
                {
                    int i;

                    enemy->manager->side->spellCard->unk_a8 = 2;
                    enemy->manager->side->stage->StartSpellCard();
                    for (i = 0; i < enemy->manager->side->stage->dialogueSpriteCount; i++)
                    {
                        enemy->manager->otherSide->stage->playerAnm->StartScript(
                            &enemy->manager->side->stage->dialogueSprites[i],
                            enemy->manager->otherSide->player->unk_303f4 +
                                enemy->manager->otherSide->stage->dialogueScriptOffset + i);
                    }
                }
            }
            break;

        case 187:

            enemy->unknownBit14 = INT_ARG(0);
            break;




        case 52:
            ScriptCallSub(enemy, instr, instr->args[0].i);
            goto restart;

        case 125:
            enemy->pendingInterrupt = INT_ARG(0);


        interrupt:
            enemy->currentCtx->code = (char *)instr + instr->size;
            if (!enemy->noCallStack)
            {
                enemy->callStack[enemy->stackDepth] = enemy->ctx;
            }
            enemy->manager->ecl.StartSub(&enemy->ctx, enemy->interrupts[enemy->pendingInterrupt]);
            if (enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            enemy->pendingInterrupt = -1;
            goto restart;

        }
    next:
        instr = (EnemyScriptInstr *)((char *)instr + instr->size);
        goto loop;
    }
done:
    if (enemy->life > 0)
    {
        int i;
        BOOL moved;
        float time;
        EnemyScriptLoop *l;

        moved = FALSE;
        l = enemy->currentCtx->loops;
        D3DXVECTOR3 savedPos = enemy->pos;
        if (enemy->currentCtx->func != NULL)
        {
            enemy->currentCtx->func(enemy, enemy->currentCtx->funcInstr);
        }
        for (i = 0; i < 8; i++, l++)
        {
            if (l->func != NULL)
            {
                l->timer += g_GameManager.timelineSpeed;
                if (l->timer >= l->duration)
                {
                    l->timer = l->duration;
                }
                time = l->timer.AsFloat() / l->duration;
                switch (l->mode)
                {
                case 1:
                    time = time * time;
                    break;
                case 2:
                    time = time * time * time;
                    break;
                case 3:
                    time = time * time * time * time;
                    break;
                case 4:
                    time = (time = 1.0f - time) * time;
                    time = 1.0f - time;
                    break;
                case 5:
                    time = (time = 1.0f - time) * time * time;
                    time = 1.0f - time;
                    break;
                case 6:
                    time = (time = 1.0f - time) * time * time * time;
                    time = 1.0f - time;
                    break;
                }
                l->func(enemy, (EnemyScriptInstr *)l, time);
                if (l->timer >= l->duration)
                {
                    l->func = NULL;
                }


                if (*(int *)&l->target == 0x461ce800 || *(int *)&l->target == 0x461cec00 ||
                    *(int *)&l->target == 0x461cf000)
                {
                    moved = TRUE;
                }
            }
        }
        if (moved)
        {
            enemy->velocity.x = enemy->pos.x - savedPos.x;
            enemy->velocity.y = enemy->pos.y - savedPos.y;
            enemy->angle = Atan2(enemy->velocity.y, enemy->velocity.x);
            enemy->pos = savedPos;
        }
    }
    if (ctxIndex == -1)
    {
        enemy->savedStackDepth = enemy->stackDepth;
    }
    else
    {
        enemy->threads[ctxIndex]->stackDepth = enemy->stackDepth;
    }
    enemy->currentCtx->code = instr;
    enemy->currentCtx->timer += g_GameManager.timelineSpeed;

    {
        int i;
        EnemyScriptThread *buf;

    nextThread:
        for (i = ctxIndex + 1; i < 4; i++)
        {
            if (enemy->threads[i] != NULL)
            {
                buf = enemy->threads[i];
                enemy->callStack = buf->callStack;
                enemy->currentCtx = &buf->ctx;
                instr = (EnemyScriptInstr *)enemy->currentCtx->code;
                enemy->currentCtx->timedCallActive = i + 1;
                enemy->stackDepth = buf->stackDepth;
                ctxIndex = i;
                goto loop;
            }
        }
    }
    enemy->callStack = enemy->savedCtx;
    enemy->currentCtx = &enemy->ctx;
    ScriptUpdateMovement(enemy);
    ScriptUpdateShooting(enemy);
    return 0;
}

void SendExAttack1(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(1, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SetSidesFlag0(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->manager->side->flag0 = instr->args[1].i;
    enemy->manager->otherSide->flag0 = instr->args[1].i;
    if (instr->args[1].i != 0)
    {
        g_SoundPlayer.QueueSound(0x34, 0);
    }
}

void SendExAttack7(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(7, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack8(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(8, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack9(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(9, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack23(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(23, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack24(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.exAttackManager->AddAttack(24, &enemy->pos, 1 - enemy->manager->sideIdx, NULL);
}







void ScatterBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet;
    int i;

    bullet = enemy->manager->side->bulletManager->bullets;
    for (i = 0; i < 0x218; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 6)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteHeight <= 16.0f)
        {
            bullet->mgr->etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.unk_d44 + 0xf);
        }
        bullet->speed = 0.0f;
        bullet->accelSpeed = 0.0f;
        bullet->turnSpeed = 0.0f;
        bullet->angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
        bullet->angularVelocity = 0.0f;
        bullet->turnAngle = 0.0f;
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->velocity.z = 0.0f;
        bullet->ClearCommands();
        bullet->SetWaitCommand(0, 0, 0x3c);
        bullet->SetAccelCommand(1, 0, 0x3c, enemy->currentCtx->floatVars[0], -999.0f);
    }
    bullet = enemy->manager->otherSide->bulletManager->bullets;
    for (i = 0; i < 0x218; i++, bullet++)
    {
        if (bullet->state == 0 || bullet->state == 6)
        {
            continue;
        }
        if (bullet->sprites.sprite0.spriteHeight <= 16.0f && bullet->sprites.unk_d44 < 0x157)
        {
            bullet->mgr->etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.unk_d44 + 0xf);
        }
        bullet->speed = 0.0f;
        bullet->accelSpeed = 0.0f;
        bullet->turnSpeed = 0.0f;
        bullet->angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
        bullet->angularVelocity = 0.0f;
        bullet->turnAngle = 0.0f;
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->velocity.z = 0.0f;
        bullet->ClearCommands();
        if (!(i & 1) && *(DWORD *)&bullet->sprites.sprite0.spriteHeight == 0x41800000)
        {
            bullet->sprite = 0;
            bullet->SetSpriteCommand(0, 0, 0, 0xf);
            bullet->SetWaitCommand(1, 0, 0x3c);
            bullet->SetAccelCommand(2, 0, 0x3c, enemy->currentCtx->floatVars[0], -999.0f);
        }
        else
        {
            bullet->SetWaitCommand(0, 0, 0x3c);
            bullet->SetAccelCommand(1, 0, 0x3c, enemy->currentCtx->floatVars[0], -999.0f);
        }
    }
    g_SoundPlayer.QueueSound(0x34, 0);
}

void SendExAttack10(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = 0.0f;
    g_GameManager.exAttackManager->AddAttack(10, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack11(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(11, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack12(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 pos;

    pos.x = enemy->currentCtx->floatVars2[0];
    pos.y = enemy->currentCtx->floatVars2[1];
    pos.z = enemy->currentCtx->floatVars[0];
    g_GameManager.exAttackManager->AddAttack(12, &pos, 1 - enemy->manager->sideIdx, NULL);
}

void SendExAttack22(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 target;

    SetPolar((Vector2 *)&target, 1.5707964f, 64.0f);
    target += enemy->pos;
    target.z = 1.5707964f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
    SetPolar((Vector2 *)&target, 1.2566371f, 64.0f);
    target += enemy->pos;
    target.z = 1.2566371f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
    SetPolar((Vector2 *)&target, 1.8849556f, 64.0f);
    target += enemy->pos;
    target.z = 1.8849556f;
    g_GameManager.exAttackManager->AddAttack(22, &enemy->pos, 1 - enemy->manager->sideIdx, &target);
}
