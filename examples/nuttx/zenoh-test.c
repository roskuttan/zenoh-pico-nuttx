#include <zenoh-pico.h>
#include <stdio.h>

int main(void)
{
    z_result_t res;

    // Obtain the default configuration (owned)
    z_owned_config_t config_owned;
    res = z_config_default(&config_owned);
    if (res < 0) {
        printf("z_config_default failed: %d\n", res);
        return res;
    }

    // Move the configuration (returns a moved config pointer)
    z_moved_config_t *config = z_config_move(&config_owned);
    if (!config) {
        printf("z_config_move failed\n");
        return -1;
    }

    // Open a Zenoh session (session is an owned session)
    z_owned_session_t session_owned;
    res = z_open(&session_owned, config, NULL);
    if (res < 0) {
        printf("z_open failed: %d\n", res);
        return res;
    }

    // Declare a resource (bind a key)
    res = z_declare_resource(&session_owned, "/demo/example");
    if (res < 0) {
        printf("z_declare_resource failed: %d\n", res);
        return res;
    }

    // Write a message to the resource (send 14 bytes)
    res = z_write(&session_owned, "/demo/example", "Hello, NuttX!", 14);
    if (res < 0) {
        printf("z_write failed: %d\n", res);
        return res;
    }

    // Close the session (cast to loaned session type)
    res = z_close((z_loaned_session_t *)&session_owned, NULL);
    if (res < 0) {
        printf("z_close failed: %d\n", res);
        return res;
    }

    printf("Zenoh-Pico session completed successfully.\n");
    return 0;
}
