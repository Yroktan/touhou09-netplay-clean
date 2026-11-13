#include "stage.h"
#include "gamemanager.h"
#include "screeneffect.h"
#include "unsorted.h"










static float HermiteInterpolate(float p0, float p1, float m0, float m1, float t)
{
    float f00;
    float f10;
    float f01;
    float f11;

    f00 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    f01 = t * t * (3.0f - 2.0f * t);
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    return f00 * p0 + f01 * p1 + f10 * m0 + f11 * m1;
}






static void UpdateEasedVector(Stage *stage, D3DXVECTOR3 *out, int i, D3DXVECTOR3 *from, D3DXVECTOR3 *to,
                              D3DXVECTOR3 *m0, D3DXVECTOR3 *m1)
{
    float t;

    if (stage->cameraFadeTimer[i] < stage->cameraFadeFrames[i])
    {
        stage->cameraFadeTimer[i]++;
        t = stage->cameraFadeTimer[i].AsFloat() / stage->cameraFadeFrames[i];
    }
    else
    {
        stage->cameraFadeTimer[i] = stage->cameraFadeFrames[i];
        t = 1.0f;
        stage->cameraFadeFrames[i] = 0;
    }
    switch (stage->cameraFadeMode[i])
    {
    case 1:
        t = 1.0f - (t = 1.0f - t) * t;
        break;
    case 2:
        t = 1.0f - (t = 1.0f - t) * t * t;
        break;
    case 3:
        t = 1.0f - (t = 1.0f - t) * t * t * t;
        break;
    case 4:
        t = t * t;
        break;
    case 5:
        t = t * t * t;
        break;
    case 6:
        t = t * t * t * t;
        break;
    }
    if (stage->cameraFadeMode[i] != 7)
    {
        *out = *to - *from;
        *out = *out * t + *from;
    }
    else
    {
        out->x = HermiteInterpolate(from->x, to->x, m0->x, m1->x, t);
        out->y = HermiteInterpolate(from->y, to->y, m0->y, m1->y, t);
        out->z = HermiteInterpolate(from->z, to->z, m0->z, m1->z, t);
    }
}

void Stage::JumpToWaitLabel()
{
    StageInstr *cmd;
    int idx;

    if (this->waitLabel != 0)
    {
        idx = 0;
        cmd = this->script;
        this->scriptIndex = 0;
        while ((cmd->opcode != 0x1f || this->waitLabel != *(int *)&cmd->args.x) && cmd->frame != -1)
        {
            cmd++;
            idx++;
        }
        if (cmd->frame != -1)
        {
            this->scriptIndex = idx + 1;
            this->scriptTime = cmd->frame;
            this->waitLabel = 0;
        }
    }
}

