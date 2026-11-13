#include "spellcard.h"
#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"
#include "sound.h"
#include "stage.h"
#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#include "CoopGame.h"
#endif

void Player::StartClearCircle1()
{
    this->AddHitCircle(&this->pos, 0.0f, 4.0f, 0x30, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 4.0f, 2, 0x30, 0);
    this->SetPlayerState(3);
    this->stateTimer = 0x30;
    this->side->effectManager->SpawnEffect(0xd, &this->pos, 1, 0xffffffff);
}

void Player::StartClearCircle2()
{
    this->AddHitCircle(&this->pos, 0.0f, 4.0f, 0x40, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 4.0f, 2, 0x40, 0);
    this->SetPlayerState(3);
    this->stateTimer = 0x40;
    this->side->effectManager->SpawnEffect(0xe, &this->pos, 1, 0xffffffff);
}

void Player::StartClearCircle3()
{
    this->AddHitCircle(&this->pos, 0.0f, 3.5f, 0x80, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 3.5f, 2, 0x80, 0);
    this->SetPlayerState(3);
    this->stateTimer = 0x80;
    this->side->effectManager->SpawnEffect(0xf, &this->pos, 1, 0xffffffff);
}



static int RunEndCallbacks(SpellCard *card)
{
    if (card->callbacks[0].end != NULL)
    {
        card->callbacks[0].end(card);
    }
    if (card->callbacks[1].end != NULL)
    {
        card->callbacks[1].end(card);
    }
    return 0;
}

void SpellCard::Reset()
{
    this->anm = g_GameManager.gui->anm;
}

