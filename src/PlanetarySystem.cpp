#include "PlanetarySystem.h"

void olcPlanetarySystem::AddPlanet(float pos_x, float pos_y, float vel_x, float vel_y, float radius)
{
    if (radius > 1.0)
    {
        sPlanet p;
        p.px = pos_x;
        p.py = pos_y;
        p.vx = vel_x;
        p.vy = vel_y;
        p.ax = 0.0;
        p.ay = 0.0;
        p.radius = radius;
        p.mass = 1.0 * 4.0 * 3.14159 * radius * radius * radius / 3.0; // density = 1.0

        p.id = vecPlanets.size();
        vecPlanets.emplace_back(p);
    }
}

void olcPlanetarySystem::PlanetCreation()
{
    // Lambda function for vector modulus
    auto modulus = [] (float x, float y)
    {
        return sqrtf(x * x + y * y);
    };

    // Right click to create (hold set size)
    if (GetMouse(1).bPressed)
    {
        vNewPlanetPos = vMouse;
        bPlanetCreate = true;
    }
    else if (GetMouse(1).bHeld && bPlanetCreate)
    {
        fNewPlanetRadius = modulus(vMouse.x - vNewPlanetPos.x, vMouse.y - vNewPlanetPos.y);
        DrawCircle(vNewPlanetPos, fNewPlanetRadius, olc::WHITE);
    }
    else if (GetMouse(1).bReleased && bPlanetCreate)
    {
        fNewPlanetRadius = modulus(vMouse.x - vNewPlanetPos.x, vMouse.y - vNewPlanetPos.y);
        bPlanetCreate = false;
        bSetPlanetVel = true;
    }

    // If size is selected, set initial velocity
    if (bSetPlanetVel && !GetMouse(0).bPressed)
    {
        vNewPlanetVel = vMouse - vNewPlanetPos; // can be scaled
    }
    else if (bSetPlanetVel && GetMouse(0).bPressed)
    {
        // Set new planet
        AddPlanet(vNewPlanetPos.x, vNewPlanetPos.y, -vNewPlanetVel.x, -vNewPlanetVel.y, fNewPlanetRadius);

        vNewPlanetPos = {0.0, 0.0};
        vNewPlanetVel = {0.0, 0.0};
        fNewPlanetRadius = 0.0;
        bSetPlanetVel = false;
        pSelectedPlanet = &vecPlanets.back();
    }
}

void olcPlanetarySystem::ForcesUpdate()
{
    // Lambda function for vector modulus
    auto modulus = [] (float x, float y)
    {
        return sqrtf(x * x + y * y);
    };

    // Reset force vector field
    vecForces.resize(vecPlanets.size());
    for (auto &f : vecForces) { f = olc::vf2d{0.0, 0.0}; }

    // Loop over all interactions (excluding itself)
    for (unsigned int i = 0; i < vecPlanets.size(); i += 1)
    {
        for (unsigned int j = i+1; j < vecPlanets.size(); j += 1)
        {
            sPlanet* planet1 = &vecPlanets[i];
            sPlanet* planet2 = &vecPlanets[j];

            float distance_size = modulus((planet2->px - planet1->px), (planet2->py - planet1->py));
            olc::vf2d versor = {(planet2->px - planet1->px) / distance_size, (planet2->py - planet1->py) / distance_size};
            olc::vf2d force  =  (planet1->mass * planet2->mass / (distance_size * distance_size)) * versor;
            vecForces[i] +=  6.0 * force; // !G = 6.0!
            vecForces[j] += -6.0 * force; // !G = 6.0!
        }
    }
}