void Stage::RunScript()
{
    StageInstr *cmd;
    D3DVECTOR tmpVec;
    int frame;

loop:
    cmd = &this->script[this->scriptIndex];
    frame = cmd->frame;
    if (this->scriptTime >= frame && frame != -1)
    {
        switch (cmd->opcode)
        {
        case 0:
            tmpVec = cmd->args;
            this->cameraPos.x = tmpVec.x;
            this->cameraPos.y = tmpVec.y;
            this->cameraPos.z = tmpVec.z;
            this->keyPos = tmpVec;
            this->keyFrame = cmd->frame;
            cmd++;
            this->nextKeyFrame = cmd->frame;
            this->nextKeyPos = cmd->args;
            break;
        case 1:
            this->fog.color = *(D3DCOLOR *)&cmd->args.x;
            this->fog.nearPlane = cmd->args.y;
            this->fog.farPlane = cmd->args.z;
            this->fogTarget = this->fog;
            break;
        case 2:
            this->fogStart = this->fog;
            this->fogFadeFrames = *(int *)&cmd->args.x;
            this->fogFadeTime = 0;
            break;
        case 5:
            if (this->cameraOffsetChanged)
            {
                this->cameraOffsetChanged = 0;
            }
            this->cameraTo.eye = this->cameraFrom.eye;
            this->cameraFrom.eye = cmd->args;
            if (this->cameraFadeFrames[0] == 0)
            {
                g_App.cameras[this->sideIdx].eye = cmd->args;
            }
            break;
        case 6:
            this->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[0] = 0;
            this->cameraFadeMode[0] = *(int *)&cmd->args.y;
            break;
        case 7:
            this->cameraTo.at = this->cameraFrom.at;
            this->cameraFrom.at = cmd->args;
            if (this->cameraFadeFrames[1] == 0)
            {
                g_App.cameras[this->sideIdx].at = cmd->args;
            }
            break;
        case 8:
            this->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[1] = 0;
            this->cameraFadeMode[1] = *(int *)&cmd->args.y;
            break;
        case 9:
            this->cameraTo.up = this->cameraFrom.up;
            this->cameraFrom.up = cmd->args;
            if (this->cameraFadeFrames[2] == 0)
            {
                g_App.cameras[this->sideIdx].up = cmd->args;
            }
            break;
        case 10:
            this->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            this->cameraFadeMode[2] = *(int *)&cmd->args.y;
            this->cameraFadeTimer[2] = 0;
            break;
        case 11:
            this->cameraTo.fov = this->cameraFrom.fov;
            this->cameraFrom.fov = cmd->args.x;
            if (this->cameraFadeFrames[3] == 0)
            {
                g_App.cameras[this->sideIdx].fov = cmd->args.x;
            }
            break;
        case 12:
            this->cameraFadeFrames[3] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[3] = 0;
            this->cameraFadeMode[3] = *(int *)&cmd->args.y;
            break;
        case 13:
            this->clearColor = *(D3DCOLOR *)&cmd->args.x;
            break;
        case 3:
            if (this->waitLabel != 0)
            {
                this->waitLabel = 0;
                break;
            }
            goto done;
        case 4:
            this->scriptIndex = *(int *)&cmd->args.x;
            this->scriptTime = *(int *)&cmd->args.y;
            this->cameraFadeFrames[0] = 0;
            this->cameraOffsetChanged = 1;
            goto done;
        case 14:
            this->cameraTo.eye = cmd->args;
            break;
        case 15:
            this->cameraFrom.eye = cmd->args;
            break;
        case 16:
            this->cameraFadeStart.eye = cmd->args;
            break;
        case 17:
            this->cameraFadeTarget.eye = cmd->args;
            break;
        case 18:
            this->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[0] = 0;
            this->cameraFadeMode[0] = 7;
            break;
        case 19:
            this->cameraTo.at = cmd->args;
            break;
        case 20:
            this->cameraFrom.at = cmd->args;
            break;
        case 21:
            this->cameraFadeStart.at = cmd->args;
            break;
        case 22:
            this->cameraFadeTarget.at = cmd->args;
            break;
        case 23:
            this->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[1] = 0;
            this->cameraFadeMode[1] = 7;
            break;
        case 24:
            this->cameraTo.up = cmd->args;
            break;
        case 25:
            this->cameraFrom.up = cmd->args;
            break;
        case 26:
            this->cameraFadeStart.up = cmd->args;
            break;
        case 27:
            this->cameraFadeTarget.up = cmd->args;
            break;
        case 28:
            this->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[2] = 0;
            this->cameraFadeMode[2] = 7;
            break;
        case 29:
            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer0, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 30:


            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer1, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 33:
            this->shakeMode = *(BYTE *)&cmd->args.x;
            this->cameraFadeFrames[4] = 0;
            this->cameraFadeTimer[4] = 0;
            this->cameraFadeMode[4] = 0;
            break;
        case 32:
            g_App.cameras[this->sideIdx].offset = cmd->args;
            break;
        case 34:
            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer2, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer2.spriteIndex = -1;
            }
            break;
        }
        this->scriptIndex++;
        goto loop;
    }
