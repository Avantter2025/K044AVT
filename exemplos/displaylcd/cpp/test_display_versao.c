#include <stdio.h>
#include "display_driver.h"

int main(void)
{
    k044_set_log_level(K044_LOG_DEBUG);

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir (requer sudo).\n");
        return 1;
    }

    printf("Versao biblioteca: %s\n", k044_version());

    uint8_t major, minor, patch;
    int r = k044_read_fw_ver(&major, &minor, &patch);
    if (r == K044_OK)
        printf("Versao firmware (0xAF): %u.%u.%u\n", major, minor, patch);
    else
        printf("Versao firmware (0xAF): nao disponivel\n");

    char buf[16] = {0};
    k044_fw_ver_str(buf, sizeof(buf));
    printf("Versao firmware (string): %s\n", buf);

    char ver[32] = {0};
    r = k044_firmware_version(ver, sizeof(ver));
    if (r == K044_OK)
        printf("Versao firmware (scancode): %s\n", ver);
    else
        printf("Versao firmware (scancode): nao disponivel\n");

    k044_close();
    printf("Teste versao concluido.\n");
    return 0;
}
