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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stm32f4xx.h"
#include "lcd_stm32f4.h"
#include "stm32f4xx_it.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// TODO: Add values for below variables
#define NS 256      // Number of samples in LUT
#define TIM2CLK 16000000  // STM Clock frequency: Hint You might want to check the ioc file
#define F_SIGNAL 244  	// Frequency of output analog signal
#define NUM_WAVEFORMS 6
#define DEBOUNCE_DELAY_MS 50
#define DAC_MAX_12B 4095u //max LUT amplitude

char* waveform_names[NUM_WAVEFORMS] = {"Sine", "Sawtooth", "Triangular", "Piano", "Guitar", "Drum"};

uint8_t current_waveform = 0;
uint32_t last_press_time = 0;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim2_ch1;

/* USER CODE BEGIN PV */
// TODO: Add code for global variables, including LUTs
const uint32_t Sin_LUT[NS] = {2047, 2097, 2147, 2198, 2248, 2298, 2347,
	2397, 2446, 2496, 2545, 2593, 2641, 2689, 2737, 2784,
	2831, 2877, 2922, 2968, 3012, 3056, 3100, 3142, 3185,
	3226, 3267, 3307, 3346, 3384, 3422, 3459, 3495, 3530,
	3564, 3597, 3630, 3661, 3692, 3721, 3749, 3777, 3803,
	3829, 3853, 3876, 3898, 3919, 3939, 3957, 3975, 3991,
	4006, 4020, 4033, 4045, 4055, 4064, 4072, 4079, 4085,
	4089, 4092, 4094, 4095, 4094, 4092, 4089, 4085, 4079,
	4072, 4064, 4055, 4045, 4033, 4020, 4006, 3991, 3975,
	3957, 3939, 3919, 3898, 3876, 3853, 3829, 3803, 3777,
	3749, 3721, 3692, 3661, 3630, 3597, 3564, 3530, 3495,
	3459, 3422, 3384, 3346, 3307, 3267, 3226, 3185, 3142,
	3100, 3056, 3012, 2968, 2922, 2877, 2831, 2784, 2737,
	2689, 2641, 2593, 2545, 2496, 2446, 2397, 2347, 2298,
	2248, 2198, 2147, 2097, 2047, 1997, 1947, 1896, 1846,
	1796, 1747, 1697, 1648, 1598, 1549, 1501, 1453, 1405,
	1357, 1310, 1263, 1217, 1172, 1126, 1082, 1038, 994,
	952, 909, 868, 827, 787, 748, 710, 672, 635, 599,
	564, 530, 497, 464, 433, 402, 373, 345, 317, 291,
	265, 241, 218, 196, 175, 155, 137, 119, 103, 88,
	74, 61, 49, 39, 30, 22, 15, 9, 5, 2, 0,
	0, 0, 2, 5, 9, 15, 22, 30, 39, 49, 61,
	74, 88, 103, 119, 137, 155, 175, 196, 218, 241,
	265, 291, 317, 345, 373, 402, 433, 464, 497, 530,
	564, 599, 635, 672, 710, 748, 787, 827, 868, 909,
	952, 994, 1038, 1082, 1126, 1172, 1217, 1263, 1310, 1357,
	1405, 1453, 1501, 1549, 1598, 1648, 1697, 1747, 1796, 1846,
	1896, 1947, 1997};