done:
    {
        int i;
        float delta;
        float t;

        int frames;

        i = 0;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].eye, i, (D3DXVECTOR3 *)&this->cameraTo.eye,
                              (D3DXVECTOR3 *)&this->cameraFrom.eye, (D3DXVECTOR3 *)&this->cameraFadeStart.eye,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.eye);
        }
        i = 1;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].at, i, (D3DXVECTOR3 *)&this->cameraTo.at,
                              (D3DXVECTOR3 *)&this->cameraFrom.at, (D3DXVECTOR3 *)&this->cameraFadeStart.at,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.at);
        }
        i = 2;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].up, i, (D3DXVECTOR3 *)&this->cameraTo.up,
                              (D3DXVECTOR3 *)&this->cameraFrom.up, (D3DXVECTOR3 *)&this->cameraFadeStart.up,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.up);
        }
        i = 3;
        frames = this->cameraFadeFrames[i];
        if (frames != 0)
        {
            if (this->cameraFadeTimer[i] < frames)
            {
                this->cameraFadeTimer[i]++;
                t = this->cameraFadeTimer[i].AsFloat() / this->cameraFadeFrames[i];
            }
            else
            {
                this->cameraFadeTimer[i] = frames;
                t = 1.0f;
                this->cameraFadeFrames[i] = 0;
            }
            switch (this->cameraFadeMode[i])
            {
            case 1:
                t = 1.0f - (t = 1.0f - t) * t;
                break;
            case 2:
                t = 1.0f - (t = 1.0f - t) * t * t;
                break;
            case 3:
                t = 1.0f - (t = 1.0f - t) * t * t * t;
                break;
            case 4:
                t = t * t;
                break;
            case 5:
                t = t * t * t;
                break;
            case 6:
                t = t * t * t * t;
                break;
            }
            g_App.cameras[this->sideIdx].fov =
                (delta = this->cameraFrom.fov - this->cameraTo.fov) * t + this->cameraTo.fov;
        }
    }
    D3DXVec3Normalize(&g_App.cameras[this->sideIdx].dir, &g_App.cameras[this->sideIdx].at);
    if (this->shakeMode)
    {
        switch (this->shakeMode)
        {
        case 1:
            g_App.cameras[this->sideIdx].offset.x =
                sinf(this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 480.0f) - 3.1415927f) * 40.0f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 480)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        case 2:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 480.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].offset.x = 70.0f * sinf(angle);
            g_App.cameras[this->sideIdx].up.x = -sinf(angle) * 0.1f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 480)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }

        case 4:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 2048.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].offset.x = sinf(angle) * 30.0f;
            g_App.cameras[this->sideIdx].offset.z = cosf(angle) * 30.0f;
            g_App.cameras[this->sideIdx].up.x = -sinf(angle) * 0.1f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 2048)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }
        case 3:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 4800.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].up.x = sinf(angle);
            g_App.cameras[this->sideIdx].up.z = cosf(angle);
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 4800)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }
        }
    }
    if (this->fogFadeFrames != 0)
    {
        float fogT;
        int j;

        this->fogFadeTime++;
        fogT = this->fogFadeTime.AsFloat() / this->fogFadeFrames;
        if (fogT >= 1.0f)
        {
            fogT = 1.0f;
        }
        for (j = 0; j < 4; j++)
        {
            ((BYTE *)&this->fog.color)[j] =
                ((float)((BYTE *)&this->fogTarget.color)[j] - (float)((BYTE *)&this->fogStart.color)[j]) * fogT +
                (float)((BYTE *)&this->fogStart.color)[j];
        }
        this->fog.nearPlane = (this->fogTarget.nearPlane - this->fogStart.nearPlane) * fogT + this->fogStart.nearPlane;
        this->fog.farPlane = (this->fogTarget.farPlane - this->fogStart.farPlane) * fogT + this->fogStart.farPlane;
        if (this->fogFadeTime >= this->fogFadeFrames)
        {
            this->fogFadeFrames = 0;
        }
    }
}





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

int Stage::OnUpdate(Stage *stage)
{
    int i;

    if (stage->tintFadeMode != 0)
    {
        if (--stage->tintFadeFrames <= 0)
        {
            stage->tintFadeMode = 0;
            stage->tintFadeFrames = 0;
        }
        else
        {
            switch (stage->tintFadeMode)
            {
            case 1:
                stage->tintColor.color = 0x80f00000;
                break;
            case 2:
                stage->tintColor.color = 0x80303040;
                break;
            }
        }
    }
    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    if (stage->side->flag0)
    {
        stage->tintColor.color = 0x40303030;
        return 1;
    }
    g_App.SetCamera(stage->sideIdx);
    if (stage->data == NULL)
    {
        return 1;
    }
    stage->JumpToWaitLabel();
    stage->RunScript();
    if (stage->script[stage->scriptIndex].opcode != 3)
    {
        stage->scriptTime++;
    }
    stage->StepObjects();
    if (stage->spellCardState >= 1)
    {
        if (stage->spellCardFrames == 60)
        {
            stage->spellCardState++;
        }
        stage->spellCardFrames++;
        for (i = 0; i < stage->dialogueSpriteCount; i++)
        {
            g_SpriteEngine->Step(&stage->dialogueSprites[i]);
        }
    }
    if (stage->backgroundLayer0.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer0);
    }
    if (stage->backgroundLayer1.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer1);
    }
    if (stage->backgroundLayer2.spriteIndex > 0)
    {
        g_SpriteEngine->Step(&stage->backgroundLayer2);
        stage->clearColor = stage->backgroundLayer2.color;
    }
    stage->unk_c68++;
    return 1;
}

int Stage::DeletedCallback(Stage *stage)
{
    if (FreesResources())
    {
        g_SpriteEngine->ReleaseAnm(4);
    }
    if (stage->sprites != NULL)
    {
        g_MemoryTracker.Free(stage->sprites);
        stage->sprites = NULL;
    }
    if (stage->data != NULL)
    {
        g_MemoryTracker.Free(stage->data);
        stage->data = NULL;
    }
    return 0;
}

