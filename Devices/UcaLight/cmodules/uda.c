#include "uda.h"

#include <stdlib.h>
#include <libgupnp/gupnp.h>

#include <pthread.h>

#define MAX_ARG_COUNT 16

struct _uda_stack {
    const GUPnPRootDevice  *root_dev;
    const GMainLoop        *glib_loop;

    pthread_t        thread;
    pthread_mutex_t  mutex;
    
    action_handler_t ahandler;
    query_handler_t  qhandler;
};

const network_params_t DEFAULT_NET_PARAMS
    = { .interface = NULL, .port = 0 };

static GUPnPContext *make_gupnp_context(const network_params_t *params)
{
    return gupnp_context_new(NULL, params->interface, params->port, NULL);
}

static char **find_action_arguments_names(GUPnPServiceActionInfo *actionInfo)
{
    int i = 0;
    char **args_names = malloc(MAX_ARG_COUNT * sizeof(char *));
    if (args_names == NULL) {
        return NULL;
    }

    for (const GList *arg = actionInfo->arguments
        ; arg != NULL && i < MAX_ARG_COUNT
        ; arg = arg->next
        )
    {
        GUPnPServiceActionArgInfo *info = arg->data;
        if (info->direction == GUPNP_SERVICE_ACTION_ARG_DIRECTION_IN) {
            args_names[i] = info->name;
            i++;
        }
    }
    args_names[i] = NULL;

    return args_names;
}

static char **find_action_arguments_values(GUPnPServiceAction *action, const char **names)
{
    int i = 0;
    char **args_values = malloc(MAX_ARG_COUNT * sizeof(char *));
    if (args_values == NULL || names == NULL) {
        return NULL;
    }

    for (const char **name = names; *name != NULL; name++) {
        const char *arg_name = *name;
        GValue *value = gupnp_service_action_get_gvalue(action, arg_name, G_TYPE_STRING);
        args_values[i] = g_value_get_string(value);
        i++;
    }
    return args_values;
}

static void set_output(GUPnPServiceAction *action, action_output_t output)
{
    if (action == NULL || output.action_out_names == NULL
                       || output.action_out_values == NULL
       ) {
        return;
    }

    int i = 0;
    for (const char **name = output.action_out_names; *name != NULL; name++) {
        const char *out_name  = *name;
        const char *out_value = output.action_out_values[i];
        
        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_string(&value, out_value);
        gupnp_service_action_set_value(action, out_name, &value);

        i++;
    }
}

static void free_output(action_output_t output)
{
    if (output.action_out_names != NULL) {
        for ( const char **name = output.action_out_names
            ; *name != NULL
            ; name++
            ) 
        {
            free(*name);
        }
    }

    if (output.action_out_values != NULL) {
        for ( const char **value = output.action_out_values
            ; *value != NULL
            ; value++
            ) 
        {
            free(*value);
        }
    }
    
    free(output.action_out_names);
    free(output.action_out_values);

    free(output.error_message);
}

static void on_action( GUPnPService *service
                     , GUPnPServiceAction *action
                     , void *uda_stack
                     )
{
    uda_stack_t *stack = uda_stack;

    fprintf(stderr, "* recived action invocation\n");

    int result = pthread_mutex_lock(&(stack->mutex));

    const char *action_name = gupnp_service_action_get_name(action);
    const char *service_name
        = gupnp_service_info_get_service_type(GUPNP_SERVICE_INFO(service));
    fprintf(stderr, "* processing action %s of %s\n", action_name, service_name);

    //printf("action %s invoked\n", action_name);

    const char **arg_names  = NULL;
    const char **arg_values = NULL;

    action_output_t output = { .action_out_names = NULL
                             , .action_out_values = NULL
                             };

    if (stack->ahandler != NULL) {

        fprintf(stderr, "* Attepmting to find action & service GObjects\n", action_name, service_name);

        GUPnPServiceIntrospection *intro
            = gupnp_service_info_get_introspection(GUPNP_SERVICE_INFO(service), NULL);
        fprintf(stderr, "* service found\n", action_name, service_name);

        GUPnPServiceActionInfo *actionInfo
            = gupnp_service_introspection_get_action(intro, action_name);
        fprintf(stderr, "* action found\n", action_name, service_name);

        arg_names  = find_action_arguments_names(actionInfo);
        arg_values = find_action_arguments_values(action, arg_names);

        action_args_t args = { .service_id = service_name
                             , .action_name = action_name
                             , .action_arg_names = arg_names
                             , .action_arg_values = arg_values
                             };

        action_output_t output = stack->ahandler(args);
        if (output.error_code <= 200) {
            set_output(action, output);
            gupnp_service_action_return(action);
        } else {
            gupnp_service_action_return_error(action, output.error_code, output.error_message);
        }

        g_object_unref(intro);
    } else {
        fprintf(stderr, "* SOAP handler missing.", action_name, service_name);
        gupnp_service_action_return_error(action, 401, "Not implemented");
    }

    free(arg_names);
    free(arg_values);
    free_output(output);

    result = pthread_mutex_unlock(&(stack->mutex));
}

