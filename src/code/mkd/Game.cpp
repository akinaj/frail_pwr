/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Game.h"
#include "PrefabMgr.h"
#include "Player.h"
#include "PhysicsObjectUserData.h"
#include "DynamicBody.h"
#include "ActorAI.h"
#include "contrib/DebugDrawer.h"
#include "ActorControllerFactory.h"
#include "Level.h"
#include "rtti/TypeManager.h"
#include "rtti/PresetMgr.h"

#ifdef ENABLE_BULLET_DEBUG_DRAW // see settings.h
#include "contrib/BtOgreExtras.h"
BtOgre::DebugDrawer* g_debugPhysicsDrawer = 0;
#endif

Game* g_game = 0;

Game::Game()
    : m_exit(false)
    , m_freelook(false)
    , m_levelName("level")
    , m_startWithFreelook(false)
    , m_pauseUpdates(false)
    , m_lastDt(0.f)
    , m_logicTime(0.f)
    , m_slomo(1.f)
    , m_prefabMgr(NULL)
    , m_actorControllerFactory(NULL)
    , m_disableShadows(false)
{
}

Game::~Game()
{

}

bool Game::init(const mkString& cmd_line)
{
    rtti::TypeManager::getInstance().finishTypeRegistration();

    m_presetMgr = new rtti::PresetMgr;
    m_presetMgr->init();

    parseCmdLine(cmd_line);
    initRendering();
    initInput();
    initPhysics();

#ifdef ENABLE_BULLET_DEBUG_DRAW
    g_debugPhysicsDrawer = new BtOgre::DebugDrawer(m_ogreSceneMgr->getRootSceneNode(), m_physicsWorld);
    m_physicsWorld->setDebugDrawer(g_debugPhysicsDrawer);
#endif

    m_prefabMgr = new PrefabMgr;
    m_prefabMgr->load("data/prefabs.db", m_physicsWorld);

    m_actorControllerFactory = new ActorControllerFactory;

    m_level = new Level();
    if (!m_level->load("data/levels/" + m_levelName + ".json"))
        return false;

    m_freelook = m_startWithFreelook;

    return true;
}

void Game::clear()
{
    m_level->clear();
    delete m_level;
    m_level = NULL;

    delete m_prefabMgr;
    m_prefabMgr = NULL;

    delete DebugDrawer::getSingletonPtr();

    m_actorControllerFactory->releaseAll();
    delete m_actorControllerFactory;
    m_actorControllerFactory = NULL;

    delete m_presetMgr;
    m_presetMgr = NULL;
}

void Game::updateVisuals(float dt)
{
    if (m_level)
        m_level->updateRendering(dt);

    const mkVec3 fvec = m_playerCam->getForwardVec();

    // update hud
    Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrFps")->setCaption("FPS: " + Ogre::StringConverter::toString(m_renderWindow->getLastFPS()));
    const int minutes = (int)m_logicTime / 60;
    const int seconds = (int)m_logicTime % 60;
    Ogre::OverlayManager::getSingleton().getOverlayElement("Game/Health")->setCaption("Time: " + Ogre::StringConverter::toString(minutes, 2, '0') + ":" + Ogre::StringConverter::toString(seconds, 2, '0'));

    char buff[512] = {0};
    sprintf(buff, "Pos: %.1f %.1f %.1f", m_playerCam->getPosition().x, m_playerCam->getPosition().y, m_playerCam->getPosition().z);
    Ogre::OverlayManager::getSingleton().getOverlayElement("Game/Pos")->setCaption(buff);
    sprintf(buff, "FV: %.2f %.2f %.2f", fvec.x, fvec.y, fvec.z);
    Ogre::OverlayManager::getSingleton().getOverlayElement("Game/ForwardVec")->setCaption(buff);
}

void Game::updateLogic(float dt)
{
    if (m_level)
        m_level->updateLogic(dt);
}

bool s_EnableBulletDebugDraw = false;
bool s_ForceLowFramerate = false;

