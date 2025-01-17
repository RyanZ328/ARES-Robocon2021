/**
  * ****************************(C) COPYRIGHT 2021 ARES@SUSTech****************************
  *                                 ___     ____   ______ _____
  *                                /   |   / __ \ / ____// ___/
  *                               / /| |  / /_/ // __/   \__ \
  *                              / ___ | / _, _// /___  ___/ / 
  *                             /_/  |_|/_/ |_|/_____/ /____/ 
  *                        Association of Robotics Engineers at SUSTech
  * 
  * @file     user_init.c
  * @author   Zou Yuanhao (11810102@mail.sustech.edu.cn)
  * @brief    硬件初始化代码，在系统开始前启动
  * @version  0.1
  * @date     2021-03-09
  * 
  * ****************************(C) COPYRIGHT 2021 ARES@SUSTech****************************
  */
#include "main.h"
#include "sbus.h"
#include "tim.h"
#include "can_comm.h"
#include "ops.h"
#include "can_encoder.h"
#include "gpio.h"
#include "can.h"
#include "interboard_spi.h"
#include "monitor_task.h"

void User_Init(void) {
  //关闭流水灯
  FLOWLED_OFF(0);
  FLOWLED_OFF(1);
  FLOWLED_OFF(2);
  FLOWLED_OFF(3);
  FLOWLED_OFF(4);
  FLOWLED_OFF(5);
  FLOWLED_OFF(6);
  FLOWLED_OFF(7);
  //初始化SBUS遥控器
  Sbus_init();
  //初始化OPS-9
  Ops_init();
  //初始化精确计时器
  HAL_TIM_Base_Start_IT(&htim6);
  //初始化加热电阻
  //HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  //初始化PWM输入捕获
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);
  //四个GPIO口上电
  HAL_GPIO_WritePin(PWR0_GPIO_Port, PWR0_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(PWR1_GPIO_Port, PWR1_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(PWR2_GPIO_Port, PWR2_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(PWR3_GPIO_Port, PWR3_Pin, GPIO_PIN_SET);
  //初始化内部CAN
  CAN_Start(INTERNAL_CAN1);
  CAN_Start(INTERNAL_CAN2);
  //初始化板间通信
  //Interboard_init();
  //CAN_Start(EXTERNAL_MCP2515);
  HAL_Delay(5);
  
}
