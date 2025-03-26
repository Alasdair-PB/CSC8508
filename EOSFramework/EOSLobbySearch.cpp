#if !PS5

#include "EOSLobbySearch.h"
#include "EOSLobbyManager.h"
#include "EOSLobbyFunctions.h"

EOSLobbySearch::EOSLobbySearch(EOSInitialisationManager& initManager)
    : eosInitManager(initManager) {
    std::cout << "[EOSLobbySearch] Constructor called. Initializing state..." << std::endl;

}

EOSLobbySearch::~EOSLobbySearch() {
    std::cout << "[EOSLobbySearch] Destructor called. Releasing handles..." << std::endl;
}

// Initiates a lobby search for a specified target lobby ID
void EOSLobbySearch::CreateLobbySearch(const char* TargetLobbyId) {
    std::cout << "[CreateLobbySearch] Attempting to create a lobby search..." << std::endl;

    // Ensures that the user is authenticated before proceeding
    if (!eosInitManager.GetLocalUserId()) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby search." << std::endl;
        return;
    }

    // Ensures that the EOS platform is initialised before proceeding
    if (!eosInitManager.GetPlatformHandle()) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    LobbyHandle = EOS_Platform_GetLobbyInterface(eosInitManager.GetPlatformHandle());

    // Ensures that the lobby interface is initialised properly
    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    // Configures the lobby search options
    EOS_Lobby_CreateLobbySearchOptions SearchOptions = {};
    SearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
    SearchOptions.MaxResults = 10;

    // Creates a new lobby search request
    EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &SearchOptions, &LobbySearchHandle);

    // Checks if the lobby search was successfully created
    if (Result == EOS_EResult::EOS_Success) {
        std::cout << "[CreateLobbySearch] Lobby search created successfully." << std::endl;

        // Sets the target lobby ID for filtering the search
        EOS_LobbySearch_SetLobbyIdOptions SetLobbyIdOptions = {};
        SetLobbyIdOptions.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
        SetLobbyIdOptions.LobbyId = TargetLobbyId;

        // Applies the search filter based on the lobby ID
        EOS_LobbySearch_SetLobbyId(LobbySearchHandle, &SetLobbyIdOptions);
        std::cout << "[CreateLobbySearch] Searching for the lobby with ID: " << TargetLobbyId << std::endl;

        // Configures the options for executing the lobby search
        EOS_LobbySearch_FindOptions FindOptions = {};
        FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
        FindOptions.LocalUserId = eosInitManager.GetLocalUserId();

        // Initiates the lobby search process
        EOS_LobbySearch_Find(LobbySearchHandle, &FindOptions, this, OnFindLobbiesComplete);
    }
    else {
        std::cerr << "[ERROR] Failed to create lobby search. Error: " << EOS_EResult_ToString(Result) << std::endl;
    }

    std::cout << "[CreateLobbySearch] Search completed. Proceeding with lobby details retrieval." << std::endl;

    running = true;

    RunUpdateLoop();

}

// New function to keep updating EOS
void EOSLobbySearch::RunUpdateLoop()
{
    while (running) {
        if (eosInitManager.PlatformHandle) {
            EOS_Platform_Tick(eosInitManager.PlatformHandle);
        }
    }
}


