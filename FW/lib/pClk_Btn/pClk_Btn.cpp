#include "pClk_Btn.h"



//--------------------------------------------------------------------------------
//  PClkButton
//--------------------------------------------------------------------------------

PClkButton::PClkButton(uint8_t pin)
    : _nPin(pin)
{

}


//--------------------------------------------------------------------------------
//  Init
//--------------------------------------------------------------------------------

void PClkButton::init(sInterData* psShared)
{
    _psIntreD = psShared;

    if (_psIntreD != nullptr)
    {
        memset((void*)&(_psIntreD->sBtnst), 0, sizeof(_psIntreD->sBtnst));
    }

    _btnQueue = xQueueCreate(10, sizeof(RawEvt));

    pinMode(_nPin, INPUT_PULLUP);

    attachInterruptArg(_nPin, _isr_handler, this, CHANGE);
}


//--------------------------------------------------------------------------------
//  Run
//--------------------------------------------------------------------------------

void PClkButton::run()
{
    if (!_psIntreD || !_btnQueue) return;

    RawEvt sEvt;

    // Receive event and set Req
    if (_psIntreD->sBtnst.nBusy_core0 == 0)
    {
        if (xQueueReceive(_btnQueue, &sEvt, pdMS_TO_TICKS(50)))
        {
            if (sEvt.eType == RawType::PRESS)
            {
                _nPressStartTime = sEvt.nTime;
            }
            else if (sEvt.eType == RawType::RELEASE && _nPressStartTime != 0)
            {
                uint32_t nDur = sEvt.nTime - _nPressStartTime;
                bool bNewReq = false;

                if (nDur >= 10000)
                {
                    _psIntreD->sBtnst.nHold_10s_req = 1;
                    bNewReq = true;
                }
                else if (nDur >= 3000)
                {
                    _psIntreD->sBtnst.nHold_3s_req = 1;
                    bNewReq = true;
                }
                else if (nDur >= 50)
                {
                    _psIntreD->sBtnst.nClick_req = 1;
                    bNewReq = true;
                }

                // Set flag 
                if (bNewReq)
                {
                    _psIntreD->sBtnst.nIsChanged = 1;
                }

                _nPressStartTime = 0;
            }
        }
    }

    // Check handshake
    if (_psIntreD->sBtnst.nBusy_core0 == 0 && _psIntreD->sBtnst.nIsChanged == 1)
    {
        if (_psIntreD->sBtnst.nClick_req == 1 && _psIntreD->sBtnst.nCick_ack == 1)
        {
            _psIntreD->sBtnst.nClick_req = 0;
            _psIntreD->sBtnst.nCick_ack = 0;
        }

        if (_psIntreD->sBtnst.nHold_3s_req == 1 && _psIntreD->sBtnst.nHold_3s_ack == 1)
        {
            _psIntreD->sBtnst.nHold_3s_req = 0;
            _psIntreD->sBtnst.nHold_3s_ack = 0;
        }

        if (_psIntreD->sBtnst.nHold_10s_req == 1 && _psIntreD->sBtnst.nHold_10s_ack == 1)
        {
            _psIntreD->sBtnst.nHold_10s_req = 0;
            _psIntreD->sBtnst.nHold_10s_ack = 0;
        }

        if (_psIntreD->sBtnst.nClick_req == 0 && _psIntreD->sBtnst.nHold_3s_req == 0 
                                              && _psIntreD->sBtnst.nHold_10s_req == 0)
        {
            _psIntreD->sBtnst.nIsChanged = 0;
        }
    }
}


//--------------------------------------------------------------------------------
//  ISR Handler
//--------------------------------------------------------------------------------

void IRAM_ATTR PClkButton::_isr_handler(void* arg)
{
    PClkButton* self = (PClkButton*)arg;
    uint32_t nNow = millis();

    if (nNow - self->_nLastIsrTime < 30) return;

    self->_nLastIsrTime = nNow;

    RawEvt sEvt;
    bool bPinState = (GPIO.in >> self->_nPin) & 1;
    sEvt.eType = (bPinState == 0) ? RawType::PRESS : RawType::RELEASE;
    sEvt.nTime = nNow;

    xQueueSendFromISR(self->_btnQueue, &sEvt, NULL);
}
