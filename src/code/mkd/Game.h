/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "Camera.h"

class ActorControllerFactory;
class Level;
namespace rtti { class PresetMgr; }

class Game : public OIS::MouseListener
{
public:
    Game();
    ~Game();

    bool init(const mkString& cmd_line);
    void clear();

    bool update(float dt);

    Ogre::SceneManager* getOgreSceneMgr() const;
    btDynamicsWorld* getPhysicsWorld() const;
    ActorControllerFactory* getActorControllerFactory() const;
    Level* getCurrentLevel() const;
    rtti::PresetMgr* getPresetMgr() const;

    float getTimeMs() const;
    float getTimeDelta() const;
    float getRealTimeDelta() const;

    virtual bool mouseMoved(const OIS::MouseEvent& arg);
    virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);

    ICamera* getCamera() const { return m_playerCam; }
    bool isKeyDown(OIS::KeyCode kc) const;

	bool isFreelookCamera();
	void setFreelookCamera(const mkVec3& position , const mkVec3& orientation);

private:
    void parseCmdLine(const mkString& cmd_line);
    void loadOgreResources();
    void initRendering();
    void initInput();
    void initPhysics();

    void updatePhysics(float dt);
    void updateInput(float dt);
    void updateLogic(float dt);
    void updateVisuals(float dt);
    
    void movePlayer(float forward, float right, float dt);

private:
    // Configuration
    mkString m_levelName;
    bool m_startWithFreelook;
    bool m_disableShadows;

private:
    Ogre::SceneManager* m_ogreSceneMgr;
    Ogre::Root* m_ogreRoot;
    Ogre::Camera* m_ogreCamera;
    Ogre::RenderWindow* m_renderWindow;

    OIS::InputManager* m_inputMgr;
    OIS::Keyboard* m_keyboard;
    OIS::Mouse* m_mouse;

    btDynamicsWorld* m_physicsWorld;

    ICamera* m_playerCam;

    bool m_exit;
    bool m_freelook;
    bool m_pauseUpdates;

    float m_lastDt;
    float m_lastRealDt;

    float m_logicTime;
    float m_slomo;

    ActorControllerFactory* m_actorControllerFactory;
    rtti::PresetMgr* m_presetMgr;

    Level* m_level;
};

extern Game* g_game;
