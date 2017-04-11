#ifndef DEF_AND_INC
#define DEF_AND_INC

#include "Qs5\SOUNDS.h"
#include "recog\MainOpt\comm_rscApp_MainOpt.h"
#include "recog\MainOpt\trig_rscApp_MainOpt.h"
#include "recog\GeneralOpt\comm_rscApp_GeneralOpt.h"   
#include "recog\SettingsOpt\comm_rscApp_SettingsOpt.h"   
#include "recog\MotionOpt\comm_rscApp_MotionOpt.h"   
#include "recog\SmartOpt\comm_rscApp_SmartOpt.h"   

#define AC_MODEL_MAIN nn_MainOpt
#define AC_MODEL_GNRL nn_GeneralOpt
#define AC_MODEL_MOTN nn_MotionOpt
#define AC_MODEL_STNG nn_SettingsOpt
#define AC_MODEL_SMRT nn_SmartOpt

#define GM_MODEL_TRG_MAIN gsearch_trig_MainOpt // for triger

//for commands
#define GM_MODEL_MAIN gsearch_comm_MainOpt //for commands
#define GM_MODEL_GNRL gsearch_comm_GeneralOpt
#define GM_MODEL_MOTN gsearch_comm_MotionOpt
#define GM_MODEL_STNG gsearch_comm_SettingsOpt
#define GM_MODEL_SMRT gsearch_comm_SmartOpt

#endif
