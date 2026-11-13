#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "gdisurface.h"
#include "unsorted.h"
#include "sprite.h"



static BYTE ModulateChannel(BYTE c, BYTE factor)
{
    DWORD channel = (DWORD)c * factor >> 7;
    if (channel >= 0x100)
    {
        channel = 0xff;
    }
    return channel;
}


#define MODULATE_CHANNEL(color, factor, i) \
    ((BYTE *)&(color))[i] = ModulateChannel(((BYTE *)&(color))[i], ((BYTE *)&(factor))[i])

int AnmLoaded::SetSprite(Sprite *vm, int sprite)
{
    if (this->file == NULL || this->loadStep != 0)
    {
        return -1;
    }
    vm->anm = this;
    vm->spriteIndex = sprite;
    vm->spriteData = &this->sprites[sprite];
    vm->spriteWidth = vm->spriteData->width;
    vm->spriteHeight = vm->spriteData->height;
    D3DXMatrixIdentity(&vm->matrix);
    D3DXMatrixIdentity(&vm->sizeMatrix);
    vm->matrix.m[0][0] = vm->spriteWidth / 256.0f;
    vm->matrix.m[1][1] = vm->spriteHeight / 256.0f;
    vm->sizeMatrix.m[0][0] = vm->spriteData->scaleX / vm->spriteData->textureWidth * vm->spriteWidth;
    vm->sizeMatrix.m[1][1] = vm->spriteData->scaleY / vm->spriteData->textureHeight * vm->spriteHeight;
    vm->drawMatrix = vm->matrix;
    return 0;
}

int Sprite::ResolveIntArgument(int value)
{
    switch (value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    }
    return value;
}

float *Sprite::ResolveFloatArgumentPtr(float *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch ((int)*value)
    {
    case 10004:
        return &this->scriptVarF0;
    case 10005:
        return &this->scriptVarF1;
    case 10006:
        return &this->scriptVarF2;
    case 10007:
        return &this->scriptVarF3;
    }
    return value;
}

int *Sprite::ResolveIntArgumentPtr(int *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch (*value)
    {
    case 10000:
        return &this->scriptVar0;
    case 10001:
        return &this->scriptVar1;
    case 10002:
        return &this->scriptVar2;
    case 10003:
        return &this->scriptVar3;
    case 10008:
        return &this->scriptVar8;
    case 10009:
        return &this->scriptVar9;
    }
    return value;
}

void SpriteEngine::DrawSprite(Sprite *vm)
{
    switch (vm->drawMode)
    {
    case 0:
        this->DrawUnrotated(vm);
        break;
    case 1:
        this->DrawNoRound(vm);
        break;
    case 2:
        this->Draw(vm);
        break;
    case 3:
        this->DrawRotatedNoRound(vm);
        break;
    case 4:
        this->DrawProjected(vm);
        break;
    case 5:
        this->DrawProjected3D(vm);
        break;
    }
}

void SpriteEngine::SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt)
{
    while (count != 0)
    {
        if (g_SpriteEngine->HasTexture(vms))
        {
            vms->SetPendingInterrupt(interrupt);
        }
        vms++;
        count--;
    }
}

float Sprite::ResolveFloatArgument(float value)
{
    switch ((int)value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    case 10011:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10012:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10010:
        return g_Rng.GetRandomF32SignedInRange(3.1415927f);
    }
    return value;
}


#define ANM_INT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveIntArgument(curInstr->args[n].i) : curInstr->args[n].i)
#define ANM_FLOAT_ARG(n) \
    ((curInstr->paramMask & (1 << (n))) ? vm->ResolveFloatArgument(curInstr->args[n].f) : curInstr->args[n].f)
#define ANM_INT_VAR(n) (*vm->ResolveIntArgumentPtr(&curInstr->args[n].i, curInstr->paramMask, (n)))
#define ANM_FLOAT_VAR(n) (*vm->ResolveFloatArgumentPtr(&curInstr->args[n].f, curInstr->paramMask, (n)))

