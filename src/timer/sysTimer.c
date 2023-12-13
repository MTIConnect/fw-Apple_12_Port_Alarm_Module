/**
 * \file sysTimer.c
 *
 * \brief System timer implementation
 *
 * Copyright (C) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *
 */

/*
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/*- Includes ---------------------------------------------------------------*/
#include "hw_timer.h"
#include "sysTimer.h"


/*****************************************************************************
*****************************************************************************/
static void placeTimer(SYS_Timer_t *timer);

/*- Variables --------------------------------------------------------------*/
volatile uint8_t SysTimerIrqCount;
static uint32_t SysTimerTime;
static SYS_Timer_t *timers;


/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void SYS_TimerInit(void)
{
	SysTimerIrqCount = 0;
	SysTimerTime = 0;
	hw_timer_init();
	timers = NULL;
}

/*************************************************************************//**
*****************************************************************************/
void SYS_TimerStart(SYS_Timer_t *timer)
{
	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	if (!SYS_TimerStarted(timer)) {
		placeTimer(timer);
	}

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
}

/*************************************************************************//**
*****************************************************************************/
void SYS_TimerRestart(SYS_Timer_t *timer)
{
	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	if (SYS_TimerStarted(timer)) {
		SYS_TimerStop(timer);
	}
	placeTimer(timer);

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
}

/*************************************************************************//**
*****************************************************************************/
void SYS_TimerStop(SYS_Timer_t *timer)
{
	SYS_Timer_t *prev = NULL;

	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	for (SYS_Timer_t *t = timers; t; t = t->next) {
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

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
}

/*************************************************************************//**
*****************************************************************************/
bool SYS_TimerStarted(SYS_Timer_t *timer)
{
	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	for (SYS_Timer_t *t = timers; t; t = t->next) {
		if (t == timer) {
			
			#ifdef HW_TIMER_LEAVE_CRITICAL
				HW_TIMER_LEAVE_CRITICAL
			#endif

			return true;
		}
	}

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif

	return false;
}

/*************************************************************************//**
*****************************************************************************/
void SYS_TimerTaskHandler(void)
{
	uint32_t elapsed;
	uint8_t cnt;

	if (0 == SysTimerIrqCount) {
		return;
	}

	cpu_irq_enter_critical();
	cnt = SysTimerIrqCount;
	SysTimerIrqCount = 0;
	cpu_irq_leave_critical();

	elapsed = cnt * HW_TIMER_INTERVAL;
	SysTimerTime += (cnt * HW_TIMER_INTERVAL);

	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	while (timers && (timers->timeout <= elapsed)) {
		SYS_Timer_t *timer = timers;

		
		elapsed -= timers->timeout;
		timers = timers->next;
		if (SYS_TIMER_PERIODIC_MODE == timer->mode) {
			placeTimer(timer);
		}

	
	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
	
		if (timer->handler) {
			timer->handler(timer);
		}
		
	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif
		
	}

	if (timers) {
		timers->timeout -= elapsed;
	}

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
}

/*************************************************************************//**
*****************************************************************************/
static void placeTimer(SYS_Timer_t *timer)
{
	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	if (timers) {
		SYS_Timer_t *prev = NULL;
		uint32_t timeout = timer->interval;

		for (SYS_Timer_t *t = timers; t; t = t->next) {
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

	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
}

/*************************************************************************//**
*****************************************************************************/
uint32_t SYS_TimerTimeout(SYS_Timer_t *timer)
{
	uint32_t timeout = 0;

	#ifdef HW_TIMER_ENTER_CRITICAL
		HW_TIMER_ENTER_CRITICAL
	#endif

	if (timers)
	{
		for (SYS_Timer_t *t = timers; t; t = t->next)
		{
			timeout += t->timeout;
			if (t == timer)
			{
				break;
			}
		}
	}
	
	#ifdef HW_TIMER_LEAVE_CRITICAL
		HW_TIMER_LEAVE_CRITICAL
	#endif
	
	return timeout;
}

/*****************************************************************************
*****************************************************************************/
void SYS_HwExpiry_Cb(void)
{
	SysTimerIrqCount++;
}

uint32_t SYS_Timer_Time(void)
{
	return SysTimerTime;
}
