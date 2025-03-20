/*
#pragma once
// EOSMenu.h
#ifndef EOSMENU_H
#define EOSMENU_H
#include <iostream>
#include <cstdlib> // For system("CLS") or system("clear")
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

using namespace NCL;
using namespace CSC8508;

class EOSMenu {
public:
    void ShowMenu();
    EOSMenu();

private:
    void AuthenticateUser();
    void DisplayOptions();
    void CreateLobby();
    void JoinLobby();
    StateMachine* stateMachine;
    void Update(float dt);

};

#endif // EOSMENU_H
*/