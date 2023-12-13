/*
 * app_buzzer.c
 *
 * Created: 10/20/2023 5:13:10 PM
 *  Author: dmcdougall
 */ 

#include <asf.h>
#include "app_buzzer.h"
//#include "app_user_options.h"
#include "conf_board.h"
#include "config.h"
#include "sysTimer.h"

#define BUZZER_ALARM_TIMEOUT 300000

#define BUZZER_REPLACE_CONFIRM_ALARM_TIMEOUT  3000
#define BUZZER_REPLACE_CONFIRM_SILENT_TIMEOUT (60000 - BUZZER_REPLACE_CONFIRM_ALARM_TIMEOUT)

// Clock ticks based on:
// Clock Frequency = 8MHz
// Time per Tick = 125ns

// Basic
#define FREQ_4500Hz 1778
#define FREQ_4200Hz 1904
#define FREQ_4000Hz 2000
#define FREQ_3000Hz 2667
#define FREQ_2700Hz 2963
#define FREQ_2637Hz 3034  // E7
#define FREQ_2000Hz 4000

// Alarm
#define BUZ_FREQ_MIN      FREQ_3000Hz
#define BUZ_FREQ_MAX      FREQ_4500Hz
#define BUZ_FREQ_RESONANT FREQ_4200Hz
#define BUZ_FREQ_INC      2
#define BUZ_FREQ_DELAY    210

static void app_buzzer_enable(enum app_buzzer_freq_t freq, uint16_t duration);
static void app_buzzer_disable(void);
static void app_buzzer_set_volume(uint8_t);

struct buzzerNote_t
{
    enum app_buzzer_freq_t freq;
    uint16_t duration;
};

