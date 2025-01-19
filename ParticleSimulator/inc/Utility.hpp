/**
  ******************************************************************************
  * @file    Simulation.hpp
  * @author  Josh Haden
  * @version V0.0.1
  * @date    18 JAN 2025
  * @brief   Header for Simulation.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

  /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UTILITY_HPP
#define __UTILITY_HPP

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define LOG_RESET				 "\033[0m"
#define LOG_BLUE				 39
#define LOG_GREEN				 118
#define LOG_DARK_RED			 124
#define LOG_RED					 196
#define LOG_MAGENTA				 200
#define LOG_YELLOW				 226

/* Exported macro ------------------------------------------------------------*/

#define SET_FG_COLOR(id)		 printf("\033[38;5;%dm", (id))
#define SET_BG_COLOR(id)		 printf("\033[48;5;%dm", (id))

#define LOG_DEBUG(format, ...)	 SET_FG_COLOR(LOG_MAGENTA);	 printf("[DEBUG] "	 LOG_RESET format "\n", ##__VA_ARGS__)
#define LOG_INFO(format, ...)	 SET_FG_COLOR(LOG_BLUE);	 printf("[INFO] "	 LOG_RESET format "\n", ##__VA_ARGS__)
#define LOG_SUCCESS(format, ...) SET_FG_COLOR(LOG_GREEN);	 printf("[SUCCESS] " LOG_RESET format "\n", ##__VA_ARGS__)
#define LOG_WARN(format, ...)	 SET_FG_COLOR(LOG_YELLOW);	 printf("[WARN] "	 LOG_RESET format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...)	 SET_FG_COLOR(LOG_DARK_RED); printf("[ERROR] "	 LOG_RESET format "\n", ##__VA_ARGS__)
#define LOG_FATAL(format, ...)   SET_FG_COLOR(LOG_RED);		 printf("[FATAL] "	 LOG_RESET format "\n", ##__VA_ARGS__)

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void ShowConsole();

/* Class definition --------------------------------------------------------- */




#endif /* __UTILITY_HPP */

/************************END OF FILE************************/