void Stage::RemoveChain(Stage *stage)
{
    if (stage != NULL)
    {
        OnRemoved(stage);
        g_Chain.Remove(stage->calcElem);
        g_Chain.Remove(stage->drawElem);
        g_Chain.Remove(stage->drawElem2);
        delete stage;
    }
}

int Stage::Load(char *dataPath)
{
    int count;
    int i;
    StageObject *obj;
    StageQuad *quad;

    this->data = (StageData *)LoadFileData(dataPath, NULL, 0);
    if (this->data == NULL)
    {
        LogPrintf(&g_Logger, "ステージデータが見つかりません。データが壊れています\r\n");
        return -1;
    }
    this->objectCount = this->data->objectCount;
    this->quadCount = this->data->quadCount;
    this->instances = (StageInstance *)(this->data->instancesOffset + (int)this->data);
    this->script = (StageInstr *)(this->data->scriptOffset + (int)this->data);
    this->objects = (StageObject **)((char *)this->data + 0x490);
    for (i = 0; i < this->objectCount; i++)
    {
        this->objects[i] = (StageObject *)((int)this->objects[i] + (int)this->data);
    }
    this->sprites = (Sprite *)g_MemoryTracker.Alloc(this->quadCount * sizeof(Sprite), "bgscroll");
    for (i = 0, count = 0; i < this->objectCount; i++)
    {
        obj = this->objects[i];
        obj->flags = 1;
        quad = obj->quads;
        while (quad->type >= 0)
        {
            this->anm->StartScriptAtOrigin(&this->sprites[count], quad->script);
            quad->spriteIndex = count;
            count++;
            quad = (StageQuad *)((char *)quad + quad->size);
        }
    }
    return 0;
}

