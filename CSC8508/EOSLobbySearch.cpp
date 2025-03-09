#include "EOSLobbySearch.h"
#include "EOSLobbyManager.h"
#include "EOSLobbyFunctions.h"

EOSLobbySearch& EOSLobbySearch::GetInstance() {
    static EOSLobbySearch instance;
    return instance;
}

EOSLobbySearch::EOSLobbySearch() {
    std::cout << "[EOSLobbySearch] Constructor called." << std::endl;
}

EOSLobbySearch::~EOSLobbySearch() {

    EOSLobbyManager& eosManager = EOSLobbyManager::GetInstance();
    EOS_HLobby LobbyHandle = eosManager.GetLobbyHandle();

    if (LobbyHandle) {
        LobbyHandle = nullptr;
    }
    if (LobbySearchHandle) {
        EOS_LobbySearch_Release(LobbySearchHandle);
    }
}

// Creates a lobby search and initiates a search for a specific lobby
void EOSLobbySearch::CreateLobbySearch(const char* TargetLobbyId) {
    std::cout << "[CreateLobbySearch] Attempting to create a lobby search..." << std::endl;

    EOSInitialisationManager& eosInitManager = EOSInitialisationManager::GetInstance();
    EOS_ProductUserId LocalUserId = eosInitManager.GetLocalUserId();
    EOS_HPlatform PlatformHandle = eosInitManager.GetPlatformHandle();

    if (!LocalUserId) {
        std::cerr << "[ERROR] LocalUserId is NULL. Authentication must be complete before creating a lobby search." << std::endl;
        return;
    }

    if (!PlatformHandle) {
        std::cerr << "[ERROR] PlatformHandle is NULL. Ensure the EOS platform is initialized." << std::endl;
        return;
    }

    EOS_HLobby LobbyHandle = EOS_Platform_GetLobbyInterface(PlatformHandle);

    if (!LobbyHandle) {
        std::cerr << "[ERROR] LobbyHandle is NULL. Lobby interface might not be initialized." << std::endl;
        return;
    }

    EOS_Lobby_CreateLobbySearchOptions SearchOptions = {};
    SearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
    SearchOptions.MaxResults = 10;

    EOS_EResult Result = EOS_Lobby_CreateLobbySearch(LobbyHandle, &SearchOptions, &LobbySearchHandle);

    if (Result == EOS_EResult::EOS_Success) {
        std::cout << "[CreateLobbySearch] Lobby search created successfully." << std::endl;

        EOS_LobbySearch_SetLobbyIdOptions SetLobbyIdOptions = {};
        SetLobbyIdOptions.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
        SetLobbyIdOptions.LobbyId = TargetLobbyId;

        EOS_LobbySearch_SetLobbyId(LobbySearchHandle, &SetLobbyIdOptions);
        std::cout << "[CreateLobbySearch] Searching for the lobby with ID: " << TargetLobbyId << std::endl;

        EOS_LobbySearch_FindOptions FindOptions = {};
        FindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
        FindOptions.LocalUserId = LocalUserId;

        EOS_LobbySearch_Find(LobbySearchHandle, &FindOptions, nullptr, OnFindLobbiesComplete);
    }
    else {
        std::cerr << "[ERROR] Failed to create lobby search. Error: " << EOS_EResult_ToString(Result) << std::endl;
    }
}

// Callback function for handling the results of the lobby search
void EOSLobbySearch::OnFindLobbiesComplete(const EOS_LobbySearch_FindCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult::EOS_Success) {
        int32_t numResults = 0;
        EOS_LobbySearch_GetSearchResultCountOptions CountOptions = {};
        CountOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;

        EOSLobbySearch& eosManager = EOSLobbySearch::GetInstance();
        EOS_HLobbySearch LobbySearchHandle = eosManager.GetLobbySearchHandle();

        numResults = EOS_LobbySearch_GetSearchResultCount(LobbySearchHandle, &CountOptions);

        std::cerr << "[OnFindLobbiesComplete] Found " << numResults << " lobbies.\n";

        if (numResults == 0) {
            std::cerr << "[DEBUG] No lobbies found. Check attributes and permissions.\n";
            return;
        }

        EOS_HLobbyDetails LobbyDetailsHandle = nullptr;
        EOS_LobbySearch_CopySearchResultByIndexOptions CopyOptions = {};
        CopyOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
        CopyOptions.LobbyIndex = 0;

        EOS_EResult Result = EOS_LobbySearch_CopySearchResultByIndex(LobbySearchHandle, &CopyOptions, &LobbyDetailsHandle);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (Result == EOS_EResult::EOS_Success) {
            std::cerr << "[LobbyData] Lobby Details Retrieved\n";

            EOS_LobbyDetails_Info* LobbyInfo = nullptr;
            EOS_LobbyDetails_CopyInfoOptions InfoOptions = {};
            InfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;

            EOS_EResult InfoResult = EOS_LobbyDetails_CopyInfo(LobbyDetailsHandle, &InfoOptions, &LobbyInfo);

            if (InfoResult == EOS_EResult::EOS_Success && LobbyInfo) {
                std::cerr << "[LobbyInfo] Lobby ID: " << LobbyInfo->LobbyId << "\n";
                std::cerr << "[LobbyInfo] Max Players: " << LobbyInfo->MaxMembers << "\n";
                std::cerr << "[LobbyInfo] Owner ID: " << LobbyInfo->LobbyOwnerUserId << "\n";

                EOS_LobbyDetails_GetMemberCountOptions MemberCountOptions = {};
                MemberCountOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST;
                int32_t memberCount = EOS_LobbyDetails_GetMemberCount(LobbyDetailsHandle, &MemberCountOptions);

                std::cerr << "[LobbyMembers] Found " << memberCount << " members in the lobby.\n";

                for (int32_t i = 0; i < memberCount; ++i) {
                    EOS_LobbyDetails_GetMemberByIndexOptions MemberByIndexOptions = {};
                    MemberByIndexOptions.ApiVersion = EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST;
                    MemberByIndexOptions.MemberIndex = i;

                    EOS_ProductUserId MemberId = EOS_LobbyDetails_GetMemberByIndex(LobbyDetailsHandle, &MemberByIndexOptions);
                    if (MemberId) {
                        char MemberIdStr[EOS_PRODUCTUSERID_MAX_LENGTH] = {};
                        int32_t BufferSize = sizeof(MemberIdStr);
                        EOS_EResult ConvertResult = EOS_ProductUserId_ToString(MemberId, MemberIdStr, &BufferSize);
                        if (ConvertResult == EOS_EResult::EOS_Success) {
                            std::cerr << "[LobbyMembers] Member " << i << " ID: " << MemberIdStr << "\n";
                            eosManager.LobbyMemberIds.push_back(MemberIdStr);
                        }
                        else {
                            std::cerr << "[LobbyMembers] Failed to convert member ID.\n";
                        }
                    }
                    else {
                        std::cerr << "[LobbyMembers] Failed to get member ID at index " << i << ".\n";
                    }
                }

                EOS_LobbyDetails_Info_Release(LobbyInfo);
            }
            else {
                std::cerr << "[ERROR] Failed to retrieve lobby info. Error: " << EOS_EResult_ToString(InfoResult) << std::endl;
            }

            EOS_LobbyDetails_Release(LobbyDetailsHandle);
        }
        else {
            std::cerr << "[LobbyData] Failed to fetch lobby data. Error: " << EOS_EResult_ToString(Result) << std::endl;
        }
    }
    else {
        std::cerr << "[OnFindLobbiesComplete] Lobby search failed. Error: " << EOS_EResult_ToString(Data->ResultCode) << std::endl;
    }
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