void SpellCard::Start(int type, int kind, int unknown, char *name)
{
#ifdef TH09_NATIVE_NETPLAY
    if (thnet::RouteSpell(this, type, kind, unknown)) return;
#endif
    int level;
    Enemy *enemy;

    if (kind < 2)
    {
        level = this->otherSide->player->unk_a0;
    }
    else
    {
        level = this->otherSide->player->unk_a4;
    }
    g_GameManager.rank.LowerUnknownCc(500);
    if (kind == 2)
    {
        if (this->side->enemyManager->FindByUnknownType(3) != NULL)
        {
            return;
        }
        if (this->otherSide->enemyManager->FindByUnknownType(3) != NULL)
        {
            enemy = this->otherSide->enemyManager->FindByUnknownType(3);
            enemy->flags |= 0x100;
            this->otherSide->player->unk_b8++;
        }
        this->otherSide->player->unk_b4++;
        if (this->callbacks[1].update != NULL)
        {
            this->callbacks[1].end(this);
            this->callbacks[1].update = NULL;
            this->callbacks[1].draw = NULL;
            this->callbacks[1].end = NULL;
        }
        this->slots[1].type = type;
        this->slots[1].unk_0 = unknown;
        this->unk_a8 = 1;
        this->slots[1].kind = 2;
        strcpy(this->name, name);
        g_SpellCardTypes[this->slots[1].type].init(this);
        this->callbacks[1].update = g_SpellCardTypes[this->slots[1].type].callbacks.update;
        this->callbacks[1].draw = g_SpellCardTypes[this->slots[1].type].callbacks.draw;
        this->callbacks[1].end = g_SpellCardTypes[this->slots[1].type].callbacks.end;
        if (this->otherSide->player->unk_a4 < 0x10)
        {
            this->otherSide->player->unk_a4++;
        }
        if (this->side->player->unk_a4 < 0x10)
        {
            this->side->player->unk_a4++;
        }
        g_SoundPlayer.QueueSound(0x35, 0);
    }
    else
    {
        if (this->callbacks[0].update != NULL)
        {
            this->callbacks[0].end(this);
            this->callbacks[0].update = NULL;
            this->callbacks[0].draw = NULL;
            this->callbacks[0].end = NULL;
        }
        this->slots[0].type = type;
        this->unk_a4 = 1;
        this->slots[0].unk_0 = unknown;
        this->slots[0].kind = kind;
        strcpy(this->name, name);
        this->otherSide->player->unk_b0++;
        g_SpellCardTypes[this->slots[0].type].init(this);
        this->callbacks[0].update = g_SpellCardTypes[this->slots[0].type].callbacks.update;
        this->callbacks[0].draw = g_SpellCardTypes[this->slots[0].type].callbacks.draw;
        this->callbacks[0].end = g_SpellCardTypes[this->slots[0].type].callbacks.end;
        if (this->otherSide->player->unk_a0 < 0x10)
        {
            this->otherSide->player->unk_a0++;
        }
        g_SoundPlayer.QueueSound(0xe, 0);
    }
    this->anm->StartScript(&this->unk_5f4[0], 0x1e);
    this->anm->StartScript(&this->unk_5f4[1], 0x1f);
    this->anm->StartScript(&this->unk_5f4[2], 0x20);
    this->anm->StartScript(&this->unk_5f4[3], 0x21);
    this->anm->StartScript(&this->unk_5f4[4], 0x22);
    this->anm->SetSprite(&this->unk_5f4[3], level / 10 + 0x19);
    this->anm->SetSprite(&this->unk_5f4[4], level % 10 + 0x19);
    this->anm->SetSprite(&this->unk_5f4[2], kind + 0x15);
    g_GameManager.unknownBits134_11 |= 1 << this->sideIdx;
    this->unk_98 = 0;
    this->side->stage->tintFadeMode = 1;
    this->side->stage->tintFadeFrames = 30;
    this->otherSide->stage->tintFadeMode = 2;
    this->otherSide->stage->tintFadeFrames = 30;
    if (this->sideIdx == 1)
    {
        this->otherSide->gui->unk_a678.anm->StartScriptAtOrigin(&this->otherSide->gui->unk_a678.sprites[0],
                                                                   this->otherSide->player->unk_303f0);
        this->otherSide->gui->unk_a678.anm->StartScriptAtOrigin(&this->otherSide->gui->unk_a678.sprites[1],
                                                                   this->otherSide->player->unk_303f0 + 1);
    }
    else
    {
        this->otherSide->gui->unk_a678.anm->StartScriptAtOrigin(&this->otherSide->gui->unk_a678.sprites[0],
                                                                   this->otherSide->player->unk_303f0 + 2);
        this->otherSide->gui->unk_a678.anm->StartScriptAtOrigin(&this->otherSide->gui->unk_a678.sprites[1],
                                                                   this->otherSide->player->unk_303f0 + 3);
    }
}

int SpellCard::OnUpdate(SpellCard *card)
{
    int i;

    if (card->unk_a4 >= 2)
    {
        if (card->callbacks[1].update == NULL && card->callbacks[0].update != NULL)
        {
            card->anm->StartScript(&card->unk_ac[0], card->sideIdx + 0x12);
            g_App.textAnm->SetSprite(&card->unk_ac[1], card->sideIdx + 4);
            card->unk_ac[1].scaleX = 1.0f;
            card->unk_ac[1].scaleY = 1.0f;
            g_SpriteEngine->RenderTextCentered(&card->unk_ac[1], 0xfffffe, 0, card->name);
        }
        card->unk_a4 = 0;
    }
    if (card->unk_a8 >= 2)
    {
        if (card->callbacks[1].update != NULL)
        {
            card->anm->StartScript(&card->unk_ac[0], card->sideIdx + 0x12);
            g_App.textAnm->SetSprite(&card->unk_ac[1], card->sideIdx + 4);
            card->unk_ac[1].scaleX = 1.0f;
            card->unk_ac[1].scaleY = 1.0f;
            g_SpriteEngine->RenderTextCentered(&card->unk_ac[1], 0xfffffe, 0, card->name);
        }
        card->unk_a8 = 0;
    }
    if (g_GameManager.unknownBits134_11 && card->unk_98 > 30)
    {
        g_GameManager.unknownBits134_11 &= ~(1 << card->sideIdx);
    }
    if (card->callbacks[0].update != NULL && card->callbacks[0].update(card))
    {
        if (card->callbacks[1].update == NULL)
        {
            card->unk_ac[0].visible = 0;
        }
        card->callbacks[0].end(card);
        card->callbacks[0].update = NULL;
        card->callbacks[0].draw = NULL;
        card->callbacks[0].end = NULL;
    }
    if (card->callbacks[1].update != NULL && card->callbacks[1].update(card))
    {
        card->unk_ac[0].visible = 0;
        card->callbacks[1].end(card);
        card->callbacks[1].update = NULL;
        card->callbacks[1].draw = NULL;
        card->callbacks[1].end = NULL;
        g_SoundPlayer.QueueSound(0xf, 0);
        card->side->stage->Reset();
    }
    if (g_SpriteEngine->Step(&card->unk_ac[0]))
    {
        card->unk_ac[0].visible = 0;
    }
    for (i = 0; i < 5; i++)
    {
        if (g_SpriteEngine->Step(&card->unk_5f4[i]))
        {
            card->unk_5f4[i].visible = 0;
        }
    }
    card->unk_98++;
    return 1;
}

