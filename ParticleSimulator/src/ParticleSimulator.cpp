/**
  ******************************************************************************
  * @file    ParticleSimulator.cpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    18 JAN 2025
  * @brief
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

/* Global variables --------------------------------------------------------- */
/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */



/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  Main program
  * @param  _In_ HINSTANCE hInstance
  * @param  _In_opt_ HINSTANCE hPrevInstance
  * @param  _In_ LPSTR lpCmdLine
  * @param  _In_ int nShowCmd
  * @retval int
  */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // Parse command-line arguments
    std::string cmdLine(lpCmdLine);

    if (cmdLine.find("--console") != std::string::npos)
    {
        ShowConsole();
    }

    Engine e;

    Simulation sim(&e);

    sim.engine->Init();

    return 0;
}


/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
