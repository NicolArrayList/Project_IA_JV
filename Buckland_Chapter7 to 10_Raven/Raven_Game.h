#ifndef RAVEN_ENV
#define RAVEN_ENV
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Raven_Game.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   this class creates and stores all the entities that make up the
//          Raven game environment. (walls, bots, health etc) and can read a
//          Raven map file and recreate the necessary geometry.
//
//          this class has methods for updating the game entities and for
//          rendering them.
//-----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <list>

#include "graph/SparseGraph.h"
#include "Raven_ObjectEnumerations.h"
#include "2d/Wall2D.h"
#include "misc/utils.h"
#include "game/EntityFunctionTemplates.h"
#include "Raven_Bot.h"
#include "Raven_BotLearning.h"
#include "navigation/pathmanager.h"
#include "Raven_Team.h"



class BaseGameEntity;
class Raven_Projectile;
class Raven_Map;
class GraveMarkers;



class Raven_Game
{
public:
    // mode
    enum class Mode {
        DeathMatch,
        Team,
        BattleRoyale,
        oneVSone
    };

private:

  //the current game map
  Raven_Map*                       m_pMap;
 
  //a list of all the bots that are inhabiting the map
  std::list<Raven_Bot*>            m_Bots;

  int                              m_Classic_nb;

  //the user may select a bot to control manually. This is a pointer to that
  //bot
  Raven_Bot*                       m_pSelectedBot;
  
  //this list contains any active projectiles (slugs, rockets,
  //shotgun pellets, etc)
  std::list<Raven_Projectile*>     m_Projectiles;

  //this class manages all the path planning requests
  PathManager<Raven_PathPlanner>*  m_pPathManager;

  std::vector<Raven_Team*>         m_Teams;

  int                              m_Team_nb_0;
  int                              m_Team_nb_1;

  unsigned int                     m_current_team_to_add_bot = 0;

  // mode
  Mode                             m_Mode;

  bool                             m_human_playing;
  bool                             m_respawn_allowed;

  //if true the game will be paused
  bool                             m_bPaused;

  std::string                      m_victory_message;

  //if true a bot is removed from the game
  bool                             m_bRemoveABot;

  //when a bot is killed a "grave" is displayed for a few seconds. This
  //class manages the graves
  GraveMarkers*                    m_pGraveMarkers;

  //this iterates through each trigger, testing each one against each bot
  void  UpdateTriggers();

  //deletes all entities, empties all containers and creates a new navgraph 
  void  Clear();

  //attempts to position a spawning bot at a free spawn point. returns false
  //if unsuccessful 
  bool AttemptToAddBot(Raven_Bot* pBot);

  //when a bot is removed from the game by a user all remaining bots
  //must be notified so that they can remove any references to that bot from
  //their memory
  void NotifyAllBotsOfRemoval(Raven_Bot* pRemovedBot)const;

  void SetTeams(int nbTeams);

    CData m_TrainingSet; //jeu d'apprentissage

    bool m_LancerApprentissage; // pour lancer l'apprentissage

    CNeuralNet m_ModeleApprentissage;

    bool AddData(std::vector<double>&data, std::vector<double>& targets);

    void TrainThread();

    bool m_estEntraine;

    int m_maxApprentissage;

    bool m_learningFromHuman;
public:
  
  Raven_Game(int nb_bot, int nb_bot_team_0, int nb_bot_team_1, Mode mode, bool human_playing);
  ~Raven_Game();

  //the usual suspects
  void Render();
  int Update();

  //loads an environment from a file
  bool LoadMap(const std::string& FileName);

  // Initialize based on constructor datas
  void InitializeGame();
  bool CheckWinCondition();

  void AddBots(unsigned int NumBotsToAdd, bool isLearningBot = false);
  void AddBotsTeam(unsigned int NumBotsToAdd, bool isLearningBot = false);
  void AddBotsToTeam(int team_id,int nb_bot, bool isLearningBot = false);
  void AddBotToTeam(int team_id, bool isLearningBot = false);
  void AddBot(Raven_Bot* rb);
  Raven_Bot* GenerateBot(bool isLearningBot = false);
  void AddRocket(Raven_Bot* shooter, Vector2D target);
  void AddRailGunSlug(Raven_Bot* shooter, Vector2D target);
  void AddShotGunPellet(Raven_Bot* shooter, Vector2D target);
  void AddBolt(Raven_Bot* shooter, Vector2D target);

  //removes the last bot to be added
  void RemoveBot();
  void RemoveSpecificBot(Raven_Bot* Bot);


  //returns true if a bot of size BoundingRadius cannot move from A to B
  //without bumping into world geometry
  bool isPathObstructed(Vector2D A, Vector2D B, double BoundingRadius = 0)const;

  //returns a vector of pointers to bots in the FOV of the given bot
  std::vector<Raven_Bot*> GetAllBotsInFOV(const Raven_Bot* pBot)const;

  //returns true if the second bot is unobstructed by walls and in the field
  //of view of the first.
  bool        isSecondVisibleToFirst(const Raven_Bot* pFirst,
                                     const Raven_Bot* pSecond)const;

  //returns true if the ray between A and B is unobstructed.
  bool        isLOSOkay(Vector2D A, Vector2D B)const;

  //starting from the given origin and moving in the direction Heading this
  //method returns the distance to the closest wall
  double       GetDistanceToClosestWall(Vector2D Origin, Vector2D Heading)const;

  
  //returns the position of the closest visible switch that triggers the
  //door of the specified ID
  Vector2D GetPosOfClosestSwitch(Vector2D botPos, unsigned int doorID)const;

  //given a position on the map this method returns the bot found with its
  //bounding radius of that position.If there is no bot at the position the
  //method returns NULL
  Raven_Bot*  GetBotAtPosition(Vector2D CursorPos)const;


  void        TogglePause(){m_bPaused = !m_bPaused;}
  
  //this method is called when the user clicks the right mouse button.
  //The method checks to see if a bot is beneath the cursor. If so, the bot
  //is recorded as selected.If the cursor is not over a bot then any selected
  // bot/s will attempt to move to that position.
  void        ClickRightMouseButton(POINTS p);

  //this method is called when the user clicks the left mouse button. If there
  //is a possessed bot, this fires the weapon, else does nothing
  void        ClickLeftMouseButton(POINTS p);

  void        HandleKeyDown(WPARAM wparam);

  //when called will release any possessed bot from user control
  void        ExorciseAnyPossessedBot();
 
  //if a bot is possessed the keyboard is polled for user input and any 
  //relevant bot methods are called appropriately
  void        GetPlayerInput()const;
  void        GetPlayerKey(Vector2D direction)const;
  Raven_Bot*  PossessedBot()const{return m_pSelectedBot;}
  void        ChangeWeaponOfPossessedBot(unsigned int weapon)const;

  
  const Raven_Map* const                   GetMap()const{return m_pMap;}
  Raven_Map* const                         GetMap(){return m_pMap;}
  const std::list<Raven_Bot*>&             GetAllBots()const{return m_Bots;}
  PathManager<Raven_PathPlanner>* const    GetPathManager(){return m_pPathManager;}
  int                                      GetNumBots()const{return m_Bots.size();}
  std::string                              GetVictoryMessage()const { return m_victory_message; }
  
  void  TagRaven_BotsWithinViewRange(BaseGameEntity* pRaven_Bot, double range)
              {TagNeighbors(pRaven_Bot, m_Bots, range);}


  Mode GetMode()const{return m_Mode;}

  CNeuralNet GetModeleApprentissage() { return m_ModeleApprentissage; }


};





#endif