bool Game::update( float dt )
{
    m_lastRealDt = dt;

    if (m_pauseUpdates)
        m_lastDt = 0.f;
    else
        m_lastDt = dt;

    DebugDrawer::getSingleton().clear();

#ifdef ENABLE_BULLET_DEBUG_DRAW
    if (s_EnableBulletDebugDraw)
    {
        g_debugPhysicsDrawer->setDebugMode(true);
        g_debugPhysicsDrawer->step();
        m_physicsWorld->debugDrawWorld();
    }
    else if (g_debugPhysicsDrawer->getDebugMode())
    {
        g_debugPhysicsDrawer->setDebugMode(false);
        g_debugPhysicsDrawer->step();
    }
#endif

    updateInput(dt);

    if (m_pauseUpdates)
        updateLogic(0.f);
    else
    {
        const float logic_dt = dt * m_slomo;
        float logic_dt_left = logic_dt;
        m_logicTime += logic_dt;

        do 
        {
            const float it_dt = std::min(logic_dt_left, dt);
            updateLogic(it_dt);

            logic_dt_left -= it_dt;
        } while (logic_dt_left > 0.001f);

        updatePhysics(logic_dt);
    }

    updateVisuals(dt);

    DebugDrawer::getSingleton().build();

    if (s_ForceLowFramerate && dt < 0.1f)
        Sleep(100);

    if (!m_ogreRoot->renderOneFrame(dt))
        return false;

    if (m_renderWindow->isClosed())
        return false;

    if (m_exit)
        return false;

    return true;
}

void Game::initRendering()
{
#ifdef _DEBUG
    const char* plugin_file = "plugins_d.cfg";
#else
    const char* plugin_file = "plugins.cfg";
#endif

    m_ogreRoot = new Ogre::Root(plugin_file);
    m_ogreRoot->showConfigDialog();
    m_ogreRoot->initialise(true);

    m_renderWindow = m_ogreRoot->getAutoCreatedWindow(); //createRenderWindow("mkdemo", 800, 600, false);

    m_ogreSceneMgr = m_ogreRoot->createSceneManager(Ogre::ST_GENERIC);
    m_ogreCamera = m_ogreSceneMgr->createCamera("main_cam");
    Ogre::Viewport* viewport = m_renderWindow->addViewport(m_ogreCamera);
    m_ogreCamera->setAspectRatio(viewport->getActualWidth() / (float)viewport->getActualHeight());

    // DebugDrawer singleton
    new DebugDrawer(m_ogreSceneMgr, .75f);

    loadOgreResources();

    // overlay
    Ogre::Overlay* hud = Ogre::OverlayManager::getSingleton().getByName("Game/HUD");
    hud->show();

    if (m_disableShadows)
        m_ogreSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    else
        m_ogreSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    m_ogreCamera->setNearClipDistance(0.1f);

    if (!m_startWithFreelook)
        m_playerCam = new CameraTPP(m_ogreCamera, NULL);
    else
        m_playerCam = new CameraFPP(m_ogreCamera, NULL);

    m_ogreSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f)); // please note it's usually changed on level deserialization with RenderSettingsSetter
}

void Game::loadOgreResources()
{
    // Loads all defined ogre resources (meshes, materials etc); not ok for for lots of content
    Ogre::ConfigFile config_file;
    config_file.load("resources.cfg");

    Ogre::ConfigFile::SectionIterator sec_iter = config_file.getSectionIterator();

    Ogre::String sec_name, type_name, arch_name;
    while (sec_iter.hasMoreElements())
    {
        sec_name = sec_iter.peekNextKey();

        Ogre::ConfigFile::SettingsMultiMap* settings = sec_iter.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator iter;

        for (iter = settings->begin(); iter != settings->end(); ++iter)
        {
            type_name = iter->first;
            arch_name = iter->second;

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                arch_name, type_name, sec_name);
        }
    }

    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Game::initInput()
{
    OIS::ParamList pl;
    uint32 window_handle = 0;

    m_renderWindow->getCustomAttribute("WINDOW", &window_handle);

    char buff[32];
    sprintf(buff, "%31d", window_handle);

    pl.insert(std::make_pair(std::string("WINDOW"), std::string(buff)));

    m_inputMgr = OIS::InputManager::createInputSystem(pl);
    m_keyboard = (OIS::Keyboard*)m_inputMgr->createInputObject(OIS::OISKeyboard, false);
    m_mouse = (OIS::Mouse*)m_inputMgr->createInputObject(OIS::OISMouse, true);

    m_mouse->setEventCallback(this);
}

