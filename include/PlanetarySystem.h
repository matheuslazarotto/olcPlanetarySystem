#include "olcPixelGameEngine.h"
#include <vector>

struct sPlanet
{
    float px, py;
    float vx, vy;
    float ax, ay;
    float ox, oy; // position storage
    float radius;
    float mass;

    int id;
};

class olcPlanetarySystem : public olc::PixelGameEngine
{
public:
    olcPlanetarySystem() { sAppName = "Planetary System"; }

private:
    float fNewPlanetRadius;
    olc::vf2d vNewPlanetPos;
    olc::vf2d vNewPlanetVel;
    olc::vf2d vWorldOffset;
    olc::vi2d vMouse;
    std::vector<sPlanet> vecPlanets;
    std::vector<olc::vf2d> vecForces;
    sPlanet *pSelectedPlanet = nullptr;
    float fBoundaryCutDist = 3500.0;
    bool bPlanetCreate = false;
    bool bSetPlanetVel = false;

    void AddPlanet(float pos_x, float pos_y, float vel_x, float vel_y, float radius);
    void BoundaryCollision();
    void HandleCollision();
    void PlanetCreation();
    void ForcesUpdate();

protected:  
    bool OnUserCreate() override;
    bool OnUserUpdate(float fElapsedTime) override;
};
