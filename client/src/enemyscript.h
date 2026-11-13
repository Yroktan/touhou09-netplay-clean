



#pragma once


struct EnemyScriptHeader
{
    int version;
    short subCount;
    short timelineCount;
    void *timelines[1];
};


struct EnemyScriptCallArgs
{
    int ints[4];
    float floats[4];
};




struct EnemyScriptsStar
{
    unsigned char pad_0[0x100];
    D3DXVECTOR3 ends[8];
};



struct EnemyScripts
{

    int Load(char *path);


    int RunEnemy(Enemy *enemy);

    void Unload();


    int StartSub(EnemyScriptState *ctx, short sub);



    int GetTimelineCount()
    {
        return this->file->timelineCount;
    }


    void *GetTimeline(int i)
    {
        return this->file->timelines[i];
    }

    EnemyScriptHeader *file;

    void **subs;
    EnemyScriptsStar star;

    EnemyScriptCallArgs callArgs;
};
ASSERT_SIZE(EnemyScripts, 0x188);




struct EnemyScriptLoop
{
    void (*func)(Enemy *enemy, EnemyScriptInstr *instr, float t);
    Timer timer;
    int duration;
    int type;
    int mode;
    float args[4];
    float target;
};
ASSERT_SIZE(EnemyScriptLoop, 0x30);


union EnemyScriptArg
{
    int i;
    float f;
};



struct EnemyScriptInstr
{

    int time;
    short opcode;

    short size;
    BYTE pad_8;
    BYTE difficultyMask;

    WORD paramMask;
    EnemyScriptArg args[9];
};



int GetIntOperand(Enemy *enemy, int id);


int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit);


float GetFloatOperand(Enemy *enemy, float id);


float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit);


void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);


void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);


struct EnemySpawnArgs
{
    int sub;
    D3DXVECTOR3 pos;
    int life;
    int item;
    int score;
};
ASSERT_SIZE(EnemySpawnArgs, 0x1c);


typedef void (*EnemyScriptFunc)(Enemy *enemy, EnemyScriptInstr *instr);

extern void (*g_EnemyScriptInterps[8])(Enemy *enemy, EnemyScriptInstr *instr, float t);

extern EnemyScriptFunc g_EnemyScriptFuncs[12];



struct EnemyScriptVars
{
    int intVars[8];
    float floatVars[8];
    int intVars2[4];
    float floatVars2[2];
    EnemyScriptCallArgs args;
};
ASSERT_SIZE(EnemyScriptVars, 0x78);




struct EnemyScriptState
{

    EnemyScripts *ecl;
    void *code;

    Timer timer;

    void (*func)(Enemy *enemy, EnemyScriptInstr *instr);
    EnemyScriptInstr *funcInstr;
    union
    {
        EnemyScriptVars vars;
        struct
        {

            int intVars[8];
            float floatVars[8];
            int intVars2[4];
            float floatVars2[2];
            EnemyScriptCallArgs args;
        };
    };
    Timer waitTimer;
    EnemyScriptLoop loops[8];
    int compareResult;

    int timedCallActive;

    short sub;
    unsigned char pad_22a[0x22c - 0x22a];
};
ASSERT_SIZE(EnemyScriptState, 0x22c);



struct EnemyScriptThread
{
    int sub;
    short pad_4;

    short stackDepth;
    EnemyScriptState ctx;

    EnemyScriptState callStack[16];
};
ASSERT_SIZE(EnemyScriptThread, 0x24f4);
