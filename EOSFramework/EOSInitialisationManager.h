#if EOSBUILD

#pragma once
#include <eos_sdk.h>
#include <iostream>
#include <functional>
#include <thread>

class EOSInitialisationManager {
public:
    EOSInitialisationManager();
    ~EOSInitialisationManager();

    void StartEOS();
    void RunUpdateLoop();
    void LoginAnonymous();
    void LoginWithAccountPortal();

    EOS_HPlatform GetPlatformHandle() const;
    EOS_ProductUserId GetLocalUserId() const;

    EOS_ProductUserId LocalUserId = nullptr;
    EOS_HPlatform PlatformHandle;
    
    void Tick();

private:
    static void OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
    void HandleSuccessfulLogin(const EOS_Auth_LoginCallbackInfo* Data);
    
    static void OnCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);
    static void OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
    
    std::thread updateThread;
    std::atomic<bool> running; // Flag to control the update loop
    
    std::function<void()> authCompleteCallback;
};

#endif