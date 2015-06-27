#ifndef SLIDERS_H_
#define SLIDERS_H_

typedef enum 
{
	BIG_DECREASE,
	SLIGHT_DECREASE,
	KEEP,
	SLIGHT_INCREASE,
	BIG_INCREASE
} slider_pos;

void sliders_Init(void);

// Returns the current slider position for the requested slider (numbering starts from zero)
slider_pos sliders_GetPos(int param_idx);

// Returns the position of the volume slider as a number from 0 to 100, 
// where 0 represents the lowest position, and 100 the highest position
int sliders_GetVolume(void);

#endif /* SLIDERS_H_ */
