#define EOSIPDISTRIBUTION_H

#include <iostream>
#include <cstring>
#include <stdint.h>
#include "eos_p2p.h"
#include "eos_sdk.h"
#include "EOSInitialisationManager.h"

class EOSIPDistribution {
public:
    static EOSIPDistribution& GetInstance();

    void SendPacket(EOS_ProductUserId TargetUserId);
    void ReceivePacket(EOS_ProductUserId TargetUserId);

private:
    static constexpr uint8_t CHANNEL = 0;
    static constexpr uint32_t PACKET_SIZE = 256;
};
