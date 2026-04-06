#include "game/game.hpp"
#include "game/states.hpp"

int main() {
    GameContext ctx;
    initGame(ctx);

    while (!WindowShouldClose()) {
        switch (ctx.state) {
            case GameState::MainMenu: updateMainMenu(ctx); break;
            case GameState::CharacterSelect: 
                                      updateCharSelect(ctx); break;
            case GameState::Playing:  updatePlaying(ctx);  break;
            case GameState::ConfirmQuit:  
                                      updateConfirmQuit(ctx);  break;
            case GameState::LevelUp:  updateLevelUp(ctx);  break;
            case GameState::GameOver: updateGameOver(ctx); break;
        }
    }

    CloseWindow();
}