static const struct buzzerNote_t patternNone[] = {{BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternError[] = {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
                                                   {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternDelayError[] =  // Delay + Error
    {{BEEP_PAUSE, 300}, {BEEP_6, 50},      {BEEP_PAUSE, 100}, {BEEP_6, 50},   {BEEP_PAUSE, 100},
     {BEEP_6, 50},      {BEEP_PAUSE, 100}, {BEEP_6, 50},      {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternSucceed[] = {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternSucceedCant[] =  // Succeed + DelayError
    {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 300}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternSucceedLowBat[] =  // Succeed + LowBat
    {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 500}, {BEEP_5, 1000}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternLowBat[] = {{BEEP_5, 1000}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternBatteryEol[] = {{BEEP_6, 500},     {BEEP_PAUSE, 500}, {BEEP_6, 500},
                                                        {BEEP_PAUSE, 500}, {BEEP_6, 500},     {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternSkeleton[] =  // lasts 5 seconds
    {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 250}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 250}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 250},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 250}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 250}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 250},
     {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
     {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternDelete[] =  // lasts 10 seconds
    {{BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_6, 50}, {BEEP_PAUSE, 200},
     {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternFactoryTest[] = {{BEEP_3, 60000}, {BEEP_REPEAT, 0}};

static const struct buzzerNote_t patternFactoryTest2[] = {{BEEP_7, 60000}, {BEEP_REPEAT, 0}};

static const struct buzzerNote_t patternProvision[] = {{BEEP_PAUSE, 800}, {BEEP_3, 150}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternAuthReplaceConfirming[] = {{BEEP_6, 50}, {BEEP_PAUSE, 200}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternBaseNoPower[] = {{BEEP_6, 100}, {BEEP_PAUSE, 100}, {BEEP_6, 100}, {BEEP_PAUSE, 500},
                                                         {BEEP_6, 100}, {BEEP_PAUSE, 100}, {BEEP_6, 100}, {BEEP_PAUSE, 500},
                                                         {BEEP_6, 100}, {BEEP_PAUSE, 100}, {BEEP_6, 100}, {BEEP_PAUSE, 500},
                                                         {BEEP_6, 100}, {BEEP_PAUSE, 100}, {BEEP_6, 100}, {BEEP_PAUSE, 500},
                                                         {BEEP_6, 100}, {BEEP_PAUSE, 100}, {BEEP_6, 100}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternPuckDeepSleep[] = {{BEEP_6, 100}, {BEEP_PAUSE, 100},  {BEEP_6, 100}, {BEEP_PAUSE, 100},
                                                           {BEEP_6, 100}, {BEEP_PAUSE, 100},  {BEEP_6, 100}, {BEEP_PAUSE, 100},
                                                           {BEEP_6, 100}, {BEEP_PAUSE, 1000}, {BEEP_6, 100}, {BEEP_PAUSE, 100},
                                                           {BEEP_6, 100}, {BEEP_PAUSE, 100},  {BEEP_6, 100}, {BEEP_PAUSE, 100},
                                                           {BEEP_6, 100}, {BEEP_PAUSE, 100},  {BEEP_6, 100}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternWarning[] = {
    {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200},
    {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200},
    {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200},
    {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 200}, {BEEP_6, 100}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternAlert[] = {
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 400},
    {BEEP_6, 100}, {BEEP_PAUSE, 400}, {BEEP_6, 100}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t patternRFIDError[] = {{BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
                                                       {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
                                                       {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 100},
                                                       {BEEP_6, 50}, {BEEP_PAUSE, 100}, {BEEP_6, 50}, {BEEP_PAUSE, 0}};

static const struct buzzerNote_t* buzzerPatterns[] = {
    patternNone,                   // BUZ_PAT_NONE
    patternError,                  // BUZ_PAT_ERROR
    patternDelayError,             // BUZ_PAT_DELAY_ERROR
    patternSucceed,                // BUZ_PAT_SUCCEED
    patternSucceedCant,            // BUZ_PAT_CANT
    patternSucceedLowBat,          // BUZ_PAT_SUCCEED_LOW_BAT
    patternLowBat,                 // BUZ_PAT_LOW_BAT
    patternBatteryEol,             // BUZ_PAT_EOL
    patternSkeleton,               // BUZ_PAT_SKELETON
    patternDelete,                 // BUZ_PAT_DELETE
    patternFactoryTest,            // BUZ_PAT_FACTORY_TEST
    patternFactoryTest2,           // BUZ_PAT_FACTORY_TEST2
    patternProvision,              // BUZ_PAT_PROVISION
    patternAuthReplaceConfirming,  // BUZ_PAT_AUTH_REPLACE_CONFIRMING
    patternBaseNoPower,            // BUZ_PAT_BASE_NO_POWER
    patternPuckDeepSleep,          // BUZ_PAT_PUCK_DEEP_SLEEP
    patternWarning,                // BUZ_PAT_WARNING
    patternAlert,                  // BUZ_PAT_ALERT
    patternRFIDError,              // BUZ_PAT_RFID_ERROR
};

static struct tcc_module tcc_instance_buzzer;
static struct tcc_config config_tcc_buzzer;

static enum app_buzzer_state_t buzzerState;
static enum app_buzzer_pattern_t buzzerPatternIdx;
static uint8_t notePointer;

/**  Timers  **/
static SYS_Timer_t app_buzzer_alarm_note_timer;

/**  Callback  **/
static void app_buzzer_alarm_note_timerHandler(struct SYS_Timer_t* timer);
static void app_buzzer_alarmCallback(struct tcc_module* const module_inst);

/**  Core  **/
static void app_buzzer_alarm_note_timerHandler(struct SYS_Timer_t* timer)
{
    // At the end of every note, disable the timer and indicate the note end as a TIMEOUT.
    buzzerState = BUZ_STATE_TIMEOUT;
    app_buzzer_disable();
}

void app_buzzer_start_pattern(enum app_buzzer_pattern_t pattern)
{
//     switch (pattern)
//     {
//         case BUZ_PAT_ALARM:
//             app_buzzer_alarm_start();
//             return;
//         case BUZ_PAT_ALERT:
//         case BUZ_PAT_WARNING:
//         case BUZ_PAT_PUCK_DEEP_SLEEP:
//         case BUZ_PAT_BASE_NO_POWER:
//         {
// //             uint8_t buzVolume = app_user_options_get_volume();
// //             if (VOLUME_OFF == buzVolume)
// //             {
// //                 return;
// //             }
// //             app_buzzer_set_volume(buzVolume);
//         }
//         break;
//         default:
//             app_buzzer_set_volume(VOLUME_MAX);
//     }

    if (BUZ_PAT_ALARM != buzzerPatternIdx)
    {
        // We only play a normal melody if we're not presently alarming.
        app_buzzer_disable();
        buzzerPatternIdx         = pattern;
        notePointer              = 0;
        struct buzzerNote_t note = buzzerPatterns[buzzerPatternIdx][0];
        app_buzzer_enable(note.freq, note.duration);
    }
}

static void app_buzzer_enable(enum app_buzzer_freq_t freq, uint16_t duration)
{
    // Start playing the specified note for the specified duration,
    // but only if we're not already running the timer!
    if (!SYS_TimerStarted(&app_buzzer_alarm_note_timer))
    {
        if (BEEP_PAUSE != freq)
        {
            uint16_t tccTopValue = 0;

            switch (freq)
            {
                case BEEP_1:
                    tccTopValue = FREQ_4500Hz;
                    break;
                case BEEP_2:
                    tccTopValue = FREQ_4200Hz;
                    break;
                case BEEP_3:
                    tccTopValue = FREQ_4000Hz;
                    break;
                case BEEP_4:
                    tccTopValue = FREQ_3000Hz;
                    break;
                case BEEP_5:
                    tccTopValue = FREQ_2700Hz;
                    break;
                case BEEP_6:
                    tccTopValue = FREQ_2637Hz;
                    break;
                case BEEP_7:
                    tccTopValue = FREQ_2000Hz;
                    break;

                default:
                    // by default, we continue whatever pitch was already playing!
                    break;
            }

            // Start the TCC unit to play the tone.
            tcc_init(&tcc_instance_buzzer, BUZZER_MODULE, &config_tcc_buzzer);
            tcc_set_top_value(&tcc_instance_buzzer, tccTopValue);
            tcc_set_compare_value(&tcc_instance_buzzer, (TCC_MATCH_CAPTURE_CHANNEL_0 + BUZZER_CHANNEL), tccTopValue / 2);
            tcc_restart_counter(&tcc_instance_buzzer);
            tcc_enable(&tcc_instance_buzzer);
        }

        // Start the note duration timer.
        app_buzzer_alarm_note_timer.interval = duration;
        app_buzzer_alarm_note_timer.mode     = SYS_TIMER_PERIODIC_MODE;
        app_buzzer_alarm_note_timer.handler  = app_buzzer_alarm_note_timerHandler;
        SYS_TimerStart(&app_buzzer_alarm_note_timer);

        buzzerState = BUZ_STATE_PROCESSING;
    }
}

static void app_buzzer_disable(void)
{
    if (SYS_TimerStarted(&app_buzzer_alarm_note_timer))
    {
        SYS_TimerStop(&app_buzzer_alarm_note_timer);
    }

    tcc_disable(&tcc_instance_buzzer);
    tcc_disable_callback(&tcc_instance_buzzer, (TCC_CALLBACK_CHANNEL_0 + BUZZER_CHANNEL));

    // Configure the pin to output and set to low to avoid high current through the speaker
    struct system_pinmux_config muxConfig;
    system_pinmux_get_config_defaults(&muxConfig);
    muxConfig.direction  = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
    muxConfig.input_pull = SYSTEM_PINMUX_PIN_PULL_NONE;
    system_pinmux_pin_set_config(BUZZER_PIN, &muxConfig);
    port_pin_set_output_level(BUZZER_PIN, false);
}

void app_buzzer_task(void)
{
    // The foreground task only acts if a melody is active and the most recent note has ended.
    if ((BUZ_PAT_NONE != buzzerPatternIdx) && (buzzerState == BUZ_STATE_TIMEOUT))
    {
        // Advance the pointer and get the next note.
        notePointer++;
        const struct buzzerNote_t* note = &buzzerPatterns[buzzerPatternIdx][notePointer];

        if (BEEP_REPEAT == note->freq)
        {  // Repeat the beep pattern from beginning
            notePointer = 0;
            note        = &buzzerPatterns[buzzerPatternIdx][0];
        }

        if (0 != note->duration)
        {  // Play next note
            app_buzzer_enable(note->freq, note->duration);
        }
        else
        {  // Pattern ends
            buzzerPatternIdx = BUZ_PAT_NONE;
            buzzerState      = BUZ_STATE_END;
        }
    }
}

void app_buzzer_init(void)
{
    tcc_get_config_defaults(&config_tcc_buzzer, BUZZER_MODULE);

    config_tcc_buzzer.counter.clock_source    = BUZZER_CLK_SRC;
    config_tcc_buzzer.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
    config_tcc_buzzer.counter.direction       = TCC_COUNT_DIRECTION_DOWN;
    config_tcc_buzzer.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;

    config_tcc_buzzer.pins.enable_wave_out_pin[BUZZER_CHANNEL] = true;
    config_tcc_buzzer.pins.wave_out_pin[BUZZER_CHANNEL]        = BUZZER_PIN;
    config_tcc_buzzer.pins.wave_out_pin_mux[BUZZER_CHANNEL]    = BUZZER_MUX;
    config_tcc_buzzer.double_buffering_enabled                 = false;

    tcc_init(&tcc_instance_buzzer, BUZZER_MODULE, &config_tcc_buzzer);

    // Configure the Buzzer pin to output and set to low to lower current draw
    struct port_config pin_conf;
    port_get_config_defaults(&pin_conf);
    pin_conf.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(BUZZER_PIN, &pin_conf);
    port_pin_set_output_level(BUZZER_PIN, false);

    buzzerPatternIdx = BUZ_PAT_NONE;
    notePointer      = 0;

    buzzerState = BUZ_STATE_IDLE;
}

static void app_buzzer_alarmCallback(struct tcc_module* const module)
{
    static uint16_t sweepCounter  = BUZ_FREQ_MAX;
    static uint8_t sweepDirection = 1;
    static uint16_t sweepDelay    = 0;

    if ((abs(sweepCounter - BUZ_FREQ_RESONANT) > BUZ_FREQ_INC) || (sweepDelay >= BUZ_FREQ_DELAY) || (sweepDirection == 1))
    {  // delay for 4.2kHz resonant frequency
        sweepDelay = 0;

        if (sweepDirection)
            sweepCounter += BUZ_FREQ_INC;
        else
            sweepCounter -= BUZ_FREQ_INC;

        if (sweepCounter <= BUZ_FREQ_MAX)
            sweepDirection = 1;
        else if (sweepCounter >= BUZ_FREQ_MIN)
            sweepDirection = 0;

        // Changes Period
        tcc_set_top_value(module, sweepCounter);
        tcc_set_compare_value(module, (TCC_MATCH_CAPTURE_CHANNEL_0 + BUZZER_CHANNEL), sweepCounter / 2);
    }
    else
        sweepDelay++;
}

void app_buzzer_alarm_start(void)
{
//     uint8_t buzVolume = app_user_options_get_volume();
//     if (VOLUME_OFF == buzVolume)
//     {
//         return;
//     }
//    app_buzzer_set_volume(buzVolume);

    app_buzzer_disable();

    buzzerPatternIdx = BUZ_PAT_ALARM;

    tcc_init(&tcc_instance_buzzer, BUZZER_MODULE, &config_tcc_buzzer);
    tcc_set_top_value(&tcc_instance_buzzer, BUZ_FREQ_MAX);
    tcc_set_compare_value(&tcc_instance_buzzer, (TCC_MATCH_CAPTURE_CHANNEL_0 + BUZZER_CHANNEL), (BUZ_FREQ_MAX / 2));
    tcc_register_callback(&tcc_instance_buzzer, app_buzzer_alarmCallback, (TCC_CALLBACK_CHANNEL_0 + BUZZER_CHANNEL));
    tcc_enable(&tcc_instance_buzzer);
    tcc_enable_callback(&tcc_instance_buzzer, (TCC_CALLBACK_CHANNEL_0 + BUZZER_CHANNEL));
}

void app_buzzer_alarm_stop(void)
{
    buzzerPatternIdx = BUZ_PAT_NONE;

    tcc_disable(&tcc_instance_buzzer);
    tcc_disable_callback(&tcc_instance_buzzer, (TCC_CALLBACK_CHANNEL_0 + BUZZER_CHANNEL));
    app_buzzer_disable();
}

enum app_buzzer_pattern_t app_buzzer_pattern_playing(void)
{
    return buzzerPatternIdx;
}

void app_buzzer_stop_pattern(enum app_buzzer_pattern_t pattern)
{
    if (buzzerPatternIdx == pattern)
    {
        buzzerPatternIdx = BUZ_PAT_NONE;

        tcc_disable(&tcc_instance_buzzer);

        port_pin_set_output_level(BUZZER_PIN, false);

        SYS_TimerStop(&app_buzzer_alarm_note_timer);

        buzzerState = BUZ_STATE_END;
    }
}

// static void app_buzzer_set_volume(uint8_t volume)
// {
// #ifndef ALT_NO_SHIFT_REG  // ALT_NO_SHIFT_REG has no volume pin, but must preserve standard code
// //     if (VOLUME_OFF == volume)
// //     {
// //         return;
// //     }
// //     else if (volume >= VOLUME_MIN && volume < VOLUME_MAX)
// //     {
// //         app_ext_gpio_set_pin(VOLUME_PIN, VOLUME_LOW);
// //     }
// //     else
// //     {
// //         // VOLUME_MAX
// //         app_ext_gpio_set_pin(VOLUME_PIN, VOLUME_HIGH);
// //     }
// #endif  // ALT_NO_SHIFT_REG
// }
