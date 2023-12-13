/*
 * slpTimer.h
 *
 * Created: 2/16/2017 11:53:22 AM
 *  Author: jcollier
 */ 


#ifndef SLPTIMER_H_
#define SLPTIMER_H_


/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "compiler.h"

/*- Types ------------------------------------------------------------------*/
typedef enum SLP_TimerMode_t {
	SLP_TIMER_INTERVAL_MODE,
	SLP_TIMER_PERIODIC_MODE,
} SLP_TimerMode_t;

typedef struct SLP_Timer_t {
	/* Internal data */
	struct SLP_Timer_t *next;
	uint32_t timeout;

	/* Timer parameters */
	uint32_t interval;
	SLP_TimerMode_t mode;
	void (*handler)(struct SLP_Timer_t *timer);
} SLP_Timer_t;

/*- Prototypes -------------------------------------------------------------*/
void SLP_TimerInit(void);
void SLP_TimerStart(SLP_Timer_t *timer);
void SLP_TimerRestart(SLP_Timer_t *timer);
void SLP_TimerStop(SLP_Timer_t *timer);
bool SLP_TimerStarted(SLP_Timer_t *timer);
uint32_t SLP_TimerTimeout(SLP_Timer_t *timer);
void SLP_HwExpiry_Cb(void);
uint32_t SLP_Timer_Time(void);

#endif /* SLPTIMER_H_ */