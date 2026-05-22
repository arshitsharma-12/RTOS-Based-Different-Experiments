/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* Definitions for UltrasonicTask */
osThreadId_t UltrasonicTaskHandle;
const osThreadAttr_t UltrasonicTask_attributes = {
  .name = "UltrasonicTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

/* Definitions for AlertTask */
osThreadId_t AlertTaskHandle;
const osThreadAttr_t AlertTask_attributes = {
  .name = "AlertTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for BuzzerTask */
osThreadId_t BuzzerTaskHandle;
const osThreadAttr_t BuzzerTask_attributes = {
  .name = "BuzzerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Definitions for DistanceQueue */
osMessageQueueId_t DistanceQueueHandle;
const osMessageQueueAttr_t DistanceQueue_attributes = {
  .name = "DistanceQueue"
};

/* USER CODE BEGIN PV */

uint16_t distance = 0;
uint16_t receivedDistance = 0;

volatile uint32_t alertDelay = 500;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void StartUltrasonicTask(void *argument);
void StartAlertTask(void *argument);
void StartBuzzerTask(void *argument);

/* USER CODE BEGIN PFP */

void delay_us(uint16_t us);
uint16_t Read_Distance(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void delay_us(uint16_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000);

  while((DWT->CYCCNT - start) < ticks);
}

uint16_t Read_Distance(void)
{
  uint32_t time = 0;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  delay_us(2);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
  delay_us(10);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET);

  DWT->CYCCNT = 0;

  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);

  time = DWT->CYCCNT / (HAL_RCC_GetHCLKFreq() / 1000000);

  return (time * 0.034) / 2;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Create the queue */
  DistanceQueueHandle = osMessageQueueNew(16,
                                          sizeof(uint16_t),
                                          &DistanceQueue_attributes);

  /* Create the threads */
  UltrasonicTaskHandle = osThreadNew(StartUltrasonicTask,
                                     NULL,
                                     &UltrasonicTask_attributes);

  AlertTaskHandle = osThreadNew(StartAlertTask,
                                NULL,
                                &AlertTask_attributes);

  BuzzerTaskHandle = osThreadNew(StartBuzzerTask,
                                 NULL,
                                 &BuzzerTask_attributes);

  /* Start scheduler */
  osKernelStart();

  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;r

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct,
                          FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA,
                    GPIO_PIN_0 | GPIO_PIN_5,
                    GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB,
                    GPIO_PIN_0,
                    GPIO_PIN_RESET);

  /* PA0 = TRIG , PA5 = BUZZER */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PA1 = ECHO */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PB0 = LED */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief Function implementing the UltrasonicTask thread.
  * @param argument: Not used
  * @retval None
  */
void StartUltrasonicTask(void *argument)
{
  for(;;)
  {
    distance = Read_Distance();

    osMessageQueuePut(DistanceQueueHandle,
                      &distance,
                      0,
                      0);

    osDelay(100);
  }
}

/**
* @brief Function implementing the AlertTask thread.
* @param argument: Not used
* @retval None
*/
void StartAlertTask(void *argument)
{
  for(;;)
  {
    if(osMessageQueueGet(DistanceQueueHandle,
                         &receivedDistance,
                         NULL,
                         osWaitForever) == osOK)
    {
      if(receivedDistance < 20)
      {
        alertDelay = 100;
      }

      else if(receivedDistance < 50)
      {
        alertDelay = 300;
      }

      else
      {
        alertDelay = 700;
      }
    }
  }
}

/**
* @brief Function implementing the BuzzerTask thread.
* @param argument: Not used
* @retval None
*/
void StartBuzzerTask(void *argument)
{
  for(;;)
  {
    HAL_GPIO_WritePin(GPIOA,
                      GPIO_PIN_5,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOB,
                      GPIO_PIN_0,
                      GPIO_PIN_SET);

    osDelay(alertDelay);

    HAL_GPIO_WritePin(GPIOA,
                      GPIO_PIN_5,
                      GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB,
                      GPIO_PIN_0,
                      GPIO_PIN_RESET);

    osDelay(alertDelay);
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();

  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}

#endif
