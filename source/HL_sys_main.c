/** @file HL_sys_main.c 
*   @brief Application main file
*   @date 11-Dec-2018
*   @version 04.07.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com  
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
#include "HL_gio.h"
#include "HL_rti.h"
#include "HL_sci.h"
/* USER CODE END */

/* Include Files */

#include "HL_sys_common.h"

/* USER CODE BEGIN (1) */
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
uint8_t Intensity_Value = 50;
bool Update_Required = TRUE;

/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    gioInit();
    rtiInit();
    sciInit();

    /* Enable RTI Compare 0 interrupt notification (Period Interrupt) */
    rtiEnableNotification(rtiREG1,rtiNOTIFICATION_COMPARE0);

    /* Start RTI Counter Block 0 (Start timer for period interrupt) */
    rtiStartCounter(rtiREG1,rtiCOUNTER_BLOCK0);


    /* Enable IRQ - Clear I flag in CPS register */
    _enable_IRQ_interrupt_();


    while(1)
    {
        Intensity_Value = sciReceiveByte(sciREG1);  /*Poll byte for intensity change*/
        if(Intensity_Value <= 100)                  /*Verify if intensity is less than 100 or not*/
        {
            Update_Required = TRUE;
        }
    }
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
    if(notification == rtiNOTIFICATION_COMPARE0)    /*Period Interrupt*/
    {
        if(Intensity_Value == 100)
        {
            /*T_OFF is zero, so we no need to set the duty cycle*/
            /*All LED's should be ON for entire period*/
            gioSetBit(gioPORTB, 6, 1);
            gioSetBit(gioPORTB, 7, 1);
            gioSetBit(gioPORTB, 0, 1);
        }
        else if (Intensity_Value == 0)
        {
            /*T_ON is zero, so we no need to set the duty cycle*/
            /*All LED's should be OFF for entire period*/
            gioSetBit(gioPORTB, 6, 0);
            gioSetBit(gioPORTB, 7, 0);
            gioSetBit(gioPORTB, 0, 0);

        }
        else
        {
            /*Intensity is in between 0 and 100, so we should need to duty cycle, to OFF the LED's after required duty cycle*/
            gioSetBit(gioPORTB, 6, 1);
            gioSetBit(gioPORTB, 7, 1);
            gioSetBit(gioPORTB, 0, 1);
            if(Update_Required == TRUE)     /*If new intensity received*/
            {
                uint32_t period_value;
                uint32_t duty_cycle_value;
                period_value = rtiREG1->CMP[0U].UDCPx;                          /*Read the period configured value*/
                duty_cycle_value =  ((period_value * Intensity_Value) / 100);   /*Calculate duty cycle based on the intensity received from UART*/
                rtiREG1->CMP[1U].COMPx = duty_cycle_value;
                rtiREG1->CMP[1U].UDCPx = duty_cycle_value;
                rtiResetCounter(rtiREG1,rtiCOUNTER_BLOCK1);
                Update_Required = FALSE;
            }
            rtiEnableNotification(rtiREG1,rtiNOTIFICATION_COMPARE1);  /* Enable RTI Compare 1 interrupt notification */
            rtiStartCounter(rtiREG1,rtiCOUNTER_BLOCK1); /* Start RTI Counter Block 0 */
        }
    }

    if(notification == rtiNOTIFICATION_COMPARE1)    /*TON completion Interrupt*/
    {
        gioSetBit(gioPORTB, 6, 0);
        gioSetBit(gioPORTB, 7, 0);
        gioSetBit(gioPORTB, 0, 0);

        /* Disable RTI Compare 1 interrupt notification */
        rtiDisableNotification(rtiREG1,rtiNOTIFICATION_COMPARE1);
        rtiStopCounter(rtiREG1,rtiCOUNTER_BLOCK1);
    }
}

/* USER CODE END */