int Stage::DrawLayer(int layer)
{
    int fog;
    StageQuad *quad;
    float range;
    StageInstance *placement = this->instances;
    int objCount = 0;
    int any = 0;
    D3DXVECTOR3 centre;
    D3DXVECTOR3 rightValue;
    float dotVal;
    StageObject *objData;
    D3DXVECTOR3 diff2;
    Sprite *spr;
    D3DXVECTOR3 origin(0.0f, 0.0f, 0.0f);
    D3DCOLOR colVal;
    float w2;
    D3DXMATRIX mat;
    StageLineQuad *altQuad;

    fog = 0xff;
    g_App.ResetView3D();
    g_SpriteEngine->SetLastCameraSetupFlag(1);
    D3DXMatrixIdentity(&mat);
    rightValue.x = g_App.camera->viewMatrix._11;
    rightValue.y = g_App.camera->viewMatrix._12;
    rightValue.z = g_App.camera->viewMatrix._13;
    D3DXVec3Normalize(&rightValue, &rightValue);
    while (placement->id >= 0)
    {
        objData = this->objects[placement->id];
        if (objData->layer == layer)
        {
            quad = objData->quads;
            centre.x = objData->pos.x + placement->pos.x - this->cameraPos.x + objData->size.x / 2.0f;
            centre.y = objData->pos.y + placement->pos.y - this->cameraPos.y + objData->size.y / 2.0f;
            centre.z = objData->pos.z + placement->pos.z - this->cameraPos.z + objData->size.z / 2.0f;
            centre = centre - (g_App.cameras->eye + g_App.cameras->offset);
            if (D3DXVec3LengthSq(&centre) > this->drawDistanceSq)
            {
                goto next;
            }
            dotVal = D3DXVec3Dot(&centre, &g_App.cameras->dir);
            range = D3DXVec3Length(&objData->size) / 2.0f + 1280.0f;
            if (dotVal > range || dotVal < 80.0f)
            {
                goto next;
            }
            objData->flags |= 2;
            any = 1;
            while (quad->type >= 0)
            {
                spr = &this->sprites[quad->spriteIndex];
                switch (quad->type)
                {
                case 0:
                    spr->pos.x = spr->posOffset.x + quad->pos.x + placement->pos.x - this->cameraPos.x;
                    spr->pos.y = spr->posOffset.y + quad->pos.y + placement->pos.y - this->cameraPos.y;
                    spr->pos.z = spr->posOffset.z + quad->pos.z + placement->pos.z - this->cameraPos.z;


                    if (quad->width != 0.0f)
                    {
                        spr->scaleX = quad->width / spr->spriteData->width;
                    }
                    if (quad->height != 0.0f)
                    {
                        spr->scaleY = quad->height / spr->spriteData->height;
                    }
                    if ((spr->autoRotate & 0xf) == 2)
                    {
                        mat._41 = spr->pos[0];
                        mat._42 = spr->pos[1];
                        mat._43 = spr->pos[2];
                        D3DXVec3Project(&centre, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                        &g_App.camera->viewMatrix, &mat);
                        if (quad->width != 0.0f)
                        {
                            w2 = quad->width;
                        }
                        else
                        {
                            w2 = spr->spriteData->width;
                        }
                        mat._41 += rightValue.x * (w2 * spr->scaleX);
                        mat._42 += rightValue.y * (w2 * spr->scaleX);
                        mat._43 += rightValue.z * (w2 * spr->scaleX);
                        D3DXVec3Project(&diff2, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                        &g_App.camera->viewMatrix, &mat);
                        diff2 = diff2 - centre;
                        spr->scaleX = D3DXVec3Length(&diff2) / w2;
                        spr->scaleY = spr->scaleX;
                        if (w2 < 0.0f)
                        {
                            spr->scaleY = -spr->scaleY;
                        }
                        diff2 = spr->pos - (g_App.camera->eye + g_App.camera->offset);
                        w2 = D3DXVec3Length(&diff2);
                        colVal = spr->color;
                        if (this->fog.nearPlane < w2)
                        {
                            w2 = (this->fog.nearPlane - w2) / (this->fog.nearPlane - this->fog.farPlane);
                            if (w2 >= 1.0f)
                            {
                                goto nextQuad;
                            }
                            ((BYTE *)&spr->color)[0] = ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * w2);
                            ((BYTE *)&spr->color)[1] = ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * w2);
                            ((BYTE *)&spr->color)[2] = ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * w2);
                            ((BYTE *)&spr->color)[3] = ((BYTE *)&spr->color)[3] * (1.0f - w2);
                        }
                        spr->pos = centre;
                        if (spr->pos.z < 0.0f || spr->pos.z > 1.0f)
                        {
                        }
                        else
                        {
                            if (fog != 0)
                            {
                                if (!g_App.GetNoFog())
                                {
                                    g_App.DisableFog();
                                }
                                fog = 0;
                            }
                            g_SpriteEngine->DrawNoRound(spr);
                            g_SpriteEngine->ClearTexture();
                            if ((spr->autoRotate & 0xf0) == 0x10 && this->fog.nearPlane > w2 && this->recordEffectSpawnPos != 0)
                            {
                                this->effectSpawnPos[this->effectSpawnPosCount] = centre;
                                this->effectSpawnPos[this->effectSpawnPosCount].z = 0.0f;
                                this->effectSpawnPosCount++;
                            }
                        }
                        spr->color = colVal;
                    }
                    else
                    {
                        if (!g_App.GetNoFog() && fog != 1)
                        {
                            if (!g_App.GetNoFog())
                            {
                                g_App.EnableFog();
                            }
                            fog = 1;
                        }
                        g_SpriteEngine->Draw3D(spr);
                    }
                    break;
                case 1:
                {
                    altQuad = (StageLineQuad *)quad;
                    D3DXVECTOR3 begin;
                    D3DXVECTOR3 endPos;
                    VertexTex1DiffuseXyzrwh vertices[4];
                    float halfW;
                    float halfW2;
                    float dist;

                    begin.x = altQuad->start.x + placement->pos.x - this->cameraPos.x;
                    begin.y = altQuad->start.y + placement->pos.y - this->cameraPos.y;
                    begin.z = altQuad->start.z + placement->pos.z - this->cameraPos.z;
                    mat._41 = begin.x;
                    mat._42 = begin.y;
                    mat._43 = begin.z;
                    D3DXVec3Project(&centre, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                    &g_App.camera->viewMatrix, &mat);
                    if (altQuad->width != 0.0f)
                    {
                        dist = altQuad->width;
                    }
                    else
                    {
                        dist = spr->spriteData->width;
                    }
                    mat._41 += rightValue.x * dist;
                    mat._42 += rightValue.y * dist;
                    mat._43 += rightValue.z * dist;
                    D3DXVec3Project(&diff2, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                    &g_App.camera->viewMatrix, &mat);
                    diff2 = diff2 - centre;
                    halfW = D3DXVec3Length(&diff2) / 2.0f;
                    diff2 = begin - (g_App.camera->eye + g_App.camera->offset);
                    dist = D3DXVec3Length(&diff2);
                    if (this->fog.nearPlane < dist)
                    {
                        dist = (this->fog.nearPlane - dist) / (this->fog.nearPlane - this->fog.farPlane);
                        if (dist < 1.0f)
                        {
                            ((BYTE *)&vertices[0].diffuse)[0] = ((BYTE *)&vertices[1].diffuse)[0] =
                                ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[1] = ((BYTE *)&vertices[1].diffuse)[1] =
                                ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[2] = ((BYTE *)&vertices[1].diffuse)[2] =
                                ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * dist);
                            ((BYTE *)&vertices[0].diffuse)[3] = ((BYTE *)&vertices[1].diffuse)[3] =
                                ((BYTE *)&spr->color)[3] * (1.0f - dist);
                        }
                        else
                        {
                            ((BYTE *)&vertices[0].diffuse)[3] = ((BYTE *)&vertices[1].diffuse)[3] = 0;
                        }
                    }
                    else
                    {
                        vertices[0].diffuse = vertices[1].diffuse = spr->color;
                    }
                    begin.x = altQuad->end.x + placement->pos.x - this->cameraPos.x;
                    begin.y = altQuad->end.y + placement->pos.y - this->cameraPos.y;
                    begin.z = altQuad->end.z + placement->pos.z - this->cameraPos.z;
                    mat._41 = begin.x;
                    mat._42 = begin.y;
                    mat._43 = begin.z;
                    D3DXVec3Project(&endPos, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                    &g_App.camera->viewMatrix, &mat);
                    if (altQuad->width != 0.0f)
                    {
                        dist = altQuad->width;
                    }
                    else
                    {
                        dist = spr->spriteData->width;
                    }
                    mat._41 += rightValue.x * dist;
                    mat._42 += rightValue.y * dist;
                    mat._43 += rightValue.z * dist;
                    D3DXVec3Project(&diff2, &origin, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                                    &g_App.camera->viewMatrix, &mat);
                    diff2 = diff2 - endPos;
                    halfW2 = D3DXVec3Length(&diff2) / 2.0f;
                    diff2 = begin - (g_App.camera->eye + g_App.camera->offset);
                    dist = D3DXVec3Length(&diff2);
                    if (this->fog.nearPlane < dist)
                    {
                        dist = (this->fog.nearPlane - dist) / (this->fog.nearPlane - this->fog.farPlane);
                        if (dist < 1.0f)
                        {
                            ((BYTE *)&vertices[2].diffuse)[0] = ((BYTE *)&vertices[3].diffuse)[0] =
                                ((BYTE *)&spr->color)[0] - (BYTE)((((BYTE *)&spr->color)[0] - ((BYTE *)&this->fog.color)[0]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[1] = ((BYTE *)&vertices[3].diffuse)[1] =
                                ((BYTE *)&spr->color)[1] - (BYTE)((((BYTE *)&spr->color)[1] - ((BYTE *)&this->fog.color)[1]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[2] = ((BYTE *)&vertices[3].diffuse)[2] =
                                ((BYTE *)&spr->color)[2] - (BYTE)((((BYTE *)&spr->color)[2] - ((BYTE *)&this->fog.color)[2]) * dist);
                            ((BYTE *)&vertices[2].diffuse)[3] = ((BYTE *)&vertices[3].diffuse)[3] =
                                ((BYTE *)&spr->color)[3] * (1.0f - dist);
                        }
                        else
                        {
                            ((BYTE *)&vertices[2].diffuse)[3] = ((BYTE *)&vertices[3].diffuse)[3] = 0;
                        }
                    }
                    else
                    {
                        vertices[2].diffuse = vertices[3].diffuse = spr->color;
                    }
                    origin = endPos - centre;
                    dist = sqrtf(origin.x * origin.x + origin.y * origin.y);
                    if (dist < 0.00001f)
                    {
                        goto nextQuad;
                    }
                    origin /= dist;
                    if (centre.z < 0.0f || centre.z > 1.0f)
                    {
                        goto nextQuad;
                    }
                    if (endPos.z < 0.0f || endPos.z > 1.0f)
                    {
                        goto nextQuad;
                    }
                    vertices[0].pos.x = origin.y * halfW + centre.x;
                    vertices[0].pos.y = centre.y - origin.x * halfW;
                    vertices[0].pos.z = centre.z;
                    vertices[1].pos.x = centre.x - origin.y * halfW;
                    vertices[1].pos.y = origin.x * halfW + centre.y;
                    vertices[1].pos.z = centre.z;
                    vertices[2].pos.x = origin.y * halfW2 + endPos.x;
                    vertices[2].pos.y = endPos.y - origin.x * halfW2;
                    vertices[2].pos.z = endPos.z;
                    vertices[3].pos.x = endPos.x - origin.y * halfW2;
                    vertices[3].pos.y = origin.x * halfW2 + endPos.y;
                    vertices[3].pos.z = endPos.z;
                    vertices[0].uv.x = vertices[2].uv.x = spr->spriteData->uvStartX + spr->uvScrollX;
                    vertices[1].uv.x = vertices[3].uv.x = spr->spriteData->uvEndX + spr->uvScrollX;
                    vertices[0].uv.y = vertices[1].uv.y = spr->spriteData->uvStartY + spr->uvScrollY;
                    vertices[2].uv.y = vertices[3].uv.y = spr->spriteData->uvEndY + spr->uvScrollY;
                    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
                    if (fog != 0)
                    {
                        if (!g_App.GetNoFog())
                        {
                            g_App.DisableFog();
                        }
                        fog = 0;
                    }
                    g_SpriteEngine->DrawVertexQuad(spr, vertices);
                    break;
                }
                }
            nextQuad:
                quad = (StageQuad *)((char *)quad + quad->size);
            }
            objCount++;
        }
    next:
        placement++;
    }
    return 0;
}