int SpellCard::OnDraw(SpellCard *card)
{
    int i;
    Sprite *sprite;

    g_App.SetView2D(card->sideIdx);
    if (card->callbacks[0].draw != NULL)
    {
        card->callbacks[0].draw(card);
    }
    if (card->callbacks[1].draw != NULL)
    {
        card->callbacks[1].draw(card);
    }
    sprite = &card->unk_ac[0];
    if (sprite->visible)
    {
        sprite->pos.x = g_GameManager.ArenaToScreenX(sprite->posOffset.x);
        sprite->pos.y = g_GameManager.ArenaToScreenY(sprite->posOffset.y);
        sprite->pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(sprite);
        card->unk_ac[1].pos = sprite->pos;
        card->unk_ac[1].pos.y += 10.0f;
        card->unk_ac[1].color = sprite->color;
        card->unk_ac[1].flags = sprite->flags;
        g_SpriteEngine->DrawUnrotated(&card->unk_ac[1]);
    }
    sprite = card->unk_5f4;
    for (i = 0; i < 5; i++, sprite++)
    {
        if (sprite->visible)
        {
            sprite->pos.x = g_GameManager.ArenaToScreenX(sprite->posOffset.x);
            sprite->pos.y = g_GameManager.ArenaToScreenY(sprite->posOffset.y);
            sprite->pos.z = 0.0f;
            g_SpriteEngine->DrawUnrotated(sprite);
        }
    }
    return 1;
}

int SpellCard::AddedCallback(SpellCard *card)
{
    card->Reset();
    return 0;
}

void SpellCard::RemoveChain(SpellCard *card)
{
    if (card != NULL)
    {
        RunEndCallbacks(card);
        g_Chain.Remove(card->calcElem);
        g_Chain.Remove(card->drawElem);
        delete card;
    }
}

SpellCard *SpellCard::RegisterChain(int sideIdx)
{
    SpellCard *card =
        (SpellCard *)g_MemoryTracker.Register(new SpellCard(), sizeof(SpellCard), "CardAttackInf");

    memset(card, 0, sizeof(SpellCard));
    card->sideIdx = sideIdx;
    card->side = &g_GameManager.sides[sideIdx];
    card->otherSide = &g_GameManager.sides[1 - sideIdx];
    card->calcElem = g_Chain.CreateElem((ChainCallback)SpellCard::OnUpdate);
    card->calcElem->addedCallback = (ChainCallback)SpellCard::AddedCallback;
    card->calcElem->arg = card;
    if (g_Chain.AddToCalcChain(card->calcElem, sideIdx + 0x11) != 0)
    {
        return NULL;
    }
    card->drawElem = g_Chain.CreateElem((ChainCallback)SpellCard::OnDraw);
    card->drawElem->arg = card;
    g_Chain.AddToDrawChain(card->drawElem, sideIdx + 0x18);
    return card;
}