void olcPlanetarySystem::HandleCollision()
{
    // Handle merging of the two planets
    // It superposes the first planet of 
    // the colliding pair with a new one 
    // with the summed mass located at 
    // the center of mass

    // Lambda
    auto CirclesOverlap = [] (float x1, float y1, float r1, float x2, float y2, float r2)
    {
        return (((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2)) ? true : false;
    };

    // Loop over all pairs (not itself)
    for (unsigned int i = 0; i < vecPlanets.size(); i += 1)
    {
        for (unsigned int j = i+1; j < vecPlanets.size(); j += 1)
        {
            sPlanet* p1 = &vecPlanets[i];
            sPlanet* p2 = &vecPlanets[j];

            if (CirclesOverlap(p1->px, p1->py, p1->radius, p2->px, p2->py, p2->radius))
            {
                // Inelastic collision merging
                float fMergedMass   = p1->mass + p2->mass;
                float fMergedRadius = cbrt(3.0 * fMergedMass / (4.0 * 3.14159));
                olc::vf2d CenterOfMass   = {(p1->mass * p1->px + p2->mass * p2->px) / fMergedMass,
                                            (p1->mass * p1->py + p2->mass * p2->py) / fMergedMass};
                olc::vf2d MergedVelocity = {(p1->mass * p1->vx + p2->mass * p2->vx) / fMergedMass,
                                            (p1->mass * p1->vy + p2->mass * p2->vy) / fMergedMass};

                // Create new merged planet on place of 'p1'
                p1->px = CenterOfMass.x;
                p1->py = CenterOfMass.y;
                p1->vx = MergedVelocity.x;
                p1->vy = MergedVelocity.y;
                p1->mass = fMergedMass;
                p1->radius = fMergedRadius;
                // AddPlanet(CenterOfMass.x, CenterOfMass.y, MergedVelocity.x, MergedVelocity.y, fMergedRadius);

                // Delete 'p2' (fix counter)
                if (pSelectedPlanet != nullptr && pSelectedPlanet->id == p2->id) { pSelectedPlanet = nullptr; }
                vecPlanets.erase(vecPlanets.begin() + p2->id);
                j -= 1;
                    
                // Reset IDs
                int newID = 0;
                for (auto &planet : vecPlanets)
                {
                    planet.id = newID;
                    newID += 1;
                }
            }
        }
    }
}

void olcPlanetarySystem::BoundaryCollision()
{
    bool bBoundaryCut = false;
    for (unsigned int i = 0; i < vecPlanets.size(); i += 1)
    {
        if (bBoundaryCut) // Reset previous delete item
        {
            i -= 1;
            vecPlanets[i].id = i;
        }
        
        bBoundaryCut = false;
        
        if (fabs(vecPlanets[i].px) > fBoundaryCutDist || fabs(vecPlanets[i].py) > fBoundaryCutDist)
        {
            if (pSelectedPlanet != nullptr && pSelectedPlanet->id == (int)i) 
            { 
                pSelectedPlanet = nullptr;
                // vWorldOffset = {0.0, 0.0};
            }
            bBoundaryCut = true;
            vecPlanets.erase(vecPlanets.begin() + i);
        }
    }
}

bool olcPlanetarySystem::OnUserCreate()
{
    // Initialize with a single planet
    AddPlanet(ScreenWidth() / 2, ScreenHeight() / 2, 0.0, 0.0, 60.0);

    AddPlanet(ScreenWidth() / 2 + 100.0, ScreenHeight() / 2, 0.0, -220.0, 4.0);
    AddPlanet(ScreenWidth() / 2 + 200.0, ScreenHeight() / 2, 0.0, -150.0, 12.0);
    AddPlanet(ScreenWidth() / 2 + 300.0, ScreenHeight() / 2, 0.0, -100.0, 5.0);
    AddPlanet(ScreenWidth() / 2 + 400.0, ScreenHeight() / 2, 0.0,  -70.0, 5.0);

    // World camera translation
    vWorldOffset = {0.0, 0.0};

    return true;
}

bool olcPlanetarySystem::OnUserUpdate(float fElapsedTime)
{
    // Lambda functions
    auto HoverOverCircle = [] (float x1, float y1, float r1, float px, float py)
    {
        return fabs((x1 - px) * (x1 - px) + (y1 - py) * (y1 - py) < (r1 * r1));
    };

    // Get mouse position
    vMouse = {GetMouseX(), GetMouseY()};

    // Handle planet creation and initialization
    PlanetCreation();

    // Select planet
    if (GetMouse(0).bPressed && !bSetPlanetVel)
    {
        pSelectedPlanet = nullptr;
        for (auto &planet : vecPlanets)
        {
            if (HoverOverCircle(planet.px, planet.py, planet.radius, vMouse.x, vMouse.y))
            {
                pSelectedPlanet = &planet;
                // vWorldOffset = {planet.px, planet.py};
                break;
            }
        }
    }

    // Delete Planet
    if (GetKey(olc::Key::DEL).bPressed || GetKey(olc::Key::BACK).bPressed)
    {
        if (pSelectedPlanet != nullptr)
        {
            vecPlanets.erase(vecPlanets.begin() + pSelectedPlanet->id);
            pSelectedPlanet = nullptr;
            // vWorldOffset = {0.0, 0.0};
        }
        // Reset id
        int newID = 0;
        for (auto &planet : vecPlanets)
        {
            planet.id = newID;
            newID += 1;
        }
    }

    // Calculate Forces on each body
    ForcesUpdate();

    // Update positions (integrate)
    for (auto &planet: vecPlanets)
    {
        // Record position before update
        planet.ox = planet.px;
        planet.oy = planet.py;
 
        // Acceleration
        planet.ax = vecForces[planet.id].x / planet.mass;
        planet.ay = vecForces[planet.id].y / planet.mass;
 
        // Planet dynamic
        planet.vx += planet.ax * fElapsedTime;
        planet.vy += planet.ay * fElapsedTime;
        planet.px += planet.vx * fElapsedTime;
        planet.py += planet.vy * fElapsedTime;

        // ....Eppur si muove
    }

    // Surprisingly... handle collisions (merge collided planets)
    HandleCollision();

    // Delete planets out of boundaries
    BoundaryCollision();

    /********************/
    /*      Render      */
    /********************/
    Clear(olc::Pixel{20, 20, 20});

    // Instructions
    DrawString({5, 5}, "(Hold) Right mouse button to set planet size", olc::WHITE, 2);
    DrawString({5, 5 + 20}, "Left mouse click to set velocity", olc::WHITE, 2);

    // Draw Planets
    for (auto planet : vecPlanets)
    {
        olc::vi2d position = {(int)(planet.px - vWorldOffset.x), (int)(planet.py - vWorldOffset.y)};
        float vel_ang = atan2f(planet.vy, planet.vx);
        olc::vi2d direction = olc::vi2d{(int) (planet.px + planet.radius * cosf(vel_ang)), 
                                        (int) (planet.py + planet.radius * sinf(vel_ang))};
        FillCircle(position, (int)planet.radius, olc::WHITE);
        DrawLine(position, direction, olc::BLUE);
    }

    // Draw planet to create
    if (bPlanetCreate)
    {
        DrawCircle(vNewPlanetPos, fNewPlanetRadius, olc::WHITE);
    }
    // Draw selected initial vel
    if (bSetPlanetVel)
    {
        DrawCircle(vNewPlanetPos, fNewPlanetRadius, olc::WHITE);
        DrawLine(vNewPlanetPos, vNewPlanetPos + vNewPlanetVel, olc::RED);
    }
    if (pSelectedPlanet != nullptr && !bPlanetCreate)
    {
        DrawCircle(olc::vf2d{pSelectedPlanet->px, pSelectedPlanet->py}, pSelectedPlanet->radius + 5, olc::YELLOW);
        DrawCircle(olc::vf2d{pSelectedPlanet->px, pSelectedPlanet->py}, pSelectedPlanet->radius + 6, olc::YELLOW);
        // Draw velocity vector
        // Draw force    vector
        // Draw traced trajectory (vector saving old positions since pSelectedPlanet was setteld)
    }

    // Debug info
    DrawString({5, ScreenHeight() - 5 - 2 * GetTextSize(" ").y}, "Planets: " + std::to_string(vecPlanets.size()), olc::WHITE, 2);
    
    // Easter Egg
    DrawString({ScreenWidth() - 5 - GetTextSize("...Eppur si muove").x, 
                ScreenHeight() - 5 - GetTextSize(" ").y}, "...eppur si muove", olc::WHITE, 1);

    return true;
}