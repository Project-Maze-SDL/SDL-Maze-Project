#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <iostream>
#include <stack>
#include <utility>
#include <random>
#include <algorithm>

using namespace std;

const int SCREEN_WIDTH = 640; //16*40
const int SCREEN_HEIGHT = 480; //12*40
const int CELL_SIZE = 40;
random_device rd;
mt19937 gen(rd());

enum GameState { MENU, PLAYING, WIN };

// Maze structure (1 = wall, 0 = path, 2 = start, 3 = exit)
int maze[12][16];

struct Player {
    float x, y;
    int width, height;
    float velX, velY;
    SDL_Texture* texture;
};

struct Game {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* wallTexture;
    SDL_Texture* playerTexture;
    SDL_Texture* exitTexture;
    TTF_Font* font;
    Player player;
    GameState state;
    int score;
    Uint32 startTime;
};

bool init(Game& game) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! Error: " << SDL_GetError() << endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "SDL_image could not initialize! Error: " << IMG_GetError() <<endl;
        return false;
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! Error: " << TTF_GetError() <<endl;
        return false;
    }

    game.window = SDL_CreateWindow("SDL2 Maze Game", 500,250,
                                 SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!game.window) {
        cerr << "Window could not be created! Error: " << SDL_GetError() <<endl;
        return false;
    }

    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!game.renderer) {
        cerr << "Renderer could not be created! Error: " << SDL_GetError() <<endl;
        return false;
    }

    
    // Load textures
    SDL_Surface* surface = IMG_Load("assets/wall.png");
    if (!surface) {
        cerr << "Couldn't load wall texture! Error: " << IMG_GetError() << endl;
        return false;
    }
    game.wallTexture = SDL_CreateTextureFromSurface(game.renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("assets/player.png");
    if (!surface) {
        cerr << "Couldn't load player texture! Error: " << IMG_GetError() << endl;
        return false;
    }
    game.playerTexture = SDL_CreateTextureFromSurface(game.renderer, surface);
    SDL_FreeSurface(surface);

    surface = IMG_Load("assets/exit.png");
    if (!surface) {
        cerr << "Couldn't load exit texture! Error: " << IMG_GetError() <<endl;
        return false;
    }
    game.exitTexture = SDL_CreateTextureFromSurface(game.renderer, surface);
    SDL_FreeSurface(surface);

    // Load font
    game.font = TTF_OpenFont("assets/arial.ttf", 24);
    if (!game.font) {
        cerr << "Failed to load font! Error: " << TTF_GetError() << endl;
        return false;
    }

    // Initialize player
    game.player = {40.0f, 40.0f, 40, 40, 0.0f, 0.0f, game.playerTexture};
    game.state = MENU;
    game.score = 0;
    game.startTime = 0;

    return true;
}

SDL_Texture* createTextTexture(Game& game, const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(game.font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game.renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderText(Game& game, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Texture* texture = createTextTexture(game, text, color);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderCopy(game.renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}

void renderMaze(Game& game) {
    for (int row = 0; row < 12; row++) {
        for (int col = 0; col < 16; col++) {
            SDL_Rect cell = {col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            
            if (maze[row][col] == 1) {
                SDL_RenderCopy(game.renderer, game.wallTexture, NULL, &cell);
            } else if (maze[row][col] == 3) {
                SDL_RenderCopy(game.renderer, game.exitTexture, NULL, &cell);
            }
        }
    }
}

void renderPlayer(Game& game) {
    SDL_Rect playerRect = {
        static_cast<int>(game.player.x), 
        static_cast<int>(game.player.y), 
        game.player.width, 
        game.player.height
    };
    SDL_RenderCopy(game.renderer, game.player.texture, NULL, &playerRect);
}

bool checkCollision(Game& game, float x, float y) {
    // Convert player coordinates to maze grid coordinates
    int leftCol = static_cast<int>(x) / CELL_SIZE;
    int rightCol = static_cast<int>(x + game.player.width - 1) / CELL_SIZE;
    int topRow = static_cast<int>(y) / CELL_SIZE;
    int bottomRow = static_cast<int>(y + game.player.height - 1) / CELL_SIZE;
    
    // Check all possible cells the player overlaps with
    for (int row = topRow; row <= bottomRow; row++) {
        for (int col = leftCol; col <= rightCol; col++) {
            if (row >= 0 && row < 12 && col >= 0 && col < 16) {
                if (maze[row][col] == 1) { // Wall collision
                    return true;
                } else if (maze[row][col] == 3) { // Exit reached
                    game.state = WIN;
                    game.score = (SDL_GetTicks() - game.startTime) / 1000;
                    return false;
                }
            } else {
                // Out of bounds counts as collision
                return true;
            }
        }
    }
    return false;
}

void handleInput(Game& game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game.state=WIN;// Just to exit the game loop
        }
        
        if (game.state == MENU && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_RETURN) {
                game.state = PLAYING;
                game.startTime = SDL_GetTicks();
            }
        }
        else if (game.state == WIN && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_RETURN) {
                game.state = MENU;
                game.player.x = 40.0f;
                game.player.y = 40.0f;
            }
        }
    }
    
    if (game.state == PLAYING) {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        game.player.velX = 0;
        game.player.velY = 0;
        
        if (keystates[SDL_SCANCODE_UP]) game.player.velY = -2.0f;
        if (keystates[SDL_SCANCODE_DOWN]) game.player.velY = 2.0f;
        if (keystates[SDL_SCANCODE_LEFT]) game.player.velX = -2.0f;
        if (keystates[SDL_SCANCODE_RIGHT]) game.player.velX = 2.0f;
    }
}