void Game::initPhysics()
{
    btCollisionConfiguration* collision_config = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* collision_dispatcher = new btCollisionDispatcher(collision_config);
    btBroadphaseInterface* overlapping_pair_cache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

    m_physicsWorld = new btDiscreteDynamicsWorld(collision_dispatcher, overlapping_pair_cache, solver, collision_config);
    m_physicsWorld->setGravity(btVector3(0, -10, 0));

    m_physicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    //m_physicsWorld->setInternalTickCallback(myTickCallback, this, false);
}

void Game::updatePhysics(float dt)
{
    if (m_pauseUpdates)
        return;

    m_physicsWorld->stepSimulation(dt, 30);

    if (!m_freelook && m_level)
        m_playerCam->setTarget(m_level->getPlayer());
    else
        m_playerCam->setTarget(NULL);
}

void Game::movePlayer( float forward, float right, float dt )
{
    static float s_FreeCamSpeed = 12.f;
    const mkVec3 forward_vec = m_playerCam->getForwardVec() * forward ;
    const mkVec3 right_vec = m_playerCam->getRightVec() * right;
    mkVec3 move_dir = (forward_vec + right_vec).normalisedCopy();
    
    if (!m_freelook && m_level)
    {
        Player* player = m_level->getPlayer();
        player->addDirection(move_dir);
        player->setSpeed(isKeyDown(OIS::KC_LSHIFT) ? 1.f : 0.5f);
    }
    else
    {
        m_playerCam->move(move_dir * s_FreeCamSpeed * getRealTimeDelta() * 5.f);
    }
}

void Game::updateInput( float dt )
{
    if (m_level)
        m_level->updateInput(dt);

    m_mouse->capture();
    m_keyboard->capture();

    if (isKeyDown(OIS::KC_ESCAPE))
        m_exit = true;

    if (isKeyDown(OIS::KC_S))
        movePlayer(-1.0f, 0.0f, dt);
    if (isKeyDown(OIS::KC_W))
        movePlayer(1.0f, 0.0f, dt);
    if (isKeyDown(OIS::KC_A))
        movePlayer(0.0f, -1.0f, dt);
    if (isKeyDown(OIS::KC_D))
        movePlayer(0.0f, 1.0f, dt);

    m_playerCam->update(dt, (float)m_mouse->getMouseState().X.rel, (float)m_mouse->getMouseState().Y.rel);

    if (isKeyDown(OIS::KC_F1))
    {
        FILE* file = fopen("player.txt", "wt");
        mkVec3 v = m_playerCam->getPosition();
        fprintf(file, "%.4ff, %.4ff, %.4ff", v.x, v.y, v.z);
        fclose(file);
    }

    if (isKeyDown(OIS::KC_F2) && !m_freelook)
    {
        mkVec3 pos = m_playerCam->getPosition();
        delete m_playerCam;
        m_playerCam = new CameraFPP(m_ogreCamera, getCurrentLevel()->getPlayer());
        m_playerCam->setTarget(getCurrentLevel()->getPlayer());
        m_playerCam->setTarget(NULL);

        m_freelook = true;
    }
    else if (isKeyDown(OIS::KC_F3) && m_freelook)
    {
        m_freelook = false;

        delete m_playerCam;
        m_playerCam = new CameraTPP(m_ogreCamera, getCurrentLevel()->getPlayer());
    }

    if (isKeyDown(OIS::KC_F5))
        s_EnableBulletDebugDraw = !isKeyDown(OIS::KC_LCONTROL);

    if (isKeyDown(OIS::KC_L))
    {
        if (s_ForceLowFramerate && isKeyDown(OIS::KC_LSHIFT))
            s_ForceLowFramerate = false;
        else if (!s_ForceLowFramerate && isKeyDown(OIS::KC_LCONTROL))
            s_ForceLowFramerate = true;
    }
    
    if (isKeyDown(OIS::KC_F9))
        m_pauseUpdates = true;
    else if (isKeyDown(OIS::KC_F10))
        m_pauseUpdates = false;

    if (isKeyDown(OIS::KC_F11))
        m_slomo = 10.f;
    else if (isKeyDown(OIS::KC_F12))
        m_slomo = 1.f;

    static float s_lastDbgAnimInput = -1.f;
    if (isKeyDown(OIS::KC_LCONTROL) && (getTimeMs() - s_lastDbgAnimInput) > 200.f && getCurrentLevel() && getCurrentLevel()->getPlayer())
    {
        if (isKeyDown(OIS::KC_Z))
        {
            getCurrentLevel()->getPlayer()->dbgPrevAnimPreview();
            getCurrentLevel()->getPlayer()->dbgPlayAnimPreview();

            s_lastDbgAnimInput = getTimeMs();
        }

        if (isKeyDown(OIS::KC_C))
        {
            getCurrentLevel()->getPlayer()->dbgNextAnimPreview();
            getCurrentLevel()->getPlayer()->dbgPlayAnimPreview();

            s_lastDbgAnimInput = getTimeMs();
        }

        if (isKeyDown(OIS::KC_X))
        {
            getCurrentLevel()->getPlayer()->dbgPlayAnimPreview();

            s_lastDbgAnimInput = getTimeMs();
        }
    }
}

