/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    MODE_NORMAL = 0,
    MODE_LINE_TRACKING
} OperationMode;

typedef struct {
    float Kp;     
    float Ki;       
    float Kd;       
    float integral; 
    float prev_error; 
    float output;  
    float max_output;
} PID_Controller;

typedef struct {
    TIM_HandleTypeDef* timer;
    int32_t prev_count;
    int32_t total_count;
    float speed_rpm;    
    float speed_pulse;  
    PID_Controller pid;
} Encoder_Motor;

int status=0;
int key_1,key_2;
int speed;
uint8_t key1_pressed = 0;
uint8_t key2_pressed = 0;
uint8_t key3_pressed = 0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SPEED_LOW 200
#define SPEED_HIGH 800
#define SPEED_TRACKING 400 

#define PULSES_PER_REV 11.0f     
#define GEAR_RATIO 30.0f          
#define TOTAL_PPR (PULSES_PER_REV * GEAR_RATIO * 4) 

// PID
#define MOTOR_PID_KP 0.8f
#define MOTOR_PID_KI 0.05f
#define MOTOR_PID_KD 0.1f
#define MOTOR_PID_MAX_OUTPUT 1000.0f

#define GRAY_THRESHOLD 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t speed_mode = 0; 
uint8_t dir_mode = 0;	
OperationMode op_mode = MODE_NORMAL;

Encoder_Motor left_motor = {
    .timer = &htim5,
    .prev_count = 0,
    .total_count = 0,
    .speed_rpm = 0,
    .speed_pulse = 0
};

Encoder_Motor right_motor = {
    .timer = &htim3,
    .prev_count = 0,
    .total_count = 0,
    .speed_rpm = 0,
    .speed_pulse = 0
};

float target_speed_left = 0;
float target_speed_right = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LineTrackingControl(void);
void PID_Init(PID_Controller* pid, float Kp, float Ki, float Kd, float max_output);
float PID_Update(PID_Controller* pid, float error, float dt);
void Encoder_Update(Encoder_Motor* motor);
void Motor_SpeedControl(Encoder_Motor* motor, float target_speed, uint8_t dir, uint8_t channel);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Motor_Set(uint8_t dir , uint16_t speed)
{
	__HAL_TIM_SET_COMPARE(&htim8 , TIM_CHANNEL_1 , speed);		
	__HAL_TIM_SET_COMPARE(&htim8 , TIM_CHANNEL_2 , speed);
	if(dir == 0)	
	{
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_SET);		
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_RESET);	
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);	   
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);		
	}
	else		
	{
		   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6,GPIO_PIN_RESET);	
		   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_7,GPIO_PIN_SET);	
		   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);	   
	     HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);		
	}
}

		void key_read(void)
		{

			key_1=HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1) == GPIO_PIN_SET;
			key_2=HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2) == GPIO_PIN_SET;

		}

uint16_t ReadGraySensors(void)
{
    uint16_t sensor_values = 0;
    
    sensor_values |= (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) << 0);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) << 1);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4) << 2);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) << 3);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) << 4);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) << 5);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7) << 6);
    sensor_values |= (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8) << 7);
    
    return sensor_values;
}

void PID_Init(PID_Controller* pid, float Kp, float Ki, float Kd, float max_output)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0;
    pid->prev_error = 0;
    pid->output = 0;
    pid->max_output = max_output;
}

float PID_Update(PID_Controller* pid, float error, float dt)
{
    float proportional = pid->Kp * error;
    
    pid->integral += error * dt;
    if(pid->integral > pid->max_output) pid->integral = pid->max_output;
    if(pid->integral < -pid->max_output) pid->integral = -pid->max_output;
    float integral = pid->Ki * pid->integral;

    float derivative = pid->Kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;

    pid->output = proportional + integral + derivative;

    if(pid->output > pid->max_output) pid->output = pid->max_output;
    if(pid->output < -pid->max_output) pid->output = -pid->max_output;
    
    return pid->output;
}

