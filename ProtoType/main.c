#include <techlib.h>
#include "demo_io.h"
#include <smartvr.h>
#include "incAndDef.h"

#define MAX_TRIES 3     // maximum # of tries to recognize a command word

void  CycleKnob(void);
void  MenuRecognized(uchar class);
void  CommRecognized(uchar class, int Mnum);

// global variables
PARAMETERPASS results;
far cdata *sound_table = &SNDTBL_SOUNDS;
//far cdata *sound_table2 = &SNDTBL_GOFIRST;

uchar knob = T2SI_DEFAULT_KNOB;  // acceptance knob setting for commands [0:4]

// Define GPIO outputs for voice commands
// from GPIO05 to GPIO14 inclusive (x = 1 ... 10)
#define COMMAND_PIN(x) 	GPIO(x) //#define COMMAND_PIN(x) 	GPIO(4+(x))
#define COMMAND_PIN_ALL	GPIO_SPAN(9,12)
// Note: GPIO09 to GPIO12 are used for DevBoard LEDs
// open jumper blocks JP10 to JP13 to disconnect these LEDs
// and free the GPIO pins for external devices

uchar tryCtr;       // # of tries remaining for commands [0:3]
uchar error;        // value returned from _T2SI

//Beep
#define Beep _PlaySnd(0, (long)sound_table, 256)

//------------------------------------------------------------------------------

