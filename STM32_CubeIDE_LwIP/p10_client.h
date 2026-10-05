/**
 * @file p10_client.h
 * @brief STM32 LwIP TCP Client driver for ESP32 P10 Countdown Display Controller.
 * 
 * Hardware: STM32 Nucleo (e.g. NUCLEO-F429ZI, NUCLEO-F767ZI, NUCLEO-H743ZI)
 * Network Stack: LwIP (Standard POSIX-like Socket API)
 */

#ifndef P10_CLIENT_H
#define P10_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default Configuration */
#define P10_DEFAULT_SERVER_IP   "172.1.1.4"
#define P10_DEFAULT_SERVER_PORT 5000
#define P10_RECV_TIMEOUT_MS     2000

/* Status Return Codes */
typedef enum {
    P10_OK = 0,
    P10_ERR_SOCKET = -1,
    P10_ERR_CONNECT = -2,
    P10_ERR_SEND = -3,
    P10_ERR_RECV = -4,
    P10_ERR_TIMEOUT = -5,
    P10_ERR_INVALID_PARAM = -6
} p10_status_t;

/**
 * @brief Initialize the P10 client with target server IP and port.
 * @param server_ip IP address string of the ESP32 (e.g. "172.1.1.4")
 * @param port TCP port of the ESP32 display server (e.g. 5000)
 */
void p10_client_init(const char *server_ip, uint16_t port);

/**
 * @brief Connect to the ESP32 P10 display controller over TCP.
 * @return P10_OK on success, or negative error code.
 */
p10_status_t p10_connect(void);

/**
 * @brief Disconnect the TCP socket.
 */
void p10_disconnect(void);

/**
 * @brief Check if the TCP client is currently connected.
 * @return true if connected, false otherwise.
 */
bool p10_is_connected(void);

/**
 * @brief Send a raw command string terminated with '\n' to the ESP32.
 * @param cmd Null-terminated command string (e.g. "COUNTDOWN:10\n" or "COUNTDOWN:10")
 * @param response Optional buffer to receive the ESP32 reply (can be NULL)
 * @param max_len Maximum length of the response buffer
 * @return P10_OK on success, or negative error code.
 */
p10_status_t p10_send_command(const char *cmd, char *response, size_t max_len);

/**
 * @brief Trigger a countdown on the P10 display.
 * @param seconds Duration in seconds (e.g. 10, 30, 60). Shows GO at finish.
 * @return P10_OK on success, or negative error code.
 */
p10_status_t p10_start_countdown(uint16_t seconds);

/**
 * @brief Pause the active countdown.
 */
p10_status_t p10_pause_countdown(void);

/**
 * @brief Resume a paused countdown.
 */
p10_status_t p10_resume_countdown(void);

/**
 * @brief Stop the countdown and clear the display.
 */
p10_status_t p10_stop_countdown(void);

/**
 * @brief Display custom text on the P10 LED screen.
 * @param text Null-terminated ASCII text to display.
 */
p10_status_t p10_set_text(const char *text);

/**
 * @brief Query the current status of the ESP32 display.
 * @param status_buf Buffer to store status string (e.g. "STATE:COUNTDOWN,SEC:15")
 * @param max_len Maximum length of status_buf
 */
p10_status_t p10_get_status(char *status_buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* P10_CLIENT_H */
