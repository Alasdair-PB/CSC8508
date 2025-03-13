#pragma once
#include <eos_sdk.h>
#include <iostream>
#include <functional>

class EOSInitialisationManager {
public:
    static EOSInitialisationManager& GetInstance();

    void StartEOS(std::function<void()> onAuthComplete);
    void LoginAnonymous();

    EOS_HPlatform GetPlatformHandle() const;
    EOS_ProductUserId GetLocalUserId() const;

private:
    EOSInitialisationManager();
    ~EOSInitialisationManager();

    static void OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
    void LoginWithAccountPortal();
    void HandleSuccessfulLogin(const EOS_Auth_LoginCallbackInfo* Data);
    static void OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);

    static void OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);
    std::function<void()> authCompleteCallback; // Store the callback
    EOS_HPlatform PlatformHandle = nullptr;
    EOS_ProductUserId LocalUserId = nullptr;
};
