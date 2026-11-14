#include <iostream>
#include <valarray>
#include <vector>
#include <SDL3/SDL.h>
#include "headers/rectangle.h"
#include <glm/glm.hpp>

#include "circle.h"

using namespace std;

//define window dimensions
constexpr int WINDOW_WIDTH{700};
constexpr int WINDOW_HEIGHT{800};

// Vertical scrolling grid related variables
constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
std::vector<Rectangle> scrollingGrid;

// Animation related variables
float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };
float projectileTimer{ 0.0f };
const float projectileSpawnInterval{ 0.3f };

//define SDL Window related variables
SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{255, 255, 255, 255};

bool quit{false};

float mouseX{-1.0f}, mouseY{-1.0f};

float displayScale{1.0f};

Rectangle player;
vector<Rectangle> enemies;
vector<Circle> playerProjectiles;


void init_player() {
    player.color = SDL_Color{ 0, 235, 0, 255 };
    player.height = 50.0f;
    player.width = 50.0f;
    player.pos.y = WINDOW_HEIGHT - player.height / 2.0f;
    player.pos.x = WINDOW_WIDTH / 2.0f;
}

void init_slice_enemies() {
    enemies.clear();
    int slice_count{3};

    float topHalfHeight = WINDOW_HEIGHT / 2.0f;
    float sliceHeight = topHalfHeight / slice_count;

    for (int i = 0; i < slice_count; i++) {
        Rectangle enemy;
        enemy.width = player.width;
        enemy.height = player.height;

        enemy.pos.x = (WINDOW_WIDTH / 2.0f) - (enemy.width / 2.0f);

        enemy.pos.y = (i * sliceHeight) + (sliceHeight / 2.0f) - (enemy.height / 2.0f);

        enemy.color = SDL_Color{ 255, 165, 0, 255 };

        enemies.push_back(enemy);
    }
}


void initScollingRectangles() {
    int numGridRectangles = (WINDOW_HEIGHT / VERTICAL_SCROLLING_GRID_SQUARE_SIZE) / 2 + 2;

    for (int i = 0; i < numGridRectangles; i++) {
        Rectangle newGridRectangleLeft, newGridRectangleRight;

        newGridRectangleLeft.width = newGridRectangleLeft.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleLeft.pos.x = newGridRectangleLeft.width / 2.0f;
        newGridRectangleLeft.pos.y = WINDOW_HEIGHT - newGridRectangleLeft.height / 2.0f - 2.0f * i * newGridRectangleLeft.height;
        newGridRectangleLeft.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleLeft.color = SDL_Color{ 235, 235, 235, 255 };

        newGridRectangleRight.width = newGridRectangleRight.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleRight.pos.x = WINDOW_WIDTH - newGridRectangleRight.width / 2.0f;
        newGridRectangleRight.pos.y = WINDOW_HEIGHT - newGridRectangleRight.height / 2.0f - 2.0f * i * newGridRectangleRight.height;
        newGridRectangleRight.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleRight.color = SDL_Color{ 235, 235, 235, 255 };

        scrollingGrid.push_back(newGridRectangleLeft);
        scrollingGrid.push_back(newGridRectangleRight);
    }
}

void shoot() {
    projectileTimer += DT;
    if (projectileTimer >= projectileSpawnInterval) {
        projectileTimer = 0.0f;

        Circle playerProjectile;
        playerProjectile.color = SDL_Color{ 255, 0, 0, 255 };
        playerProjectile.radius = 8.0f;

        playerProjectile.pos = glm::vec2(player.pos.x + (player.width / 2.0f), player.pos.y);

        playerProjectiles.push_back(playerProjectile);
    }

    float projectileSpeed = 600.0f;
    for (int i = 0; i < playerProjectiles.size(); i++) {
        playerProjectiles.at(i).pos.y -= projectileSpeed * DT;
    }

    for (int i = playerProjectiles.size() - 1; i >= 0; i--) {
        if (playerProjectiles.at(i).pos.y < -50.0f) {
            playerProjectiles.erase(playerProjectiles.begin() + i);
        }
    }
}