void Stage::Reset()
{
    this->spellCardState = 0;
}

int Stage::StepObjects()
{
    int busy;
    int i;
    Sprite *sprite;
    StageObject *obj;
    StageQuad *quad;

    for (i = 0; i < this->objectCount; i++)
    {
        obj = this->objects[i];
        if (obj->flags & 1)
        {
            busy = 0;
            quad = obj->quads;
            while (quad->type >= 0)
            {
                sprite = &this->sprites[quad->spriteIndex];
                switch (quad->type)
                {
                case 0:
                    g_SpriteEngine->Step(sprite);
                    break;
                case 1:
                    g_SpriteEngine->Step(sprite);
                    break;
                }
                if (sprite->scriptPos != NULL)
                {
                    busy++;
                }
                quad = (StageQuad *)((char *)quad + quad->size);
            }
            if (busy == 0)
            {
                obj->flags &= ~1;
            }
        }
    }
    return 0;
}

int Stage::OnDraw(Stage *stage)
{
    D3DCOLOR color;

    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_SpriteEngine->FlushQuadBatch();
    g_SpriteEngine->ClearTexture();
    g_App.SetView(stage->sideIdx);
    if (stage->clearScreenOnce != 0)
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
        stage->clearScreenOnce = 0;
    }
    if (stage->tintColor.a > 0)
    {
        g_SpriteEngine->SetColorModulation(stage->tintColor.color);
    }
    if ((stage->clearColor & 0xff000000) == 0xff000000)
    {
        color = stage->fog.color;
        if (g_SpriteEngine->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 2);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 1);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 0);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 3);
        }
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    }
    else if (stage->clearColor != 0)
    {
        float area[4];

        area[0] = 32.0f;
        area[1] = 16.0f;
        area[2] = 416.0f;
        area[3] = 464.0f;
        DrawColorRect(area, stage->fog.color);
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    else
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    if (stage->spellCardState <= 1)
    {
        if (stage->backgroundLayer0.spriteIndex > 0)
        {
            stage->backgroundLayer0.pos.x = g_GameManager.ArenaToScreenX(-144.0f);
            stage->backgroundLayer0.pos.z = 0.99f;
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer0);
        }
        if (stage->backgroundLayer1.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer1);
        }
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (g_SpriteEngine->modulateColorEnabled == 0)
    {
        g_App.SetRenderState(D3DRS_FOGCOLOR, stage->fog.color);
    }
    else
    {
        color = stage->fog.color;
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 2);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 1);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 0);
        g_App.SetRenderState(D3DRS_FOGCOLOR, color);
    }
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->fog.nearPlane);
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->fog.farPlane);
    if (!g_App.GetNoFog())
    {
        g_App.EnableFog();
    }
    if (stage->spellCardState <= 1)
    {
        stage->DrawLayer(0);
        stage->DrawLayer(1);
    }
    if (stage->keepColorModulation == 0)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    return 1;
}

