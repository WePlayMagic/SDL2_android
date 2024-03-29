#include "Game.h"
#include "SDL_mixer.h"
#include "EventHandler.h"
#include "glm.hpp"
#include "PlayState.h"
#include "MainMenuState.h"

Game::Game() {

    SDL_Log("Game constructor");

    running = true;
}

Game::~Game() {
    SDL_Log("Game destructor");

    endGame();
}

void Game::init() {

    SDL_Log("Game init");
    if(0 == SDL_Init(SDL_INIT_EVERYTHING))
    {
        // SET ATTRIBUTE ONLY after initialize
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // on antialiasing sdl
        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2); //subsamples for each pixel

        SDL_GetCurrentDisplayMode(0, &DM);
        screenWidth = DM.w;
        screenHeight = DM.h;

        SDL_Log("screenWidth = %i, screenHeight = %i",screenWidth , screenHeight);

        // Create an application window with the following settings:
        window = SDL_CreateWindow( "Game", 0, 0, screenWidth, screenHeight,
                SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL |SDL_WINDOW_RESIZABLE );


        if(NULL == window)
        {
            SDL_Log("window create error");
            return;
        } else
        {
            SDL_Log("window created");
        }

        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

        glContext = SDL_GL_CreateContext(window);

        SDL_Log("GL_VERSION = %s", glGetString(GL_VERSION));

    } else
    {
        SDL_Log("SDL init error");
        return;
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
    {
        SDL_Log( "SDL mixer init error" );
        return;
    }


    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // background color after clear screen

    int ff;
    glGetIntegerv(GL_BLEND, &ff); // проверить что включено(ff станет = 1) а что нет(ff = 0)
    SDL_Log( "GL_BLEND = %i", ff );
    glGetIntegerv(GL_DEPTH_TEST, &ff);
    SDL_Log( "GL_DEPTH_TEST = %i", ff );

    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &ff);
    SDL_Log( "SDL_GL_MULTISAMPLEBUFFERS = %i", ff );

    gameStateMachine = new GameStateMachine;
    gameStateMachine->pushState(new MainMenuState);

}

void Game::updateEventHandler() {
    EventHandler::instance()->updateEventHandler();
}

void Game::update() { //physics

    gameStateMachine->update();

    if(EventHandler::instance()->checkFingerEvent(FINGER_DOWN))
    {
        SDL_Log("EventHandler switch(event.type) : SDL_FINGER_DOWN");
        glm::vec2 fingerPos =  EventHandler::instance()->getFingerDownPos();
        SDL_Log("Position x = %f, y = %f", fingerPos.x, fingerPos.y);
    }

    EventHandler::instance()->resetFingerEvents();
}

void Game::draw() {
    //clear all possible buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //	| GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

    gameStateMachine->draw(); // draw new graphics in dependency of previously calculated physics

    glFinish(); // waiting to finish drawing
    //glFlush(); // NOT waiting to finish drawing
    SDL_GL_SwapWindow(window);
}

void Game::playSound() {
    gameStateMachine->playSound();
}

void Game::endGame() {

    running = false;

    int statesCount = gameStateMachine->getStatesCount();
    SDL_Log("statesCount %i", statesCount);
    for(int i = 0; i < statesCount; i++)
    {
        gameStateMachine->popState();
    }
    gameStateMachine->update();

    delete gameStateMachine;

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}