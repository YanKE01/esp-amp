/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdint.h>
#include <stdio.h>
#include "esp_amp.h"
#include "esp_amp_platform.h"

#include "event.h"
#include "rpc_service.h"

#define TAG "rpc_client"

static esp_amp_rpmsg_dev_t rpmsg_dev;
static esp_amp_rpc_client_stg_t rpc_client_stg;


/* rpc client user-defined poll function */
static void rpc_client_poll(void *args)
{
    esp_amp_rpmsg_dev_t *rpmsg_dev = (esp_amp_rpmsg_dev_t *)args;
    static uint32_t cnt = 0;

    if (cnt++ % 1000 == 0) {
        printf("SUB: client: polling\r\n");
    }

    esp_amp_rpmsg_poll(rpmsg_dev);
    esp_amp_platform_delay_ms(1);
}

int main(void)
{
    printf("SUB: Hello!!\r\n");

    assert(esp_amp_init() == 0);
    assert(esp_amp_rpmsg_sub_init(&rpmsg_dev, true, true) == 0);

    esp_amp_rpc_client_cfg_t cfg = {
        .client_id = RPC_DEMO_CLIENT,
        .server_id = RPC_DEMO_SERVER,
        .rpmsg_dev = &rpmsg_dev,
        .stg = &rpc_client_stg,
        .poll_cb = rpc_client_poll, /* user-defined poll function */
        .poll_arg = &rpmsg_dev,
    };

    esp_amp_rpc_client_t client = esp_amp_rpc_client_init(&cfg);
    assert(client != NULL);

    /* notify link up with main core */
    esp_amp_event_notify(EVENT_SUBCORE_READY);

    for (int i = 0; i < 10; i++) {
        printf("SUB: client: iteration %d\r\n", i + 1);
        while (esp_amp_rpmsg_poll(&rpmsg_dev) == 0);
        esp_amp_platform_delay_us(100000);
    }

    /* wait for more response */
    while (true) {
        while (esp_amp_rpmsg_poll(&rpmsg_dev) == 0);
        esp_amp_platform_delay_us(100000);
    }

    printf("SUB: Bye!!\r\n");
    abort();
}