void Encoder_Update(Encoder_Motor* motor)
{
    static uint32_t last_time = 0;
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - last_time) / 1000.0f;
    if(dt <= 0) dt = 0.01f;

    int32_t current_count = (int32_t)__HAL_TIM_GET_COUNTER(motor->timer);
    int32_t delta = current_count - motor->prev_count;

    if(delta > 0x7FFF) {
        delta -= 0xFFFF;
    } else if(delta < -0x7FFF) {
        delta += 0xFFFF;
    }
    
    motor->total_count += delta;
    motor->prev_count = current_count;

    if(dt > 0) {
        motor->speed_pulse = delta / dt;
        motor->speed_rpm = (motor->speed_pulse / TOTAL_PPR) * 60.0f;
    }
    
    last_time = current_time;
}

void Motor_SpeedControl(Encoder_Motor* motor, float target_speed, uint8_t dir, uint8_t channel)
{
    Encoder_Update(motor);
    
    static uint32_t last_time = 0;
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - last_time) / 1000.0f;
    if(dt <= 0) dt = 0.01f; 
    last_time = current_time;
    
    float error = target_speed - motor->speed_pulse;
    
    float pid_output = PID_Update(&motor->pid, error, dt);
    
    uint16_t pwm = (uint16_t)fabs(pid_output);
    if(pwm > 1000) pwm = 1000;
    
    if(dir == 0) {
        if(channel == TIM_CHANNEL_1) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_RESET);
        }
        else if(channel == TIM_CHANNEL_2) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
        }
    } else {
        if(channel == TIM_CHANNEL_1) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_SET);
        }
        else if(channel == TIM_CHANNEL_2) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
        }
    }
    
    __HAL_TIM_SET_COMPARE(&htim8, channel, pwm);
}

void LineTrackingControl(void)
{
    uint16_t sensors = ReadGraySensors();
    int16_t error = 0;
    
    if(sensors & 0x01) error -= 4;
    if(sensors & 0x02) error -= 3;
    if(sensors & 0x04) error -= 2;
    if(sensors & 0x08) error -= 1;
    if(sensors & 0x10) error += 1;
    if(sensors & 0x20) error += 2;
    if(sensors & 0x40) error += 3;
    if(sensors & 0x80) error += 4;

    float base_speed = SPEED_TRACKING;
    target_speed_left = base_speed - error * 20.0f;
    target_speed_right = base_speed + error * 20.0f;
    
    target_speed_left = fmaxf(0, fminf(target_speed_left, 1000));
    target_speed_right = fmaxf(0, fminf(target_speed_right, 1000));

    if(target_speed_left > 0) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)target_speed_left);
    } else {
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
    }

    if(target_speed_right > 0) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4   , GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)target_speed_right);
    } else {
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	
	HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	
	PID_Init(&left_motor.pid, MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD, MOTOR_PID_MAX_OUTPUT);
  PID_Init(&right_motor.pid, MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD, MOTOR_PID_MAX_OUTPUT);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_SET)  
      {
          if (!key1_pressed)  
          {
              key1_pressed = 1;
              speed_mode = !speed_mode;
              uint16_t speed = (speed_mode == 0) ? SPEED_LOW : SPEED_HIGH;
              Motor_Set(dir_mode, speed);
              HAL_Delay(50);  
          }
      }
      else
      {
          key1_pressed = 0;  
      }
//
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET) 
      {
          if (!key2_pressed)  
          {
              key2_pressed = 1;
              dir_mode = !dir_mode;
              uint16_t speed = (speed_mode == 0) ? SPEED_LOW : SPEED_HIGH;
              Motor_Set(dir_mode, speed);
              HAL_Delay(50);  
          }
      }
       else
       {
           key2_pressed = 0;  
       }
//
      if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET) 
      {
          if (!key3_pressed)  
          {
              key3_pressed = 1;
              op_mode = (op_mode == MODE_NORMAL) ? MODE_LINE_TRACKING : MODE_NORMAL;
						
              if(op_mode == MODE_LINE_TRACKING) {
                  PID_Init(&left_motor.pid, MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD, MOTOR_PID_MAX_OUTPUT);
                  PID_Init(&right_motor.pid, MOTOR_PID_KP, MOTOR_PID_KI, MOTOR_PID_KD, MOTOR_PID_MAX_OUTPUT);
              }
              HAL_Delay(50);  
          }
      }
      else
      {
          key3_pressed = 0;  
      }
      
      if(op_mode == MODE_LINE_TRACKING) {
          LineTrackingControl();
      }
      
      HAL_Delay(10); 
			 
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */