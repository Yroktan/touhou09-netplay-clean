

#include "../ascii.h"
#include "../archive.h"
#include "../bullet.h"
#include "../effect.h"
#include "../ending.h"
#include "../enemy.h"
#include "../exattack.h"
#include "../gamemanager.h"
#include "../gdisurface.h"
#include "../gui.h"
#include "../lzss.h"
#include "../mainmenu.h"
#include "../midi.h"
#include "../network.h"
#include "../player.h"
#include "../scoredat.h"
#include "../screeneffect.h"
#include "../spellcard.h"
#include "../sprite.h"
#include "../stage.h"
#include "../unsorted.h"
#include "../sound.h"

int InitSpellCardType0(SpellCard *card);
int IsSpellCardDurationOver(SpellCard *card);
int ReturnZero(ExAttack *attack);
int FreePrimarySpellCardData(SpellCard *card);
int InitSpellCardType1(SpellCard *card);
int InitSpellCardType2(SpellCard *card);
int IsSpellCardEnemyDefeated(SpellCard *card);
int FreeSecondarySpellCardData(SpellCard *card);
int InitSpellCardTypes3And6(SpellCard *card);
int InitSpellCardTypes4And7(SpellCard *card);
int InitSpellCardTypes5And8(SpellCard *card);
void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t);
void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t);
void SendExAttack1(Enemy *enemy, EnemyScriptInstr *instr);
void SetSidesFlag0(Enemy *enemy, EnemyScriptInstr *instr);
void ScatterBullets(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack7(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack9(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack8(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack10(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack11(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack12(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack22(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack23(Enemy *enemy, EnemyScriptInstr *instr);
void SendExAttack24(Enemy *enemy, EnemyScriptInstr *instr);
int InitExAttackType0(ExAttack *attack);
int UpdateExAttackTypes0And1(ExAttack *attack);
int InitExAttackType1(ExAttack *attack);
int InitExAttackType2(ExAttack *attack);
int UpdateExAttackType2(ExAttack *attack);
int InitExAttackType3(ExAttack *attack);
int UpdateExAttackType3(ExAttack *attack);
int InitExAttackType4(ExAttack *attack);
int UpdateExAttackType4(ExAttack *attack);
int FreeExAttackType4(ExAttack *attack);
int InitExAttackType5(ExAttack *attack);
int UpdateExAttackTypes5And10(ExAttack *attack);
int InitExAttackType6(ExAttack *attack);
int UpdateExAttackType6(ExAttack *attack);
int InitExAttackType7(ExAttack *attack);
int UpdateExAttackType7(ExAttack *attack);
int InitExAttackType8(ExAttack *attack);
int UpdateExAttackTypes8And9(ExAttack *attack);
int InitExAttackType9(ExAttack *attack);
int InitExAttackType10(ExAttack *attack);
int InitExAttackTypes11And12(ExAttack *attack);
int UpdateExAttackType11(ExAttack *attack);
int UpdateExAttackType12(ExAttack *attack);
int InitExAttackType13(ExAttack *attack);
int UpdateExAttackType13(ExAttack *attack);
int InitExAttackType14(ExAttack *attack);
int UpdateExAttackTypes14And22(ExAttack *attack);
int DrawExAttackTypes14And22(ExAttack *attack);
int InitExAttackType15(ExAttack *attack);
int UpdateExAttackType15(ExAttack *attack);
int InitExAttackType16(ExAttack *attack);
int UpdateExAttackTypes16And23(ExAttack *attack);
int InitExAttackType17(ExAttack *attack);
int UpdateExAttackType17(ExAttack *attack);
int InitExAttackType18(ExAttack *attack);
int UpdateExAttackTypes18And24(ExAttack *attack);
int InitExAttackType19(ExAttack *attack);
int UpdateExAttackType19(ExAttack *attack);
int DrawExAttackTypes19And21(ExAttack *attack);
int InitExAttackType20(ExAttack *attack);
int UpdateExAttackType20(ExAttack *attack);
int InitExAttackType21(ExAttack *attack);
int UpdateExAttackType21(ExAttack *attack);
int InitExAttackType22(ExAttack *attack);
int InitExAttackType23(ExAttack *attack);
int InitExAttackType24(ExAttack *attack);
int InitExAttackType25(ExAttack *attack);
int UpdateExAttackType25(ExAttack *attack);
int InitExAttackType26(ExAttack *attack);
int UpdateExAttackType26(ExAttack *attack);
int SendComboAttacksCharacter0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter1(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter2(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter3(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter4(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter5(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter6(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter7(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter8(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter9(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter10(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter11(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter12(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter13(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter14(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int SendComboAttacksCharacter15(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
int IsWithinCircularFocusArea(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter1(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacters2And10(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter4(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter6(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter7(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter8(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter9(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter11(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter14(Player *player, D3DXVECTOR3 *pos);
int CheckFocusAreaCharacter15(Player *player, D3DXVECTOR3 *pos);
int UpdateFocusedEnemyDrift(Player *player, Enemy *enemy);
int UpdateFocusedEnemyCharacter2(Player *player, Enemy *enemy);
int UpdateFocusedEnemyCharacter4(Player *player, Enemy *enemy);
int FirePlayerShotType1(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType2(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType3(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType4(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType5(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType6(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int FirePlayerShotType7(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
int UpdatePlayerShotType1(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType2(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType3(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType4(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType5(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType6(Player *player, PlayerBullet *bullet);
int UpdatePlayerShotType7(Player *player, PlayerBullet *bullet);
int DrawPlayerShotType1(Player *player, PlayerBullet *bullet);
int DrawPlayerShotType2(Player *player, PlayerBullet *bullet);
int HitPlayerShotType1(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);
int HitPlayerShotType2(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);
int InitEffectTypes1And2(Effect *effect);
int UpdateEffectTypes1And2(Effect *effect);
int InitEffectTypes3And4(Effect *effect);
int UpdateEffectTypes3And4(Effect *effect);
int EffectInitRing(Effect *effect);
int UpdateEffectFromSprite(Effect *effect);
int EffectDrawRing(Effect *effect);
int EffectUpdateFollowPlayer(Effect *effect);
int InitEffectTypes10And18And24And25And26And28And33(Effect *effect);
int UpdateEffectType10(Effect *effect);
int DrawEffectType10(Effect *effect);
int InitOrientedEffect(Effect *effect);
int InitEffectType16(Effect *effect);
int UpdateEffectType16(Effect *effect);
int DrawEffectType16(Effect *effect);
int InitEffectType17(Effect *effect);
int UpdateEffectType17(Effect *effect);
int DrawEffectType17(Effect *effect);
int UpdateEffectType18(Effect *effect);
int DrawEffectType18(Effect *effect);
int UpdateEffectType24(Effect *effect);
int DrawEffectType24(Effect *effect);
int UpdateEffectType25(Effect *effect);
int DrawEffectType25(Effect *effect);
int UpdateEffectType26(Effect *effect);
int DrawEffectType26(Effect *effect);
int InitEffectTypes27And36(Effect *effect);
int UpdateEffectType27(Effect *effect);
int DrawEffectType27(Effect *effect);
int UpdateEffectType28(Effect *effect);
int DrawEffectType28(Effect *effect);
int InitEffectType29(Effect *effect);
int UpdateEffectType29(Effect *effect);
int DrawEffectType29(Effect *effect);
int InitEffectType30(Effect *effect);
int UpdateEffectType30(Effect *effect);
int DrawEffectType30(Effect *effect);
int InitEffectType31(Effect *effect);
int UpdateEffectType31(Effect *effect);
int DrawEffectType31(Effect *effect);
int InitEffectType32(Effect *effect);
int UpdateEffectType32(Effect *effect);
int DrawEffectType32(Effect *effect);
int UpdateEffectType33(Effect *effect);
int DrawEffectType33(Effect *effect);
int InitEffectType34(Effect *effect);
int UpdateEffectType34(Effect *effect);
int DrawEffectType34(Effect *effect);
int InitEffectType35(Effect *effect);
int UpdateEffectType35(Effect *effect);
int UpdateEffectType36(Effect *effect);
int DrawEffectType36(Effect *effect);
int InitEffectType37(Effect *effect);
int UpdateEffectType37(Effect *effect);
int DrawEffectType37(Effect *effect);

static GameManagerUnknownF0 g_StoryRounds4a2300[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 4, 5, 50, 51, 52, 7, 96 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 32 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 3, 4, 40, 41, 42, 6, 32 },
    { 4, 3, 4, 40, 41, 42, 6, 128 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 0, 1, 10, 11, 12, 3, 128 },
    { 5, 0, 1, 13, 14, 15, 3, 128 },
    { 5, 1, 2, 23, 24, 25, 4, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2450[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 4, 5, 50, 51, 52, 7, 96 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 32 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 2, 3, 4, 40, 41, 42, 6, 32 },
    { 2, 2, 3, 30, 31, 32, 5, 32 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 3, 4, 40, 41, 42, 6, 32 },
    { 3, 2, 3, 30, 31, 32, 5, 32 },
    { 4, 3, 4, 40, 41, 42, 6, 128 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 128 },
    { 5, 1, 2, 23, 24, 25, 4, 128 },
    { 5, 0, 0, 3, 4, 5, 2, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a25d0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 4, 5, 50, 51, 52, 7, 96 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 32 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 3, 4, 40, 41, 42, 6, 32 },
    { 2, 2, 3, 30, 31, 32, 5, 128 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 3, 4, 40, 41, 42, 6, 128 },
    { 3, 2, 3, 30, 31, 32, 5, 128 },
    { 3, 0, 0, 0, 1, 2, 2, 128 },
    { 3, 0, 1, 10, 11, 12, 3, 128 },
    { 4, 3, 4, 40, 41, 42, 6, 128 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 128 },
    { 4, 0, 1, 10, 11, 12, 3, 128 },
    { 5, 7, 8, 80, 81, 82, 10, 256 },
    { 6, 10, 11, 110, 111, 112, 13, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2740[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 4, 5, 50, 51, 52, 7, 96 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 32 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 6, 7, 70, 71, 72, 9, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 2, 0, 0, 0, 1, 2, 2, 48 },
    { 2, 0, 1, 10, 11, 12, 3, 48 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 3, 4, 40, 41, 42, 6, 96 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 0, 0, 0, 1, 2, 2, 48 },
    { 3, 0, 1, 10, 11, 12, 3, 48 },
    { 4, 3, 4, 40, 41, 42, 6, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 24 },
    { 4, 0, 1, 10, 11, 12, 3, 24 },
    { 5, 1, 2, 20, 21, 22, 4, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a28c0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 4, 5, 50, 51, 52, 7, 96 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 32 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 2, 3, 30, 31, 32, 5, 128 },
    { 3, 0, 0, 0, 1, 2, 2, 128 },
    { 3, 0, 1, 10, 11, 12, 3, 128 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 128 },
    { 4, 0, 1, 10, 11, 12, 3, 128 },
    { 5, 1, 2, 20, 21, 22, 4, 128 },
    { 6, 10, 11, 110, 111, 112, 13, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2a10[] = {
    { 0, 6, 7, 70, 71, 72, 9, 96 },
    { 1, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 1, 2, 20, 21, 22, 4, 64 },
    { 2, 0, 1, 10, 11, 12, 3, 64 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 2, 1, 2, 20, 21, 22, 4, 64 },
    { 2, 0, 1, 10, 11, 12, 3, 64 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 4, 3, 30, 31, 32, 5, 64 },
    { 3, 1, 2, 20, 21, 22, 4, 64 },
    { 3, 0, 1, 10, 11, 12, 3, 64 },
    { 3, 0, 0, 0, 1, 2, 2, 64 },
    { 4, 7, 8, 80, 81, 82, 10, 96 },
    { 4, 4, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 64 },
    { 4, 0, 1, 10, 11, 12, 3, 64 },
    { 4, 0, 0, 0, 1, 2, 2, 64 },
    { 5, 4, 4, 40, 41, 42, 6, 256 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 8, 9, 90, 91, 92, 11, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2ba0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 6, 7, 70, 71, 72, 9, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 96 },
    { 2, 5, 4, 40, 41, 42, 6, 128 },
    { 3, 5, 4, 40, 41, 42, 6, 128 },
    { 3, 7, 8, 80, 81, 82, 10, 96 },
    { 3, 1, 2, 20, 21, 22, 4, 128 },
    { 3, 0, 1, 10, 11, 12, 3, 128 },
    { 3, 0, 0, 0, 1, 2, 2, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 0, 1, 10, 11, 12, 3, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 128 },
    { 5, 5, 3, 30, 31, 32, 5, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 8, 9, 90, 91, 92, 11, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2ce0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 64 },
    { 1, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 5, 6, 60, 61, 62, 8, 64 },
    { 2, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 6, 4, 40, 41, 42, 6, 64 },
    { 2, 6, 3, 30, 31, 32, 5, 64 },
    { 3, 6, 3, 30, 31, 32, 5, 64 },
    { 3, 6, 4, 40, 41, 42, 6, 64 },
    { 3, 5, 6, 60, 61, 62, 8, 64 },
    { 3, 7, 8, 80, 81, 82, 10, 64 },
    { 3, 6, 3, 30, 31, 32, 5, 64 },
    { 3, 6, 4, 40, 41, 42, 6, 64 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 0, 1, 10, 11, 12, 3, 128 },
    { 5, 0, 0, 3, 4, 5, 2, 128 },
    { 6, 10, 11, 110, 111, 112, 13, 256 },
    { 7, 8, 9, 90, 91, 92, 11, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2e30[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 1, 2, 20, 21, 22, 4, 48 },
    { 3, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 5, 6, 60, 61, 62, 8, 128 },
    { 3, 0, 1, 10, 11, 12, 3, 48 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 64 },
    { 4, 0, 1, 10, 11, 12, 3, 64 },
    { 4, 0, 0, 0, 1, 2, 2, 64 },
    { 5, 7, 4, 40, 41, 42, 6, 128 },
    { 6, 10, 11, 110, 111, 112, 13, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a2f60[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 1, 2, 20, 21, 22, 4, 48 },
    { 2, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 0, 1, 10, 11, 12, 3, 48 },
    { 3, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 1, 2, 20, 21, 22, 4, 64 },
    { 3, 0, 1, 10, 11, 12, 3, 48 },
    { 3, 1, 4, 40, 41, 42, 6, 64 },
    { 3, 7, 8, 80, 81, 82, 10, 64 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 64 },
    { 4, 0, 1, 10, 11, 12, 3, 64 },
    { 4, 1, 4, 40, 41, 42, 6, 64 },
    { 5, 0, 0, 0, 1, 2, 2, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a30e0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 1, 2, 20, 21, 22, 4, 96 },
    { 2, 0, 0, 0, 1, 2, 2, 64 },
    { 2, 7, 8, 80, 81, 82, 10, 64 },
    { 3, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 1, 2, 20, 21, 22, 4, 64 },
    { 3, 0, 0, 0, 1, 2, 2, 64 },
    { 3, 3, 4, 40, 41, 42, 6, 64 },
    { 3, 7, 8, 80, 81, 82, 10, 64 },
    { 4, 2, 3, 30, 31, 32, 5, 128 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 0, 0, 0, 1, 2, 2, 128 },
    { 4, 3, 4, 40, 41, 42, 6, 128 },
    { 5, 0, 1, 10, 11, 12, 3, 128 },
    { 6, 10, 11, 110, 111, 112, 13, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a3260[] = {
    { 0, 14, 7, 70, 71, 72, 2, 96 },
    { 0, 14, 6, 60, 61, 62, 2, 64 },
    { 1, 14, 7, 70, 71, 72, 2, 128 },
    { 1, 14, 6, 60, 61, 62, 2, 64 },
    { 2, 14, 7, 70, 71, 72, 2, 128 },
    { 2, 14, 6, 60, 61, 62, 2, 128 },
    { 2, 14, 8, 80, 81, 82, 2, 128 },
    { 3, 14, 4, 40, 41, 42, 2, 128 },
    { 3, 14, 0, 0, 1, 2, 2, 128 },
    { 3, 14, 1, 10, 11, 12, 2, 128 },
    { 3, 14, 8, 80, 81, 82, 2, 96 },
    { 4, 14, 4, 40, 41, 42, 2, 128 },
    { 4, 14, 0, 0, 1, 2, 2, 128 },
    { 4, 14, 1, 10, 11, 12, 2, 128 },
    { 5, 14, 2, 20, 21, 22, 2, 128 },
    { 6, 8, 9, 90, 91, 92, 2, 256 },
    { 7, 11, 12, 120, 121, 122, 2, 256 },
    { 8, 12, 13, 130, 131, 132, 2, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a33a0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 3, 4, 40, 41, 42, 6, 64 },
    { 1, 7, 8, 80, 81, 82, 10, 64 },
    { 1, 3, 4, 40, 41, 42, 6, 64 },
    { 1, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 1, 2, 20, 21, 22, 4, 96 },
    { 3, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 1, 2, 20, 21, 22, 4, 64 },
    { 3, 3, 4, 40, 41, 42, 6, 64 },
    { 3, 7, 8, 80, 81, 82, 10, 64 },
    { 4, 2, 3, 30, 31, 32, 5, 96 },
    { 4, 1, 2, 20, 21, 22, 4, 64 },
    { 4, 3, 4, 40, 41, 42, 6, 64 },
    { 4, 7, 8, 80, 81, 82, 10, 64 },
    { 5, 0, 1, 10, 11, 12, 3, 128 },
    { 6, 8, 9, 90, 91, 92, 11, 256 },
    { 7, 11, 0, 0, 1, 2, 2, 256 },
    { 8, 12, 13, 130, 131, 132, 15, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

static GameManagerUnknownF0 g_StoryRounds4a34e0[] = {
    { 0, 4, 5, 50, 51, 52, 7, 128 },
    { 0, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 4, 5, 50, 51, 52, 7, 128 },
    { 1, 6, 7, 70, 71, 72, 9, 128 },
    { 1, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 5, 6, 60, 61, 62, 8, 128 },
    { 2, 7, 8, 80, 81, 82, 10, 64 },
    { 2, 3, 4, 40, 41, 42, 6, 64 },
    { 2, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 3, 4, 40, 41, 42, 6, 64 },
    { 3, 2, 3, 30, 31, 32, 5, 96 },
    { 3, 7, 8, 80, 81, 82, 10, 64 },
    { 4, 1, 2, 20, 21, 22, 4, 128 },
    { 4, 3, 4, 40, 41, 42, 6, 64 },
    { 4, 2, 3, 30, 31, 32, 5, 96 },
    { 5, 0, 1, 10, 11, 12, 3, 128 },
    { 6, 9, 10, 100, 101, 102, 12, 256 },
    { 7, 11, 12, 120, 121, 122, 14, 256 },
    { 8, 12, 0, 0, 1, 2, 2, 256 },
    { 9, 0, -1, 200, 0, 0, 2, 256 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

const int g_PlayerCpuDirButtons[9] = {
    0, 16, 32, 64, 128, 80, 144, 96, 160,
};

const int g_PlayerCpuAltDirs[2][9] = {
    0, 3, 4, 2, 2, 6, 5, 8, 7,
    0, 4, 3, 1, 1, 7, 8, 5, 6,
};

const int g_BulletTypeScripts[23][5] = {
    0, 18, 19, 20, 15,
    1, 21, 22, 23, 16,
    2, 21, 22, 23, 16,
    3, 21, 22, 23, 16,
    4, 21, 22, 23, 16,
    5, 21, 22, 23, 16,
    6, 21, 22, 23, 16,
    7, 24, 24, 24, 17,
    8, 24, 24, 24, 17,
    9, 24, 24, 24, 17,
    25, 27, 27, 27, 26,
    96, 21, 22, 23, 16,
    97, 21, 22, 23, 16,
    98, 21, 22, 23, 16,
    99, 24, 24, 24, 17,
    100, 24, 24, 24, 17,
    101, 21, 22, 23, 16,
    102, 21, 22, 23, 16,
    103, 24, 24, 24, 17,
    104, 24, 24, 24, 17,
    105, 24, 24, 24, 17,
    106, 21, 22, 23, 16,
    107, 24, 24, 24, 17,
};

int g_PlayerCpuDirOrder[12][10] = {
    0, 0, 4, 6, 8, 1, 5, 3, 7, 2,
    0, 0, 1, 6, 5, 4, 3, 8, 7, 2,
    0, 0, 1, 5, 6, 3, 4, 7, 8, 2,
    0, 0, 3, 5, 7, 1, 6, 4, 8, 2,
    0, 0, 4, 8, 6, 2, 7, 3, 5, 1,
    0, 0, 4, 3, 2, 8, 7, 6, 5, 1,
    0, 0, 3, 4, 2, 7, 8, 5, 6, 1,
    0, 0, 3, 7, 5, 2, 8, 4, 6, 1,
    0, 0, 4, 8, 8, 2, 7, 3, 7, 2,
    0, 0, 4, 3, 2, 8, 7, 8, 7, 2,
    0, 0, 3, 4, 2, 7, 8, 7, 8, 2,
    0, 0, 3, 7, 7, 2, 8, 4, 8, 2,
};

int g_BulletColorOffsets16[16] = {
    0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 0,
};

int g_BulletColorOffsets32[8] = {
    0, 1, 1, 2, 2, 3, 4, 0,
};

int g_Unknown4a1128[21] = {
    9, 8, 11, 20, 15, 25, 9, 9, 10, 10, 20, 10, 9, 10, 10, 10, 20, 20, 20, 20, 20,
};



int g_Unknown4a117c[15] = {
    1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

int g_Unknown4a1500[5][9][5] = {
    {
    5, 4, 3, 2, 1,
    5, 4, 3, 2, 1,
    5, 4, 3, 2, 1,
    8, 8, 4, 2, 1,
    10, 10, 5, 3, 1,
    22, 12, 6, 3, 1,
    26, 14, 7, 4, 1,
    40, 16, 8, 4, 1,
    55, 30, 9, 5, 1,
},
    {
    5, 4, 3, 2, 1,
    5, 4, 3, 2, 1,
    5, 4, 3, 2, 1,
    10, 10, 12, 9, 7,
    12, 18, 15, 11, 9,
    38, 20, 18, 15, 10,
    45, 32, 25, 20, 15,
    50, 32, 25, 20, 15,
    60, 59, 45, 40, 30,
},
    {
    8, 5, 3, 2, 1,
    8, 5, 3, 2, 1,
    8, 5, 3, 2, 1,
    10, 10, 12, 9, 7,
    12, 18, 15, 11, 9,
    45, 20, 18, 15, 10,
    50, 32, 25, 20, 15,
    50, 32, 25, 20, 15,
    60, 59, 45, 40, 30,
},
    {
    12, 8, 3, 2, 1,
    12, 8, 3, 2, 1,
    12, 8, 3, 2, 1,
    15, 10, 12, 9, 7,
    18, 18, 15, 11, 9,
    45, 20, 18, 15, 10,
    50, 32, 25, 20, 15,
    50, 32, 25, 20, 15,
    60, 59, 45, 40, 30,
},
    {
    61, 61, 61, 61, 61,
    62, 62, 61, 61, 61,
    63, 63, 61, 61, 61,
    64, 64, 61, 61, 61,
    65, 65, 61, 61, 61,
    66, 65, 61, 61, 61,
    67, 65, 64, 61, 61,
    68, 66, 64, 61, 61,
    70, 69, 68, 67, 65,
},
};

int g_Unknown4a1884[5] = {
    10, 30, 50, 60, 0,
};

int g_CharacterValuesA10[16] = {
    10, 16, 17, 18, 26, 24, 27, 25, 28, 29, 30, 31, 32, 33, 36, 37,
};

int g_CharacterValuesAd0[16] = {
    23, 23, 32, 23, 23, 23, 22, 25, 28, 28, 26, 25, 25, 27, 22, 22,
};

int g_CharacterValuesB10[16] = {
    42, 42, 45, 45, 38, 41, 41, 43, 40, 42, 40, 40, 41, 44, 34, 41,
};

int g_CharacterValuesB50[16] = {
    15, 15, 24, 15, 15, 15, 14, 17, 20, 20, 18, 17, 17, 19, 14, 14,
};

int g_CharacterValuesB90[16] = {
    29, 29, 38, 31, 29, 29, 29, 10, 13, 13, 11, 10, 10, 12, 29, 29,
};

int g_CharacterValuesBd0[16] = {
    10, 10, 19, 10, 10, 10, 9, 12, 15, 15, 13, 12, 12, 14, 9, 9,
};

const DWORD g_SeekOrigins[3] = {
    0, 1, 2,
};

const float g_Half = 0.5f;

const D3DFORMAT g_TextureFormats[6] = {
    (D3DFORMAT)0,
    (D3DFORMAT)21,
    (D3DFORMAT)25,
    (D3DFORMAT)23,
    (D3DFORMAT)20,
    (D3DFORMAT)26,
};

const int g_TextureFormatSizes[6] = {
    4, 4, 2, 2, 3, 2,
};

SpellCardType g_SpellCardTypes[9] = {
    { (SpellCardCallback)InitSpellCardType0, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardType1, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardType2, (SpellCardCallback)IsSpellCardEnemyDefeated, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreeSecondarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes3And6, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes4And7, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes5And8, (SpellCardCallback)IsSpellCardEnemyDefeated, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreeSecondarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes3And6, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes4And7, (SpellCardCallback)IsSpellCardDurationOver, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreePrimarySpellCardData },
    { (SpellCardCallback)InitSpellCardTypes5And8, (SpellCardCallback)IsSpellCardEnemyDefeated, (SpellCardCallback)ReturnZero, (SpellCardCallback)FreeSecondarySpellCardData },
};

void (*g_EnemyScriptInterps[8])(Enemy *enemy, EnemyScriptInstr *instr, float t) = {
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveLinear,
    (void (*)(Enemy *, EnemyScriptInstr *, float))ScriptMoveHermite,
};

EnemyScriptFunc g_EnemyScriptFuncs[12] = {
    (EnemyScriptFunc)SendExAttack1,
    (EnemyScriptFunc)SetSidesFlag0,
    (EnemyScriptFunc)ScatterBullets,
    (EnemyScriptFunc)SendExAttack7,
    (EnemyScriptFunc)SendExAttack9,
    (EnemyScriptFunc)SendExAttack8,
    (EnemyScriptFunc)SendExAttack10,
    (EnemyScriptFunc)SendExAttack11,
    (EnemyScriptFunc)SendExAttack12,
    (EnemyScriptFunc)SendExAttack22,
    (EnemyScriptFunc)SendExAttack23,
    (EnemyScriptFunc)SendExAttack24,
};

ExAttackType g_ExAttackTypes[27] = {
    { (ExAttackCallback)InitExAttackType0, (ExAttackCallback)UpdateExAttackTypes0And1, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType1, (ExAttackCallback)UpdateExAttackTypes0And1, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType2, (ExAttackCallback)UpdateExAttackType2, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType3, (ExAttackCallback)UpdateExAttackType3, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType4, (ExAttackCallback)UpdateExAttackType4, (ExAttackCallback)ReturnZero, (ExAttackCallback)FreeExAttackType4 },
    { (ExAttackCallback)InitExAttackType5, (ExAttackCallback)UpdateExAttackTypes5And10, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType6, (ExAttackCallback)UpdateExAttackType6, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType7, (ExAttackCallback)UpdateExAttackType7, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType8, (ExAttackCallback)UpdateExAttackTypes8And9, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType9, (ExAttackCallback)UpdateExAttackTypes8And9, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType10, (ExAttackCallback)UpdateExAttackTypes5And10, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackTypes11And12, (ExAttackCallback)UpdateExAttackType11, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackTypes11And12, (ExAttackCallback)UpdateExAttackType12, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType13, (ExAttackCallback)UpdateExAttackType13, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType14, (ExAttackCallback)UpdateExAttackTypes14And22, (ExAttackCallback)DrawExAttackTypes14And22, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType15, (ExAttackCallback)UpdateExAttackType15, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType16, (ExAttackCallback)UpdateExAttackTypes16And23, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType17, (ExAttackCallback)UpdateExAttackType17, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType18, (ExAttackCallback)UpdateExAttackTypes18And24, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType19, (ExAttackCallback)UpdateExAttackType19, (ExAttackCallback)DrawExAttackTypes19And21, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType20, (ExAttackCallback)UpdateExAttackType20, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType21, (ExAttackCallback)UpdateExAttackType21, (ExAttackCallback)DrawExAttackTypes19And21, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType22, (ExAttackCallback)UpdateExAttackTypes14And22, (ExAttackCallback)DrawExAttackTypes14And22, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType23, (ExAttackCallback)UpdateExAttackTypes16And23, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType24, (ExAttackCallback)UpdateExAttackTypes18And24, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType25, (ExAttackCallback)UpdateExAttackType25, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
    { (ExAttackCallback)InitExAttackType26, (ExAttackCallback)UpdateExAttackType26, (ExAttackCallback)ReturnZero, (ExAttackCallback)ReturnZero },
};

PlayerComboFunc g_CharacterFuncs9d0[16] = {
    (PlayerComboFunc)SendComboAttacksCharacter0,
    (PlayerComboFunc)SendComboAttacksCharacter1,
    (PlayerComboFunc)SendComboAttacksCharacter2,
    (PlayerComboFunc)SendComboAttacksCharacter3,
    (PlayerComboFunc)SendComboAttacksCharacter4,
    (PlayerComboFunc)SendComboAttacksCharacter5,
    (PlayerComboFunc)SendComboAttacksCharacter6,
    (PlayerComboFunc)SendComboAttacksCharacter7,
    (PlayerComboFunc)SendComboAttacksCharacter8,
    (PlayerComboFunc)SendComboAttacksCharacter9,
    (PlayerComboFunc)SendComboAttacksCharacter10,
    (PlayerComboFunc)SendComboAttacksCharacter11,
    (PlayerComboFunc)SendComboAttacksCharacter12,
    (PlayerComboFunc)SendComboAttacksCharacter13,
    (PlayerComboFunc)SendComboAttacksCharacter14,
    (PlayerComboFunc)SendComboAttacksCharacter15,
};

PlayerPosFunc g_CharacterFuncsA50[16] = {
    (PlayerPosFunc)IsWithinCircularFocusArea,
    (PlayerPosFunc)CheckFocusAreaCharacter1,
    (PlayerPosFunc)CheckFocusAreaCharacters2And10,
    (PlayerPosFunc)IsWithinCircularFocusArea,
    (PlayerPosFunc)CheckFocusAreaCharacter4,
    (PlayerPosFunc)IsWithinCircularFocusArea,
    (PlayerPosFunc)CheckFocusAreaCharacter6,
    (PlayerPosFunc)CheckFocusAreaCharacter7,
    (PlayerPosFunc)CheckFocusAreaCharacter8,
    (PlayerPosFunc)CheckFocusAreaCharacter9,
    (PlayerPosFunc)CheckFocusAreaCharacters2And10,
    (PlayerPosFunc)CheckFocusAreaCharacter11,
    (PlayerPosFunc)IsWithinCircularFocusArea,
    (PlayerPosFunc)IsWithinCircularFocusArea,
    (PlayerPosFunc)CheckFocusAreaCharacter14,
    (PlayerPosFunc)CheckFocusAreaCharacter15,
};

PlayerEnemyFunc g_CharacterFuncsA90[16] = {
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyCharacter2,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyCharacter4,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
    (PlayerEnemyFunc)UpdateFocusedEnemyDrift,
};

ShotFireFunc g_ShotFireFuncs[9] = {
    NULL,
    (ShotFireFunc)FirePlayerShotType1,
    (ShotFireFunc)FirePlayerShotType2,
    (ShotFireFunc)FirePlayerShotType3,
    (ShotFireFunc)FirePlayerShotType4,
    (ShotFireFunc)FirePlayerShotType5,
    (ShotFireFunc)FirePlayerShotType6,
    (ShotFireFunc)FirePlayerShotType7,
    NULL,
};

ShotUpdateFunc g_ShotUpdateFuncs[8] = {
    NULL,
    (ShotUpdateFunc)UpdatePlayerShotType1,
    (ShotUpdateFunc)UpdatePlayerShotType2,
    (ShotUpdateFunc)UpdatePlayerShotType3,
    (ShotUpdateFunc)UpdatePlayerShotType4,
    (ShotUpdateFunc)UpdatePlayerShotType5,
    (ShotUpdateFunc)UpdatePlayerShotType6,
    (ShotUpdateFunc)UpdatePlayerShotType7,
};

ShotUpdateFunc g_ShotDrawFuncs[4] = {
    NULL,
    (ShotUpdateFunc)DrawPlayerShotType1,
    (ShotUpdateFunc)DrawPlayerShotType2,
    NULL,
};

ShotHitFunc g_ShotHitFuncs[4] = {
    NULL,
    (ShotHitFunc)HitPlayerShotType1,
    (ShotHitFunc)HitPlayerShotType2,
    NULL,
};

EffectInfo g_EffectInfos[38] = {
    { 28, 0, 1, NULL, NULL, NULL },
    { 33, 0, 1, (int (*)(Effect *))InitEffectTypes1And2, (int (*)(Effect *))UpdateEffectTypes1And2, NULL },
    { 34, 0, 1, (int (*)(Effect *))InitEffectTypes1And2, (int (*)(Effect *))UpdateEffectTypes1And2, NULL },
    { 35, 0, 1, (int (*)(Effect *))InitEffectTypes3And4, (int (*)(Effect *))UpdateEffectTypes3And4, NULL },
    { 36, 0, 1, (int (*)(Effect *))InitEffectTypes3And4, (int (*)(Effect *))UpdateEffectTypes3And4, NULL },
    { 32, 0, 1, NULL, NULL, NULL },
    { 86, 0, 1, (int (*)(Effect *))EffectInitRing, (int (*)(Effect *))UpdateEffectFromSprite, (int (*)(Effect *))EffectDrawRing },
    { 63, 0, 1, NULL, (int (*)(Effect *))EffectUpdateFollowPlayer, NULL },
    { 87, 0, 1, (int (*)(Effect *))EffectInitRing, (int (*)(Effect *))UpdateEffectFromSprite, (int (*)(Effect *))EffectDrawRing },
    { 85, 0, 1, NULL, NULL, NULL },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType10, (int (*)(Effect *))DrawEffectType10 },
    { 89, 0, 1, (int (*)(Effect *))InitOrientedEffect, NULL, NULL },
    { 90, 0, 1, (int (*)(Effect *))InitOrientedEffect, NULL, NULL },
    { 91, 0, 1, (int (*)(Effect *))EffectInitRing, (int (*)(Effect *))UpdateEffectFromSprite, (int (*)(Effect *))EffectDrawRing },
    { 92, 0, 1, (int (*)(Effect *))EffectInitRing, (int (*)(Effect *))UpdateEffectFromSprite, (int (*)(Effect *))EffectDrawRing },
    { 93, 0, 1, (int (*)(Effect *))EffectInitRing, (int (*)(Effect *))UpdateEffectFromSprite, (int (*)(Effect *))EffectDrawRing },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType16, (int (*)(Effect *))UpdateEffectType16, (int (*)(Effect *))DrawEffectType16 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType17, (int (*)(Effect *))UpdateEffectType17, (int (*)(Effect *))DrawEffectType17 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType18, (int (*)(Effect *))DrawEffectType18 },
    { 53, 0, 1, NULL, NULL, NULL },
    { 28, 0, 1, NULL, NULL, NULL },
    { 29, 0, 1, NULL, NULL, NULL },
    { 30, 0, 1, NULL, NULL, NULL },
    { 31, 0, 1, NULL, NULL, NULL },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType24, (int (*)(Effect *))DrawEffectType24 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType25, (int (*)(Effect *))DrawEffectType25 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType26, (int (*)(Effect *))DrawEffectType26 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes27And36, (int (*)(Effect *))UpdateEffectType27, (int (*)(Effect *))DrawEffectType27 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType28, (int (*)(Effect *))DrawEffectType28 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType29, (int (*)(Effect *))UpdateEffectType29, (int (*)(Effect *))DrawEffectType29 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType30, (int (*)(Effect *))UpdateEffectType30, (int (*)(Effect *))DrawEffectType30 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType31, (int (*)(Effect *))UpdateEffectType31, (int (*)(Effect *))DrawEffectType31 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType32, (int (*)(Effect *))UpdateEffectType32, (int (*)(Effect *))DrawEffectType32 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes10And18And24And25And26And28And33, (int (*)(Effect *))UpdateEffectType33, (int (*)(Effect *))DrawEffectType33 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType34, (int (*)(Effect *))UpdateEffectType34, (int (*)(Effect *))DrawEffectType34 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType35, (int (*)(Effect *))UpdateEffectType35, NULL },
    { 88, 0, 1, (int (*)(Effect *))InitEffectTypes27And36, (int (*)(Effect *))UpdateEffectType36, (int (*)(Effect *))DrawEffectType36 },
    { 88, 0, 1, (int (*)(Effect *))InitEffectType37, (int (*)(Effect *))UpdateEffectType37, (int (*)(Effect *))DrawEffectType37 },
};

char *g_EndingFiles[14] = {
    "end00.end",
    "end01.end",
    "end02.end",
    "end03.end",
    "end04.end",
    "end05.end",
    "end06.end",
    "end07.end",
    "end08.end",
    "end09.end",
    "end10.end",
    "end11.end",
    "end12.end",
    "end13.end",
};

char *g_StageFiles[16][2] = {
    { "world01.anm", "world01.std" },
    { "world00.anm", "world00.std" },
    { "world03.anm", "world03.std" },
    { "world04.anm", "world04.std" },
    { "world05.anm", "world05.std" },
    { "world06.anm", "world06.std" },
    { "world07.anm", "world07.std" },
    { "world01.anm", "world01.std" },
    { "world09.anm", "world09.std" },
    { "world10.anm", "world10.std" },
    { "world11.anm", "world11.std" },
    { "world12.anm", "world12.std" },
    { "world13.anm", "world13.std" },
    { "world09.anm", "world09m.std" },
    { "world11.anm", "world11m.std" },
    { "world13.anm", "world13m.std" },
};

char *g_CharacterFiles[16][6] = {
    { "pl00.sht", "pl00.anm", "pl00.ecl", "pl00.msg", "pl00_match.msg", "pl00b.anm" },
    { "pl01.sht", "pl01.anm", "pl01.ecl", "pl01.msg", "pl01_match.msg", "pl01b.anm" },
    { "pl02.sht", "pl02.anm", "pl02.ecl", "pl02.msg", "pl02_match.msg", "pl02b.anm" },
    { "pl03.sht", "pl03.anm", "pl03.ecl", "pl03.msg", "pl03_match.msg", "pl03b.anm" },
    { "pl04.sht", "pl04.anm", "pl04.ecl", "pl04.msg", "pl04_match.msg", "pl04b.anm" },
    { "pl05.sht", "pl05.anm", "pl05.ecl", "pl05.msg", "pl05_match.msg", "pl05b.anm" },
    { "pl06.sht", "pl06.anm", "pl06.ecl", "pl06.msg", "pl06_match.msg", "pl06b.anm" },
    { "pl07.sht", "pl07.anm", "pl07.ecl", "pl07.msg", "pl07_match.msg", "pl07b.anm" },
    { "pl08.sht", "pl08.anm", "pl08.ecl", "pl08.msg", "pl08_match.msg", "pl08b.anm" },
    { "pl09.sht", "pl09.anm", "pl09.ecl", "pl09.msg", "pl09_match.msg", "pl09b.anm" },
    { "pl10.sht", "pl10.anm", "pl10.ecl", "pl10.msg", "pl10_match.msg", "pl10b.anm" },
    { "pl11.sht", "pl11.anm", "pl11.ecl", "pl11.msg", "pl11_match.msg", "pl11b.anm" },
    { "pl12.sht", "pl12.anm", "pl12.ecl", "pl12.msg", "pl12_match.msg", "pl12b.anm" },
    { "pl13.sht", "pl13.anm", "pl13.ecl", "pl13.msg", "pl13_match.msg", "pl13b.anm" },
    { "pl14.sht", "pl14.anm", "pl14.ecl", "pl14.msg", "pl14_match.msg", "pl14b.anm" },
    { "pl15.sht", "pl15.anm", "pl15.ecl", "pl15.msg", "pl15_match.msg", "pl15b.anm" },
};

char *g_Pl06FcAnmFiles[2] = {
    "pl06_fc_s.anm",
    "pl06b_fc_s.anm",
};

char *g_ReplayCharacterNames[16] = {
    "\227\354\226\262",
    "\226\202\227\235\215\271",
    "\215\347\226\351",
    "\227d\226\262",
    "\227\351\220\345",
    "\203`\203\213\203m",
    "\203\212\203\212\203J",
    "\203~\203X\203e\203B\203A",
    "\202\304\202\356",
    "\227H\215\201",
    "\225\266",
    "\203\201\203f\203B\203X\203\223",
    "\217\254\222\254",
    "\216l\213G\211f\225P",
    "\203\201\203\213\203\211\203\223",
    "\203\213\203i\203T",
};

char *g_ReplayModeNames[6] = {
    "Story",
    "Extra",
    "Match \220l vs \220l",
    "Match \220l vs \216\256",
    "Match \216\256 vs \220l",
    "Match \216\256 vs \216\256",
};

char *g_ReplayDifficultyNames[5] = {
    "Easy",
    "Normal",
    "Hard",
    "Lunatic",
    "Extra",
};

char *g_DemoReplayPaths[3] = {
    "demo/demorpy0.rpy",
    "demo/demorpy1.rpy",
    "demo/demorpy2.rpy",
};

char *g_ReplayMenuCharacterNames[17] = {
    "Reimu   ",
    "Marisa  ",
    "Sakuya  ",
    "Youmu   ",
    "Reisen  ",
    "Cirno   ",
    "Lyrica",
    "Mystia  ",
    "Tewi    ",
    "Yuka    ",
    "Aya     ",
    "Medicine",
    "Komachi ",
    "Sikieiki",
    "Marlin  ",
    "Lunasa  ",
    "     ",
};

char *g_ReplayMenuDifficultyNames[5] = {
    "Easy   ",
    "Normal ",
    "Hard   ",
    "Lunatic",
    "Extra  ",
};

char *g_ReplayMenuModeNames[2] = {
    "Story Mode",
    "Extra Mode",
};

char *g_ReplayMenuMatchNames[4] = {
    "Human vs Human",
    "Human vs Com",
    "Com vs Human",
    "Com vs Com",
};

char *g_ScoreRankNames[5] = {
    "1st",
    "2nd",
    "3rd",
    "4th",
    "5th",
};

char *g_MusicNotHeardComment[8] = {
    " ***** \222\215\210\323 ***** ",
    "  \202\261\202\314\213\310\202\315\202\334\202\276\203Q\201[\203\200\222\206\202\305\215\304\220\266\202\263\202\352\202\275\202\261\202\306\202\252\202\240\202\350\202\334\202\271\202\361\201B",
    "  \210\352\211\236\222\256\202\255\202\261\202\306\202\315\217o\227\210\202\334\202\267\202\252\201A\213\310\226\274\202\306\203R\203\201\203\223\203g\202\315\214\251\202\351\202\261\202\306\202\252\217o\227\210\202\334\202\271\202\361\201B",
    " ",
    " ",
    " ",
    " ",
    " ",
};

char *g_OptionsHelpTexts[9] = {
    "\203v\203\214\203C\203\204\201[\202\314\217\211\212\372\220\224\202\360\225\317\215X\202\265\202\334\202\267\201B\201i\217\211\212\372\220\335\222\350\201@\202R\201j",
    "\211\346\226\312\202\314\220F\220\224\202\360\225\317\215X\202\265\202\334\202\267\201B\202R\202Q\202a\202h\202s\202\276\202\306\215\305\202\340\343Y\227\355\202\311\225\\\216\246\202\263\202\352\202\334\202\267\201B",
    "\202a\202f\202l\202\314\215\304\220\266\225\373\226@\202\360\225\317\215X\202\265\202\334\202\267\201B\201\226\214\273\215\335\202\334\202\276MIDI\202\315\226\242\221\316\211\236\202\305\202\267",
    "\202a\202f\202l\202\314\211\271\227\312\202\360\222\262\220\256\202\265\202\334\202\267",
    "\214\370\211\312\211\271\202\314\211\271\227\312\202\360\222\262\220\256\202\265\202\334\202\267",
    "\203E\203B\203\223\203h\203E\202\251\203t\203\213\203X\203N\203\212\201[\203\223\202\251\221I\221\360\202\265\202\334\202\267",
    "\221S\202\304\217\211\212\372\220\335\222\350\202\311\202\265\202\334\202\267",
    "\221\200\215\354\214n\202\314\220\335\222\350\202\360\202\265\202\334\202\267",
    "\202\250\202\242\202\273\202\352\202\306\217I\227\271\202\265\202\334\202\267",
};

char *g_KeyConfigHelpTexts[10] = {
    "\202P\202o\201A\202Q\202o\202\360\221I\221\360\202\265\202\334\202\267",
    "PAD1,2:\203p\203b\203h\216g\227p\201AFull,Left,Right:\203L\201[\203{\201[\203h\216g\227p",
    "\203`\203\203\201[\203W\203{\203^\203\223\202\314\223\256\215\354\201ASlow:\222\341\221\254\210\332\223\256\201ACharge:\203`\203\203\201[\203W",
    "\203V\203\207\203b\203g\203{\203^\203\223\202\360\220\335\222\350\202\265\202\334\202\267",
    "\203{\203\200\203{\203^\203\223\202\360\220\335\222\350\202\265\202\334\202\267",
    "\222\341\221\254\201A\213y\202\321\203`\203\203\201[\203W\203{\203^\203\223\202\360\220\335\222\350\202\265\202\334\202\267",
    "\203\201\203b\203Z\201[\203W\203X\203L\203b\203v\203{\203^\203\223\202\360\220\335\222\350\202\265\202\334\202\267",
    "\203|\201[\203Y\203{\203^\203\223\202\360\220\335\222\350\202\265\202\334\202\267",
    "\217\211\212\372\220\335\222\350\202\311\226\337\202\265\202\334\202\267",
    "\202\250\202\250\202\346\202\273\217I\227\271\202\265\202\334\202\267",
};

char *g_TitleHelpTexts[8] = {
    "\211\324\211f\222\313\203\202\201[\203h\202\360\212J\216n\202\265\202\334\202\267",
    "\203G\203L\203X\203g\203\211\203\202\201[\203h\202\360\212J\216n\202\265\202\334\202\267",
    "\221\316\220l\220\355\201A\221\316CPU\220\355\201A\203E\203H\203b\203`\203\202\201[\203h\202\360\212J\216n\202\265\202\334\202\267",
    "\203\212\203v\203\214\203C\202\360\215\304\220\266\202\265\202\334\202\267",
    "\213L\230^\202\360\214\251\202\334\202\267",
    "\211\271\212y\216\272\202\311\223\374\202\350\202\334\202\267",
    "\212e\216\355\220\335\222\350\202\305\202\253\202\334\202\267",
    "\202\242\202\353\202\242\202\353\202\306\217I\227\271\202\265\202\334\202\267",
};


int g_UnusedCryptFlag = 1;

char *g_FileModes[3] = {
    "r",
    "w",
    "a",
};

char *g_SoundPaths[0x27] = {
    "se_plst00.wav",
    "se_enep00.wav",
    "se_pldead00.wav",
    "se_power0.wav",
    "se_power1.wav",
    "se_tan00.wav",
    "se_tan01.wav",
    "se_tan02.wav",
    "se_ok00.wav",
    "se_cancel00.wav",
    "se_select00.wav",
    "se_gun00.wav",
    "se_cat00.wav",
    "se_lazer00.wav",
    "se_lazer01.wav",
    "se_enep01.wav",
    "se_damage00.wav",
    "se_kira00.wav",
    "se_kira01.wav",
    "se_kira02.wav",
    "se_extend.wav",
    "se_graze.wav",
    "se_pause.wav",
    "se_cardget.wav",
    "se_damage01.wav",
    "se_timeout2.wav",
    "se_invalid.wav",
    "se_slash.wav",
    "se_charge00.wav",
    "se_charge01b.wav",
    "se_exattack.wav",
    "se_eterase.wav",
    "se_gosp.wav",
    "se_life1.wav",
    "se_playerdead.wav",
    "se_chargeup.wav",
    "se_warning.wav",
    "se_timestop0.wav",
    "se_powerup.wav",
};

char *g_NameAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,:;_@abcdefghijklmnopqrstuvwxyz+-/*=%0123456789#!?'\"$(){}[]<>&\\|~^ --";

char g_StoryCharacters[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 9, 12, 13, 0, 0,
};

char g_VsCpuCharacters[16] = {
    0, 1, 2, 3, 4, 5, 6, 14, 15, 7, 8, 10, 11, 9, 12, 13,
};

char g_CharacterOrder[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 9, 12, 13, 14, 15,
};

char g_ScoreCharacters[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 9, 12, 13, 0, 0,
};

char g_VsCharacters[16] = {
    0, 1, 2, 3, 4, 5, 6, 14, 15, 7, 8, 10, 11, 9, 12, 13,
};

BYTE g_CryptMagic[3] = {
    133, 164, 218,
};

unsigned char g_TimelineOrder[0x100] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

short g_LastPressedKey = 32;

GameManagerUnknownF0 g_MatchStages[17] = {
    { 9, 0, 0, 200, 0, 0, 2, 86 },
    { 9, 0, 1, 200, 0, 0, 3, 86 },
    { 9, 1, 2, 200, 0, 0, 4, 86 },
    { 9, 2, 3, 200, 0, 0, 5, 86 },
    { 9, 3, 4, 200, 0, 0, 6, 86 },
    { 9, 4, 5, 200, 0, 0, 7, 86 },
    { 9, 5, 6, 200, 0, 0, 8, 86 },
    { 9, 6, 7, 200, 0, 0, 9, 86 },
    { 9, 0, 8, 200, 0, 0, 10, 86 },
    { 9, 13, 9, 200, 0, 0, 11, 86 },
    { 9, 9, 10, 200, 0, 0, 12, 86 },
    { 9, 14, 11, 200, 0, 0, 13, 86 },
    { 9, 11, 12, 200, 0, 0, 14, 86 },
    { 9, 15, 13, 200, 0, 0, 15, 86 },
    { 9, 5, 14, 200, 0, 0, 8, 86 },
    { 9, 5, 15, 200, 0, 0, 8, 86 },
    { -1, -1, -1, -1, 0, 0, 0, 0 },
};

GameManagerUnknownF0 *g_StoryOpponents[14] = {
    g_StoryRounds4a2300,
    g_StoryRounds4a2450,
    g_StoryRounds4a25d0,
    g_StoryRounds4a2740,
    g_StoryRounds4a28c0,
    g_StoryRounds4a2a10,
    g_StoryRounds4a2ba0,
    g_StoryRounds4a2ce0,
    g_StoryRounds4a2e30,
    g_StoryRounds4a2f60,
    g_StoryRounds4a30e0,
    g_StoryRounds4a3260,
    g_StoryRounds4a33a0,
    g_StoryRounds4a34e0,
};

CryptParams g_CryptParams[8] = {
    { 93, 27, 55, 170, 64, 10240 },
    { 116, 81, 233, 187, 64, 12288 },
    { 113, 193, 81, 204, 1024, 1024 },
    { 138, 3, 25, 221, 1024, 1024 },
    { 149, 171, 205, 238, 512, 4096 },
    { 183, 18, 52, 255, 1024, 1024 },
    { 157, 53, 151, 17, 128, 10240 },
    { 170, 153, 55, 119, 1024, 4096 },
};

FormatInfo g_FormatInfos[7] = {
    { 22, 32, 0, 16711680, 65280, 255 },
    { 21, 32, -16777216, 16711680, 65280, 255 },
    { 24, 16, 0, 31744, 992, 31 },
    { 23, 16, 0, 63488, 2016, 31 },
    { 25, 16, 32768, 31744, 992, 31 },
    { 26, 16, 61440, 3840, 240, 15 },
    { -1, 0, 0, 0, 0, 0 },
};

SoundInfo g_SoundInfos[0x36] = {
    { 0, -2100, 0 },
    { 0, -2400, 0 },
    { 1, -1600, 5 },
    { 1, -1900, 5 },
    { 2, -1500, 100 },
    { 3, -1100, 100 },
    { 4, -1100, 100 },
    { 5, -1900, 50 },
    { 6, -2200, 50 },
    { 7, -2400, 50 },
    { 8, -1500, 100 },
    { 9, -1500, 100 },
    { 10, -1800, 10 },
    { 11, -1800, 10 },
    { 12, -1100, 100 },
    { 5, -1100, 50 },
    { 13, -1300, 50 },
    { 14, -1400, 50 },
    { 15, -1300, 100 },
    { 0, -1900, 0 },
    { 16, -1080, 0 },
    { 0, -1900, 0 },
    { 5, -900, 20 },
    { 6, -1800, 20 },
    { 7, -1800, 20 },
    { 17, -1100, 50 },
    { 18, -1300, 50 },
    { 19, -1500, 50 },
    { 20, -500, 140 },
    { 0, -1900, 0 },
    { 21, -1100, 20 },
    { 38, -300, 100 },
    { 21, -1200, 20 },
    { 17, -500, 50 },
    { 22, -800, 100 },
    { 23, -800, 100 },
    { 0, -1900, 0 },
    { 24, -1100, 0 },
    { 25, -300, 100 },
    { 26, -200, 100 },
    { 27, 0, 100 },
    { 27, -600, 100 },
    { 8, -400, 100 },
    { 28, -1000, 100 },
    { 29, -1000, 100 },
    { 30, -400, 100 },
    { 31, -700, 0 },
    { 32, -700, 0 },
    { 33, -350, 100 },
    { 34, -500, 100 },
    { 35, -900, 100 },
    { 36, -900, 100 },
    { 37, -900, 100 },
    { 12, -500, 100 },
};

BgmInfo g_BgmInfos[19] = {
    { 0, "th09_00.mid", -1, 0 },
    { 2, "th09_01.mid", 0, 1 },
    { 3, "th09_00b.mid", 1, 2 },
    { 4, "th09_02.mid", 2, 3 },
    { 5, "th07_10_b.mid", 3, 4 },
    { 6, "th08_12.mid", 4, 5 },
    { 7, "th09_05.mid", 5, 6 },
    { 8, "th07_09.mid", 6, 7 },
    { 9, "th09_07.mid", 7, 8 },
    { 10, "th09_10.mid", 8, 9 },
    { 11, "th09_13.mid", 9, 12 },
    { 12, "th09_08_2.mid", 10, 10 },
    { 13, "th09_12.mid", 11, 11 },
    { 14, "th09_09.mid", 12, 13 },
    { 15, "th09_11.mid", 13, 14 },
    { 1, "th09_00c.mid", -1, 15 },
    { 18, "th09_15.mid", -1, 16 },
    { 16, "th09_14.mid", -1, 17 },
    { 17, "th09_17.mid", -1, 18 },
};


PlayerCpuLevel g_PlayerCpuLevels[71] = {
    { { 0, 0, 0 }, { 0, 5, 10 }, 3 },
    { { 0, 0, 0 }, { 0, 6, 10 }, 3 },
    { { 0, 0, 0 }, { 0, 7, 15 }, 3 },
    { { 0, 0, 0 }, { 0, 8, 15 }, 3 },
    { { 0, 0, 0 }, { 0, 9, 15 }, 3 },
    { { 0, 0, 10 }, { 0, 10, 10 }, 3 },
    { { 0, 0, 10 }, { 0, 10, 10 }, 3 },
    { { 0, 0, 10 }, { 0, 10, 15 }, 3 },
    { { 0, 0, 10 }, { 0, 15, 15 }, 3 },
    { { 0, 0, 10 }, { 0, 15, 20 }, 3 },
    { { 10, 5, 10 }, { 5, 10, 10 }, 2 },
    { { 10, 5, 10 }, { 5, 10, 10 }, 2 },
    { { 10, 5, 10 }, { 5, 10, 15 }, 2 },
    { { 10, 5, 10 }, { 5, 15, 15 }, 2 },
    { { 10, 5, 10 }, { 5, 15, 20 }, 2 },
    { { 10, 5, 15 }, { 5, 10, 10 }, 2 },
    { { 10, 5, 15 }, { 5, 10, 10 }, 2 },
    { { 10, 5, 15 }, { 5, 10, 15 }, 2 },
    { { 10, 5, 15 }, { 5, 15, 15 }, 2 },
    { { 10, 5, 15 }, { 5, 15, 20 }, 2 },
    { { 15, 10, 20 }, { 25, 10, 20 }, 1 },
    { { 15, 10, 20 }, { 30, 10, 20 }, 1 },
    { { 15, 10, 20 }, { 30, 10, 25 }, 1 },
    { { 15, 10, 20 }, { 30, 15, 25 }, 1 },
    { { 15, 10, 20 }, { 30, 15, 30 }, 1 },
    { { 20, 10, 20 }, { 25, 10, 20 }, 1 },
    { { 20, 10, 20 }, { 30, 10, 20 }, 1 },
    { { 20, 10, 20 }, { 30, 10, 25 }, 1 },
    { { 20, 10, 20 }, { 30, 15, 25 }, 1 },
    { { 20, 10, 20 }, { 30, 15, 30 }, 1 },
    { { 20, 15, 30 }, { 25, 10, 20 }, 0 },
    { { 20, 15, 30 }, { 30, 10, 20 }, 0 },
    { { 20, 15, 30 }, { 30, 10, 25 }, 0 },
    { { 20, 15, 30 }, { 30, 15, 25 }, 0 },
    { { 20, 15, 30 }, { 30, 15, 30 }, 0 },
    { { 20, 15, 30 }, { 25, 20, 20 }, 0 },
    { { 20, 15, 30 }, { 30, 20, 20 }, 0 },
    { { 20, 15, 30 }, { 30, 20, 25 }, 0 },
    { { 20, 15, 30 }, { 30, 25, 25 }, 0 },
    { { 20, 15, 30 }, { 30, 25, 30 }, 0 },
    { { 30, 20, 50 }, { 25, 10, 20 }, 0 },
    { { 30, 20, 50 }, { 30, 10, 20 }, 0 },
    { { 30, 20, 50 }, { 30, 10, 25 }, 0 },
    { { 30, 20, 50 }, { 30, 10, 25 }, 0 },
    { { 30, 20, 50 }, { 30, 10, 30 }, 0 },
    { { 30, 20, 60 }, { 25, 10, 20 }, 0 },
    { { 30, 20, 60 }, { 30, 10, 20 }, 0 },
    { { 30, 20, 60 }, { 30, 10, 25 }, 0 },
    { { 30, 20, 60 }, { 30, 10, 25 }, 0 },
    { { 30, 20, 60 }, { 30, 10, 30 }, 0 },
    { { 40, 20, 60 }, { 25, 10, 20 }, 0 },
    { { 40, 20, 60 }, { 30, 10, 20 }, 0 },
    { { 40, 20, 70 }, { 30, 10, 20 }, 0 },
    { { 40, 20, 70 }, { 30, 10, 20 }, 0 },
    { { 40, 20, 70 }, { 30, 10, 20 }, 0 },
    { { 50, 20, 80 }, { 25, 10, 20 }, 0 },
    { { 50, 20, 90 }, { 30, 10, 20 }, 0 },
    { { 50, 20, 100 }, { 30, 10, 20 }, 0 },
    { { 50, 20, 110 }, { 30, 10, 20 }, 0 },
    { { 50, 20, 120 }, { 30, 10, 20 }, 0 },
    { { 9999999, 9999999, 9999999 }, { 9999999, 9999999, 9999999 }, 0 },
    { { 20, 30, 20 }, { 20, 20, 0 }, 0 },
    { { 20, 30, 22 }, { 20, 20, 0 }, 0 },
    { { 20, 30, 24 }, { 23, 20, 0 }, 0 },
    { { 25, 30, 26 }, { 23, 25, 0 }, 0 },
    { { 40, 30, 28 }, { 25, 25, 0 }, 0 },
    { { 45, 30, 40 }, { 25, 10, 0 }, 0 },
    { { 50, 30, 45 }, { 10, 10, 0 }, 0 },
    { { 60, 30, 50 }, { 10, 10, 0 }, 0 },
    { { 70, 30, 55 }, { 10, 15, 0 }, 0 },
    { { 70, 30, 120 }, { 10, 15, 0 }, 0 },
};



DWORD g_UnusedColorTable[12][4] = {
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
    { 0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0 },
};

int g_UnusedIndexTable[9][3] = {
    { 1, 2, 0 }, { 3, 4, 0 }, { 5, 6, 0 },
    { 7, 8, 0 }, { 7, 9, 0 }, { 10, 11, 0 },
    { 12, 13, 15 }, { 12, 14, 15 }, { 16, 17, 0 },
};


#define TH09_TABLE(name, address, count, stride) \
    ASSERT_SIZE(name, (count) * (stride)); \
    typedef char assert_stride_##name[sizeof((name)[0]) == (stride) ? 1 : -1];
#include "exe/data_tables.h"
#undef TH09_TABLE
