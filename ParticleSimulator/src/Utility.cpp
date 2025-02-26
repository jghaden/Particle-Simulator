/**
  ******************************************************************************
  * @file    Utility.cpp
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

#include "Utility.hpp"

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
  * @brief  Exit program
  * @param  code
  * @retval None
  */
void Exit(int code)
{
    LOG_INFO("Exiting program: %d", code);

    exit(code);
}


/**
  * @brief  Show console window with ANSI support
  * @param  None
  * @retval None
  */
void ShowConsole()
{
    // Allocate a console
    AllocConsole();

    // Redirect standard output to the console
    FILE* file = nullptr;

    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
    freopen_s(&file, "CONIN$", "r", stdin);

    // Enable ANSI escape codes
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
