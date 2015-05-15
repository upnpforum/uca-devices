/**
 *
 * Copyright 2013-2014 UPnP Forum All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE FREEBSD PROJECT "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE OR WARRANTIES OF 
 * NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE FREEBSD PROJECT OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official 
 * policies, either expressed or implied, by the UPnP Forum.
 *
 **/

#pragma once

typedef struct _action_args {
    const char *service_id;
    const char *action_name;
    const char **action_arg_names;
    const char **action_arg_values;
    void *service_token;
} action_args_t;

typedef struct _action_output {
    unsigned int error_code;
    const char *error_message;
    const char **action_out_names;
    const char **action_out_values;
} action_output_t;

typedef struct _query_args {
    const char *service_id;
    const char *variable_name;
} query_args_t;

typedef struct _network_params {
    const char *interface;
    unsigned int port;
} network_params_t;

typedef struct _upnp_params {
    const char *root_description;
    const char *description_dir;
} upnp_params_t;

typedef struct _notification_params {
    const char *service_id;
    const char *var_name;
    const char *value;
    void *service_token;
} notification_params_t;

struct _uda_stack;
typedef struct _uda_stack uda_stack_t;

typedef action_output_t (*action_handler_t)(const action_args_t args);
typedef void (*query_handler_t)(const uda_stack_t *stack, const query_args_t args);


uda_stack_t *create_uda_stack( const network_params_t *net_params
                             , const upnp_params_t   *upnp_params
                             , const action_handler_t action_handler
                             , const query_handler_t  query_handler
                             );

int start_uda_stack(uda_stack_t *stack);

void stop_uda_stack(uda_stack_t* stack);

void destroy_uda_stack(uda_stack_t *stack);

void send_notification(uda_stack_t *stack, notification_params_t params);
