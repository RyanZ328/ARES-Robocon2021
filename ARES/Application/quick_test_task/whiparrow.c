/**
  * ****************************(C) COPYRIGHT 2021 ARES@SUSTech****************************
  *                                 ___     ____   ______ _____
  *                                /   |   / __ \ / ____// ___/
  *                               / /| |  / /_/ // __/   \__ \
  *                              / ___ | / _, _// /___  ___/ / 
  *                             /_/  |_|/_/ |_|/_____/ /____/ 
  *                        Association of Robotics Engineers at SUSTech
  * 
  * @file     quick_test_task.c
  * @author   Zou Yuanhao (11810102@mail.sustech.edu.cn)
  * @brief    
  * @version  0.1
  * @date     2021-03-14
  * 
  * ****************************(C) COPYRIGHT 2021 ARES@SUSTech****************************
  */
#include "quick_test_task.h"
#include "RM_motor.h"
#include "feedback_task.h"
#include "quintic_trajectory.h"

static uint32_t last_wake;
uint8_t         zeroed   = 0;
int32_t         timebase = 0;
static fp32     set_angle;
static fp32     set_rads;
static fp32     setrpm;
Motor *         test_motor[2];

PID_ControllerParam        whip_speed_pidparam = {.general  = PIDPOS_CONTROLLER,
                                           .Int_type = BACK_CALCULATION_INT,
                                           .kP       = 19.0f,
                                           .kI       = 0.0f,
                                           .kD       = 0.0f,
                                           .kB       = 0.03f,
                                           .max_Iout = 5000,
                                           .N        = 45};
PID_Controller             whip_angle_pid;
PID_ControllerParam        whip_angle_pid_param   = {.general  = PIDPOS_CONTROLLER,
                                            .Int_type = BACK_CALCULATION_INT,
                                            .kP       = 3000.0f,
                                            .kI       = 0.0f,
                                            .kD       = 0.0f,
                                            .kB       = 0.3f,
                                            .max_Iout = 5000,
                                            .N        = 70};
static fp32                whip_angle_O_Hlim      = 8000;
static fp32                whip_angle_O_Llim      = -8000;
static fp32                whip_angle_I_loop_Llim = 0;
static fp32                whip_angle_I_loop_Hlim = 0;
static ControllerConstrain whip_angle_constrain   = {.I_loop_Hlim = &whip_angle_I_loop_Hlim,
                                                   .I_loop_Llim = &whip_angle_I_loop_Llim,
                                                   .O_Hlim      = &whip_angle_O_Hlim,
                                                   .O_Llim      = &whip_angle_O_Llim};

QuinticTraj whip_traj;

static PID_ControllerParam dir_speed_pidparam = {.general  = PIDPOS_CONTROLLER,
                                                 .Int_type = BACK_CALCULATION_INT,
                                                 .kP       = 1.0f,
                                                 .kI       = 1.0f,
                                                 .kD       = 0.0f,
                                                 .kB       = 0.03f,
                                                 .max_Iout = 5000,
                                                 .N        = 45};

static PID_ControllerParam dir_angle_pidparam = {.general  = PIDPOS_CONTROLLER,
                                                 .Int_type = BACK_CALCULATION_INT,
                                                 .kP       = 1.0f,
                                                 .kI       = 0.0f,
                                                 .kD       = 0.0f,
                                                 .kB       = 0.03f,
                                                 .max_Iout = 5000,
                                                 .N        = 45};

static fp32       ag;
MotorInstructType doubleMotorCtrl0(Motor *motor, Controller *controller) {
  ag       = (2 * PI * motor->status.cumulativeTurn + motor->status.angle - motor->status.zero) / M3508_REDUCTION_RATIO;
  fp32 rpm = M3508_REDUCTION_RATIO * 1000 * 9.5492965855137201461330258023509f * set_rads +
             controllerUpdate(controller, &set_angle, &ag, NULL);
  ((RM_Motor *)motor)->set_current =
      controllerUpdate((Controller *)&((RM_Motor *)motor)->speed_pid, &rpm, &motor->status.speed, NULL);
  return INSTRUCT_CURRENT;
}
MotorInstructType doubleMotorCtrl1(Motor *motor, Controller *controller) {
  (void)controller;
  ((RM_Motor *)motor)->set_current = -((RM_Motor *)test_motor[0])->set_current;
  return INSTRUCT_CURRENT;
}

void quick_test_task_diabled() {
  vTaskDelay(200);

  quinticTraj_init(&whip_traj, DEG2RAD(-30.0f), -0.002, -0.0001, DEG2RAD(-200.0f), 0, 0, 500.0);
  while (test_motor[0] == NULL || test_motor[1] == NULL) {
    test_motor[0] = CAN_Find_Motor(RM_MOTOR, INTERNAL_CAN1, 4);
    test_motor[1] = CAN_Find_Motor(RM_MOTOR, INTERNAL_CAN1, 5);
    vTaskDelay(100);
  }

  PID_ControllerInit(&whip_angle_pid, &whip_angle_constrain, &whip_angle_pid_param, 10);

  Motor_SetAltController(test_motor[0], (Controller *)&whip_angle_pid, doubleMotorCtrl0);
  Motor_SetAltController(test_motor[1], NULL, doubleMotorCtrl1);
  Motor_SetSpeedPID(test_motor[0], &whip_speed_pidparam);
  static fp32 temp_kp;
  temp_kp = whip_speed_pidparam.kP;

  Motor_SetSpeedPID()

      for (;;) {
    //if(sbus.channel[9]>0.8f){Motor_SetSpeed(test_motor[0],setrpm,2);}else{Motor_SetSpeed(test_motor[0],100,2);}
    last_wake = xTaskGetTickCount();
    if (zeroed) {
      if (sbus.channel[9] > 0.8f && timebase <= 0) {
        timebase = 2200;
        quinticTraj_start(&whip_traj);

      } else if (timebase > 0) {
        timebase--;
        quinticTraj_calc(&whip_traj, TRAJ_DIR_FORWARD, &set_angle, &set_rads, NULL);

      } else if (timebase == 0) {
        quinticTraj_start(&whip_traj);
        timebase--;
        quinticTraj_calc(&whip_traj, TRAJ_DIR_BACKWARD, &set_angle, &set_rads, NULL);
      } else if (timebase < 0) {
        quinticTraj_calc(&whip_traj, TRAJ_DIR_BACKWARD, &set_angle, &set_rads, NULL);
      }
      Motor_AltControl(test_motor[0], 4);
      Motor_AltControl(test_motor[1], 4);
    } else {

      whip_speed_pidparam.kP = 2.0f;
      Motor_SetSpeed(test_motor[0], 1000, 2);
      Motor_SetCurrent(test_motor[1], 0, 2);
      if (test_motor[0]->status.speed == 0 && test_motor[0]->status.current != 0) {
        ++timebase;
      } else {
        timebase = 0;
      }
      if (timebase > 500) {
        timebase = -1;
        Motor_Zero(test_motor[0]);
        Motor_Zero(test_motor[1]);
        zeroed                 = 1;
        whip_speed_pidparam.kP = temp_kp;
      }
    }
    setrpm = M3508_REDUCTION_RATIO * 1000.0f * 9.5492965855137201461330258023509f * set_rads;
    vTaskDelayUntil(&last_wake, 1);
  }
}
