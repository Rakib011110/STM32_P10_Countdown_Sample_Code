/**
 * @file p10_client.c
 * @brief STM32 LwIP TCP Client implementation for ESP32 P10 Countdown Display Controller.
 * 
 * Target Framework: STM32CubeIDE + FreeRTOS + LwIP
 */

#include "p10_client.h"
#include <string.h>
#include <stdio.h>

/* LwIP Socket Includes */
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"

static char g_server_ip[32] = P10_DEFAULT_SERVER_IP;
static uint16_t g_server_port = P10_DEFAULT_SERVER_PORT;
static int g_sock_fd = -1;

void p10_client_init(const char *server_ip, uint16_t port) {
    if (server_ip != NULL && strlen(server_ip) > 0) {
        strncpy(g_server_ip, server_ip, sizeof(g_server_ip) - 1);
        g_server_ip[sizeof(g_server_ip) - 1] = '\0';
    }
    if (port > 0) {
        g_server_port = port;
    }
}

p10_status_t p10_connect(void) {
    if (g_sock_fd >= 0) {
        p10_disconnect();
    }

    /* Create TCP Socket */
    g_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sock_fd < 0) {
        return P10_ERR_SOCKET;
    }

    /* Set receive timeout */
    struct timeval tv;
    tv.tv_sec = P10_RECV_TIMEOUT_MS / 1000;
    tv.tv_usec = (P10_RECV_TIMEOUT_MS % 1000) * 1000;
    setsockopt(g_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    /* Prepare server address */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(g_server_port);
    serv_addr.sin_addr.s_addr = inet_addr(g_server_ip);

    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        close(g_sock_fd);
        g_sock_fd = -1;
        return P10_ERR_INVALID_PARAM;
    }

    /* Connect to ESP32 */
    if (connect(g_sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(g_sock_fd);
        g_sock_fd = -1;
        return P10_ERR_CONNECT;
    }

    return P10_OK;
}

void p10_disconnect(void) {
    if (g_sock_fd >= 0) {
        close(g_sock_fd);
        g_sock_fd = -1;
    }
}

bool p10_is_connected(void) {
    return (g_sock_fd >= 0);
}

p10_status_t p10_send_command(const char *cmd, char *response, size_t max_len) {
    if (cmd == NULL) {
        return P10_ERR_INVALID_PARAM;
    }

    /* Auto-connect if not currently connected */
    if (g_sock_fd < 0) {
        p10_status_t conn_res = p10_connect();
        if (conn_res != P10_OK) {
            return conn_res;
        }
    }

    /* Format command line with trailing newline */
    char tx_buffer[128];
    size_t cmd_len = strlen(cmd);
    if (cmd_len >= sizeof(tx_buffer) - 2) {
        return P10_ERR_INVALID_PARAM;
    }

    strcpy(tx_buffer, cmd);
    if (tx_buffer[cmd_len - 1] != '\n') {
        tx_buffer[cmd_len] = '\n';
        tx_buffer[cmd_len + 1] = '\0';
        cmd_len++;
    }

    /* Transmit over TCP */
    int bytes_sent = send(g_sock_fd, tx_buffer, cmd_len, 0);
    if (bytes_sent < 0) {
        p10_disconnect(); // socket broken
        return P10_ERR_SEND;
    }

    /* If caller wants response, wait with timeout */
    if (response != NULL && max_len > 0) {
        memset(response, 0, max_len);
        int bytes_recv = recv(g_sock_fd, response, max_len - 1, 0);
        if (bytes_recv < 0) {
            return P10_ERR_RECV;
        }
        response[bytes_recv] = '\0';
        
        // Strip trailing carriage return or newline
        while (bytes_recv > 0 && (response[bytes_recv - 1] == '\r' || response[bytes_recv - 1] == '\n')) {
            response[--bytes_recv] = '\0';
        }
    }

    return P10_OK;
}

p10_status_t p10_start_countdown(uint16_t seconds) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "COUNTDOWN:%u\n", seconds);
    return p10_send_command(cmd, NULL, 0);
}

p10_status_t p10_pause_countdown(void) {
    return p10_send_command("PAUSE\n", NULL, 0);
}

p10_status_t p10_resume_countdown(void) {
    return p10_send_command("RESUME\n", NULL, 0);
}

p10_status_t p10_stop_countdown(void) {
    return p10_send_command("STOP\n", NULL, 0);
}

p10_status_t p10_set_text(const char *text) {
    if (text == NULL) return P10_ERR_INVALID_PARAM;
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "TEXT:%s\n", text);
    return p10_send_command(cmd, NULL, 0);
}

p10_status_t p10_get_status(char *status_buf, size_t max_len) {
    return p10_send_command("STATUS\n", status_buf, max_len);
}
