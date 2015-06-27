#ifndef EFFECTS_H_
#define EFFECTS_H_

#include "ascii/ascii.h"
#include "utils.h"

#define MAX_EFFECTS 10
#define EFFECT_NAME_LENGTH 9
#define MAX_EFFECT_PARAMS 4
#define PARAM_NAME_LENGTH 5

#define NO_PARAM_MSG "----"
#define NO_PARAM_MSG_LENGHT 4 // 4 == strlen("----")

#define VOLUME_MSG "Volume"
#define VOLUME_MSG_LENGTH 6 // 6 == strlen("Volume")

#define PARAM_VALUE_LENGTH 4

typedef struct
{
	bool in_use;
	char name[PARAM_NAME_LENGTH];
	
	int2d min;
	int2d max;
	int2d regular;
	int2d current;
} effect_parameter_data_t;

typedef struct
{
	bool in_use;
	char name[EFFECT_NAME_LENGTH];
	effect_parameter_data_t params[MAX_EFFECT_PARAMS];	
} effect_data_t;

extern effect_data_t effects[MAX_EFFECTS];

extern int currEffect;
extern int numLoadedEffects;

void initializeEffects(void);

// This blocks until the host sends the sync message, where all of the effects are sent
// with their parameters and the min, max and regular values for each of them.
// When loadEffectsFromHost returns, effects and numLoadedEffects contain valid data.
void loadEffectsFromHost(void);


#endif /* EFFECTS_H_ */