// Callback function for handling the results of the lobby search
void EOSLobbySearch::OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data) {

    EOSLobbySearch* self = static_cast<EOSLobbySearch*>(Data->ClientData); // Cast back

    // Checks if the lobby search was successful
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        int32_t numResults = 0;

        // Initialises options for retrieving the number of lobbies found
        EOS_LobbySearch_GetSearchResultCountOptions CountOptions = {};
        CountOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

        // Gets the number of lobbies found in the search
        numResults = EOS_LobbySearch_GetSearchResultCount(self->LobbySearchHandle, &CountOptions);

        std::cerr << "[OnFindLobbiesComplete] Found " << numResults << " lobbies.\n";

        // Checks if no lobbies were found and logs debug information
        if (numResults == 0) {
            std::cerr << "[DEBUG] No lobbies found. Check attributes and permissions.\n";
            return;
        }

        // Configures options for copying a lobby search result by index
        EOS_LobbySearch_CopySearchResultByIndexOptions CopyOptions = {};
        CopyOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
        CopyOptions.LobbyIndex = 0;

        // Attempts to copy the lobby search result
        EOS_EResult Result = EOS_LobbySearch_CopySearchResultByIndex(self->LobbySearchHandle, &CopyOptions, &self->LobbyDetailsHandle);

        // Adds a brief delay to ensure proper retrieval
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (Result == EOS_EResult::EOS_Success) {
            std::cerr << "[LobbyData] Lobby Details Retrieved\n";

            // Pointer to store retrieved lobby information
            EOS_LobbyDetails_Info* LobbyInfo = nullptr;
            EOS_LobbyDetails_CopyInfoOptions InfoOptions = {};
            InfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

            // Attempts to copy the lobby details information
            EOS_EResult InfoResult = EOS_LobbyDetails_CopyInfo(self->LobbyDetailsHandle, &InfoOptions, &LobbyInfo);

            if (InfoResult == EOS_EResult::EOS_Success && LobbyInfo) {
                // Logs the lobby details retrieved
                std::cerr << "[LobbyInfo] Lobby ID: " << LobbyInfo->LobbyId << "\n";
                std::cerr << "[LobbyInfo] Max Players: " << LobbyInfo->MaxMembers << "\n";
                std::cerr << "[LobbyInfo] Owner ID: " << LobbyInfo->LobbyOwnerUserId << "\n";

                // Retrieves the number of members in the lobby
                EOS_LobbyDetails_GetMemberCountOptions MemberCountOptions = {};
                MemberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;
                int32_t memberCount = EOS_LobbyDetails_GetMemberCount(self->LobbyDetailsHandle, &MemberCountOptions);

                std::cerr << "[LobbyMembers] Found " << memberCount << " members in the lobby.\n";

                // Iterates through each member in the lobby and retrieves their user ID
                for (int32_t i = 0; i < memberCount; ++i) {
                    // Configures options to get a lobby member by index
                    EOS_LobbyDetails_GetMemberByIndexOptions MemberByIndexOptions = {};
                    MemberByIndexOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
                    MemberByIndexOptions.MemberIndex = i;

                    // Retrieves the member's ProductUserId
                    EOS_ProductUserId MemberId = EOS_LobbyDetails_GetMemberByIndex(self->LobbyDetailsHandle, &MemberByIndexOptions);

                    // Validates and converts the member ID to a string
                    if (MemberId) {
                        char MemberIdStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
                        int32_t BufferSize = sizeof(MemberIdStr);
                        EOS_EResult ConvertResult = EOS_ProductUserId_ToString(MemberId, MemberIdStr, &BufferSize);

                        // Logs the member ID if conversion is successful
                        if (ConvertResult == EOS_EResult::EOS_Success) {
                            std::cerr << "[LobbyMembers] Member " << i << " ID: " << MemberIdStr << "\n";
                            self->LobbyMemberIds.push_back(MemberIdStr);
                        }
                        else {
                            std::cerr << "[LobbyMembers] Failed to convert member ID.\n";
                        }
                    }
                    else {
                        std::cerr << "[LobbyMembers] Failed to get member ID at index " << i << ".\n";
                    }
                }
            }
            else {
                std::cerr << "[ERROR] Failed to retrieve lobby info. Error: " << EOS_EResult_ToString(InfoResult) << std::endl;
            }
        }
        else {
            std::cerr << "[LobbyData] Failed to fetch lobby data. Error: " << EOS_EResult_ToString(Result) << std::endl;
        }
    }
    else {
        std::cerr << "[OnFindLobbiesComplete] Lobby search failed. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }

    std::cout << "Search complete" << std::endl;

    self->running = false;
}

// Returns the current lobby search handle
EOS_HLobbySearch EOSLobbySearch::GetLobbySearchHandle() const {
    return LobbySearchHandle;
}

// Returns the current lobby details handle
EOS_HLobbyDetails EOSLobbySearch::GetLobbyDetailsHandle() const {
    return LobbyDetailsHandle;
}

// Returns a list of member IDs in the found lobby
std::vector<std::string> EOSLobbySearch::GetLobbyMemberIds() const {
    return LobbyMemberIds;
}

#endif