const uint32_t Saw_LUT[NS] = {0, 16, 32, 48, 64, 80, 96, 112, 128,
	144, 160, 176, 192, 208, 224, 240, 256, 273, 289,
	305, 321, 337, 353, 369, 385, 401, 417, 433, 449,
	465, 481, 497, 513, 529, 546, 562, 578, 594, 610,
	626, 642, 658, 674, 690, 706, 722, 738, 754, 770,
	786, 802, 819, 835, 851, 867, 883, 899, 915, 931,
	947, 963, 979, 995, 1011, 1027, 1043, 1059, 1075, 1092,
	1108, 1124, 1140, 1156, 1172, 1188, 1204, 1220, 1236, 1252,
	1268, 1284, 1300, 1316, 1332, 1348, 1365, 1381, 1397, 1413,
	1429, 1445, 1461, 1477, 1493, 1509, 1525, 1541, 1557, 1573,
	1589, 1605, 1621, 1638, 1654, 1670, 1686, 1702, 1718, 1734,
	1750, 1766, 1782, 1798, 1814, 1830, 1846, 1862, 1878, 1894,
	1911, 1927, 1943, 1959, 1975, 1991, 2007, 2023, 2039, 2055,
	2071, 2087, 2103, 2119, 2135, 2151, 2167, 2184, 2200, 2216,
	2232, 2248, 2264, 2280, 2296, 2312, 2328, 2344, 2360, 2376,
	2392, 2408, 2424, 2440, 2457, 2473, 2489, 2505, 2521, 2537,
	2553, 2569, 2585, 2601, 2617, 2633, 2649, 2665, 2681, 2697,
	2713, 2730, 2746, 2762, 2778, 2794, 2810, 2826, 2842, 2858,
	2874, 2890, 2906, 2922, 2938, 2954, 2970, 2986, 3003, 3019,
	3035, 3051, 3067, 3083, 3099, 3115, 3131, 3147, 3163, 3179,
	3195, 3211, 3227, 3243, 3259, 3276, 3292, 3308, 3324, 3340,
	3356, 3372, 3388, 3404, 3420, 3436, 3452, 3468, 3484, 3500,
	3516, 3532, 3549, 3565, 3581, 3597, 3613, 3629, 3645, 3661,
	3677, 3693, 3709, 3725, 3741, 3757, 3773, 3789, 3805, 3822,
	3838, 3854, 3870, 3886, 3902, 3918, 3934, 3950, 3966, 3982,
	3998, 4014, 4030, 4046, 4062, 4078, 4095};

const uint32_t Triangle_LUT[NS] = {0, 31, 63, 95, 127, 159, 191, 223, 255,
	287, 319, 351, 383, 415, 447, 479, 511, 543, 575,
	607, 639, 671, 703, 735, 767, 799, 831, 863, 895,
	927, 959, 991, 1023, 1055, 1087, 1119, 1151, 1183, 1215,
	1247, 1279, 1311, 1343, 1375, 1407, 1439, 1471, 1503, 1535,
	1567, 1599, 1631, 1663, 1695, 1727, 1759, 1791, 1823, 1855,
	1887, 1919, 1951, 1983, 2015, 2047, 2079, 2111, 2143, 2175,
	2207, 2239, 2271, 2303, 2335, 2367, 2399, 2431, 2463, 2495,
	2527, 2559, 2591, 2623, 2655, 2687, 2719, 2751, 2783, 2815,
	2847, 2879, 2911, 2943, 2975, 3007, 3039, 3071, 3103, 3135,
	3167, 3199, 3231, 3263, 3295, 3327, 3359, 3391, 3423, 3455,
	3487, 3519, 3551, 3583, 3615, 3647, 3679, 3711, 3743, 3775,
	3807, 3839, 3871, 3903, 3935, 3967, 3999, 4031, 4063, 4095,
	4063, 4031, 3999, 3967, 3935, 3903, 3871, 3839, 3807, 3775,
	3743, 3711, 3679, 3647, 3615, 3583, 3551, 3519, 3487, 3455,
	3423, 3391, 3359, 3327, 3295, 3263, 3231, 3199, 3167, 3135,
	3103, 3071, 3039, 3007, 2975, 2943, 2911, 2879, 2847, 2815,
	2783, 2751, 2719, 2687, 2655, 2623, 2591, 2559, 2527, 2495,
	2463, 2431, 2399, 2367, 2335, 2303, 2271, 2239, 2207, 2175,
	2143, 2111, 2079, 2047, 2015, 1983, 1951, 1919, 1887, 1855,
	1823, 1791, 1759, 1727, 1695, 1663, 1631, 1599, 1567, 1535,
	1503, 1471, 1439, 1407, 1375, 1343, 1311, 1279, 1247, 1215,
	1183, 1151, 1119, 1087, 1055, 1023, 991, 959, 927, 895, 863,
	831, 799, 767, 735, 703, 671, 639, 607, 575, 543, 511,
	479, 447, 415, 383, 351, 319, 287, 255, 223, 191, 159,
	127, 95, 63, 31};