int SpriteEngine::Step(Sprite *vm)
{
    AnmInstr *curInstr;
    AnmInstr *fallback;
    int k;
    float t;

    if (vm->scriptPos == NULL)
    {
        return 1;
    }
    if (vm->scriptPaused)
    {
        return 0;
    }
    if (vm->pendingInterrupt != 0)
    {
        goto interrupt;
    }
    while (curInstr = vm->scriptPos, curInstr->time <= vm->scriptTime.AsFrames())
    {
        switch (curInstr->opcode)
        {
        case -1:
        case 1:
            vm->shown = 0;
        case 2:
            vm->scriptPos = NULL;
            return 1;
        case 3:
            vm->shown = 1;
            vm->anm->SetSprite(vm, ANM_INT_ARG(0));
            vm->spriteTime = vm->scriptTime.AsFrames();
            break;
        case 7:
            vm->scaleX = ANM_FLOAT_ARG(0);
            vm->scaleY = ANM_FLOAT_ARG(1);
            vm->flags |= 8;
            break;
        case 8:
            vm->alpha = ANM_INT_ARG(0);
            break;
        case 9:
            vm->colorRgb[2] = ANM_INT_ARG(0);
            vm->colorRgb[1] = ANM_INT_ARG(1);
            vm->colorRgb[0] = ANM_INT_ARG(2);
            break;
        case 85:
            vm->color2Alpha = ANM_INT_ARG(0);
            break;
        case 84:
            vm->color2Rgb[2] = ANM_INT_ARG(0);
            vm->color2Rgb[1] = ANM_INT_ARG(1);
            vm->color2Rgb[0] = ANM_INT_ARG(2);
            break;
        case 4:
            vm->scriptTime = curInstr->args[1].i;
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[0].i);
            continue;
        case 5:
            ANM_INT_VAR(0)--;
            if (ANM_INT_ARG(0) > 0)
            {
                vm->scriptTime = curInstr->args[2].i;
                vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[1].i);
                continue;
            }
            break;
        case 10:
            vm->flip ^= 1;
            vm->scaleX *= -1.0f;
            vm->flags |= 8;
            break;
        case 24:
            vm->usePosOffset = curInstr->args[0].i;
            break;
        case 11:
            vm->flip ^= 2;
            vm->scaleY *= -1.0f;
            vm->flags |= 8;
            break;
        case 12:
            vm->rotation.x = ANM_FLOAT_ARG(0);
            vm->rotation.y = ANM_FLOAT_ARG(1);
            vm->rotation.z = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 13:
            vm->rotationSpeed.x = ANM_FLOAT_ARG(0);
            vm->rotationSpeed.y = ANM_FLOAT_ARG(1);
            vm->rotationSpeed.z = ANM_FLOAT_ARG(2);
            vm->flags |= 4;
            break;
        case 14:
            vm->scaleSpeedX = ANM_FLOAT_ARG(0);
            vm->scaleSpeedY = ANM_FLOAT_ARG(1);
            break;
        case 29:
            vm->interpDuration[4] = 0;
            vm->interpFrame[4] = ANM_INT_ARG(2);
            vm->scaleInterpEasing = 0;
            *(D3DXVECTOR2 *)&vm->scaleTween = *(D3DXVECTOR2 *)&vm->scaleX;
            vm->scaleTween.z = ANM_FLOAT_ARG(0);
            vm->colorTween.x = ANM_FLOAT_ARG(1);
            break;
        case 15:
            ((BYTE *)&vm->colorTween.y)[3] = vm->alpha;
            ((BYTE *)&vm->colorTween.z)[3] = curInstr->args[0].i;
            vm->interpDuration[2] = 0;
            vm->interpFrame[2] = ANM_INT_ARG(1);
            vm->alphaInterpEasing = 0;
            break;
        case 16:
            vm->blendMode = curInstr->args[0].i != 0;
            break;
        case 82:
            vm->blendMode = curInstr->args[0].i;
            break;
        case 6:
            if (!vm->usePosOffset)
            {
                vm->pos = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            else
            {
                vm->posOffset = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            }
            break;
        case 19:
            vm->posInterpEasing = 6;
            goto posInterp;
        case 18:
            vm->posInterpEasing = 4;
            goto posInterp;
        case 17:
            vm->posInterpEasing = 0;
        posInterp:
            if (!vm->usePosOffset)
            {
                vm->moveStart = vm->pos;
            }
            else
            {
                vm->moveStart = vm->posOffset;
            }
            vm->moveEnd = D3DXVECTOR3(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            vm->interpFrame[0] = ANM_INT_ARG(3);
            vm->interpDuration[0] = 0;
            break;
        case 79:
            if (vm->waitTimer == 0)
            {
                vm->waitTimer = ANM_INT_ARG(0);
            }
            else
            {
                vm->waitTimer--;
            }
            if (vm->waitTimer <= 0)
            {
                vm->waitTimer = 0;
                break;
            }
            vm->scriptTime--;
            goto done;
        case 23:
            vm->shown = 0;
        case 20:
            if (vm->pendingInterrupt == 0)
            {
                vm->stopped = 1;
                vm->scriptTime--;
                goto done;
            }
        interrupt:
            fallback = NULL;
            curInstr = vm->scriptStart;
            while ((curInstr->opcode != 21 || vm->pendingInterrupt != curInstr->args[0].i) &&
                   curInstr->opcode != -1)
            {
                if (curInstr->opcode == 21 && curInstr->args[0].i == -1)
                {
                    fallback = curInstr;
                }
                curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            }
            vm->pendingInterrupt = 0;
            vm->stopped = 0;
            if (curInstr->opcode != 21)
            {
                if (fallback == NULL)
                {
                    vm->scriptTime--;
                    goto done;
                }
                curInstr = fallback;
            }
            vm->interruptReturnTime = vm->scriptTime;
            vm->interruptReturnPos = vm->scriptPos;
            curInstr = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
            vm->scriptPos = curInstr;
            vm->scriptTime = vm->scriptPos->time;
            vm->shown = 1;
            continue;
        case 89:
            vm->scriptTime = vm->interruptReturnTime;
            vm->scriptPos = vm->interruptReturnPos;
            continue;
        case 28:
            vm->shown = curInstr->args[0].i;
            break;
        case 22:
            vm->anchor = 3;
            break;
        case 25:
            vm->autoRotate = curInstr->args[0].i;
            break;
        case 26:
            vm->uvScrollX += ANM_FLOAT_ARG(0);
            if (vm->uvScrollX >= 1.0f)
            {
                vm->uvScrollX -= 1.0f;
            }
            else if (vm->uvScrollX < 0.0f)
            {
                vm->uvScrollX += 1.0f;
            }
            break;
        case 27:
            vm->uvScrollY += ANM_FLOAT_ARG(0);
            if (vm->uvScrollY >= 1.0f)
            {
                vm->uvScrollY -= 1.0f;
            }
            else if (vm->uvScrollY < 0.0f)
            {
                vm->uvScrollY += 1.0f;
            }
            break;
        case 80:
            vm->uvScrollSpeedX = ANM_FLOAT_ARG(0);
            break;
        case 81:
            vm->uvScrollSpeedY = ANM_FLOAT_ARG(0);
            break;
        case 30:
            vm->zWriteDisabled = curInstr->args[0].i;
            break;
        case 31:
            vm->cameraSetup = curInstr->args[0].i;
            break;
        case 32:
            vm->interpDuration[0] = 0;
            vm->interpFrame[0] = ANM_INT_ARG(0);
            vm->posInterpEasing = curInstr->args[1].i;
            if (!vm->usePosOffset)
            {
                vm->moveStart = vm->pos;
            }
            else
            {
                vm->moveStart = vm->posOffset;
            }
            vm->moveEnd.x = ANM_FLOAT_ARG(2);
            vm->moveEnd.y = ANM_FLOAT_ARG(3);
            vm->moveEnd.z = ANM_FLOAT_ARG(4);
            break;
        case 33:
            vm->interpDuration[1] = 0;
            vm->interpFrame[1] = ANM_INT_ARG(0);
            vm->colorInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->colorTween.y)[2] = vm->colorRgb[2];
            ((BYTE *)&vm->colorTween.y)[1] = vm->colorRgb[1];
            ((BYTE *)&vm->colorTween.y)[0] = vm->colorRgb[0];
            ((BYTE *)&vm->colorTween.z)[2] = ANM_INT_ARG(2);
            ((BYTE *)&vm->colorTween.z)[1] = ANM_INT_ARG(3);
            ((BYTE *)&vm->colorTween.z)[0] = ANM_INT_ARG(4);
            break;
        case 34:
            vm->interpDuration[2] = 0;
            vm->interpFrame[2] = ANM_INT_ARG(0);
            vm->alphaInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->colorTween.y)[3] = vm->alpha;
            ((BYTE *)&vm->colorTween.z)[3] = ANM_INT_ARG(2);
            break;
        case 86:
            vm->interpDuration[5] = 0;
            vm->interpFrame[5] = ANM_INT_ARG(0);
            vm->color2InterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->color2TweenStart)[2] = vm->color2Rgb[2];
            ((BYTE *)&vm->color2TweenStart)[1] = vm->color2Rgb[1];
            ((BYTE *)&vm->color2TweenStart)[0] = vm->color2Rgb[0];
            ((BYTE *)&vm->color2TweenEnd)[2] = ANM_INT_ARG(2);
            ((BYTE *)&vm->color2TweenEnd)[1] = ANM_INT_ARG(3);
            ((BYTE *)&vm->color2TweenEnd)[0] = ANM_INT_ARG(4);
            break;
        case 87:
            vm->interpDuration[6] = 0;
            vm->interpFrame[6] = ANM_INT_ARG(0);
            vm->color2AlphaInterpEasing = curInstr->args[1].i;
            ((BYTE *)&vm->color2TweenStart)[3] = vm->color2Alpha;
            ((BYTE *)&vm->color2TweenEnd)[3] = ANM_INT_ARG(2);
            break;
        case 35:
            vm->interpDuration[3] = 0;
            vm->interpFrame[3] = ANM_INT_ARG(0);
            vm->rotationInterpEasing = curInstr->args[1].i;
            vm->rotateStart = vm->rotation;
            vm->rotateEnd.x = ANM_FLOAT_ARG(2);
            vm->rotateEnd.y = ANM_FLOAT_ARG(3);
            vm->rotateEnd.z = ANM_FLOAT_ARG(4);
            vm->flags |= 4;
            break;
        case 36:
            vm->interpDuration[4] = 0;
            vm->interpFrame[4] = ANM_INT_ARG(0);
            vm->scaleInterpEasing = curInstr->args[1].i;
            *(D3DXVECTOR2 *)&vm->scaleTween = *(D3DXVECTOR2 *)&vm->scaleX;
            vm->scaleTween.z = ANM_FLOAT_ARG(2);
            vm->colorTween.x = ANM_FLOAT_ARG(3);
            vm->flags |= 8;
            break;
        case 83:
            vm->drawMode = curInstr->args[0].i;
            break;
        case 37:
            ANM_INT_VAR(0) = ANM_INT_ARG(1);
            break;
        case 38:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1);
            break;
        case 49:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) + ANM_INT_ARG(2);
            break;
        case 50:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) + ANM_FLOAT_ARG(2);
            break;
        case 51:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) - ANM_INT_ARG(2);
            break;
        case 52:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) - ANM_FLOAT_ARG(2);
            break;
        case 53:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) * ANM_INT_ARG(2);
            break;
        case 54:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) * ANM_FLOAT_ARG(2);
            break;
        case 55:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) / ANM_INT_ARG(2);
            break;
        case 56:
            ANM_FLOAT_VAR(0) = ANM_FLOAT_ARG(1) / ANM_FLOAT_ARG(2);
            break;
        case 57:
            ANM_INT_VAR(0) = ANM_INT_ARG(1) % ANM_INT_ARG(2);
            break;
        case 58:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(1), ANM_FLOAT_ARG(2));
            break;
        case 39:
            ANM_INT_VAR(0) += ANM_INT_ARG(1);
            break;
        case 40:
            ANM_FLOAT_VAR(0) += ANM_FLOAT_ARG(1);
            break;
        case 41:
            ANM_INT_VAR(0) -= ANM_INT_ARG(1);
            break;
        case 42:
            ANM_FLOAT_VAR(0) -= ANM_FLOAT_ARG(1);
            break;
        case 43:
            ANM_INT_VAR(0) *= ANM_INT_ARG(1);
            break;
        case 44:
            ANM_FLOAT_VAR(0) *= ANM_FLOAT_ARG(1);
            break;
        case 45:
            ANM_INT_VAR(0) /= ANM_INT_ARG(1);
            break;
        case 46:
            ANM_FLOAT_VAR(0) /= ANM_FLOAT_ARG(1);
            break;
        case 47:
            ANM_INT_VAR(0) %= ANM_INT_ARG(1);
            break;
        case 48:
            ANM_FLOAT_VAR(0) = fmodf(ANM_FLOAT_ARG(0), ANM_FLOAT_ARG(1));
            break;
        case 59:
            ANM_INT_VAR(0) = g_Rng.GetRandomU32InRange(ANM_INT_ARG(1));
            break;
        case 60:
            ANM_FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(ANM_FLOAT_ARG(1));
            break;
        case 61:
            ANM_FLOAT_VAR(0) = sinf(ANM_FLOAT_ARG(1));
            break;
        case 62:
            ANM_FLOAT_VAR(0) = cosf(ANM_FLOAT_ARG(1));
            break;
        case 63:
            ANM_FLOAT_VAR(0) = tanf(ANM_FLOAT_ARG(1));
            break;
        case 64:
            ANM_FLOAT_VAR(0) = acosf(ANM_FLOAT_ARG(1));
            break;
        case 65:
            ANM_FLOAT_VAR(0) = atanf(ANM_FLOAT_ARG(1));
            break;
        case 66:
            ANM_FLOAT_VAR(0) = AddNormalizeAngle(ANM_FLOAT_ARG(0), 0.0f);
            break;
        case 67:
            if (ANM_INT_ARG(0) == ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 68:
            if (ANM_FLOAT_ARG(0) == ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 69:
            if (ANM_INT_ARG(0) != ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 70:
            if (ANM_FLOAT_ARG(0) != ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 71:
            if (ANM_INT_ARG(0) < ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 72:
            if (ANM_FLOAT_ARG(0) < ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 73:
            if (ANM_INT_ARG(0) <= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 74:
            if (ANM_FLOAT_ARG(0) <= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 75:
            if (ANM_INT_ARG(0) > ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 76:
            if (ANM_FLOAT_ARG(0) > ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 77:
            if (ANM_INT_ARG(0) >= ANM_INT_ARG(1))
            {
                goto jump;
            }
            break;
        case 78:
            if (ANM_FLOAT_ARG(0) >= ANM_FLOAT_ARG(1))
            {
                goto jump;
            }
            break;
        case 88:
            vm->useColor2 = curInstr->args[0].b[1];
            break;
        jump:
            vm->scriptTime = curInstr->args[3].i;
            vm->scriptPos = (AnmInstr *)((BYTE *)vm->scriptStart + curInstr->args[2].i);
            continue;
        }
        vm->scriptPos = (AnmInstr *)((BYTE *)curInstr + curInstr->size);
    }

done:
    if (vm->rotationSpeed.x != 0.0f)
    {
        vm->rotation.x = AddNormalizeAngle(vm->rotation.x, g_App.gameSpeed * vm->rotationSpeed.x);
        vm->flags |= 4;
    }
    if (vm->rotationSpeed.y != 0.0f)
    {
        vm->rotation.y = AddNormalizeAngle(vm->rotation.y, g_App.gameSpeed * vm->rotationSpeed.y);
        vm->flags |= 4;
    }
    if (vm->rotationSpeed.z != 0.0f)
    {
        vm->rotation.z = AddNormalizeAngle(vm->rotation.z, g_App.gameSpeed * vm->rotationSpeed.z);
        vm->flags |= 4;
    }
    for (k = 0; k < 7; k++)
    {
        if (vm->interpFrame[k] > 0)
        {
            vm->interpDuration[k]++;
            if (vm->interpDuration[k] >= vm->interpFrame[k].AsFrames())
            {
                t = 1.0f;
                vm->interpFrame[k] = 0;
            }
            else
            {
                t = vm->interpDuration[k].AsFloat() / vm->interpFrame[k].AsFloat();
            }
            switch ((&vm->posInterpEasing)[k])
            {
            case 1:
                t = t * t;
                break;
            case 2:
                t = t * t * t;
                break;
            case 3:
                t = (t = t * t) * t;
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
                t = (t = (t = 1.0f - t) * t) * t;
                t = 1.0f - t;
                break;
            }
            switch (k)
            {
            case 0:
                if (!vm->usePosOffset)
                {
                    vm->pos.x = (vm->moveEnd.x - vm->moveStart.x) * t + vm->moveStart.x;
                    vm->pos.y = (vm->moveEnd.y - vm->moveStart.y) * t + vm->moveStart.y;
                    vm->pos.z = (vm->moveEnd.z - vm->moveStart.z) * t + vm->moveStart.z;
                }
                else
                {
                    vm->posOffset.x = (vm->moveEnd.x - vm->moveStart.x) * t + vm->moveStart.x;
                    vm->posOffset.y = (vm->moveEnd.y - vm->moveStart.y) * t + vm->moveStart.y;
                    vm->posOffset.z = (vm->moveEnd.z - vm->moveStart.z) * t + vm->moveStart.z;
                }
                break;
            case 1:
                vm->colorRgb[2] = ((float)((BYTE *)&vm->colorTween.z)[2] - ((BYTE *)&vm->colorTween.y)[2]) * t +
                                 ((BYTE *)&vm->colorTween.y)[2];
                vm->colorRgb[1] = ((float)((BYTE *)&vm->colorTween.z)[1] - ((BYTE *)&vm->colorTween.y)[1]) * t +
                                 ((BYTE *)&vm->colorTween.y)[1];
                vm->colorRgb[0] = ((float)((BYTE *)&vm->colorTween.z)[0] - ((BYTE *)&vm->colorTween.y)[0]) * t +
                                 ((BYTE *)&vm->colorTween.y)[0];
                break;
            case 2:
                vm->alpha = ((float)((BYTE *)&vm->colorTween.z)[3] - ((BYTE *)&vm->colorTween.y)[3]) * t +
                            ((BYTE *)&vm->colorTween.y)[3];
                break;
            case 5:
                vm->color2Rgb[2] = ((float)((BYTE *)&vm->color2TweenEnd)[2] - ((BYTE *)&vm->color2TweenStart)[2]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[2];
                vm->color2Rgb[1] = ((float)((BYTE *)&vm->color2TweenEnd)[1] - ((BYTE *)&vm->color2TweenStart)[1]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[1];
                vm->color2Rgb[0] = ((float)((BYTE *)&vm->color2TweenEnd)[0] - ((BYTE *)&vm->color2TweenStart)[0]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[0];
                break;
            case 6:
                vm->color2Alpha = ((float)((BYTE *)&vm->color2TweenEnd)[3] - ((BYTE *)&vm->color2TweenStart)[3]) * t +
                                  ((BYTE *)&vm->color2TweenStart)[3];
                break;
            case 3:
                vm->rotation.x = AddNormalizeAngle((vm->rotateEnd.x - vm->rotateStart.x) * t, vm->rotateStart.x);
                vm->rotation.y = AddNormalizeAngle((vm->rotateEnd.y - vm->rotateStart.y) * t, vm->rotateStart.y);
                vm->rotation.z = AddNormalizeAngle((vm->rotateEnd.z - vm->rotateStart.z) * t, vm->rotateStart.z);
                vm->flags |= 4;
                break;
            case 4:
                vm->scaleX = (vm->scaleTween.z - vm->scaleTween.x) * t + vm->scaleTween.x;
                vm->scaleY = (vm->colorTween.x - vm->scaleTween.y) * t + vm->scaleTween.y;
                vm->flags |= 8;
                break;
            }
        }
    }
    if (vm->scaleSpeedY != 0.0f)
    {
        vm->scaleY += g_App.gameSpeed * vm->scaleSpeedY;
        vm->flags |= 8;
    }
    if (vm->scaleSpeedX != 0.0f)
    {
        vm->scaleX += g_App.gameSpeed * vm->scaleSpeedX;
        vm->flags |= 8;
        vm->flags |= 4;
    }
    vm->uvScrollX = vm->uvScrollX + vm->uvScrollSpeedX;
    if (vm->uvScrollX >= 1.0f)
    {
        vm->uvScrollX -= 1.0f;
    }
    else if (vm->uvScrollX < 0.0f)
    {
        vm->uvScrollX += 1.0f;
    }
    vm->uvScrollY = vm->uvScrollY + vm->uvScrollSpeedY;
    if (vm->uvScrollY >= 1.0f)
    {
        vm->uvScrollY -= 1.0f;
    }
    else if (vm->uvScrollY < 0.0f)
    {
        vm->uvScrollY += 1.0f;
    }
    vm->scriptTime++;
    this->stepCount++;
    return 0;
}

#undef ANM_INT_ARG
#undef ANM_FLOAT_ARG
#undef ANM_INT_VAR
#undef ANM_FLOAT_VAR

void SpriteEngine::StepSprites(Sprite *vms, int count)
{
    while (count != 0)
    {
        if (vms->scriptIndex >= 0)
        {
            g_SpriteEngine->Step(vms);
        }
        vms++;
        count--;
    }
}

void AnmLoaded::SetScript(Sprite *vm, void *script)
{
    if (script == NULL || this->loadStep != 0)
    {
        memset(vm, 0, sizeof(Sprite));
    }
    else
    {
        vm->Initialize();
        vm->anmSlot = this->slot;
        vm->anm = this;
        vm->flags &= ~0x600;
        vm->scriptStart = (AnmInstr *)script;
        vm->scriptPos = vm->scriptStart;
        vm->scriptTime = 0;
        vm->flags &= ~1;
        g_SpriteEngine->Step(vm);
        g_SpriteEngine->scriptStartCount++;
    }
}

void AnmLoaded::StartSpriteSeries(Sprite *vms, int script, int count)
{
    while (count != 0)
    {
        this->StartScriptAtOrigin(vms, script);
        vms->baseSpriteIndex = vms->spriteIndex;
        script++;
        vms++;
        count--;
    }
}

void SpriteEngine::BeginQuadBatch()
{
    this->batchQuadCount = 0;
    this->batchCursor = this->batchVertices;
    this->batchStart = this->batchCursor;
}

void SpriteEngine::FlushQuadBatch()
{
    if (this->batchQuadCount == 0)
    {
        return;
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, this->batchQuadCount * 2, this->batchStart,
                                     sizeof(VertexTex1DiffuseXyzrwh));
    this->batchStart = this->batchCursor;
    this->batchQuadCount = 0;
    this->batchDrawCount++;
}

int SpriteEngine::AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad)
{
    this->batchCursor[0] = quad[0];
    this->batchCursor[1] = quad[1];
    this->batchCursor[2] = quad[2];
    this->batchCursor[3] = quad[1];
    this->batchCursor[4] = quad[2];
    this->batchCursor[5] = quad[3];
    this->batchCursor += 6;
    this->batchQuadCount++;
    return 0;
}

void SpriteEngine::RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy)
{
    out[0] = x * cosine - y * sine + cx;
    out[1] = x * sine + y * cosine + cy;
}

int SpriteEngine::ProjectSprite(Sprite *vm)
{
    float c;
    float angle;
    float s;
    float x;
    float y;
    float h;
    float w;

    angle = vm->rotation.z;
    __asm
    {
        fld angle
        fsincos
        fstp c
        fstp s
    }
    D3DXMATRIX world;
    D3DXVECTOR3 origin;
    D3DXVECTOR3 edge;
    D3DXVECTOR3 d;
    D3DXVECTOR3 o(0.0f, 0.0f, 0.0f);


    D3DXMatrixIdentity(&world);
    memcpy(&world._41, vm->pos, sizeof(D3DXVECTOR3));
    D3DXVec3Project(&origin, &o, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                    &g_App.camera->viewMatrix, &world);
    if (origin.z < 0.0f || origin.z > 1.0f)
    {
        return -1;
    }
    D3DXVec3Project(&edge, &g_App.camera->cameraRight, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                    &g_App.camera->viewMatrix, &world);
    d = edge - origin;
    x = D3DXVec3Length(&d) * 0.5f;
    w = vm->spriteWidth * vm->scaleX * x;
    h = vm->spriteHeight * vm->scaleY * x;
    x = origin.x;
    y = origin.y;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -w, h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, w, h, s, c, x, y);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = origin.z;
    if ((vm->flags >> 11 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += w;
        g_DiffuseQuad[1].pos.x += w;
        g_DiffuseQuad[2].pos.x += w;
        g_DiffuseQuad[3].pos.x += w;
    }
    if ((vm->flags >> 11 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += h;
        g_DiffuseQuad[1].pos.y += h;
        g_DiffuseQuad[2].pos.y += h;
        g_DiffuseQuad[3].pos.y += h;
    }
    return 0;
}

void SpriteEngine::ProjectSprite3D(Sprite *vm)
{
    D3DXMATRIX worldMatrix;
    D3DXMATRIX rotationMatrix;
    DWORD anchor;

    if ((vm->flags >> 16 & 1) == 0 && ((vm->flags >> 3 & 1) != 0 || (vm->flags >> 2 & 1) != 0))
    {
        vm->drawMatrix = vm->matrix;
        vm->drawMatrix._11 *= vm->scaleX;
        vm->drawMatrix._22 *= vm->scaleY;
        vm->flags &= ~8;
        if (vm->rotation.x != 0.0)
        {
            D3DXMatrixRotationX(&rotationMatrix, vm->rotation.x);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.y != 0.0)
        {
            D3DXMatrixRotationY(&rotationMatrix, vm->rotation.y);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.z != 0.0)
        {
            D3DXMatrixRotationZ(&rotationMatrix, vm->rotation.z);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        vm->flags &= ~4;
    }
    worldMatrix = vm->drawMatrix;
    anchor = vm->flags >> 11;
    if ((anchor & 1) == 0)
    {
        worldMatrix._41 = vm->pos.x;
    }
    else
    {
        worldMatrix._41 = Fabs(vm->spriteWidth * vm->scaleX / 2.0f) + vm->pos.x;
    }
    if ((anchor & 2) == 0)
    {
        worldMatrix._42 = vm->pos.y;
    }
    else
    {
        worldMatrix._42 = Fabs(vm->spriteHeight * vm->scaleY / 2.0f) + vm->pos.y;
    }
    worldMatrix._43 = vm->pos.z;
    D3DXVec3Project(&g_DiffuseQuad[0].pos, &this->quad[0].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[1].pos, &this->quad[1].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[2].pos, &this->quad[2].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[3].pos, &this->quad[3].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    this->lastWorldMatrix = worldMatrix;
}

void SpriteEngine::SetRenderStateForSprite(Sprite *vm)
{
    D3DCOLOR color;

    if (this->blendMode != (vm->flags >> 4 & 3))
    {
        this->FlushQuadBatch();
        this->blendMode = vm->flags >> 4 & 3;
        switch (this->blendMode)
        {
        case 0:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case 1:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }
    color = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
    if (this->modulateColorEnabled != 0)
    {
        MODULATE_CHANNEL(color, this->modulateColor, 2);
        MODULATE_CHANNEL(color, this->modulateColor, 1);
        MODULATE_CHANNEL(color, this->modulateColor, 0);
        MODULATE_CHANNEL(color, this->modulateColor, 3);
    }
    if (this->currentColor != color)
    {
        this->FlushQuadBatch();
        this->currentColor = color;
        g_App.d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->currentColor);
    }
    if (this->zWriteDisabled != (vm->flags >> 13 & 1))
    {
        this->FlushQuadBatch();
        this->zWriteDisabled = vm->flags >> 13 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    if (this->lastCameraSetupFlag != (vm->flags >> 15 & 1))
    {
        this->FlushQuadBatch();
        this->lastCameraSetupFlag = vm->flags >> 15 & 1;
        if (this->lastCameraSetupFlag == 0)
        {
            g_App.ResetView();
        }
        else
        {
            g_App.ResetView3D();
        }
    }
    this->renderStateSetCount++;
}

int SpriteEngine::FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    float width;
    float step;
    float u;
    float u0;
    VertexTex1DiffuseXyzrwh *cur;
    int i;
    float v;

    if (count < 3)
    {
        return -1;
    }
    u0 = vm->spriteData->uvEndX + vm->uvScrollX;
    width = vm->spriteData->uvEndX - vm->spriteData->uvStartX;
    v = vm->spriteData->uvStartY + vm->uvScrollY;
    cur = vertices;
    step = width / ((count + 1) / 2 - 1);
    for (i = 0, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    v = vm->spriteData->uvEndY + vm->uvScrollY;
    cur = vertices + 1;
    for (i = 1, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    return 0;
}

int SpriteEngine::FillVerticalStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    float width;
    float step;
    float u;
    float u0;
    VertexTex1DiffuseXyzrwh *cur;
    int i;
    float v;



    if (count < 3)
    {
        return -1;
    }
    u0 = vm->spriteData->uvEndY + vm->uvScrollY;
    width = vm->spriteData->uvEndY - vm->spriteData->uvStartY;
    v = vm->spriteData->uvStartX + vm->uvScrollX;
    cur = vertices;
    step = width / ((count + 1) / 2 - 1);
    for (i = 0, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.y = u;
        cur->uv.x = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    v = vm->spriteData->uvEndX + vm->uvScrollX;
    cur = vertices + 1;
    for (i = 1, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.y = u;
        cur->uv.x = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    return 0;
}

void SpriteEngine::SetBlendModeForSprite(Sprite *vm)
{
    if (this->blendMode != (vm->flags >> 4 & 3))
    {
        this->FlushQuadBatch();
        this->blendMode = vm->flags >> 4 & 3;
        switch (this->blendMode)
        {
        case 0:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case 1:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }
    if (this->zWriteDisabled != (vm->flags >> 13 & 1))
    {
        this->zWriteDisabled = vm->flags >> 13 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    this->renderStateSetCount++;
}

int SpriteEngine::DrawQuad(Sprite *vm, int flags)
{
    float bottom;
    float minY;
    float minX;
    float maxX;
    D3DCOLOR diffuse;

    g_DiffuseQuad[0].pos.x += this->shakeX;
    g_DiffuseQuad[0].pos.y += this->shakeY;
    g_DiffuseQuad[1].pos.x += this->shakeX;
    g_DiffuseQuad[1].pos.y += this->shakeY;
    g_DiffuseQuad[2].pos.x += this->shakeX;
    g_DiffuseQuad[2].pos.y += this->shakeY;
    g_DiffuseQuad[3].pos.x += this->shakeX;
    g_DiffuseQuad[3].pos.y += this->shakeY;
    if (flags & 1)
    {
        __asm
        {
            fld dword ptr [g_DiffuseQuad + 0x00]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x1c]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x04]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x3c]
            frndint
            fsub g_Half
            fst dword ptr [g_DiffuseQuad + 0x3c]
            fstp dword ptr [g_DiffuseQuad + 0x58]
            fst dword ptr [g_DiffuseQuad + 0x04]
            fstp dword ptr [g_DiffuseQuad + 0x20]
            fst dword ptr [g_DiffuseQuad + 0x1c]
            fstp dword ptr [g_DiffuseQuad + 0x54]
            fst dword ptr [g_DiffuseQuad + 0x00]
            fstp dword ptr [g_DiffuseQuad + 0x38]
        }
    }
    g_DiffuseQuad[0].uv.x = g_DiffuseQuad[2].uv.x = vm->spriteData->uvStartX + vm->uvScrollX;
    g_DiffuseQuad[1].uv.x = g_DiffuseQuad[3].uv.x = vm->spriteData->uvEndX + vm->uvScrollX;
    g_DiffuseQuad[0].uv.y = g_DiffuseQuad[1].uv.y = vm->spriteData->uvStartY + vm->uvScrollY;
    g_DiffuseQuad[2].uv.y = g_DiffuseQuad[3].uv.y = vm->spriteData->uvEndY + vm->uvScrollY;
    maxX = g_DiffuseQuad[0].pos.x > g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    maxX = g_DiffuseQuad[2].pos.x > maxX ? g_DiffuseQuad[2].pos.x : maxX;
    maxX = g_DiffuseQuad[3].pos.x > maxX ? g_DiffuseQuad[3].pos.x : maxX;
    bottom = g_DiffuseQuad[0].pos.y > g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    bottom = g_DiffuseQuad[2].pos.y > bottom ? g_DiffuseQuad[2].pos.y : bottom;
    bottom = g_DiffuseQuad[3].pos.y > bottom ? g_DiffuseQuad[3].pos.y : bottom;
    minX = g_DiffuseQuad[0].pos.x < g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    minX = g_DiffuseQuad[2].pos.x < minX ? g_DiffuseQuad[2].pos.x : minX;
    minX = g_DiffuseQuad[3].pos.x < minX ? g_DiffuseQuad[3].pos.x : minX;
    minY = g_DiffuseQuad[0].pos.y < g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    minY = g_DiffuseQuad[2].pos.y < minY ? g_DiffuseQuad[2].pos.y : minY;
    minY = g_DiffuseQuad[3].pos.y < minY ? g_DiffuseQuad[3].pos.y : minY;
    if (maxX < g_App.camera->viewport.X || g_App.camera->viewport.Y > bottom ||
        g_App.camera->viewport.X + g_App.camera->viewport.Width < minX ||
        g_App.camera->viewport.Y + g_App.camera->viewport.Height < minY)
    {
        return 0;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    if ((flags & 2) == 0)
    {
        diffuse = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(diffuse, this->modulateColor, 2);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 1);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 0);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 3);
        }
        g_DiffuseQuad[0].diffuse = diffuse;
        g_DiffuseQuad[1].diffuse = diffuse;
        g_DiffuseQuad[2].diffuse = diffuse;
        g_DiffuseQuad[3].diffuse = diffuse;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(g_DiffuseQuad);
    return 0;
}

int SpriteEngine::DrawQuadMirrored(Sprite *vm, int flags)
{
    float bottom;
    float minY;
    float minX;
    float maxX;
    D3DCOLOR diffuse;

    g_DiffuseQuad[0].pos.x += this->shakeX;
    g_DiffuseQuad[0].pos.y += this->shakeY;
    g_DiffuseQuad[1].pos.x += this->shakeX;
    g_DiffuseQuad[1].pos.y += this->shakeY;
    g_DiffuseQuad[2].pos.x += this->shakeX;
    g_DiffuseQuad[2].pos.y += this->shakeY;
    g_DiffuseQuad[3].pos.x += this->shakeX;
    g_DiffuseQuad[3].pos.y += this->shakeY;
    if (flags & 1)
    {
        __asm
        {
            fld dword ptr [g_DiffuseQuad + 0x00]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x1c]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x04]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x3c]
            frndint
            fsub g_Half
            fst dword ptr [g_DiffuseQuad + 0x3c]
            fstp dword ptr [g_DiffuseQuad + 0x58]
            fst dword ptr [g_DiffuseQuad + 0x04]
            fstp dword ptr [g_DiffuseQuad + 0x20]
            fst dword ptr [g_DiffuseQuad + 0x1c]
            fstp dword ptr [g_DiffuseQuad + 0x54]
            fst dword ptr [g_DiffuseQuad + 0x00]
            fstp dword ptr [g_DiffuseQuad + 0x38]
        }
    }
    g_DiffuseQuad[0].uv.x = g_DiffuseQuad[2].uv.x = vm->spriteData->uvEndX + vm->uvScrollX;
    g_DiffuseQuad[1].uv.x = g_DiffuseQuad[3].uv.x = vm->spriteData->uvStartX + vm->uvScrollX;
    g_DiffuseQuad[0].uv.y = g_DiffuseQuad[1].uv.y = vm->spriteData->uvStartY + vm->uvScrollY;
    g_DiffuseQuad[2].uv.y = g_DiffuseQuad[3].uv.y = vm->spriteData->uvEndY + vm->uvScrollY;
    maxX = g_DiffuseQuad[0].pos.x > g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    maxX = g_DiffuseQuad[2].pos.x > maxX ? g_DiffuseQuad[2].pos.x : maxX;
    maxX = g_DiffuseQuad[3].pos.x > maxX ? g_DiffuseQuad[3].pos.x : maxX;
    bottom = g_DiffuseQuad[0].pos.y > g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    bottom = g_DiffuseQuad[2].pos.y > bottom ? g_DiffuseQuad[2].pos.y : bottom;
    bottom = g_DiffuseQuad[3].pos.y > bottom ? g_DiffuseQuad[3].pos.y : bottom;
    minX = g_DiffuseQuad[0].pos.x < g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    minX = g_DiffuseQuad[2].pos.x < minX ? g_DiffuseQuad[2].pos.x : minX;
    minX = g_DiffuseQuad[3].pos.x < minX ? g_DiffuseQuad[3].pos.x : minX;
    minY = g_DiffuseQuad[0].pos.y < g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    minY = g_DiffuseQuad[2].pos.y < minY ? g_DiffuseQuad[2].pos.y : minY;
    minY = g_DiffuseQuad[3].pos.y < minY ? g_DiffuseQuad[3].pos.y : minY;
    if (maxX < g_App.camera->viewport.X || g_App.camera->viewport.Y > bottom ||
        g_App.camera->viewport.X + g_App.camera->viewport.Width < minX ||
        g_App.camera->viewport.Y + g_App.camera->viewport.Height < minY)
    {
        return 0;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    if ((flags & 2) == 0)
    {
        diffuse = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(diffuse, this->modulateColor, 2);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 1);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 0);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 3);
        }
        g_DiffuseQuad[0].diffuse = diffuse;
        g_DiffuseQuad[1].diffuse = diffuse;
        g_DiffuseQuad[2].diffuse = diffuse;
        g_DiffuseQuad[3].diffuse = diffuse;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(g_DiffuseQuad);
    return 0;
}

int SpriteEngine::DrawUnrotated(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleX / 2.0f;
    h = vm->spriteHeight * vm->scaleY / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 1);
}

int SpriteEngine::DrawUnrotatedMirrored(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleX / 2.0f;
    h = vm->spriteHeight * vm->scaleY / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuadMirrored(vm, 1);
}

int SpriteEngine::Draw(Sprite *vm)
{
    float x0;
    float cy;
    float halfH;
    float halfWidth;
    float cosine;
    float angle;
    float s;

    if (vm->rotation.z == 0.0f)
    {
        return this->DrawUnrotated(vm);
    }
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    angle = vm->rotation.z;
    __asm
    {
        fld angle
        fsincos
        fstp cosine
        fstp s
    }
    x0 = vm->pos.x;
    cy = vm->pos.y;
    halfWidth = vm->spriteWidth * vm->scaleX / 2.0f;
    halfH = vm->spriteHeight * vm->scaleY / 2.0f;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -halfWidth, halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, halfWidth, halfH, s, cosine, x0, cy);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    if ((vm->flags >> 11 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += halfWidth;
        g_DiffuseQuad[1].pos.x += halfWidth;
        g_DiffuseQuad[2].pos.x += halfWidth;
        g_DiffuseQuad[3].pos.x += halfWidth;
    }
    if ((vm->flags >> 11 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += halfH;
        g_DiffuseQuad[1].pos.y += halfH;
        g_DiffuseQuad[2].pos.y += halfH;
        g_DiffuseQuad[3].pos.y += halfH;
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawRotatedNoRound(Sprite *vm)
{
    float x0;
    float cy;
    float halfH;
    float halfWidth;
    float cosine;
    float angle;
    float s;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    angle = vm->rotation.z;
    if (angle != 0.0f)
    {
        __asm
        {
            fld angle
            fsincos
            fstp cosine
            fstp s
        }
        x0 = vm->pos.x;
        cy = vm->pos.y;
        halfWidth = vm->spriteWidth * vm->scaleX / 2.0f;
        halfH = vm->spriteHeight * vm->scaleY / 2.0f;
        this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -halfWidth, -halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[1].pos, halfWidth, -halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -halfWidth, halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[3].pos, halfWidth, halfH, s, cosine, x0, cy);
        g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
        if ((vm->flags >> 11 & 3) & 1)
        {
            g_DiffuseQuad[0].pos.x += halfWidth;
            g_DiffuseQuad[1].pos.x += halfWidth;
            g_DiffuseQuad[2].pos.x += halfWidth;
            g_DiffuseQuad[3].pos.x += halfWidth;
        }
        if ((vm->flags >> 11 & 3) & 2)
        {
            g_DiffuseQuad[0].pos.y += halfH;
            g_DiffuseQuad[1].pos.y += halfH;
            g_DiffuseQuad[2].pos.y += halfH;
            g_DiffuseQuad[3].pos.y += halfH;
        }
    }
    else
    {
        float w;
        float h;


        w = vm->spriteWidth * vm->scaleX / 2.0f;
        h = vm->spriteHeight * vm->scaleY / 2.0f;
        if (((vm->flags >> 11 & 3) & 1) == 0)
        {
            g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
            g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
        }
        else
        {
            g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
            g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x + w;
        }
        if (((vm->flags >> 11 & 3) & 2) == 0)
        {
            g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
            g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
        }
        else
        {
            g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
            g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y + h;
        }
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawNoRound(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleX / 2.0f;
    h = vm->spriteHeight * vm->scaleY / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawProjected(Sprite *vm)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->ProjectSprite(vm) != 0)
    {
        return -1;
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawProjected3D(Sprite *vm)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    this->ProjectSprite3D(vm);
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::Draw3D(Sprite *vm)
{
    D3DXMATRIX rotationMatrix;
    D3DXMATRIX worldMatrix;
    D3DMATRIX textureMatrix;
    DWORD anchor;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if ((vm->flags >> 16 & 1) == 0 && ((vm->flags >> 3 & 1) != 0 || (vm->flags >> 2 & 1) != 0))
    {
        vm->drawMatrix = vm->matrix;
        vm->drawMatrix._11 *= vm->scaleX;
        vm->drawMatrix._22 *= vm->scaleY;
        vm->flags &= ~8;
        if (vm->rotation.x != 0.0)
        {
            D3DXMatrixRotationX(&rotationMatrix, vm->rotation.x);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.y != 0.0)
        {
            D3DXMatrixRotationY(&rotationMatrix, vm->rotation.y);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.z != 0.0)
        {
            D3DXMatrixRotationZ(&rotationMatrix, vm->rotation.z);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        vm->flags &= ~4;
    }
    worldMatrix = vm->drawMatrix;
    anchor = vm->flags >> 11;
    if ((anchor & 1) == 0)
    {
        worldMatrix._41 = vm->pos.x;
    }
    else
    {
        worldMatrix._41 = Fabs(vm->spriteWidth * vm->scaleX / 2.0f) + vm->pos.x;
    }
    if ((anchor & 2) == 0)
    {
        worldMatrix._42 = vm->pos.y;
    }
    else
    {
        worldMatrix._42 = Fabs(vm->spriteHeight * vm->scaleY / 2.0f) + vm->pos.y;
    }
    worldMatrix._41 += this->shakeX;
    worldMatrix._42 += this->shakeY;
    this->SetRenderStateForSprite(vm);
    worldMatrix._43 = vm->pos.z;
    g_App.d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

    if (this->currentSpriteData != vm->spriteData || vm->uvScrollX != 0.0f || vm->uvScrollX != 0.0f)
    {
        this->currentSpriteData = vm->spriteData;
        textureMatrix = vm->sizeMatrix;
        textureMatrix._31 = vm->spriteData->uvStartX + vm->uvScrollX;
        textureMatrix._32 = vm->spriteData->uvStartY + vm->uvScrollY;
        g_App.d3dDevice->SetTransform(D3DTS_TEXTURE0, &textureMatrix);
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 2)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
        g_App.d3dDevice->SetStreamSource(0, this->vertexBuffer, sizeof(VertexTex1Xyz));
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        this->vertexFormat = 2;
    }
    g_App.d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    return 0;
}

int SpriteEngine::DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 3)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        this->vertexFormat = 3;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, sizeof(vertices[0]));
    return 0;
}

int SpriteEngine::DrawVertexQuad(Sprite *vm, VertexTex1DiffuseXyzrwh *quad)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(quad);
    return 0;
}

int SpriteEngine::DrawColoredStrip(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 4)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        this->vertexFormat = 4;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, sizeof(vertices[0]));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    return 0;
}

int SpriteEngine::DrawColoredFan(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 4)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        this->vertexFormat = 4;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, vertices, sizeof(vertices[0]));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    return 0;
}

int SpriteEngine::DrawVertexFan(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 3)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        this->vertexFormat = 3;
    }
    this->SetBlendModeForSprite(vm);
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, vertices, sizeof(vertices[0]));
    return 0;
}

