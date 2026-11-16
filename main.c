#include "raylib.h"
#include <math.h>

typedef enum GameState {
    STATE_PLAYING,
    STATE_ENTERING_CAVE,
    STATE_FADING_OUT,
    STATE_INSIDE_CAVE,
    STATE_FADING_IN
} GameState;

int main(void) {
    InitWindow(1, 1, "Purple Talisman");
    
    Texture2D map_img = LoadTexture("res/map.png");
    SetWindowSize(map_img.width, map_img.height);
    
    SetTargetFPS(60);

    Texture2D down_imgs[3] = { LoadTexture("res/down.png"), LoadTexture("res/down2.png"), LoadTexture("res/down3.png") };
    Texture2D up_imgs[3]   = { LoadTexture("res/up.png"), LoadTexture("res/up2.png"), LoadTexture("res/up3.png") };
    Texture2D left_imgs[2] = { LoadTexture("res/left.png"), LoadTexture("res/left2.png") };
    Texture2D right_imgs[2]= { LoadTexture("res/right.png"), LoadTexture("res/right2.png") };

    Vector2 position = { (float)map_img.width / 2.0f, (float)map_img.height / 2.0f };
    float scale = 3.0f;
    
    int current_frame = 0;
    float frame_timer = 0.0f;
    float frame_speed = 0.15f;
    int direction = 0; 

    Camera2D camera = { 0 };
    camera.target = position;
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.zoom = 1.0f;

    Vector2 cave_point = { 964.0f, 334.0f };
    
    GameState state = STATE_PLAYING;
    float fade_alpha = 0.0f;
    float enter_timer = 0.0f;
    
    bool can_enter_cave = true;
    float player_alpha = 1.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        bool is_moving = false;

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();

        if (state == STATE_PLAYING) {
            player_alpha = 1.0f;
            
            if (IsKeyDown(KEY_RIGHT)) { position.x += 150.0f * deltaTime; direction = 3; is_moving = true; }
            if (IsKeyDown(KEY_LEFT))  { position.x -= 150.0f * deltaTime; direction = 2; is_moving = true; }
            if (IsKeyDown(KEY_UP))    { position.y -= 150.0f * deltaTime; direction = 1; is_moving = true; }
            if (IsKeyDown(KEY_DOWN))  { position.y += 150.0f * deltaTime; direction = 0; is_moving = true; }

            float dist_x = fabsf(position.x - cave_point.x);
            float dist_y = fabsf(position.y - cave_point.y);
            
            if (dist_x < 15.0f && dist_y < 15.0f && can_enter_cave) {
                state = STATE_ENTERING_CAVE;
                direction = 1;
                enter_timer = 0.0f;
                can_enter_cave = false;
            }
            
            if (dist_x > 40.0f || dist_y > 40.0f) {
                can_enter_cave = true;
            }
        } 
        else if (state == STATE_ENTERING_CAVE) {
            enter_timer += deltaTime;
            direction = 1;
            
            if (position.y > cave_point.y - 15.0f) {
                position.y -= 20.0f * deltaTime;
            }

            if (enter_timer > 0.5f) {
                player_alpha -= deltaTime * 3.0f;
                if (player_alpha < 0.0f) player_alpha = 0.0f;
            }

            if (enter_timer > 1.0f) {
                state = STATE_FADING_OUT;
            }
        }
        else if (state == STATE_FADING_OUT) {
            fade_alpha += deltaTime * 2.0f;
            if (fade_alpha >= 1.0f) {
                fade_alpha = 1.0f;
                state = STATE_INSIDE_CAVE;
            }
        }
        else if (state == STATE_INSIDE_CAVE) {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
                fade_alpha = 1.0f;
                state = STATE_FADING_IN;
            }
        }
        else if (state == STATE_FADING_IN) {
            fade_alpha -= deltaTime * 2.0f;
            if (fade_alpha <= 0.0f) {
                fade_alpha = 0.0f;
                position = cave_point;
                direction = 0;
                player_alpha = 1.0f;
                state = STATE_PLAYING;
            }
        }

        if (state == STATE_PLAYING) {
            if (is_moving) {
                frame_timer += deltaTime;
                if (frame_timer >= frame_speed) {
                    frame_timer = 0.0f;
                    current_frame++;
                }
            } else {
                current_frame = 0;
            }
        }

        Texture2D current_texture = down_imgs[0];
        
        if (direction == 0) { current_texture = down_imgs[current_frame % 3]; }
        else if (direction == 1) { current_texture = up_imgs[current_frame % 3]; }
        else if (direction == 2) { current_texture = left_imgs[current_frame % 2]; }
        else if (direction == 3) { current_texture = right_imgs[current_frame % 2]; }

        float scale_x = (float)GetScreenWidth() / map_img.width;
        float scale_y = (float)GetScreenHeight() / map_img.height;
        camera.zoom = (scale_x > scale_y) ? scale_x : scale_y;

        camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        
        float half_width = (GetScreenWidth() / 2.0f) / camera.zoom;
        float half_height = (GetScreenHeight() / 2.0f) / camera.zoom;

        camera.target = position;

        if (camera.target.x - half_width < 0) camera.target.x = half_width;
        if (camera.target.x + half_width > map_img.width) camera.target.x = map_img.width - half_width;
        
        if (camera.target.y - half_height < 0) camera.target.y = half_height;
        if (camera.target.y + half_height > map_img.height) camera.target.y = map_img.height - half_height;

        BeginDrawing();
            ClearBackground(BLACK);
            
            BeginMode2D(camera);
                DrawTexture(map_img, 0, 0, WHITE);
                if (state != STATE_INSIDE_CAVE && player_alpha > 0.0f) {
                    DrawTextureEx(current_texture, position, 0.0f, scale, Fade(WHITE, player_alpha));
                }
            EndMode2D();
            
            if (fade_alpha > 0.0f) {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, fade_alpha));
            }

            DrawText(TextFormat("X: %.2f", position.x), 10, 10, 20, RED);
            DrawText(TextFormat("Y: %.2f", position.y), 10, 35, 20, RED);

            if (state == STATE_INSIDE_CAVE) {
                DrawText("You are inside the cave!", 10, 60, 20, GREEN);
                DrawText("Press SPACE to leave", 10, 85, 20, YELLOW);
            }
            
        EndDrawing();
    }

    for(int i=0; i<3; i++) { UnloadTexture(down_imgs[i]); UnloadTexture(up_imgs[i]); }
    for(int i=0; i<2; i++) { UnloadTexture(left_imgs[i]); UnloadTexture(right_imgs[i]); }
    
    UnloadTexture(map_img);

    CloseWindow();
    return 0;
}
