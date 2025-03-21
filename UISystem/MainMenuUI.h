#pragma once
#include "imgui.h"

namespace NCL {
	namespace UI {

		class MainMenuUI {
		public:
			MainMenuUI();
			~MainMenuUI();

			int DrawMainMenu();
			int DrawLobbyMenu();

			int GetMenuOption() {
				return menuOption;
			}

		protected:
			int menuOption = 0;
			enum menuOptions { none, startOffline, startServer, StartClient, eosOption, hostLobby, joinLobby, lobby }; //Relates to menuOptions in MainMenu.h
		};
	}
}