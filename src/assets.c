#include <stdint.h>

typedef struct {
    const char *id;
    const char *name;
    const char *uuid;
} asset_t;

static asset_t g_asset;

void init_assets(void)
{
    g_asset.id   = "1";
    g_asset.name = "pet_tracker";
    g_asset.uuid = "abc-123-def-456";
}

const char* get_ID(void)
{
    return g_asset.id;
}
