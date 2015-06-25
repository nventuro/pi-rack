#ifndef SLIDERS_H_
#define SLIDERS_H_

#define SLIDERS_AMOUNT 5

typedef struct
{	
	int *value;
	int min;
	int max;
} slider_data_t;

void sliders_Init(void);
void sliders_Set(slider_data_t data[]);

#endif /* SLIDERS_H_ */