void main(void)
{
    
    uchar class;        // index of recognized word from _T2SI
    

    // Configure command GPIOs as outputs initially low
    _SetOutput(COMMAND_PIN_ALL, 0xFFFF);

    // At reset, give a beep, initialize variables, then announce
    // saying Main Menu w/o any purpose.... need to delete it

    _PlaySnd(0, (long)sound_table, 256);

    // Enable abort handler when Button A pressed
      _SetCalloutGPIOEvent(BTN_A, 0);

    // Main loop. Recognize the trigger word with no timeout.
    // NOTE: default timeout for a trigger grammar is 0 = no timeout
    // Trigger recognition returns when a trigger word is recognized
    // with High Confidence or when certain errors are detected.

    while (1)
    {
		_WritePin(COMMAND_PIN_ALL,0xFFFF);
		Beep;
        _SetupCallout(CH_GPIO);
        error = _T2SI((long)&AC_MODEL_MAIN, (long)&GM_MODEL_TRG_MAIN, knob,
                      10, T2SI_DEFAULT_TRAILING, &results);
        _SetupCallout(CH_DISABLE);

        // Trigger error processing
        if (error != ERR_OK)
        {
            // user interrupt from HMM_H handler allows knob adjustment
            if ( error == ERR_INT_BLOCK )
            {
                CycleKnob();
                continue;      // continue listening for trigger
            }

            // ERR_DATACOL_TIMEOUT will not occur if default timeout of 0 is used.
            // Other errors, e.g. ERR_T2SI_XXX, indicate problems with T2SI data
            // files or with parameters to T2SI that should be fixed during
            // development and will not occur in a finished application.

            
         //   break;
            continue;
			
        }

        // Trigger recognized, beep and initialize command retry count
        //Beep;
        tryCtr = MAX_TRIES;
		_PlaySnd(SND_MainMenu, (long)sound_table, 256);
		
        while (tryCtr)
        {
            // recognize a command word with 3 second timeout------------------------------nope
            // NOTE: default timeout for a command grammar is 3------------------------------nope
			Beep;
            _SetupCallout(CH_GPIO);
            error = _T2SI((long)&AC_MODEL_MAIN, (long)&GM_MODEL_MAIN, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
            class = results.pp_b;
            _SetupCallout(CH_DISABLE);

            // on "none-of-the-above"(NOTA) or low confidence, signal error
            // and, if retries remain, continue recognizing command
            // NOTE: Screen out NOTAs before checking for recognized command

            if ((class == G_comm_MainOpt_nota) || (error == ERR_RECOG_LOW_CONF) || (error == ERR_RECOG_MID_CONF))
            {
                //_RedOn();       // signal "not recognized"
                _PlaySnd(SND_PleaseRepeat, (long)sound_table, 256);
                tryCtr--;       // count this try
                continue;       // If any tries remain, resume command recognition
            }                   // else program will go back to waiting for trigger

            // on high or mid confidence, perform action for recognized command
            if ((error == ERR_OK) )//|| (error == ERR_RECOG_MID_CONF
            {
                MenuRecognized(class);
                break;          // go back to waiting for trigger
            }

            // Command error processing
            // user interrupt from HMM_H handler. Allow knob adjustment
            // then go back to waiting for trigger
            if ( error == ERR_INT_BLOCK )
            {
                CycleKnob();
                break;
            }

            // timeout exceeded, go back to waiting for trigger
            if ( error == ERR_DATACOL_TIMEOUT )
			{
                //_RedOn();       // signal "not recognized"
               /* _PlaySnd(SND_pleaserepeat, (long)sound_table, 256);
                tryCtr--;       // count this try*/
                continue;       // If any tries remain, resume command recognition
			}
            // ignore other errors, such as ERR_DATACOL_TOO_SOFT,
            // and continue recognizing command

        } // end Command loop
    } // end Trigger loop
}

//------------------------------------------------------------------------------
void MenuRecognized(uchar class)
{
	int Mnum;
// Come here on HIGH /*or MID*/ confidence command recognition to
// perform action for the recognized command
//
// class has index of command recognized
// The comm_rscApp_xxx.h file contains indices for command vocabulary words
//
// This shows typical results dispatching to perform command-specific actions
// Here they are all lumped together to announce: "You said word X"

    //_RedOff();                      // turn off any error signal
    switch (class)
    {
    case G_comm_MainOpt_MOTION_CONTROL:
		_WritePin(COMMAND_PIN(9),0);
        _PlaySnd(SND_MotionCtrlMenu, (long)sound_table, 256);
		error = _T2SI((long)&AC_MODEL_MOTN, (long)&GM_MODEL_MOTN, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
		class = results.pp_b;
		tryCtr = MAX_TRIES;
		while(tryCtr--)
		{
			if ((error == ERR_OK) )
			{
				Mnum = 1;
				CommRecognized(class, Mnum);
				break;
			}
			else	
			{
				_PlaySnd(SND_PleaseRepeat, (long)sound_table, 256);
				error = _T2SI((long)&AC_MODEL_MOTN, (long)&GM_MODEL_MOTN, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
				class = results.pp_b;
				continue;
				
			}
		}
        break;
		
    case G_comm_MainOpt_GENERAL_COMMANDS:
		_WritePin(COMMAND_PIN(10),0);
		_PlaySnd(SND_GnrlCmdMn, (long)sound_table, 256);
		error = _T2SI((long)&AC_MODEL_GNRL, (long)&GM_MODEL_GNRL, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
		class = results.pp_b;
		tryCtr = MAX_TRIES;
		while(tryCtr--)
		{
			if ((error == ERR_OK) )
			{
				Mnum = 2;
				CommRecognized(class, Mnum);
				break;
			}
			else	
			{
				_PlaySnd(SND_PleaseRepeat, (long)sound_table, 256);
				error = _T2SI((long)&AC_MODEL_GNRL, (long)&GM_MODEL_GNRL, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
				class = results.pp_b;
				continue;
				
			}
		}
		
		break;
		
    case G_comm_MainOpt_SMART_MOVES:
		_WritePin(COMMAND_PIN(11),0);
		_PlaySnd(SND_SmrtMvsMenu, (long)sound_table, 256);
		error = _T2SI((long)&AC_MODEL_SMRT, (long)&GM_MODEL_SMRT, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
		class = results.pp_b;
		tryCtr = MAX_TRIES;
		while(tryCtr--)
		{
			if ((error == ERR_OK) )
			{
				Mnum = 3;
				CommRecognized(class, Mnum);
				break;
			}
			else	
			{
				_PlaySnd(SND_PleaseRepeat, (long)sound_table, 256);
				error = _T2SI((long)&AC_MODEL_SMRT, (long)&GM_MODEL_SMRT, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
				class = results.pp_b;
				continue;
				
			}
		}
		
		break;
		
    case G_comm_MainOpt_SETTINGS:
		_WritePin(COMMAND_PIN(12),0);
		_PlaySnd(SND_SettingsMenu, (long)sound_table, 256);
		error = _T2SI((long)&AC_MODEL_STNG, (long)&GM_MODEL_STNG, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
		class = results.pp_b;
		tryCtr = MAX_TRIES;
		while(tryCtr--)
		{
			if ((error == ERR_OK) )
			{
				Mnum = 4;
				CommRecognized(class, Mnum);
				break;
			}
			else	
			{
				_PlaySnd(SND_PleaseRepeat, (long)sound_table, 256);
				error = _T2SI((long)&AC_MODEL_STNG, (long)&GM_MODEL_STNG, knob,
                          10, T2SI_DEFAULT_TRAILING, &results);
				class = results.pp_b;
				continue;
				
			}
		}
		break;
        // speech feedback
       // _PlaySnd(SND_yousaid1 + class - G_comm_sample_ONE, (long)sound_table2, 256);
        // set matching GPIO high
       // _WritePin(COMMAND_PIN(1 + class - G_comm_sample_ONE), 0xFFFF);


    default: // will not come here if all vocab words are checked above
        break;
    }
}


void CommRecognized(uchar class, int Mnum)
{
	switch(Mnum)
	{
		case 1:
			_PlaySnd(SND_yccn + class, (long)sound_table, 256);
			_PlaySnd(SND_FromMotion, (long)sound_table, 256);
			break;
		
		case 2:
			_PlaySnd(SND_yccn + class, (long)sound_table, 256);
			_PlaySnd(SND_FromGeneral, (long)sound_table, 256);
			break;
		
		case 3:
			_PlaySnd(SND_yccn + class, (long)sound_table, 256);
			_PlaySnd(SND_FromSmart, (long)sound_table, 256);
			break;
		
		case 4:
			_PlaySnd(SND_yccn + class, (long)sound_table, 256);
			_PlaySnd(SND_FromSettings, (long)sound_table, 256);
			break;
	}	
}


//------------------------------------------------------------------------------
void CycleKnob(void)
{/*
    // Announce the knob (acceptance level, knob range is 0-4)
    // If the A button remains pressed, cycle knob until button is released.
    _PlaySnd(SND_level00+knob, (long)sound_table, 256);
    _Wait10MilliSecs(40);

    while ( ButtonAPressed )
    {
        if ( ++knob > T2SI_MAX_KNOB )
            knob = T2SI_MIN_KNOB;
        _PlaySnd(SND_level00+knob, (long)sound_table, 256);
        _Wait10MilliSecs(40);
    }
*/}