int Stage::OnDraw2(Stage *stage)
{
    float fog;
    int i;

    g_App.SetView(stage->sideIdx);
    if (stage->spellCardState <= 1)
    {
        stage->DrawLayer(2);
        stage->DrawLayer(3);
        if (!g_App.GetNoFog())
        {
            g_App.DisableFog();
        }
    }
    if (stage->tintColor.a > 0)
    {
        g_SpriteEngine->SetColorModulation(stage->tintColor.color);
    }
    stage->tintColor.a = 0;
    stage->tintColor.r = 0x80;
    stage->tintColor.g = 0x80;
    stage->tintColor.b = 0x80;
    g_SpriteEngine->FlushQuadBatch();
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    if (stage->spellCardState >= 1)
    {
        for (i = 0; i < stage->dialogueSpriteCount; i++)
        {
            stage->dialogueSprites[i].pos.x = g_GameManager.ArenaToScreenX(stage->dialogueSprites[i].posOffset.x);
            stage->dialogueSprites[i].pos.y = g_GameManager.ArenaToScreenY(stage->dialogueSprites[i].posOffset.y);
            stage->dialogueSprites[i].pos.z = stage->dialogueSprites[i].posOffset.z;
            g_SpriteEngine->DrawAndFlush(&stage->dialogueSprites[i]);
        }
        if (stage->spellCardDrawCallback != NULL)
        {
            stage->spellCardDrawCallback();
        }
    }
    g_SpriteEngine->SetLastCameraSetupFlag(0);
    g_App.ResetView();
    fog = 1000.0f;
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog);
    fog = 2000.0f;
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog);
    if (stage->keepColorModulation == 0)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    stage->keepColorModulation = 0;
    stage->recordEffectSpawnPos = 0;
    return 1;
}