void SpriteEngine::CreateQuad()
{
    VertexTex1Xyz *data;

    this->quad[0].pos.x = this->quad[2].pos.x = -128.0f;
    this->quad[1].pos.x = this->quad[3].pos.x = 128.0f;
    this->quad[0].pos.y = this->quad[1].pos.y = -128.0f;
    this->quad[2].pos.y = this->quad[3].pos.y = 128.0f;
    this->quad[0].pos.z = this->quad[1].pos.z = this->quad[2].pos.z = this->quad[3].pos.z = 0.0f;
    this->quad[0].uv.x = this->quad[2].uv.x = 0.0f;
    this->quad[1].uv.x = this->quad[3].uv.x = 1.0f;
    this->quad[0].uv.y = this->quad[1].uv.y = 0.0f;
    this->quad[2].uv.y = this->quad[3].uv.y = 1.0f;
    g_DiffuseQuadXyz[0].pos = this->quad[0].pos;
    g_DiffuseQuadXyz[1].pos = this->quad[1].pos;
    g_DiffuseQuadXyz[2].pos = this->quad[2].pos;
    g_DiffuseQuadXyz[3].pos = this->quad[3].pos;
    g_DiffuseQuadXyz[0].uv.x = this->quad[0].uv.x;
    g_DiffuseQuadXyz[0].uv.y = this->quad[0].uv.y;
    g_DiffuseQuadXyz[1].uv.x = this->quad[1].uv.x;
    g_DiffuseQuadXyz[1].uv.y = this->quad[1].uv.y;
    g_DiffuseQuadXyz[2].uv.x = this->quad[2].uv.x;
    g_DiffuseQuadXyz[2].uv.y = this->quad[2].uv.y;
    g_DiffuseQuadXyz[3].uv.x = this->quad[3].uv.x;
    g_DiffuseQuadXyz[3].uv.y = this->quad[3].uv.y;
    g_App.d3dDevice->CreateVertexBuffer(sizeof(this->quad), 0, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_MANAGED,
                                        &this->vertexBuffer);
    this->vertexBuffer->Lock(0, 0, (BYTE **)&data, 0);
    memcpy(data, this->quad, sizeof(this->quad));
    this->vertexBuffer->Unlock();
    g_App.d3dDevice->SetStreamSource(0, g_SpriteEngine->vertexBuffer, sizeof(VertexTex1Xyz));
}