static void on_query( GUPnPService *service
                    , char *variable
                    , GValue *value
                    , void *uda_stack
                    )
{
    puts("TODO: implement gupnp query handler\n");
}

static void register_action_hadler( const uda_stack_t *stack
                                  , const GUPnPServiceInfo *service
                                  )
{
    char signal_buffer[256];

    GUPnPServiceIntrospection *intro
        = gupnp_service_info_get_introspection((GUPnPServiceInfo *)service, NULL);
    const GList *names = gupnp_service_introspection_list_actions(intro);

    for (const GList *name = names; name != NULL; name = name->next) {
        GUPnPServiceActionInfo *info = name->data;
        sprintf(signal_buffer, "action-invoked::%s", info->name);
        g_signal_connect( (GUPnPServiceInfo *)service
                        , signal_buffer
                        , G_CALLBACK(on_action)
                        , (void *)stack
                        );
    }
}

/* TODO: subdevices should not be ignored */
static void register_handlers( const uda_stack_t *stack
                             , GUPnPDeviceInfo *device
                             )
{
    const GList *services = gupnp_device_info_list_services(device);
    
    for ( const GList *service = services
        ; service != NULL
        ; service = service->next
        )
    {
        GUPnPServiceInfo *info = service->data;
        gupnp_service_thaw_notify(info);
        register_action_hadler(stack, info);
        g_signal_connect(info, "query-variable", G_CALLBACK(on_query), (void *)stack);
    }

}

static void thread_procedure(void *uda_stack)
{
    uda_stack_t *stack = uda_stack;
    /* Activate the root device, so that it announces itself */
    gupnp_root_device_set_available((GUPnPRootDevice *)stack->root_dev, TRUE);
    /* Run the main glib loop (blocking) */
    g_main_loop_run((GMainLoop *)stack->glib_loop);
}

uda_stack_t *create_uda_stack( const network_params_t *net_params
                             , const upnp_params_t    *upnp_params
                             , const action_handler_t action_handler
                             , const query_handler_t  query_handler
                             )
{
    /* Initialize required subsystems */
    #if !GLIB_CHECK_VERSION(2,35,0)
      g_type_init();
    #endif

    GUPnPContext *context;
    if (net_params == NULL) {
        context = make_gupnp_context(&DEFAULT_NET_PARAMS);
    } else {
        context = make_gupnp_context(net_params);
    }
    
    uda_stack_t *stack = malloc(sizeof(uda_stack_t));
    if (stack == NULL) {
        return NULL;
    }

    int result = pthread_mutex_init(&(stack->mutex), NULL);

    stack->ahandler = action_handler;
    stack->qhandler = query_handler;

    stack->root_dev
        = gupnp_root_device_new( context, upnp_params->root_description
                               , upnp_params->description_dir
                               );
    
    register_handlers( stack
                     , GUPNP_DEVICE_INFO(stack->root_dev)
                     );

    stack->glib_loop = g_main_loop_new(NULL, FALSE);

    return stack;
}

int start_uda_stack(uda_stack_t *stack)
{
    int result = pthread_create(&(stack->thread), NULL, thread_procedure, (void *)stack);
    if (result != 0) {
        return result;
    }

    return 0;
}

void stop_uda_stack(uda_stack_t *stack)
{
    g_main_loop_quit((GMainLoop *)stack->glib_loop);
    pthread_join(stack->thread, NULL);
    gupnp_root_device_set_available((GUPnPRootDevice *)stack->root_dev, FALSE);
}

void destroy_uda_stack(uda_stack_t *stack)
{
    stop_uda_stack(stack);

    g_main_loop_unref((GMainLoop *)stack->glib_loop);
    pthread_mutex_destroy(&(stack->mutex));

    free(stack);
}

void send_notification(uda_stack_t *stack, notification_params_t params)
{
    GUPnPService *service = params.service_token;

    if (service == NULL) {
        return;
    }

    //GValue value = G_VALUE_INIT;
    //g_value_init(&value, G_TYPE_STRING);
    //g_value_set_string(&value, params.value);
    //
    //gupnp_service_notify_value(service, params.var_name, &value);

    gupnp_service_notify(service,
                         params.var_name, G_TYPE_STRING, params.value,
                         NULL);
    gupnp_service_thaw_notify(service);
}

