/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "Bot.h"
#include "CONTROLService.h"

DigitalOut  led1(p7);
DigitalOut  motorleft_A(p28); //Motor left
DigitalOut  motorleft_B(p25); //
DigitalOut  motorright_A(p24); //Motor right
DigitalOut  motorright_B(p23); //
DigitalOut  Relay_3A(p22); //Motor skill Q (may bao)
DigitalOut  Relay_3B(p21); //
DigitalOut  kichdien(p9);  //relay ac inverter 12DC -220AC
DigitalOut  kichdienB(p16);
Serial      pc(p10, p11);

const static char     DEVICE_NAME[] = "BOT BATTLE";
static const uint16_t uuid16_list[] = {CONTROLService::CONTROL_SERVICE_UUID};
static uint8_t g_cmd=0;

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

CONTROLService *CONTROLServicePtr;
Bot	Bot_battle;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void) params;
    BLE::Instance().gap().startAdvertising();
    led1 = 0;
    pc.printf("\n\r Disconnection \n\r");
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
	pc.printf("\n\r Connected\n\r");
	Bot_battle.connection();
	pc.printf("\n\r %d \n\r",Bot_battle.a);
}

void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == CONTROLServicePtr->getValueHandle()) && (params->len == 1)) {
        g_cmd = params->data[0];
        
    }
    pc.printf("\n\r value %d ",g_cmd);

}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);

    bool initialValueForStateCharacteristic = false;
    CONTROLServicePtr = new CONTROLService(ble, initialValueForStateCharacteristic);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
	pc.baud(115200);
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}