int SpriteEngine::CreateTexture(AnmTexture *tex, int width, int height, int format)
{
    D3DXCreateTexture(g_App.d3dDevice, width, height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED, &tex->texture);
    return 0;
}


int SpriteEngine::PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file)
{
    void *data;
    int size;
    char *name;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Logger, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Logger, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name != '@')
        {
            data = LoadFileData(name, &size, 1);
            if (data == NULL)
            {
                LogError(&g_Logger, "テクスチャ %s が読み込めません。データが失われてるか壊れています\r\n", name);
                return -1;
            }
            anm->textures[index].size = size;
            anm->textures[index].data = data;
        }
    }
    return highest + 1;
}

void SpriteEngine::ReleaseTexture(AnmTexture *tex)
{
    if (tex->texture != NULL)
    {
        tex->texture->Release();
        tex->texture = NULL;
    }
    if (tex->data != NULL)
    {
        g_MemoryTracker.Free(tex->data);
    }
}

void AnmLoaded::LoadSprite(int index, SpriteData *sprite)
{
    this->sprites[index] = *sprite;
    this->sprites[index].uvStartX = this->sprites[index].startX / this->sprites[index].textureWidth;
    this->sprites[index].uvEndX = this->sprites[index].endX / this->sprites[index].textureWidth;
    this->sprites[index].uvStartY = this->sprites[index].startY / this->sprites[index].textureHeight;
    this->sprites[index].uvEndY = this->sprites[index].endY / this->sprites[index].textureHeight;
    this->sprites[index].width = (this->sprites[index].endX - this->sprites[index].startX) / sprite->scaleX;
    this->sprites[index].height = (this->sprites[index].endY - this->sprites[index].startY) / sprite->scaleY;
}

