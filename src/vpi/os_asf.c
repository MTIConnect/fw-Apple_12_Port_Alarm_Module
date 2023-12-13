/*
 * os_asf.c
 *
 * Created: 11/1/2016 2:50:10 PM
 *  Author: enochl
 */ 
 #include <vpi/os.h>
 #include <interrupt.h>



 void vpOs_init(void)
 {
	 
 }
 
 
 
 void vpOs_criticalRegionEnter(void)
 {
	cpu_irq_enter_critical();
 }
 
 
 
 void vpOs_criticalRegionLeave(void)
 {
	cpu_irq_leave_critical();
 }