const uint16_t Piano_LUT[NS] = {1901, 984, 2131, 1887, 1896, 1979, 1888, 1941, 1872,
		1878, 1909, 1903, 1876, 1965, 1790, 1783, 1842, 1889, 2163,
		1892, 1817, 1731, 1929, 1637, 1778, 1858, 2306, 1820, 1801,
		2746, 1853, 1691, 1901, 1827, 1835, 1910, 1895, 1774, 1872,
		1892, 1918, 1902, 1895, 1923, 1870, 1575, 1891, 1894, 1917,
		1802, 2101, 1756, 1808, 1905, 2483, 2012, 1814, 1809, 1782,
		2083, 1892, 2600, 1790, 1894, 1907, 1616, 2176, 1786, 2125,
		1768, 1908, 1923, 1861, 1846, 1880, 1908, 1876, 1801, 2152,
		1844, 1836, 1743, 2256, 1913, 1838, 2034, 1740, 1596, 2092,
		1987, 1737, 1914, 1917, 1014, 1709, 1768, 2011, 1979, 1874,
		1929, 1900, 1799, 1934, 1903, 1887, 1916, 1900, 1912, 1888,
		2237, 2097, 2028, 1883, 1646, 1509, 2040, 1910, 1843, 1982,
		1849, 1907, 2639, 2286, 1980, 1874, 1605, 1743, 1931, 1863,
		2199, 2081, 1902, 2118, 1922, 1924, 1883, 1911, 1853, 1840,
		1901, 1897, 2116, 1996, 1867, 1809, 1631, 2134, 1861, 1818,
		2120, 1808, 1776, 2254, 1565, 1934, 2074, 1852, 1374, 2165,
		1974, 1893, 1897, 1858, 1881, 1864, 1975, 1883, 1908, 1909,
		1906, 1886, 1883, 1882, 2026, 1818, 1893, 1759, 2185, 2043,
		2254, 898, 1478, 1735, 1847, 2181, 1781, 1393, 1795, 1668,
		1777, 1907, 1971, 702, 2822, 1699, 1882, 2020, 1968, 1943,
		1908, 1980, 1855, 1920, 1907, 2303, 1204, 2014, 1899, 1853,
		1556, 1837, 1847, 1639, 1902, 1901, 1699, 2033, 2150, 1831,
		1884, 2410, 1950, 1725, 1933, 1755, 1804, 1869, 1896, 1951,
		2004, 1873, 1929, 1881, 1887, 1900, 1888, 1631, 1687, 1902,
		1692, 2626, 2130, 1845, 1705, 2289, 2050, 1922, 1921, 723,
		1350, 2255, 1922, 2231, 2199, 2134, 1992
};

