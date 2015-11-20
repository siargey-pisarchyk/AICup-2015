#include "MyStrategy.h"
#include "CommonDefines.h"
#include "Cell.h"
#include "PathUtils.h"
#include "DebugFunctions.h"
#include "UsefullFunctions.h"

using namespace std;

int const BACWARD_DUR = 130;

void MyStrategy::move(const Car& self, const World& world, const Game& game, Move& move)
{

    if (world.getTick() < 300)
        return;
    auto curCell = GetCell(self.getX(), self.getY(), game);
    
    if (m_visitedCells.empty() || m_visitedCells.back() != curCell)
        m_visitedCells.push_back(curCell);
    
    
#ifdef LOG
    if(world.getTick() == 1)
        PrintMap(world.getTilesXY());
#endif
//        move.setEnginePower(1.0);
    
    for (Car const & car: world.getCars())
    {
        if (car.isTeammate())
            continue;
        double dAngleDeg = FDeg(self.getAngleTo(car));
        double dist = self.getDistanceTo(car);
        if (dAngleDeg < 15
            && dist < 1.5 * game.getTrackTileSize())
                move.setThrowProjectile(true);
        
        
        for (int i = 3; i < 8 && i < m_visitedCells.size(); ++i)
        {
            auto carCell = GetCell(car, game);
            if (carCell == m_visitedCells[m_visitedCells.size() - i])
                move.setSpillOil(true);
        }
    }
    
        if (m_bBackwardMove)
            return BackwardMove(self, world, game, move);
        m_ForvardTick++;
        double speedModule = hypot(self.getSpeedX(), self.getSpeedY());


        if (m_ForvardTick > 30 && world.getTick() > 230)
            if (speedModule < 1e-1 && m_dPrevSpeed < 1e-1)
            {
                m_bBackwardMove = true;
                m_BacwardTick = BACWARD_DUR;
                m_BackwardWheelAngle = -self.getWheelTurn();
                return BackwardMove(self, world, game, move);
            }
        m_dPrevSpeed = speedModule;
    
    

    
    Cell finish = {self.getNextWaypointX(), self.getNextWaypointY()};
    Cell start = curCell;
    
    vector<Cell> path = GetClosestPath(world, start, finish);
    
    Cell start2 = finish;
    auto nextWaypointIndex = self.getNextWaypointIndex() + 1;
    if (nextWaypointIndex >= world.getWaypoints().size())
        nextWaypointIndex = 0;
    Cell finish2 = {world.getWaypoints()[nextWaypointIndex][0], world.getWaypoints()[nextWaypointIndex][1]};
    vector<Cell> nextPath = GetClosestPath(world, start2, finish2);
    for (size_t i = 1; i < nextPath.size(); ++i)
        path.push_back(nextPath[i]);
   
    
    if (path.empty())
        path.push_back({0,0});
    while (path.size() < 3)
        path.push_back(path.back());
            
    if (path.size() >= 6)
    {
        double target4X = (path[4].m_x + 0.5) * game.getTrackTileSize();
        double target4Y = (path[4].m_y + 0.5) * game.getTrackTileSize();
        if (self.getAngleTo(target4X, target4Y) < 20*PI/180)
            move.setUseNitro(true);
    }
    
    
    Cell nextTarget = path[1];
    Cell nextTarget2 = path[2];
    
#ifdef LOG
    cout << "s " << start << " wp " << finish <<  " wp2 " << finish << endl << "path ";
    for (auto cell: path)
        cout << cell << " : ";
    cout << " target " << nextTarget << endl;
#endif

    
    double nextWaypointX = (nextTarget.m_x + 0.5) * game.getTrackTileSize();
    double nextWaypointY = (nextTarget.m_y + 0.5) * game.getTrackTileSize();

    double nextWaypointX2 = (nextTarget2.m_x + 0.5) * game.getTrackTileSize();
    double nextWaypointY2 = (nextTarget2.m_y + 0.5) * game.getTrackTileSize();

    double k= 0.4;
    nextWaypointX = (k*nextWaypointX + (1-k)*nextWaypointX2);
    nextWaypointY = (k*nextWaypointY + (1-k)*nextWaypointY2);
    
    double cornerTileOffset = 0.15 * game.getTrackTileSize();
    
//    switch (GetCellType(world.getTilesXY(), nextTarget))
//    {
//        case LEFT_TOP_CORNER:
//            nextWaypointX += cornerTileOffset;
//            nextWaypointY += cornerTileOffset;
//            break;
//        case RIGHT_TOP_CORNER:
//            nextWaypointX -= cornerTileOffset;
//            nextWaypointY += cornerTileOffset;
//            break;
//        case LEFT_BOTTOM_CORNER:
//            nextWaypointX += cornerTileOffset;
//            nextWaypointY -= cornerTileOffset;
//            break;
//        case RIGHT_BOTTOM_CORNER:
//            nextWaypointX -= cornerTileOffset;
//            nextWaypointY -= cornerTileOffset;
//            break;
//    }

    double angleToWaypoint = self.getAngleTo(nextWaypointX, nextWaypointY);

    
    move.setWheelTurn(angleToWaypoint * 32.0 / PI);
//    move.setWheelTurn(1);
    move.setEnginePower(1);
    
    //            if (speedModule * speedModule * abs(angleToWaypoint) > 2.5 * 2.5 * PI)
    //            {
    //                move.setBrake(true);
    //            }

}


void MyStrategy::BackwardMove(const model::Car& self, const model::World& world,
                  const model::Game& game, model::Move& move)
{
    m_BacwardTick--;
    if (m_BacwardTick < 0)
    {
        m_bBackwardMove = false;
        move.setEnginePower(0.75);
        move.setWheelTurn(0);
        m_ForvardTick = 0;
    }
    else
    {
         move.setEnginePower(-1);
         if (m_BacwardTick < BACWARD_DUR / 3)
         {
             move.setEnginePower(1);
             move.setWheelTurn(0);
             
         }
         else
            move.setWheelTurn(0);
        if (m_BacwardTick < BACWARD_DUR / 6)
            move.setBrake(true);
            
    }
}

MyStrategy::MyStrategy() { }
