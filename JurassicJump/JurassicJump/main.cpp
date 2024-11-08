#include <sl.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>

const int windowWidth = 1920;
const int windowHeight = 1030;
const int groundHeight = 100;
const int cactusWidth = 85;
const int cactusHeight = 140;
const int powerUpWidth = 20;
const int powerUpHeight = 20;
const float powerUpDuration = 10.0f;
const float powerUpSpawnInterval = 10.0f;
const float powerUpRespawnInterval = 20.0f;
const int pterodactylWidth = 100;
const int pterodactylHeight = 85;

float lastCactusX = -200.0f; // Position of the last cactus
float lastCactusTime = 0.0f; // Time when the last cactus was spawned
float lastPowerUpX = -200.0f; // Position of the last power-up
float lastPowerUpTime = 0.0f; // Time when the last power-up was spawned
const float minDistance = 150.0f; // Minimum distance between cactus and power-up
const float minTimeGap = 1.0f; // Minimum time gap between cactus and power-up spawn

struct Cactus {
    float x, y;
    bool passed; // Member to track if the T-Rex has passed this cactus

    // Constructor
    Cactus(float _x, float _y)
        : x(_x), y(_y), passed(false) {}
};

struct PowerUp {
    float x, y;
    bool active;
};

struct Pterodactyl {
    float x, y;
    bool passed; // Member to track if the T-Rex has passed this pterodactyl

    // Constructor
    Pterodactyl(float _x, float _y)
        : x(_x), y(_y), passed(false) {}
};

enum GameState {
    START,
    PLAYING,
    GAME_OVER
};

int trexTexture1;
int trexTexture2;
int backgroundTexture;
int powerUpSound; // Identifier for power up sound
int hitHurtSound; // Identifier for hit/hurt sound
int jumpSound; // Identifier for jump sound
int treeTexture; // Identifier for tree texture
int pterodactylTexture1;
int pterodactylTexture2;// Identifier for pterodactyl texture
int gameFont; // Identifier for the loaded font
int dinoMenu;
int backgroundMusic; //Background Music


int frameCounter = 0;
int blinkCounter = 0; // Counter for blinking animation
int score = 0; // Score variable
const int frameSwitchInterval = 10; // Change every 10 frames

bool isDescending = false;

void drawTrex(float x, float y, bool isDescending, bool powerUpActive) {
    slPush();

    // Blinking effect if powerUpActive
    if (powerUpActive && (blinkCounter / 5 % 2 == 0)) {
        slSetForeColor(1.0, 1.0, 1.0, 0.0); // Make T-Rex invisible
    }
    else {
        slSetForeColor(1.0, 1.0, 1.0, 1.0); // Ensure full alpha for no color change
    }

    slSetSpriteTiling(1.0, 1.0);
    slSetSpriteScroll(0.0, 0.0);

    if (isDescending) {
        slSprite(trexTexture2, x, y, 130, 130); // Use descending texture
    }
    else if (frameCounter / frameSwitchInterval % 2 == 0) {
        slSprite(trexTexture1, x, y, 130, 130);
    }
    else {
        slSprite(trexTexture2, x, y, 130, 130);
    }

    slPop();
}

void drawCactus(const Cactus& cactus) {
    slPush();
    slSetSpriteTiling(1.0, 1.0);
    slSetSpriteScroll(0.0, 0.0);
    slSetForeColor(1.0, 1.0, 1.0, 1.0); // Ensure full alpha for no color change
    slSprite(treeTexture, cactus.x, cactus.y, cactusWidth, cactusHeight);
    slPop();
}

void drawPowerUp(const PowerUp& powerUp) {
    if (powerUp.active) {
        slPush();
        slSetForeColor(1.0, 0.0, 0.0, 1.0);
        slRectangleFill(powerUp.x, powerUp.y, powerUpWidth, powerUpHeight);
        slPop();
    }
}

