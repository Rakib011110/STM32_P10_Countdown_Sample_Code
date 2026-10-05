/**
 * @file main.c
 * @brief Sample STM32 Application to trigger P10 Countdown via Ethernet.
 * 
 * Target Board: STM32 Nucleo-144 (e.g. NUCLEO-F429ZI, NUCLEO-F767ZI)
 * Environment: STM32CubeIDE + LwIP + FreeRTOS
 * 
 * Hardware Connections:
 * - STM32 Ethernet RJ45 connected to the same switch/router as ESP32 (or direct cross/auto-MDIX).
 * - STM32 IP:  172.1.1.100 (or DHCP in same subnet)
 * - Netmask:   255.255.255.0
 * - Gateway:   172.1.1.1
 * - ESP32 IP:  172.1.1.4:5000
 */

#include "main.h"
#include "lwip.h"
#include "p10_client.h"
#include <stdio.h>

/* USER CODE BEGIN PV */
/* Set the ESP32 IP address and TCP Port */
#define ESP32_P10_IP    "172.1.1.4"
#define ESP32_P10_PORT  5000
/* USER CODE END PV */

/**
 * @brief FreeRTOS task or standalone function demonstrating P10 countdown control.
 */
void P10_Countdown_Demo_Task(void *argument)
{
    printf("\r\n========================================\r\n");
    printf("   STM32 P10 Countdown Controller\r\n");
    printf("   Target ESP32: %s:%d\r\n", ESP32_P10_IP, ESP32_P10_PORT);
    printf("========================================\r\n");

    /* Initialize P10 client driver */
    p10_client_init(ESP32_P10_IP, ESP32_P10_PORT);

    /* Wait a moment for LwIP link to become fully up */
    osDelay(2000);

    /* Attempt TCP Connection */
    printf("[STM32] Connecting to ESP32...\r\n");
    p10_status_t status = p10_connect();
    if (status != P10_OK) {
        printf("[STM32] ERROR: Failed to connect (Code: %d)\r\n", status);
        printf("[STM32] Please verify IP, subnet mask, and Ethernet cable.\r\n");
    } else {
        printf("[STM32] Connected successfully!\r\n");

        /* 1. Show custom greeting message */
        printf("[STM32] Sending welcome text...\r\n");
        p10_set_text("READY");
        osDelay(2000);

        /* 2. Trigger 15-second countdown */
        printf("[STM32] Starting 15-second countdown...\r\n");
        p10_start_countdown(15);

        /* 3. Query status every 3 seconds during the countdown */
        for (int i = 0; i < 5; i++) {
            osDelay(3000);
            char status_buf[64] = {0};
            if (p10_get_status(status_buf, sizeof(status_buf)) == P10_OK) {
                printf("[STM32] ESP32 Display Status: %s\r\n", status_buf);
            }
        }

        /* 4. Display finishes with "GO" on ESP32 side automatically */
        printf("[STM32] Countdown finished (display showed GO).\r\n");
    }

    /* Keep task alive / idle */
    for(;;)
    {
        /* Example: Press User Button on Nucleo (B1) to re-trigger 30-sec countdown */
        // if (HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET) {
        //     printf("[STM32] Button pressed! Starting 30-sec countdown...\r\n");
        //     p10_start_countdown(30);
        //     osDelay(1000);
        // }
        osDelay(1000);
    }
}
