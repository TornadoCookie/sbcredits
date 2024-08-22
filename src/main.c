#include <raylib.h>
#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <raymath.h>

#define PROGRAM_VERSION "v0.0.0"

#define TEXT_COLOR (Color){221, 84, 112, 255}

typedef struct KeyValueList {
    int count;
    char **keys;
    cJSON **values;
} KeyValueList;

KeyValueList getKeyValueList(cJSON *json)
{
    json = json->child;
    KeyValueList list = { 0 };
    while (json)
    {
        list.count++;
        list.keys = realloc(list.keys, list.count * sizeof(char *));
        list.values = realloc(list.values, list.count * sizeof(cJSON *));
        list.keys[list.count - 1] = json->string;
        list.values[list.count - 1] = json;
        json = json->next;
    }

    return list;
}

#define NAME_TEXT_SIZE 60
#define TITLE_TEXT_SIZE 40

void DrawOutlinedTextEx(Font font, const char *text, Vector2 pos, float fontSize, float spacing, Color tint, int outlineSize, Color outlineColor) {
    DrawTextEx(font, text, (Vector2){pos.x - outlineSize, pos.y - outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, (Vector2){pos.x + outlineSize, pos.y - outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, (Vector2){pos.x - outlineSize, pos.y + outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, (Vector2){pos.x + outlineSize, pos.y + outlineSize}, fontSize, spacing, outlineColor);
    DrawTextEx(font, text, (Vector2){pos.x, pos.y}, fontSize, spacing, tint);
}

void DrawCreditText(Font font, const char *text, Vector2 pos, float size, float spacing, Color tint)
{
    DrawOutlinedTextEx(font, text, pos, size, spacing, tint, 1, BLACK);
}

void DrawCreditName(Font font, const char *name, int y_val, Color tint)
{
    DrawCreditText(font, name, (Vector2){GetScreenWidth()/2 - MeasureTextEx(font, name, NAME_TEXT_SIZE, 1).x / 2, y_val+TITLE_TEXT_SIZE}, NAME_TEXT_SIZE, 1, tint);
}

void DrawCreditTitle(Font font, const char *title, int y_val, Color tint)
{
    DrawCreditText(font, title, (Vector2){GetScreenWidth()/2 - MeasureTextEx(font, title, TITLE_TEXT_SIZE, 1).x / 2, y_val}, TITLE_TEXT_SIZE, 1, tint);
}

void DrawCreditSingle(Font font, const char *title, const char *name, int y_val, Color tint)
{
    DrawCreditTitle(font, title, y_val, tint);
    DrawCreditName(font, name, y_val+MeasureTextEx(font, title, TITLE_TEXT_SIZE, 1).y-TITLE_TEXT_SIZE, tint);
}

void DrawCastCredit(Font font, const char *title, Color tint, cJSON *json)
{
    DrawCreditTitle(font, title, 80, tint);

    KeyValueList list = getKeyValueList(json);

    for (int i = 0; i < list.count; i++)
    {
        DrawCreditText(font, cJSON_GetStringValue(list.values[i]), (Vector2){40, 120+50*i}, TITLE_TEXT_SIZE, 1, tint);
        for (int px = 40 + MeasureTextEx(font, cJSON_GetStringValue(list.values[i]), TITLE_TEXT_SIZE, 1).x; px < GetScreenWidth()-40-MeasureTextEx(font, list.keys[i], 30, 1).x; px += MeasureTextEx(font, ". ", 40, 1).x)
        {
            DrawCreditText(font, ". ", (Vector2){px, 120+50*i}, 40, 1, TEXT_COLOR);
        }
        DrawCreditText(font, list.keys[i], (Vector2){GetScreenWidth()-40-MeasureTextEx(font, list.keys[i], 30, 1).x, 125+50*i}, 30, 1, tint);
    }

    free(list.keys);
    free(list.values);
}

void DrawCreditMulti(Font font, const char *title, cJSON *names, int y_val, Color tint)
{
    DrawCreditTitle(font, title, y_val, tint);
    for (int i = 0; i < cJSON_GetArraySize(names); i++)
    {
        DrawCreditName(font, cJSON_GetStringValue(cJSON_GetArrayItem(names, i)), y_val+NAME_TEXT_SIZE*i, tint);
    }
}

#define MULTI_HORIZ_X_PADDING 20
void DrawCreditMultiHoriz(Font font, const char *title, cJSON *names, int y_val, Color tint)
{
    DrawCreditTitle(font, title, y_val, tint);
    int totalSize = 0;
    for (int i = 0; i < cJSON_GetArraySize(names); i++)
    {
        totalSize += MeasureTextEx(font, cJSON_GetStringValue(cJSON_GetArrayItem(names, i)), NAME_TEXT_SIZE, 1).x;
        totalSize += MULTI_HORIZ_X_PADDING;
    }
    int sizeOffset = 0;
    for (int i = 0; i < cJSON_GetArraySize(names); i++)
    {
        DrawCreditText(font, cJSON_GetStringValue(cJSON_GetArrayItem(names, i)), (Vector2){GetScreenWidth()/2 - totalSize / 2 + sizeOffset, y_val + TITLE_TEXT_SIZE}, NAME_TEXT_SIZE, 1, tint);
        sizeOffset += MeasureTextEx(font, cJSON_GetStringValue(cJSON_GetArrayItem(names, i)), NAME_TEXT_SIZE, 1).x;
        sizeOffset += MULTI_HORIZ_X_PADDING;
    }
}

#define CREDIT_Y_PADDING 20

int main(int argc, char **argv)
{
    printf("Spongebob Credits Generator (sbcredits) " PROGRAM_VERSION "\n");
    if (argc != 2)
    {
        printf("Required argument <*.json> not there..\n");
        return 1;
    }

    const char *jsonText = LoadFileText(argv[1]);
    cJSON *json = cJSON_Parse(jsonText);

    cJSON *settings = cJSON_GetArrayItem(json, 0);
    cJSON *widescreenSetting = cJSON_GetObjectItem(settings, "widescreen");
    bool widescreen = false;

    if (widescreenSetting) widescreen = widescreenSetting->type - 1;

    if (widescreen)
    {
        printf("Widescreen unsupported at this time (%#x).\n", widescreenSetting->type);
        return 1;
    }

    InitWindow(640, 480, "SBCredits " PROGRAM_VERSION);
    SetTargetFPS(60);
    InitAudioDevice();

    Music music = LoadMusicStream("assets/music.mp3");
    Font font = LoadFont("assets/font.otf");
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    Texture2D background = LoadTexture("assets/sd.png");
    Texture2D createdbysteve = LoadTexture("assets/createdbysteve.png");

    int i = 1;
    int iFrame = 0;
    const int iFrameMax = 80;
    cJSON *currentFrame = NULL;

    PlayMusicStream(music);
    
    while (!WindowShouldClose())
    {
        // Update
        iFrame++;
        if (iFrame > iFrameMax)
        {
            i++;
            if (i == cJSON_GetArraySize(json)) break;
            iFrame = 0;
            currentFrame = NULL;
        }
        if (currentFrame == NULL)
        {
            currentFrame = cJSON_GetArrayItem(json, i);
            printf("On frame %d\n", i);
        }
        UpdateMusicStream(music);

        // Draw
        BeginDrawing();
        DrawTexture(background, 0, 0, WHITE);

        KeyValueList list = getKeyValueList(currentFrame);
        SetTextLineSpacing(TITLE_TEXT_SIZE);

        int creditYSize = 0;
        for (int j = 0; j < list.count; j++)
        {
            switch (list.values[j]->type)
            {
                case cJSON_String:
                {
                    creditYSize += MeasureTextEx(font, list.keys[j], TITLE_TEXT_SIZE, 1).y + NAME_TEXT_SIZE;
                } break;
                case cJSON_Object:
                {
                    if (cJSON_HasObjectItem(list.values[j], "horiz"))
                    {
                        creditYSize += TITLE_TEXT_SIZE + NAME_TEXT_SIZE;
                    }
                } break;
                case cJSON_Array:
                {
                    creditYSize += TITLE_TEXT_SIZE + NAME_TEXT_SIZE*cJSON_GetArraySize(list.values[j]);
                } break;
                default:
                {
                    printf("Can't handle name type: %#x\n", list.values[j]->type);
                } break;
            }
            creditYSize += CREDIT_Y_PADDING;
        }


        int creditOffset = GetScreenHeight() / 2 - creditYSize / 2;
        for (int j = 0; j < list.count; j++)
        {
            switch (list.values[j]->type)
            {
                case cJSON_String:
                {
                    DrawCreditSingle(font, list.keys[j], cJSON_GetStringValue(list.values[j]), creditOffset, TEXT_COLOR);
                    creditOffset += MeasureTextEx(font, list.keys[j], TITLE_TEXT_SIZE, 1).y + NAME_TEXT_SIZE;
                } break;
                case cJSON_Object:
                {
                    if (cJSON_HasObjectItem(list.values[j], "horiz"))
                    {
                        DrawCreditMultiHoriz(font, list.keys[j], cJSON_GetObjectItem(list.values[j], "data"), creditOffset, TEXT_COLOR);
                        creditOffset += TITLE_TEXT_SIZE + NAME_TEXT_SIZE;
                    }
                    else if (TextIsEqual(list.keys[j], "CAST"))
                    {
                        DrawCastCredit(font, list.keys[j], TEXT_COLOR, list.values[j]);
                    }
                    else
                    {
                        printf("Can't handle frame %d item %d.\n", i, j);
                    }
                } break;
                case cJSON_Array:
                {
                    DrawCreditMulti(font, list.keys[j], list.values[j], creditOffset, TEXT_COLOR);
                    creditOffset += TITLE_TEXT_SIZE + NAME_TEXT_SIZE*cJSON_GetArraySize(list.values[j]);
                } break;
                default:
                {
                    printf("Can't handle name type: %#x\n", list.values[j]->type);
                } break;
            }
            creditOffset += CREDIT_Y_PADDING;
        }

        free(list.keys);
        free(list.values);

        EndDrawing();
    }

    int noPlayingCount = 0;
    const int noPlayingCountMax = 60;

    music.looping = false;

    int fade = 0;

    while (!WindowShouldClose())
    {
        if (fade < 255) {fade+=2; if (fade > 255) fade = 255;}
        if (!IsMusicStreamPlaying(music))
        {
            noPlayingCount++;
            if (noPlayingCount > noPlayingCountMax) break;
        }

        UpdateMusicStream(music);

        BeginDrawing();

        DrawTexture(background, 0, 0, WHITE);
        DrawTexturePro(createdbysteve, (Rectangle){0, 0, createdbysteve.width, createdbysteve.height}, (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, (Vector2){0, 0}, 0, (Color){255, 255, 255, fade});

        EndDrawing();
    }

    return 0;
}
