#include "hw_specific.h"
#include "stm32h7xx.h"
#include "thread.h"

namespace RODOS {
/**
 * hwInitContext()
 * Builds an initial exception-return stack frame for a new thread.
 *
 * Cortex-M7 hardware automatically stacks/unstacks xPSR, PC, LR, R12, R3-R0
 * on exception entry/exit. We fake that frame so the first "return from ISR"
 * lands in threadStartupWrapper(object).
 *
 *  Stack layout built here (top = lowest address, grows downward):
 *  [ EXC_RETURN sentinel ] <- returned as initial SP
 *  [ r4 – r11            ]   (callee-saved, zeroed)
 *  ---- hardware auto-stacked frame (8 words) ----
 *  [ r0  = object        ]
 *  [ r1  = 0             ]
 *  [ r2  = 0             ]
 *  [ r3  = 0             ]
 *  [ r12 = 0             ]
 *  [ LR  = 0xFFFFFFFD    ]  EXC_RETURN: return to Thread mode, use PSP
 *  [ PC  = threadStartupWrapper ]
 *  [ xPSR = 0x01000000   ]  Thumb bit set
 */
long* hwInitContext(long* stack, void* object) {
    // Stack pointer starts at top of allocated region; move down for alignment
    stack--;   // padding to keep 8-byte alignment after the frame

    // --- Hardware exception frame (stacked by CPU on interrupt entry) ---
    *stack-- = 0x01000000UL;                        // xPSR  (Thumb bit)
    *stack-- = (long)threadStartupWrapper;          // PC
    *stack-- = 0xFFFFFFFDUL;                        // LR    (EXC_RETURN: PSP, Thread)
    *stack-- = 0;                                   // R12
    *stack-- = 0;                                   // R3
    *stack-- = 0;                                   // R2
    *stack-- = 0;                                   // R1
    *stack-- = (long)object;                        // R0    (1st argument)

    // --- Software-saved callee registers R4–R11 ---
    for (int i = 0; i < 8; i++) {
        *stack-- = 0;
    }

    // Return pointer to current top-of-stack (what SP will be restored to)
    return stack + 1;
}
}

extern "C" {

	void Error_Handler() {
		while (1) { }
	}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock)
 *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks)
 *            AHB Prescaler                  = 2
 *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
 *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
 *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
 *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 5
 *            PLL_N                          = 160
 *            PLL_P                          = 2
 *            PLL_Q                          = 4
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Flash Latency(WS)              = 4
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;   // 5 MHz
  RCC_OscInitStruct.PLL.PLLN = 192; // 960 MHz
  RCC_OscInitStruct.PLL.PLLP = 2;   // PLLCLK = SYSCLK = 480 MHz
  RCC_OscInitStruct.PLL.PLLQ = 116; // PLL1Q used for FDCAN = 10 MHz
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure bus clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_D3PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_PWREx_EnableUSBVoltageDetector();

  /*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
          (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
          The I/O Compensation Cell activation  procedure requires :
        - The activation of the CSI clock
        - The activation of the SYSCFG clock
        - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR

          To do this please uncomment the following code
  */
  __HAL_RCC_CSI_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  HAL_EnableCompensationCell();
}

/**
 * @brief  Configure the MPU attributes
 * @param  None
 * @retval None
 */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * hwInit()
 * Initializes core STM32H747 hardware via HAL:
 * - System clock, SysTick, FPU, cache.
 */
void hwInit() {
	/*System clock, voltage scaling configuration done only once by Cortex-M7*/

  // Configure the MPU attributes
  MPU_Config();

  // Enable the CPU Cache
  SCB_EnableICache();
  SCB_EnableDCache();

	/* STM32H7xx HAL library initialization:
			- Systick timer is configured by default as source of time base, but user
				can eventually implement his proper time base source (a general purpose
				timer for example or other time source), keeping in mind that Time base
				duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
				handled in milliseconds basis.
			- Set NVIC Group Priority to 4
			- Low Level Initialization
	*/
  HAL_Init();

  /* Configure the system clock to 400 MHz */
  SystemClock_Config();

	/* Enable CPU2 (Cortex-M4) boot regardless of option byte values */
  HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);
}



void hwResetAndReboot() {
    NVIC_SystemReset();   // Cortex-M CMSIS call
}

void sp_partition_yield() { }   // bare-metal stub
void startIdleThread()    { }   // bare-metal stub

} // extern "C"