void SpriteEngine::RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused,
                                            int fontHeight, int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor,
                                            char *text, float scaleX, float scaleY)
{
    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }
    if (destHeight <= 0)
    {
        destHeight = 15;
    }
    if (fontHeight > 8)
    {
        RenderTextToTextureLarge(x, y, width, unused, fontHeight * scaleX, destHeight * scaleY, textColor,
                                 shadowColor, text, texture);
    }
    else
    {
        RenderTextToTexture(x, y, width, unused, 8, 8, textColor, shadowColor, text, texture);
    }
}

void SpriteEngine::RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    char buf[0x80];
    int fontW;

    fontW = sprite->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->RenderTextToTextureIndex(sprite->spriteData->texture, sprite->spriteData->startX,
                                   sprite->spriteData->startY, sprite->spriteData->textureWidth,
                                   sprite->spriteData->textureHeight, fontW, sprite->fontHeight, textColor,
                                   shadowColor, buf, sprite->spriteData->scaleX, sprite->spriteData->scaleY);
    sprite->flags |= 1;
}

void SpriteEngine::RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x44];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX / 2.0f -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 4.0f;
    this->RenderTextToTextureIndex(vm->spriteData->texture, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW,
                                   vm->fontHeight, color, shadowColor, buf, vm->spriteData->scaleX,
                                   vm->spriteData->scaleY);
    vm->flags |= 1;
}

