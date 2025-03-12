#pragma once
// EOSMenu.h
#ifndef EOSMENU_H
#define EOSMENU_H

#include <iostream>
#include <cstdlib> // For system("CLS") or system("clear")
#include "EOSInitialisationManager.h"
#include "EOSLobbyManager.h"

class EOSMenu {
public:
    void ShowMenu();

private:
    void AuthenticateUser();
    void ClearConsole();
    void DisplayOptions();
    void CreateLobby();
    void JoinLobby();
};

#endif // EOSMENU_H
