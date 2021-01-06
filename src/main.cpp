/* N-Body interactive planetary */
/*          system              */
/*                              */
/* Made in (06/01/2021)         */
/*                              */
/* To run the program, within a */
/* terminal compile it with:    */ 
/*                              */
/* make clean && make           */
/*                              */
/* And run it by typing:        */
/*                              */
/* vblank_mode=0 ./application  */
/*                              */

#include "PlanetarySystem.h"

int main()
{
    // App window settings
    olcPlanetarySystem application;
    int32_t ScreenWidth  = 1240;
    int32_t ScreenHeight = 920;
 
    // Main game loop  
    if (application.Construct(ScreenWidth, ScreenHeight, 1, 1))
    {
        application.Start();
    }

    return 0;
}