const uint16_t Guitar_LUT[NS] = {1970, 2182, 1907, 2239, 1849, 1914, 2075, 1897, 1970,
		2029, 1824, 2213, 1893, 1803, 1598, 1860, 2175, 1849, 1958,
		2057, 1943, 1940, 1980, 1953, 2020, 1978, 2091, 1750, 1968,
		1989, 1967, 1986, 1967, 1827, 2670, 1815, 2173, 1776, 1834,
		2423, 1737, 2200, 1793, 1837, 2419, 1914, 2177, 1493, 1952,
		2118, 1868, 1973, 2014, 1965, 1973, 1943, 1958, 1994, 1925,
		2068, 2105, 1973, 1944, 1972, 1965, 1640, 2069, 1924, 1878,
		2134, 1812, 1968, 2118, 1611, 2120, 2054, 1916, 1816, 2702,
		1807, 1757, 2190, 1965, 1982, 1945, 1975, 1957, 1963, 1972,
		1968, 1998, 2034, 1767, 2053, 1892, 1953, 1985, 1944, 1964,
		1878, 2086, 1863, 1919, 2099, 1852, 2034, 2316, 1969, 1920,
		2045, 1695, 1723, 1710, 1661, 2113, 1902, 2325, 1656, 2076,
		2015, 1742, 2124, 1575, 1783, 2169, 1599, 2356, 1721, 1906,
		1299, 2214, 2222, 1512, 2194, 2192, 1581, 2047, 2207, 1683,
		1914, 2188, 1936, 2077, 2497, 1605, 1842, 2027, 1962, 1935,
		1991, 1984, 1960, 1972, 1869, 2033, 2048, 1917, 2001, 1956,
		1966, 1980, 1818, 1929, 1896, 2356, 1676, 2067, 1952, 1802,
		2304, 1653, 1991, 2094, 1321, 2325, 1791, 2139, 2119, 1930,
		1994, 1961, 1958, 1962, 1983, 1981, 1716, 2149, 1838, 1865,
		2007, 1940, 1966, 1974, 1746, 2037, 2035, 1833, 2057, 1977,
		1901, 2036, 1793, 1808, 1894, 1872, 1671, 2179, 1255, 1969,
		1963, 1941, 1988, 1959, 1968, 2015, 1930, 1976, 1969, 2161,
		1961, 1997, 2071, 1904, 1967, 1983, 1846, 2240, 1705, 2007,
		2151, 1766, 2041, 2051, 1936, 1816, 2055, 1710, 1836, 2437,
		2517, 2306, 2351, 1597, 1925, 2107, 1729, 2445, 1684, 1863,
		2078, 2087, 2416, 1697, 654, 2118, 1997};

const uint16_t Drum_LUT[NS] = {2047, 139, 3442, 2221, 1882, 1687, 2226, 1991, 2024,
		4095, 3246, 2552, 1866, 1755, 2171, 2012, 2048, 3437, 2152,
		2174, 2084, 2045, 2043, 2046, 2053, 730, 924, 1813, 2549,
		1815, 2214, 1951, 2008, 1811, 1963, 2099, 2047, 2059, 2035,
		2044, 2042, 2001, 2196, 2097, 2043, 2049, 2037, 2048, 2044,
		3990, 2226, 2255, 2091, 2049, 2053, 2060, 2063, 1465, 2106,
		2033, 2046, 1439, 1340, 1140, 1968, 256, 878, 2242, 2208,
		2081, 1904, 2071, 2052, 2065, 2171, 2054, 2057, 2054, 2118,
		2046, 2076, 827, 2129, 2209, 2121, 2054, 2056, 2038, 2053,
		3707, 1966, 2087, 1528, 2495, 1816, 2040, 2088, 1554, 2013,
		2079, 2033, 2015, 2052, 2040, 2050, 1872, 2246, 1326, 2328,
		1912, 2175, 1991, 2052, 714, 1536, 2010, 2056, 2028, 2062,
		2028, 2066, 1837, 2149, 2044, 2011, 2908, 2656, 2337, 2252,
		3961, 2130, 1924, 1525, 2279, 2041, 2012, 2081, 4092, 2173,
		2050, 1466, 2297, 1988, 2020, 1995, 2023, 1463, 1941, 1984,
		2026, 2043, 2041, 1997, 0, 2262, 1986, 2227, 1891, 2145,
		1968, 2146, 2179, 2076, 2040, 2053, 2045, 2035, 2048, 2060,
		1821, 2132, 2021, 2089, 2028, 2048, 2052, 1777, 1892, 2045,
		1934, 2030, 2082, 2055, 2061, 2091, 1987, 2034, 2049, 3893,
		1471, 1291, 2136, 1972, 0, 2237, 1979, 2422, 1808, 2135,
		2113, 1917, 2193, 2065, 2077, 2025, 2066, 2067, 2049, 4067,
		2676, 2152, 1968, 2082, 2009, 2060, 2048, 3, 3240, 2409,
		1742, 2077, 2050, 2037, 2118, 1678, 2057, 2005, 2067, 2045,
		2050, 2049, 2050, 1157, 2571, 1150, 2044, 2096, 2172, 2002,
		1978, 4073, 2867, 2371, 2044, 2044, 2035, 2034, 2025, 1853,
		1967, 2070, 2046, 964, 2843, 2726, 1886};