void update(Game& game) {
    if (game.state != PLAYING) return;
    
    float newX = game.player.x + game.player.velX;
    float newY = game.player.y + game.player.velY;
    
    if (!checkCollision(game, newX, game.player.y)) {
        game.player.x = newX;
    }
    if (!checkCollision(game, game.player.x, newY)) {
        game.player.y = newY;
    }
}

void render(Game& game) {
    // Clear screen
    SDL_SetRenderDrawColor(game.renderer, 50, 50, 50, 255);
    SDL_RenderClear(game.renderer);
    
    if (game.state == MENU) {
        renderText(game, "MAZE GAME", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, {255, 255, 255, 255});
        renderText(game, "Press ENTER to Start", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20, {255, 255, 255, 255});
    }
    else if (game.state == PLAYING) {
        renderMaze(game);
        renderPlayer(game);
        
        // Display timer
        std::string timeText = "Time: " + std::to_string((SDL_GetTicks() - game.startTime) / 1000);
        renderText(game, timeText, 10, 10, {255, 255, 255, 255});
    }
    else if (game.state == WIN) {
        renderText(game, "YOU ESCAPED!", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 50, {255, 255, 255, 255});
        renderText(game, "Time: " + std::to_string(game.score) + " seconds", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, {255, 255, 255, 255});
        renderText(game, "Press ENTER to return to menu", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 + 50, {255, 255, 255, 255});
    }
    
    // Update screen
    SDL_RenderPresent(game.renderer);
}

void close(Game& game) {
    SDL_DestroyTexture(game.wallTexture);
    SDL_DestroyTexture(game.playerTexture);
    SDL_DestroyTexture(game.exitTexture);
    TTF_CloseFont(game.font);
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void carvePath(int x, int y) {
    int directions[4][2] = {{0, -2}, {0, 2}, {-2, 0}, {2, 0}};
    shuffle(begin(directions), end(directions), gen);

    for (int i = 0; i < 4; i++) {
        int nx = x + directions[i][0];
        int ny = y + directions[i][1];

        if (nx > 0 && nx < 16 && ny > 0 && ny < 12 && maze[ny][nx] == 1) {
            maze[ny][nx] = 0;
            maze[y + (ny - y)/2][x + (nx - x)/2] = 0;
            
            // Render each step (optional - shows maze generation)
            //renderMaze(game);
            //SDL_Delay(50); // Small delay to see the generation
            
            carvePath(nx, ny);
        }
    }
}

int main(int argc, char* argv[]) {

    int i,j;

    for(i=0;i<12;i++){
        for(j=0;j<16;j++){
            maze[i][j]=1;
        }
    }
    maze[1][1] = 2;
    maze[10][14] = 3;

    carvePath(1, 1);
    
    // Set start and exit
    maze[1][1] = 2;
    maze[10][14] = 3;
    
    Game game;
    
    if (!init(game)) {
        close(game);
        return 1;
    }
    
    bool running = true;
    while (running) {
        handleInput(game);
        update(game);
        render(game);
        
        if (game.state == WIN && SDL_GetTicks() - game.startTime > 5000) {
            running = false;
        }
        
        SDL_Delay(20); // ~60 FPS
    }
    
    close(game);
    return 0;
}