bool initWindow() {
    bool success{true};

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        } else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                   backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void processEvents() {
    //Check for events in queue
    while (SDL_PollEvent(&currentEvent)) {
        //User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        //Mouse event -> pressed button
        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                SDL_GetMouseState(&mouseX, &mouseY);
                mouseX /= displayScale;
                mouseY /= displayScale;
                std::cout << "Left mouse click => " << "x: " << mouseX << ", y: " << mouseY << std::endl;
                shoot();
            }
        }

        //Mouse event -> mouse movement
        //Mouse event -> mouse movement
        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
            mouseX = currentEvent.motion.x / displayScale;
            mouseY = currentEvent.motion.y / displayScale;

            if (currentEvent.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
                std::cout << "Mouse move while left clicking => " << "x: " << mouseX << ", y: " << mouseY << std::endl;
            }
        }

        //Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
                case SDLK_UP:
                    std::cout << "UP arrow key was pressed" << std::endl;
                    break;
                case SDLK_DOWN:
                    std::cout << "DOWN arrow key was pressed" << std::endl;
                    break;
                case SDLK_LEFT:
                    std::cout << "LEFT arrow key was pressed" << std::endl;
                    break;
                case SDLK_RIGHT:
                    std::cout << "RIGHT arrow key was pressed" << std::endl;
                    break;
                case SDLK_R:
                    std::cout << "R key was pressed" << std::endl;
                    break;
                case SDLK_S:
                    std::cout << "S key was pressed" << std::endl;
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default:
                    break;
            }
        }
    }
}

void animate() {
    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
    bool isLeftPressed = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) != 0;

    if (isLeftPressed) {
        projectileTimer += DT;
        if (projectileTimer >= projectileSpawnInterval) {
            projectileTimer = 0.0f;

            Circle playerProjectile;
            playerProjectile.color = SDL_Color{ 255, 0, 0, 255 };
            playerProjectile.radius = 8.0f;

            playerProjectile.pos = glm::vec2(player.pos.x + (player.width / 2.0f), player.pos.y);

            playerProjectiles.push_back(playerProjectile);
        }
    }
    else {
        projectileTimer = projectileSpawnInterval;
    }

    float projectileSpeed = 600.0f;
    for (int i = 0; i < playerProjectiles.size(); i++) {
        playerProjectiles.at(i).pos.y -= projectileSpeed * DT;
    }

    for (int i = playerProjectiles.size() - 1; i >= 0; i--) {
        if (playerProjectiles.at(i).pos.y < -50.0f) {
            playerProjectiles.erase(playerProjectiles.begin() + i);
        }
    }

    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }

    if (mouseX >= 0 && mouseY >= 0) {
        float playerSpeed = 300.0f;
        glm::vec2 mousePos(mouseX, mouseY);
        glm::vec2 toMouse = mousePos - player.pos;
        float distance = glm::length(toMouse);

        if (distance > 0.1f) {
            glm::vec2 direction = glm::normalize(toMouse);
            float movementThisFrame = playerSpeed * DT;
            player.pos += direction * movementThisFrame;
        }
    }
}

void drawFrame() {
    //Clear the background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scene
    player.draw(renderer);
    for (int i = 0; i < enemies.size(); i++) {
        enemies.at(i).draw(renderer);
    }

    //Draw scrolling grid

    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }

    for (int i = 0; i < playerProjectiles.size(); i++) {
        playerProjectiles.at(i).draw(renderer);
    }

    //Update window
    SDL_RenderPresent(renderer);
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}

int main() {
    //Initialize window
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    SDL_zero(currentEvent);
    bool hide_cursor = SDL_HideCursor();

	initScollingRectangles();
    init_player();
    init_slice_enemies();

    //Game loop
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
    }

    cleanup();
    return 0;
}
