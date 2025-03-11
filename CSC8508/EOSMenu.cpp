// EOSMenu.cpp
#include "EOSMenu.h"
#include "EOSLobbyFunctions.h"
#include "EOSLobbySearch.h"

void EOSMenu::ShowMenu() {
    AuthenticateUser(); // Start authentication
    // Don't call ClearConsole() or DisplayOptions() here
}

void EOSMenu::AuthenticateUser() {
    std::cout << "[EOSMenu] Starting authentication..." << std::endl;

    // Pass a lambda to be executed after authentication completes
    EOSInitialisationManager::GetInstance().StartEOS([this]() {
        std::cout << "[EOSMenu] Authentication and user creation complete. Clearing console..." << std::endl;
        ClearConsole();
        DisplayOptions(); // Now called only after authentication completes
        });
}


void EOSMenu::ClearConsole() {
#ifdef _WIN32
    system("CLS"); // Clear console for Windows
#else
    system("clear"); // Clear console for Unix/Linux
#endif
}

void EOSMenu::DisplayOptions() {
    int choice = 0;
    while (choice != 3) {
        std::cout << "\n=== EOS Lobby Menu ===" << std::endl;
        std::cout << "1. Create Lobby" << std::endl;
        std::cout << "2. Join Lobby" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            CreateLobby();
            break;
        case 2:
            JoinLobby();
            break;
        case 3:
            std::cout << "Exiting..." << std::endl;
            return;
        default:
            std::cout << "Invalid choice, please try again." << std::endl;
        }
    }
}

void EOSMenu::CreateLobby() {
    EOSLobbySearch& lobbySearch = EOSLobbySearch::GetInstance();
    lobbySearch.searchComplete = false;
    std::cout << "[EOSMenu] Creating lobby..." << std::endl;

    EOSLobbyManager& lobbyManager = EOSLobbyManager::GetInstance();
    lobbyManager.CreateLobby(); // Request to create the lobby

    // Wait until the lobby ID is assigned by the callback
    while (lobbyManager.LobbyId[0] == '\0') {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[EOSMenu] Lobby created successfully with ID: " << lobbyManager.LobbyId << std::endl;

    // Now start the search using the newly assigned lobby ID
    
    lobbySearch.CreateLobbySearch(lobbyManager.LobbyId);

    // Wait for the search to complete
    while (!lobbySearch.IsSearchComplete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[EOSMenu] Lobby search complete. Updating lobby details..." << std::endl;


    if (!EOSLobbySearch::GetInstance().GetLobbyDetailsHandle()) {
        std::cerr << "[ERROR] Lobby details handle is NULL. UpdateLobbyDetails() will not be called." << std::endl;
        return;
    }
    EOSLobbyFunctions::GetInstance().UpdateLobbyDetails();


}



void EOSMenu::JoinLobby() {
    std::string lobbyId;
    std::cout << "Enter the Lobby ID to search and join: ";
    std::cin >> lobbyId;

    if (lobbyId.empty()) {
        std::cerr << "[ERROR] Lobby ID cannot be empty!" << std::endl;
        return;
    }

    EOSLobbySearch& lobbySearch = EOSLobbySearch::GetInstance();
    lobbySearch.CreateLobbySearch(lobbyId.c_str());

    std::cout << "[JoinLobby] Waiting for search to complete...\n";

    while (!lobbySearch.IsSearchComplete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[JoinLobby] Search complete. Attempting to join lobby...\n";
}