// TODO: Equation to calculate TIM2_Ticks
uint32_t TIM2_Ticks = (uint32_t)(TIM2CLK/(Fsignal*NS)); // How often to write new LUT value (or rather what you need timer 2 to count up to)
uint32_t DestAddress = (uint32_t) &(TIM3->CCR3); // Write LUT TO TIM3->CCR3 to modify PWM duty cycle
uint32_t *waveform_LUTs[NUM_WAVEFORMS] = {Sin_LUT, Saw_LUT, Triangle_LUT, Piano_LUT, Guitar_LUT, Drum_LUT};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void EXTI0_IRQHandler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // TODO: Start TIM3 in PWM mode on channel 3h
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  // TODO: Start TIM2 in Output Compare (OC) mode on channel 1
  htim2.Init.Period = TIM2_Ticks - 1;
  HAL_TIM_Base_Init(&htim2);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, TIM2_Ticks / 2u);
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);

  // TODO: Start DMA in IT mode on TIM2->CH1. Source is LUT and Dest is TIM3->CCR3; start with Sine LUT
  HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t)Sin_LUT, (uint32_t)&TIM3->CCR3, NS);
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);

  // TODO: Write current waveform to LCD(Sine is the first waveform)
  init_LCD();
  lcd_command(CLEAR);
  lcd_putstring("Waveform: ");
  lcd_putstring("Sine");
  // TODO: Enable DMA (start transfer from LUT to CCR)
  __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* TIM2_CH1 DMA Init */
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_tim2_ch1.Instance = DMA1_Stream5;
  hdma_tim2_ch1.Init.Channel = DMA_CHANNEL_3;         // TIM2_CH1 is on channel 3
  hdma_tim2_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH; // Memory -> TIM3->CCR3
  hdma_tim2_ch1.Init.PeriphInc = DMA_PINC_DISABLE;    // Peripheral address fixed
  hdma_tim2_ch1.Init.MemInc = DMA_MINC_ENABLE;        // Memory address increments
  hdma_tim2_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_tim2_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;            // Repeat LUT automatically
  hdma_tim2_ch1.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_tim2_ch1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_tim2_ch1) != HAL_OK)
  {
      Error_Handler();
  }

  /* Link DMA handle to TIM2 handle */
  __HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_CC1], hdma_tim2_ch1);
  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = DAC_MAX_12B;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // -------------------------------
  // LCD pins configuration
  // -------------------------------
  // Configure PC14 (RS) and PC15 (E) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Configure PB8 (D4) and PB9 (D5) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configure PA12 (D6) and PA15 (D7) as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Set all LCD pins LOW initially
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_RESET);


  // -------------------------------
  // Button0 configuration (PA0)
  // -------------------------------
  GPIO_InitStruct.Pin = Button0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // Interrupt on rising edge
  GPIO_InitStruct.Pull = GPIO_PULLUP;         // Use pull-up resistor
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Enable and set EXTI line 0 interrupt priority
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(Button0_Pin); // Clear interrupt flags

	// TODO: Debounce using HAL_GetTick()
	uint32_t now = HAL_GetTick();
	if (now - last_press_time < DEBOUNCE_DELAY_MS)
	{
		last_press_time = now;
	}

	// TODO: Disable DMA transfer and abort IT, then start DMA in IT mode with new LUT and re-enable transfer
	__HAL_TIM_DISABLE_DMA(&htim2, TIM_DMA_CC1);
	HAL_DMA_Abort_IT(&hdma_tim2_ch1);
	// HINT: Consider using C's "switch" function to handle LUT changes

	current_waveform = (current_waveform + 1) % NUM_WAVEFORMS;

	HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t)waveform_LUTs[current_waveform], (uint32_t)&TIM3->CCR3, NS);

	__HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_CC1);

	lcd_command(CLEAR);
	lcd_putstring("Waveform: ");
	lcd_putstring(waveform_names[current_waveform]);
}
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