void drawPterodactyl(const Pterodactyl& pterodactyl) {
    slPush();
    slSetForeColor(1.0, 1.0, 1.0, 1.0); // Ensure full alpha for no color change
    slSetSpriteTiling(1.0, 1.0);
    slSetSpriteScroll(0.0, 0.0);
    if (frameCounter / frameSwitchInterval % 2 == 0) {
        slSprite(pterodactylTexture1, pterodactyl.x, pterodactyl.y, pterodactylWidth, pterodactylHeight);
    }
    else {
        slSprite(pterodactylTexture2, pterodactyl.x, pterodactyl.y, pterodactylWidth, pterodactylHeight);
    }
    slPop();
}

bool checkCollision(float trexX, float trexY, const Cactus& cactus) {
    return trexX < cactus.x + cactusWidth &&
        trexX + 50 > cactus.x &&
        trexY < cactus.y + cactusHeight &&
        trexY + 50 > cactus.y;
}

bool checkCollision(float trexX, float trexY, const Pterodactyl& pterodactyl) {
    return trexX < pterodactyl.x + pterodactylWidth &&
        trexX + 80 > pterodactyl.x &&
        trexY < pterodactyl.y + pterodactylHeight &&
        trexY + 80 > pterodactyl.y;
}

bool checkPowerUpCollision(float trexX, float trexY, const PowerUp& powerUp) {
    return trexX < powerUp.x + powerUpWidth &&
        trexX + 90 > powerUp.x &&
        trexY < powerUp.y + powerUpHeight &&
        trexY + 90 > powerUp.y;
}

void resetGame(float& trexX, float& trexY, float& jumpSpeed, bool& isJumping, bool& isMouseClicked, GameState& gameState,
    std::vector<Cactus>& cacti, std::vector<PowerUp>& powerUps, std::vector<Pterodactyl>& pterodactyls,
    float& powerUpEffectTimer, bool& powerUpActive, float& powerUpSpawnTimer, float& cactusSpawnTimer,
    bool& canSpawnPowerUp, float& powerUpReminderTimer, bool& isDescending) {
    trexX = 100;
    trexY = groundHeight + 50;
    jumpSpeed = 0;
    isJumping = false;
    isMouseClicked = false;
    gameState = PLAYING;
    cacti.clear();
    powerUps.clear();
    pterodactyls.clear(); // Reset pterodactyls
    powerUpEffectTimer = 0;
    powerUpActive = false;
    powerUpSpawnTimer = 0;
    cactusSpawnTimer = 0;
    canSpawnPowerUp = true;
    powerUpReminderTimer = 0;
    isDescending = false; // Reset descending flag
    blinkCounter = 0; // Reset blink counter
    score = 0; // Reset score
}

void drawScore(int score, float x, float y) {
    slPush();
    slSetFont(gameFont, 24); // Use the loaded font
    slSetFontSize(24);
    slSetTextAlign(SL_ALIGN_CENTER);
    slSetForeColor(1.0, 0.5, 0.0, 1.0); // Set text color to orange
    slText(windowWidth / 2, windowHeight - 100, ("Score: " + std::to_string(score)).c_str());
    slPop();
}

void drawButton(float x, float y, float width, float height, const char* text) {
    slPush();
    slSetForeColor(0.0, 0.0, 0.0, 1.0);
    slRectangleOutline(x, y, width, height);
    slPop();

    slPush();
    slSetForeColor(0.6, 0.3, 0.0, 1.0);
    slRectangleFill(x, y, width, height);
    slPop();

    slPush();
    slSetForeColor(1.0, 1.0, 0.0, 1.0); // Warna kuning
    slSetFont(gameFont, 24); // Use the loaded font
    slSetFontSize(24);
    slSetTextAlign(SL_ALIGN_CENTER);
    slText(x, y, text);
    slPop();
}

