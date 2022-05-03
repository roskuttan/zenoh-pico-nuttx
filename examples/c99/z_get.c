/*
 * Copyright (c) 2017, 2021 ADLINK Technology Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
 * which is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *
 * Contributors:
 *   ADLINK zenoh team, <zenoh@adlink-labs.tech>
 */
#include <stdio.h>
#include <string.h>

#include "zenoh-pico.h"

int main(int argc, char **argv)
{
    z_init_logger();

    z_str_t expr = "/demo/example/**";
    if (argc > 1)
    {
        expr = argv[1];
    }
    z_owned_config_t config = z_config_default();
    if (argc > 2)
    {
        z_config_insert(z_config_loan(&config), Z_CONFIG_PEER_KEY, z_string_make(argv[2]));
    }

    z_config_insert(z_config_loan(&config), Z_CONFIG_USER_KEY, z_string_make("user"));
    z_config_insert(z_config_loan(&config), Z_CONFIG_PASSWORD_KEY, z_string_make("password"));

    printf("Openning session...\n");
    z_owned_session_t s = z_open(z_config_move(&config));
    if (!z_session_check(&s))
    {
        printf("Unable to open session!\n");
        exit(-1);
    }

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_session_loan(&s));
    zp_start_lease_task(z_session_loan(&s));

    printf("Sending Query '%s'...\n", expr);
    z_query_target_t target = z_query_target_default();
    target.target = Z_TARGET_ALL;
    z_owned_keyexpr_t keyexpr = z_expr_new(expr);
    z_owned_reply_data_array_t replies = z_get_collect(z_session_loan(&s), z_keyexpr_loan(&keyexpr), "", target, z_query_consolidation_default());

    for (unsigned int i = 0; i < z_reply_data_array_len(z_reply_data_array_loan(&replies));++i)
    {
        printf(">> Received ('%s': '%.*s')\n",
               z_reply_data_array_get(z_reply_data_array_loan(&replies), i)->sample.key.suffix,
               (int)z_reply_data_array_get(z_reply_data_array_loan(&replies), i)->sample.value.len, z_reply_data_array_get(z_reply_data_array_loan(&replies), i)->sample.value.start);
    }
    z_reply_data_array_clear(z_reply_data_array_move(&replies));
    z_keyexpr_clear(z_keyexpr_move(&keyexpr));

    zp_stop_read_task(z_session_loan(&s));
    zp_stop_lease_task(z_session_loan(&s));
    z_close(z_session_move(&s));
    return 0;
}
