#pragma once
#include <eos_sdk.h>
#include <iostream>

class EOSInitialisationManager {
public:
    static EOSInitialisationManager& GetInstance();

    void StartEOS();
    void LoginAnonymous();

    EOS_HPlatform GetPlatformHandle() const;
    EOS_ProductUserId GetLocalUserId() const;

private:
    EOSInitialisationManager();
    ~EOSInitialisationManager();

    static void OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
    static void OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);

    EOS_HPlatform PlatformHandle = nullptr;
    EOS_ProductUserId LocalUserId = nullptr;
};