int Stage::AddedCallback(Stage *stage)
{
    int i;
    AnmLoaded *anm;

    stage->scriptTime = 0;
    stage->scriptIndex = 0;
    stage->cameraPos.x = 0.0f;
    stage->cameraPos.y = 0.0f;
    stage->cameraPos.z = 0.0f;
    stage->fogFadeFrames = 0;
    stage->dialogueScriptOffset = 0;
    stage->dialogueSpriteCount = 2;
    anm = g_SpriteEngine->GetAnm(4);
    if (anm->file != NULL && strcmp(anm->path, g_StageFiles[g_GameManager.stage][0]) == 0)
    {
        stage->anm = anm;
    }
    else
    {
        stage->anm = g_SpriteEngine->PreloadAnm(4, g_StageFiles[g_GameManager.stage][0]);
        if (stage->anm == NULL)
        {
            return -1;
        }
    }
    stage->playerAnm = g_SpriteEngine->GetAnm(stage->sideIdx + 5);
    if (stage->Load(g_StageFiles[g_GameManager.stage][1]) != 0)
    {
        return -1;
    }
    stage->fog.color = 0xff000000;
    stage->fog.nearPlane = 200.0f;
    stage->fog.farPlane = 500.0f;
    stage->shakeMode = 0;
    for (i = 0; i < 4; i++)
    {
        stage->cameraFadeFrames[i] = 0;
        stage->cameraFadeTimer[i] = 0;
    }
    stage->waitLabel = 0;
    stage->drawDistanceSq = 1822500.0f;
    if (g_GameManager.stage == 5)
    {
        stage->drawDistanceSq = 3062500.0f;
    }
    else if (g_GameManager.stage == 9)
    {
        stage->drawDistanceSq = 3062500.0f;
    }
    else if (g_GameManager.stage == 11)
    {
        stage->drawDistanceSq = 4202500.0f;
    }
    else if (g_GameManager.stage == 12 || g_GameManager.stage == 15)
    {
        stage->drawDistanceSq = 6002500.0f;
    }
    stage->Reset();
    return 0;
}

Stage::Stage()
{
    memset(this, 0, sizeof(Stage));
}

Stage *Stage::RegisterChain(int sideIdx)
{
    Stage *stage = (Stage *)g_MemoryTracker.Register(new Stage(), sizeof(Stage), "BGScrollInf");

    stage->unk_c68 = 0;
    stage->sideIdx = sideIdx;
    stage->side = &g_GameManager.sides[sideIdx];
    stage->otherSide = &g_GameManager.sides[1 - sideIdx];
    stage->calcElem = g_Chain.CreateElem((ChainCallback)Stage::OnUpdate);
    stage->calcElem->arg = stage;
    stage->calcElem->addedCallback = (ChainCallback)Stage::AddedCallback;
    stage->calcElem->deletedCallback = (ChainCallback)Stage::DeletedCallback;
    if (g_Chain.AddToCalcChain(stage->calcElem, sideIdx + 8) != 0)
    {
        return NULL;
    }
    stage->drawElem = g_Chain.CreateElem((ChainCallback)Stage::OnDraw);
    stage->drawElem2 = g_Chain.CreateElem((ChainCallback)Stage::OnDraw2);
    stage->drawElem->arg = stage;
    stage->drawElem2->arg = stage;
    g_Chain.AddToDrawChain(stage->drawElem, sideIdx + 6);
    g_Chain.AddToDrawChain(stage->drawElem2, sideIdx + 8);
    return stage;
}