void SpriteEngine::ReleaseSurface(int surface)
{
    if (this->surfaces[surface] != NULL)
    {
        this->surfaces[surface]->Release();
        this->surfaces[surface] = NULL;
    }
    if (this->surfacesBackup[surface] != NULL)
    {
        this->surfacesBackup[surface]->Release();
        this->surfacesBackup[surface] = NULL;
    }
    if (this->surfaceFiles[surface] != NULL)
    {
        g_MemoryTracker.Free(this->surfaceFiles[surface]);
    }
    this->surfaceFiles[surface] = NULL;
}

void SpriteEngine::CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = this->surfaceInfos[surface].Width;
    rect.bottom = this->surfaceInfos[surface].Height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    IDirect3DSurface8 *dest;
    RECT rect;
    RECT destinationRect;

    if (this->anms[texture].textures[0].texture == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->anms[texture].textures[0].texture->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        backBuffer->Release();
        return;
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (D3DXLoadSurfaceFromSurface(dest, NULL, &destinationRect, backBuffer, NULL, &rect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        backBuffer->Release();
        return;
    }
    dest->Release();
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    RECT destinationRect;

    this->FlushQuadBatch();
    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    this->surfaceInfos[surface].Width = dstWidth;
    this->surfaceInfos[surface].Height = dstHeight;
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto done;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto done;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, &destinationRect, backBuffer, NULL, &rect,
                                   D3DX_DEFAULT, 0) != D3D_OK)
    {
        goto done;
    }
    D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, this->surfaces[surface], NULL, NULL,
                               D3DX_DEFAULT, 0);

