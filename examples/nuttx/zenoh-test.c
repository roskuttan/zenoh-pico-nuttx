#include <stdio.h>
#include <stdlib.h>
#include <zenoh-pico.h>  // Adjust the include path as needed

int main(int argc, FAR char *argv[])
{
    printf("Starting Zenoh-Pico demo...\n");

    // Obtain the default Zenoh-Pico configuration.
    z_owned_config_t config = z_config_default();

    // Optionally, set a serial locator if needed:
    // z_config_insert(&config, Z_CONFIG_CONNECT, "serial:/dev/ttyS4#baud=115200");

    // Open a Zenoh-Pico session.
    z_owned_session_t session = z_open(z_move(config));
    if (!z_check(session)) {
        printf("Zenoh-Pico initialization FAILED!\n");
        return EXIT_FAILURE;
    }

    printf("Zenoh-Pico initialization SUCCEEDED!\n");

    // Close the session.
    z_close(z_move(session));
    return EXIT_SUCCESS;
}