void manageMusic(int gameState, bool& isMusicPlaying, int& currentPlayingMusic) {
    // Music constants
    const int MENU_MUSIC = 1;
    const int GAMEPLAY_MUSIC = 2;

    if (gameState == START || gameState == GAME_OVER) {
        if (currentPlayingMusic != MENU_MUSIC) {
            slSoundStopAll(); // Stop any currently playing music
            slSoundLoop(dinoMenu);
            currentPlayingMusic = MENU_MUSIC;
            isMusicPlaying = true;
        }
    }
    else if (gameState == PLAYING) {
        if (currentPlayingMusic != GAMEPLAY_MUSIC) {
            slSoundStopAll(); // Stop any currently playing music
            slSoundLoop(backgroundMusic);
            currentPlayingMusic = GAMEPLAY_MUSIC;
            isMusicPlaying = true;
        }
    }
}

bool isMouseOverButton(float mouseX, float mouseY, float x, float y, float width, float height) {
    return mouseX > x - width / 2 && mouseX < x + width / 2 && mouseY > y - height / 2 && mouseY < y + height / 2;
}

int main() {
    slWindow(windowWidth, windowHeight, "T-Rex Game", false);

    float trexX = 110;
    float trexY = groundHeight + 50;
    float jumpSpeed = 0;
    const float gravity = -0.5f;
    bool isJumping = false;
    bool isMouseClicked = false;
    GameState gameState = START;

    std::vector<Cactus> cacti;
    std::vector<PowerUp> powerUps;
    std::vector<Pterodactyl> pterodactyls;
    float powerUpEffectTimer = 0;
    bool powerUpActive = false;
    float powerUpSpawnTimer = 0;
    bool canSpawnPowerUp = true;
    float powerUpReminderTimer = 0;
    const float reminderDuration = 2.0f;

    float powerUpRespawnTimer = 0;

    float cactusSpawnTimer = 0;

    srand(static_cast<unsigned>(time(0)));

    trexTexture1 = slLoadTexture("Tyrex1.png");
    trexTexture2 = slLoadTexture("Tyrex2.png");
    backgroundTexture = slLoadTexture("background.png"); // Load the background texture
    powerUpSound = slLoadWAV("powerup.wav"); // Load the power up sound
    hitHurtSound = slLoadWAV("hithurt.wav"); // Load the hit/hurt sound
    jumpSound = slLoadWAV("jump.wav"); // Load the jump sound
    treeTexture = slLoadTexture("stone.png"); // Load the tree texture
    pterodactylTexture1 = slLoadTexture("Ptero1.png"); // Load the pterodactyl texture
    pterodactylTexture2 = slLoadTexture("Ptero2.png"); // Load the pterodactyl texture
    gameFont = slLoadFont("Maximum Voltage.ttf"); // Load the font file
    dinoMenu = slLoadWAV("menu2.wav");
    backgroundMusic = slLoadWAV("bm.wav");
    double scrollX = 0.0;

    // Track the current playing music
    bool isMusicPlaying = false;
    int currentPlayingMusic = 0;

    while (!slShouldClose() && !slGetKey(SL_KEY_ESCAPE)) {
        slSetSpriteScroll(scrollX, 0.0);
        double dt = slGetDeltaTime();
        slRender();

        // Draw the background
        slSetForeColor(1.0, 1.0, 1.0, 1.0);
        slSprite(backgroundTexture, windowWidth / 2, windowHeight / 2, windowWidth, windowHeight);
        scrollX += 0.001; // Scroll background
        if (scrollX >= 1.0)
            scrollX = 0.0;
        
        manageMusic(gameState, isMusicPlaying, currentPlayingMusic);

        // Update game objects
        if (gameState == START) {
            
            slPush();
            drawButton(windowWidth / 2, windowHeight / 2, 200, 65, "START");
            
            if (slGetMouseButton(SL_MOUSE_BUTTON_LEFT)) {
                float mouseX = slGetMouseX();
                float mouseY = slGetMouseY();
                if (isMouseOverButton(mouseX, mouseY, windowWidth / 2, windowHeight / 2, 200, 70)) {
                    gameState = PLAYING;
                }
            }
            slPop();
            // Display main / start text
            slPush();
            slSetFont(gameFont, 36); // Use the loaded font
            slSetFontSize(36);
            slSetTextAlign(SL_ALIGN_CENTER);
            slSetForeColor(1.0, 0.5, 0.0, 1.0); // Set text color to orange
            slText(windowWidth / 2, windowHeight - 100, "Jurassic Jump");
            slPop();

        }
        else if (gameState == PLAYING) {
            if (slGetMouseButton(SL_MOUSE_BUTTON_LEFT) && !isJumping) {
                jumpSpeed = 18.5f;
                isJumping = true;
                slSoundPlay(jumpSound); // Play the jump sound
            }

            if (isJumping) {
                trexY += jumpSpeed;
                jumpSpeed += gravity;

                if (trexY <= groundHeight + 50) {
                    trexY = groundHeight + 50;
                    isJumping = false;
                }
            }

            // Check for right mouse button press for quick descent
            if (slGetMouseButton(SL_MOUSE_BUTTON_RIGHT) && isJumping) {
                jumpSpeed = -16.0f; // Set the jump speed to a high negative value for quick descent
                isDescending = true; // Set descending flag
            }
            else {
                isDescending = false; // Reset descending flag
            }

            // Draw T-Rex with descending and power-up active flags
            drawTrex(trexX, trexY, isDescending, powerUpActive);

            // Draw ground
            slSetForeColor(0.3, 0.8, 0.3, 1.0);
            slRectangleFill(windowWidth / 2, groundHeight / 2, windowWidth, groundHeight);

            // Draw and update cacti
            for (auto& cactus : cacti) {
                cactus.x -= 7;
                drawCactus(cactus);
            }
            cacti.erase(std::remove_if(cacti.begin(), cacti.end(), [](const Cactus& cactus) {
                return cactus.x < -cactusWidth;
                }), cacti.end());

            // Draw and update power-ups
            for (auto& powerUp : powerUps) {
                powerUp.x -= 5.5;
                if (powerUp.active) {
                    drawPowerUp(powerUp);
                }
            }
            powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(), [](const PowerUp& powerUp) {
                return powerUp.x < -powerUpWidth;
                }), powerUps.end());

            // Draw and update pterodactyls
            for (auto& pterodactyl : pterodactyls) {
                pterodactyl.x -= 8.5;
                drawPterodactyl(pterodactyl);
            }
            pterodactyls.erase(std::remove_if(pterodactyls.begin(), pterodactyls.end(), [](const Pterodactyl& pterodactyl) {
                return pterodactyl.x < -pterodactylWidth;
                }), pterodactyls.end());

            // Spawn new cacti
            cactusSpawnTimer += dt;
            if (cactusSpawnTimer >= 6.0f / (1.0f + score / 50.0f)) { // Adjust spawn frequency
                float randomX = static_cast<float>(windowWidth) + rand() % 200;
                float randomY = groundHeight + 30;

                // Check if the new cactus position is far enough from the last power-up
                if (abs(randomX - lastPowerUpX) >= minDistance && (slGetTime() - lastPowerUpTime) >= minTimeGap) {
                    cacti.push_back(Cactus{ randomX, randomY });
                    lastCactusX = randomX;
                    lastCactusTime = slGetTime();
                    cactusSpawnTimer = 0;
                }
            }

            // Spawn new power-ups
            powerUpSpawnTimer += dt;
            if (powerUpSpawnTimer >= powerUpSpawnInterval && !powerUpActive) {
                float randomX = static_cast<float>(windowWidth) + rand() % 200;
                float randomY = groundHeight + 50;

                // Check if the new power-up position is far enough from the last cactus
                if (abs(randomX - lastCactusX) >= minDistance && (slGetTime() - lastCactusTime) >= minTimeGap) {
                    powerUps.push_back(PowerUp{ randomX, randomY, true });
                    lastPowerUpX = randomX;
                    lastPowerUpTime = slGetTime();
                    powerUpSpawnTimer = 0;
                    canSpawnPowerUp = false; // Disable spawn power-up during invincibility mode
                    powerUpReminderTimer = 0;
                }
            }


            // Spawn new pterodactyls
            static float pterodactylSpawnTimer = 0;
            pterodactylSpawnTimer += dt;
            if (pterodactylSpawnTimer >= 8.0f / (1.0f + score / 50.0f)) { // Adjust spawn frequency
                float randomX = static_cast<float>(windowWidth) + rand() % 200; // Randomize horizontal position
                // Gunakan posisi vertikal yang tetap
                float randomY = groundHeight + 180; // Tetapkan posisi vertikal
                pterodactyls.push_back(Pterodactyl{ randomX, randomY });
                pterodactylSpawnTimer = 0;
            }

            // Check collisions with cacti
            for (const auto& cactus : cacti) {
                if (!powerUpActive && checkCollision(trexX, trexY, cactus)) {
                    slSoundPlay(hitHurtSound); // Play the hit/hurt sound
                    gameState = GAME_OVER;
                }
            }

            // Check collisions with power-ups
            for (auto& powerUp : powerUps) {
                if (powerUp.active && checkPowerUpCollision(trexX, trexY, powerUp)) {
                    powerUp.active = false;
                    powerUpEffectTimer = powerUpDuration;
                    powerUpActive = true;
                    slSoundPlay(powerUpSound); // Play the power up sound
                }
            }

            // Check collisions with pterodactyls
            for (const auto& pterodactyl : pterodactyls) {
                if (!powerUpActive && checkCollision(trexX, trexY, pterodactyl)) {
                    slSoundPlay(hitHurtSound); // Play the hit/hurt sound
                    gameState = GAME_OVER;
                }
            }

            // Update power-up effect timer
            if (powerUpActive) {
                powerUpEffectTimer -= dt;
                if (powerUpEffectTimer <= 0) {
                    powerUpActive = false;
                }
            }

            // Update power-up reminder timer
            if (!powerUpActive) {
                powerUpReminderTimer += dt;
                if (powerUpReminderTimer >= reminderDuration) {
                    powerUpReminderTimer = 0;
                    canSpawnPowerUp = true;
                }
            }

            // Increment frame counter and blink counter
            frameCounter++;
            blinkCounter++;

            // Update score when passing obstacles
            for (auto& cactus : cacti) {
                if (trexX > cactus.x && !cactus.passed) {
                    cactus.passed = true;
                    score++;
                }
            }
            for (auto& pterodactyl : pterodactyls) {
                if (trexX > pterodactyl.x && !pterodactyl.passed) {
                    pterodactyl.passed = true;
                    score++;
                }
            }

            // Draw score
            drawScore(score, 1100, 700);
        }
        else if (gameState == GAME_OVER) {

            slSetFontSize(36);
            slSetTextAlign(SL_ALIGN_CENTER);
            slSetForeColor(1.0, 0.0, 0.0, 1.0);
            slText(windowWidth / 2, windowHeight / 2 + 100, "GAME OVER");
            slSetForeColor(1.0, 0.5, 0.0, 1.0); // Set text color to orange
            slSetFontSize(24);
            slText(windowWidth / 2, windowHeight / 2 + 50, ("Your Score: " + std::to_string(score)).c_str());
            slSetForeColor(1.0, 0.0, 0.0, 1.0);
            drawButton(windowWidth / 2, windowHeight / 2, 200, 65, "RESTART");
            

            double mouseX = slGetMouseX();
            double mouseY = slGetMouseY();

            if (slGetMouseButton(SL_MOUSE_BUTTON_LEFT) && isMouseOverButton(mouseX, mouseY, windowWidth / 2, windowHeight / 2, 200, 70)) {
                resetGame(trexX, trexY, jumpSpeed, isJumping, isMouseClicked, gameState, cacti, powerUps, pterodactyls,
                    powerUpEffectTimer, powerUpActive, powerUpSpawnTimer, cactusSpawnTimer, canSpawnPowerUp, powerUpReminderTimer, isDescending);
                
            }
           

        }
    }

    slClose();
    return 0;
}