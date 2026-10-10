#ifndef ID_ROTATION_H
#define ID_ROTATION_H
#include <stdint.h>
#include <stdbool.h>
void id_rot_init(uint8_t *id);
void id_rot_tick(void);
bool id_rot_cambio(void);
void id_rot_aceptar(void);
#endif
