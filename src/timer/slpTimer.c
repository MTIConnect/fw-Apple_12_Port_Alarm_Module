/*
 * slpTimer.c
 *
 * Created: 2/16/2017 11:53:41 AM
 *  Author: jcollier
 */ 


/*- Includes ---------------------------------------------------------------*/
#include "hw_timer.h"
#include "slpTimer.h"


/*****************************************************************************
*****************************************************************************/
static void placeTimer(SLP_Timer_t *timer);

/*- Variables --------------------------------------------------------------*/
static SLP_Timer_t *timers;
volatile uint32_t SlpTimerTime;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void SLP_TimerInit(void)
{
	SlpTimerTime = 0;
	hw_timer_init();
	timers = NULL;
}

/*************************************************************************//**
*****************************************************************************/
void SLP_TimerStart(SLP_Timer_t *timer)
{
	if (!SLP_TimerStarted(timer)) {
		placeTimer(timer);
	}
}

/*************************************************************************//**
*****************************************************************************/
void SLP_TimerRestart(SLP_Timer_t *timer)
{
	if (SLP_TimerStarted(timer)) {
		SLP_TimerStop(timer);
	}
	placeTimer(timer);
}

/*************************************************************************//**
*****************************************************************************/
void SLP_TimerStop(SLP_Timer_t *timer)
{
	SLP_Timer_t *prev = NULL;

	cpu_irq_enter_critical();

	for (SLP_Timer_t *t = timers; t; t = t->next) {
		if (t == timer) {
			if (prev) {
				prev->next = t->next;
			} else {
				timers = t->next;
			}

			if (t->next) {
				t->next->timeout += timer->timeout;
			}

			break;
		}

		prev = t;
	}

	cpu_irq_leave_critical();
}

/*************************************************************************//**
*****************************************************************************/
bool SLP_TimerStarted(SLP_Timer_t *timer)
{
	cpu_irq_enter_critical();

	for (SLP_Timer_t *t = timers; t; t = t->next) {
		if (t == timer) {
			cpu_irq_leave_critical();
			return true;
		}
	}
	cpu_irq_leave_critical();

	return false;
}

/*************************************************************************//**
*****************************************************************************/
static void placeTimer(SLP_Timer_t *timer)
{
	cpu_irq_enter_critical();
	
	if (timers) {
		SLP_Timer_t *prev = NULL;
		uint32_t timeout = timer->interval;

		for (SLP_Timer_t *t = timers; t; t = t->next) {
			if (timeout < t->timeout) {
				t->timeout -= timeout;
				break;
			} else {
				timeout -= t->timeout;
			}

			prev = t;
		}

		timer->timeout = timeout;

		if (prev) {
			timer->next = prev->next;
			prev->next = timer;
		} else {
			timer->next = timers;
			timers = timer;
		}
	} else {
		timer->next = NULL;
		timer->timeout = timer->interval;
		timers = timer;
	}

	cpu_irq_leave_critical();
}

/*************************************************************************//**
*****************************************************************************/
uint32_t SLP_TimerTimeout(SLP_Timer_t *timer)
{
	uint32_t timeout = 0;

	cpu_irq_enter_critical();

	if (timers)
	{
		for (SLP_Timer_t *t = timers; t; t = t->next)
		{
			timeout += t->timeout;
			if (t == timer)
			{
				break;
			}
		}
	}

	cpu_irq_leave_critical();

	return timeout;
}

/*****************************************************************************
*****************************************************************************/
void SLP_HwExpiry_Cb(void)
{
	uint32_t elapsed;

	elapsed = HW_TIMER_INTERVAL;
	SlpTimerTime += HW_TIMER_INTERVAL;

	while (timers && (timers->timeout <= elapsed)) {
		SLP_Timer_t *timer = timers;

		elapsed -= timers->timeout;
		timers = timers->next;
		if (SLP_TIMER_PERIODIC_MODE == timer->mode) {
			placeTimer(timer);
		}

		if (timer->handler) {
			timer->handler(timer);
		}
	}

	if (timers) {
		timers->timeout -= elapsed;
	}
}

uint32_t SLP_Timer_Time(void)
{
	return SlpTimerTime;
}
