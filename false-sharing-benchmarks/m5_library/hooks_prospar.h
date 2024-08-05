#ifndef HOOK_PROSPAR_H
#define HOOK_PROSPAR_H 1

#ifdef __cplusplus
extern "C" {
#endif

void init_prospar() __attribute__((constructor));
void roi_begin();
void roi_end();

#ifdef __cplusplus
}
#endif

#endif // HOOK_PROSPAR_H
