#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <eos_sdk.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_lobby.h>

extern EOS_HPlatform PlatformHandle;
extern EOS_ProductUserId LocalUserId;

void StartEOS();
void LoginAnonymous();
void OnAuthLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
void OnConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
void CreateLobby();
void OnLobbyCreated(const EOS_Lobby_CreateLobbyCallbackInfo* Data);
void CreateLobbySearch();
void OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data);