/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Game.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmd_line, int)
{
    g_game = new Game;

    if (g_game->init(cmd_line))
    {
        unsigned long last_time = Ogre::Root::getSingleton().getTimer()->getMicroseconds();
        float dt = 0.001f;

        Ogre::Root::getSingleton().renderOneFrame(); // force single frame so stuff like bounding boxes (used by logic) update

        while (true)
        {
            Ogre::WindowEventUtilities::messagePump();

            if (!g_game->update(dt))
                break;

            const unsigned long cur_time = Ogre::Root::getSingleton().getTimer()->getMicroseconds();
            const unsigned long delta = cur_time - last_time;
            last_time = cur_time;
            dt = std::min<float>(0.2f, delta * 0.000001f);
        }
    }

    g_game->clear();
    delete g_game;

    return 0;
}