done:
    if (backBuffer != NULL)
    {
        backBuffer->Release();
        backBuffer = NULL;
    }
}



static int AdjustTextureFormat(int format)
{
    if (g_App.GetForce16BitTextures())
    {
        if (g_TextureFormats[format] == D3DFMT_A8R8G8B8 || g_TextureFormats[format] == D3DFMT_UNKNOWN)
        {
            format = 5;
        }
        else if (g_TextureFormats[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }
    return format;
}


int SpriteEngine::LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey)
{
    format = AdjustTextureFormat(format);
    if (D3DXCreateTextureFromFileInMemoryEx(g_App.d3dDevice, tex->data, tex->size, 0, 0, 0, 0, g_TextureFormats[format],
                                            D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &tex->texture) != D3D_OK)
    {
        return -1;
    }
    return 0;
}


int SpriteEngine::CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format)
{
    BYTE *dstRow;
    BYTE *srcRow;
    int y;
    D3DLOCKED_RECT rect;
    AnmImage *img;
    IDirect3DSurface8 *level;
    IDirect3DSurface8 *srcSurface;

    srcSurface = NULL;
    level = NULL;
    format = AdjustTextureFormat(format);
    img = image;
    g_App.d3dDevice->CreateImageSurface(img->width, img->height, g_TextureFormats[img->format], &srcSurface);
    srcSurface->LockRect(&rect, NULL, 0);
    for (y = 0; y < img->height; y++)
    {
        dstRow = (BYTE *)rect.pBits + y * rect.Pitch;
        srcRow = &image->data[y * img->width * g_TextureFormatSizes[img->format]];
        memcpy(dstRow, srcRow, img->width * g_TextureFormatSizes[img->format]);
    }
    srcSurface->UnlockRect();
    if (D3DXCreateTexture(g_App.d3dDevice, img->width, img->height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED,
                          &tex->texture) != D3D_OK)
    {
        goto fail;
    }
    tex->texture->GetSurfaceLevel(0, &level);
    if (D3DXLoadSurfaceFromSurface(level, NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_LINEAR, 0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return -1;
}


AnmLoaded *SpriteEngine::PreloadAnmFile(int slot, char *path)
{
    AnmFile *header;
    int numScripts;
    int result;
    AnmFile *data;
    AnmLoaded *anm;
    int anmCount;
    int spriteCount;
    int i;

    if (slot >= 0x14)
    {
        LogError(&g_Logger, "テクスチャ格納先が足りません\r\n");
        return NULL;
    }
    if (this->anms[slot].file != NULL)
    {
        this->anms[slot].releaseRequested = 1;
        while (this->anms[slot].releaseRequested != 0 && !g_App.closeSubThread)
        {
            Sleep(1);
        }
    }
    data = (AnmFile *)LoadFileData(path, NULL, 0);
    anmCount = 0;
    numScripts = 0;
    spriteCount = 0;
    i = 0;
    anm = &this->anms[slot];
    if (data == NULL)
    {
        return NULL;
    }
    anm->slot = slot;
    anm->file = data;
    strcpy(anm->path, path);
    header = data;
    while (1)
    {
        anmCount++;
        numScripts += header->numScripts;
        spriteCount += header->numSprites;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    anm->anmCount = anmCount;
    anm->textures = (AnmTexture *)g_MemoryTracker.Alloc(anmCount * sizeof(AnmTexture), GLOBAL_H);
    memset(anm->textures, 0, anmCount * sizeof(AnmTexture));
    anm->sprites = (SpriteData *)g_MemoryTracker.Alloc(spriteCount * sizeof(SpriteData), GLOBAL_H);
    anm->scripts = (void **)g_MemoryTracker.Alloc(numScripts * sizeof(void *), GLOBAL_H);
    header = data;
    anmCount = 0;
    spriteCount = 0;
    numScripts = 0;
    while (1)
    {
        result = this->PreloadAnmTexture(anm, i, &spriteCount, &numScripts, header);
        if (result < 0)
        {
            return NULL;
        }
        i++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    return anm;
}

AnmLoaded *SpriteEngine::PreloadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    anm = this->PreloadAnmFile(slot, path);
    if (anm == NULL)
    {
        return NULL;
    }
    anm->loadStep = 1;
    while (anm->loadStep != 0 && !g_App.closeSubThread)
    {
        Sleep(1);
    }
    return g_App.closeSubThread ? NULL : anm;
}


int SpriteEngine::LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file)
{
    SpriteData data;
    int *entries;
    int k;
    AnmSprite *rawSprite;
    char *name;
    D3DSURFACE_DESC desc;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Logger, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Logger, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name == '@')
        {
            this->CreateTexture(&anm->textures[index], anmHeader->width, anmHeader->height, anmHeader->format);
        }
        else if (this->LoadTexture(&anm->textures[index], anmHeader->format, anmHeader->colorKey) != 0)
        {
            LogError(&g_Logger, "テクスチャ %s が作成できません。データが失われてるか壊れています\r\n", name);
            return -1;
        }
    }
    else if (this->CreateTextureFromImage(&anm->textures[index],
                                          (AnmImage *)((BYTE *)anmHeader + anmHeader->imageOffset),
                                          anmHeader->format) != 0)
    {
        LogError(&g_Logger, "テクスチャが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anm->textures[index].texture->SetPriority(anmHeader->priority);
    anm->textures[index].texture->PreLoad();
    anm->textures[index].texture->GetLevelDesc(0, &desc);
    entries = anmHeader->spriteOffsets;
    for (k = 0; k < anmHeader->numSprites; k++, entries++)
    {
        rawSprite = (AnmSprite *)((char *)anmHeader + *entries);
        data.sourceFileIndex = anm->slot;
        data.texture = anm->textures[index].texture;
        data.scaleX = (float)desc.Width / anmHeader->width;
        data.scaleY = (float)desc.Height / anmHeader->height;
        data.startX = data.scaleX * rawSprite->x;
        data.startY = data.scaleY * rawSprite->y;
        data.endX = (rawSprite->x + rawSprite->width) * data.scaleX;
        data.endY = (rawSprite->y + rawSprite->height) * data.scaleY;
        data.textureWidth = desc.Width;
        data.textureHeight = desc.Height;
        anm->LoadSprite(spriteOffset, &data);
        spriteOffset++;
    }
    for (k = 0; k < anmHeader->numScripts; k++, entries += 2)
    {
        anm->scripts[scriptOffset] = (char *)anmHeader + entries[1];
        scriptOffset++;
    }
    return highest + 1;
}

void SpriteEngine::ReleaseAnm(int slot)
{
    int i;

    if (slot < 0 || slot >= ARRAY_SIZE(this->anms))
    {
        return;
    }
    if (this->anms[slot].file != NULL)
    {
        for (i = 0; i < this->anms[slot].anmCount; i++)
        {
            this->ReleaseTexture(&this->anms[slot].textures[i]);
        }
        g_MemoryTracker.Free(this->anms[slot].textures);
        g_MemoryTracker.Free(this->anms[slot].sprites);
        g_MemoryTracker.Free(this->anms[slot].scripts);
        g_MemoryTracker.Free(this->anms[slot].file);
        memset(&this->anms[slot], 0, sizeof(AnmLoaded));
    }
}


int SpriteEngine::LoadSurface(int surface, char *path)
{
    void *data;
    int size;
    IDirect3DSurface8 *srcSurface;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    if (this->surfaceFiles[surface] == NULL)
    {
        data = LoadFileData(path, &size, 0);
        if (data == NULL)
        {
            LogError(&g_Logger, "%sが読み込めないです。\r\n", path);
            return -1;
        }
    }
    else
    {
        data = this->surfaceFiles[surface];
        size = this->surfaceFileSizes[surface];
        this->surfaceFiles[surface] = NULL;
    }
    if (g_App.d3dDevice->CreateImageSurface(640, 1024, g_App.presentParams.BackBufferFormat, &srcSurface) != D3D_OK)
    {
        return -1;
    }
    if (D3DXLoadSurfaceFromFileInMemory(srcSurface, NULL, NULL, data, size, NULL, D3DX_FILTER_NONE, 0,
                                        &this->surfaceInfos[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryTracker.Free(data);
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryTracker.Free(data);
    return -1;
}

int SpriteEngine::PreloadSurface(int surface, char *path)
{
    void *data;
    int size;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    data = LoadFileData(path, &size, 0);
    if (data == NULL)
    {
        LogError(&g_Logger, "%sが読み込めないです。\r\n", path);
        return -1;
    }
    this->surfaceFiles[surface] = data;
    this->surfaceFileSizes[surface] = size;
    return 0;
}

AnmLoaded *SpriteEngine::PostloadAnm(AnmLoaded *anm)
{
    AnmFile *header;
    int scriptOffset;
    int result;
    AnmFile *data;
    int done;
    int spriteOffset;
    int index;

    data = anm->file;
    done = 0;
    scriptOffset = 0;
    spriteOffset = 0;
    index = 0;
    header = data;
    while (1)
    {
        if (done == anm->loadStep - 1)
        {
            result = this->LoadAnmSprites(anm, index, spriteOffset, scriptOffset, header);
            if (result < 0)
            {
                anm->loadStep = 0;
                return NULL;
            }
        }
        spriteOffset += header->numSprites;
        scriptOffset += header->numScripts;
        index++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
        done++;
        if (done == anm->loadStep)
        {
            anm->loadStep++;
            return anm;
        }
    }
    anm->loadStep = 0;
    return anm;
}

int SpriteEngine::StepAnmLoading()
{
    int i;

    for (i = 0; i < ARRAY_SIZE(this->anms); i++)
    {
        if (this->anms[i].releaseRequested)
        {
            this->ReleaseAnm(i);
            this->anms[i].releaseRequested = 0;
        }
        else if (this->anms[i].loadStep != 0)
        {
            if (this->PostloadAnm(&this->anms[i]) == NULL)
            {
                return -1;
            }
        }
    }
    return 0;
}

SpriteEngine::SpriteEngine()
{
    memset(this, 0, sizeof(*this));
    g_TexQuad[0].w = g_TexQuad[1].w = g_TexQuad[2].w = g_TexQuad[3].w = 1.0f;
    g_TexQuad[0].uv.x = 0.0f;
    g_TexQuad[0].uv.y = 0.0f;
    g_TexQuad[1].uv.x = 1.0f;
    g_TexQuad[1].uv.y = 0.0f;
    g_TexQuad[2].uv.x = 0.0f;
    g_TexQuad[2].uv.y = 1.0f;
    g_TexQuad[3].uv.x = 1.0f;
    g_TexQuad[3].uv.y = 1.0f;
    g_DiffuseQuad[0].w = g_DiffuseQuad[1].w = g_DiffuseQuad[2].w = g_DiffuseQuad[3].w = 1.0f;
    g_DiffuseQuad[0].uv.x = 0.0f;
    g_DiffuseQuad[0].uv.y = 0.0f;
    g_DiffuseQuad[1].uv.x = 1.0f;
    g_DiffuseQuad[1].uv.y = 0.0f;
    g_DiffuseQuad[2].uv.x = 0.0f;
    g_DiffuseQuad[2].uv.y = 1.0f;
    g_DiffuseQuad[3].uv.x = 1.0f;
    g_DiffuseQuad[3].uv.y = 1.0f;
    this->vertexBuffer = NULL;
    this->currentTexture = NULL;
    this->blendMode = 0;
    this->colorOp = 0;
    this->currentColor = 1;
    this->vertexFormat = 0;
    this->lastCameraSetupFlag = 0xff;
    this->zWriteDisabled = 0;
    this->pendingTexCopyTexture = -1;
    this->pendingSurfaceCopySurface = -1;
}

SpriteEngine::~SpriteEngine()
{
}

AnmLoaded *SpriteEngine::LoadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    anm = this->PreloadAnmFile(slot, path);
    if (anm != NULL)
    {
        anm->loadStep = 1;
        while (anm->loadStep != 0)
        {
            anm = this->PostloadAnm(anm);
        }
    }
    return anm;
}