bool Game::mouseMoved( const OIS::MouseEvent& arg )
{
    return false;
}

bool Game::mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID id )
{
    return true;
}

bool Game::mouseReleased( const OIS::MouseEvent& arg, OIS::MouseButtonID id )
{
    if (id == OIS::MB_Right)
    {
        if (getCurrentLevel()->getPlayer())
            getCurrentLevel()->getPlayer()->startRangedAttack();
    }
    else if (id == OIS::MB_Left)
    {
        if (getCurrentLevel()->getPlayer())
            getCurrentLevel()->getPlayer()->startMeleeAttack();
    }


    return false;
}

Ogre::SceneManager* Game::getOgreSceneMgr() const
{
    return m_ogreSceneMgr;
}

btDynamicsWorld* Game::getPhysicsWorld() const
{
    return m_physicsWorld;
}

PrefabMgr* Game::getPrefabMgr() const
{
    return m_prefabMgr;
}

ActorControllerFactory* Game::getActorControllerFactory() const
{
    return m_actorControllerFactory;
}

rtti::PresetMgr* Game::getPresetMgr() const
{
    return m_presetMgr;
}

float Game::getTimeMs() const
{
    return m_logicTime * 1000.f;
}

float Game::getTimeDelta() const
{
    return m_lastDt;
}

void extractTokens(const mkString& input_string, std::vector<mkString>& out_tokens)
{
    int current_token_start = -1;
    for (int i = 0; i < (int)input_string.size(); ++i)
    {
        const char ch = input_string[i];

        if (ch == ' ')
        {
            if (current_token_start != -1)
            {
                const int token_len = i - current_token_start;
                const mkString token = input_string.substr(current_token_start, token_len);
                out_tokens.push_back(token);

                current_token_start = -1;
            }
        }
        else
        {
            if (current_token_start == -1)
                current_token_start = i;
        }
    }

    if (current_token_start != -1)
    {
        const mkString token = input_string.substr(current_token_start);
        out_tokens.push_back(token);
    }
}

#define CHK_ARG(arg) MK_ASSERT_MSG(next_token_ptr && !next_token_ptr->empty() && (*next_token_ptr)[0] != '-', "Missing argument for command line parameter " arg);
void Game::parseCmdLine(const mkString& cmd_line)
{
    std::vector<mkString> tokens;
    extractTokens(cmd_line, tokens);

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const mkString& token = tokens[i];
        const mkString* next_token_ptr = (i < tokens.size() - 1) ? &tokens[i+1] : NULL;

        if (token == "-freelook")
            m_startWithFreelook = true;
        else if (token == "-disable_shadows")
            m_disableShadows = true;
        else if (token == "-level")
        {
            CHK_ARG("-level");
            m_levelName = *next_token_ptr;
            ++i;
        }
        else
        {
            Ogre::StringStream ss;
            ss << "Unrecognized command line parameter '" << token << "'";
            MessageBoxA(NULL, ss.str().c_str(), NULL, MB_OK);
        }
    }
}

bool Game::isKeyDown( OIS::KeyCode kc ) const
{
    return m_keyboard->isKeyDown(kc);
}

Level* Game::getCurrentLevel() const
{
    return m_level;
}

float Game::getRealTimeDelta() const
{
    return m_lastRealDt;
}
