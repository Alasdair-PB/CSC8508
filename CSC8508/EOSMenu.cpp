#include "EOSMenu.h"
#include "EOSLobbyFunctions.h"
#include "EOSLobbySearch.h"

void EOSMenu::ShowMenu() {
    AuthenticateUser();
}

void EOSMenu::AuthenticateUser() {
    std::cout << "[EOSMenu] Starting authentication..." << std::endl;

    EOSInitialisationManager::GetInstance().StartEOS([this]() {
        std::cout << "[EOSMenu] Authentication and user creation complete. Clearing console..." << std::endl;
        system("CLS");
        DisplayOptions(); 
        });
}

// Displays the EOS Lobby Menu and handles user choices
void EOSMenu::DisplayOptions() {
    int choice = 0;

    // Continues displaying the menu until the user chooses to exit
    while (choice != 3) {
        std::cout << "\n=== EOS Lobby Menu ===" << std::endl;
        std::cout << "1. Create Lobby" << std::endl;
        std::cout << "2. Join Lobby" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        // Checks the user's input and executes the corresponding function
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

// Creates a new lobby and initiates a search to retrieve its details
void EOSMenu::CreateLobby() {
    EOSLobbySearch& lobbySearch = EOSLobbySearch::GetInstance();
    lobbySearch.searchComplete = false;
    std::cout << "[EOSMenu] Creating lobby..." << std::endl;

    // Requests to create the lobby
    EOSLobbyManager& lobbyManager = EOSLobbyManager::GetInstance();
    lobbyManager.CreateLobby();

    // Wait until the lobby ID is assigned by the callback
    while (lobbyManager.LobbyId[0] == '\0') {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[EOSMenu] Lobby created successfully with ID: " << lobbyManager.LobbyId << std::endl;

    // Initiates a search using the newly assigned lobby ID
    lobbySearch.CreateLobbySearch(lobbyManager.LobbyId);

    // Wait for the search to complete
    while (!lobbySearch.IsSearchComplete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[EOSMenu] Lobby search complete. Updating lobby details..." << std::endl;

    // Ensures the lobby details handle is valid before proceeding
    if (!EOSLobbySearch::GetInstance().GetLobbyDetailsHandle()) {
        std::cerr << "[ERROR] Lobby details handle is NULL. UpdateLobbyDetails() will not be called." << std::endl;
        return;
    }

    // Updates lobby details now that the search is complete
    EOSLobbyFunctions::GetInstance().UpdateLobbyDetails();
}

// Allows the user to search for and join a specified lobby
void EOSMenu::JoinLobby() {
    // Prompts the user to enter the Lobby ID they wish to join
    std::string lobbyId;
    std::cout << "Enter the Lobby ID to search and join: ";
    std::cin >> lobbyId;

    // Ensures the user has entered a valid Lobby ID
    if (lobbyId.empty()) {
        std::cerr << "[ERROR] Lobby ID cannot be empty!" << std::endl;
        return;
    }

    // Initiates a lobby search with the specified Lobby ID
    EOSLobbySearch& lobbySearch = EOSLobbySearch::GetInstance();
    lobbySearch.CreateLobbySearch(lobbyId.c_str());

    std::cout << "[JoinLobby] Waiting for search to complete...\n";

    // Waits for the search process to complete
    while (!lobbySearch.IsSearchComplete()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EOS_Platform_Tick(EOSInitialisationManager::GetInstance().GetPlatformHandle());
    }

    std::cout << "[JoinLobby] Search complete. Attempting to join lobby...\n";

    // Creates an instance of EOSLobbyFunctions to handle the joining process
    EOSLobbyFunctions lobbyFunctions;
    lobbyFunctions.JoinLobby();
}
