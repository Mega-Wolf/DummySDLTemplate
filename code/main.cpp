#include "settings.h"

#include "../helpers/sdl_layer.h"

void Init() {
}

void Update(color32* array, int width, int height, inputs* inputs) {
    // TODO(Tobi): remove when using them; just here to prevent warnings
    UNUSED_PARAM(width);
    UNUSED_PARAM(height);
    UNUSED_PARAM(inputs);

    // TODO(Tobi): Just for testing; Delete please
    array[5000] = GREEN;
    array[5001] = GREEN;
    array[5002] = GREEN;

    array[5512] = GREEN;
    array[5513] = GREEN;
    array[5514] = GREEN;

    array[6024] = GREEN;
    array[6025] = GREEN;
    array[6026] = GREEN;
}

void Exit() {
}
