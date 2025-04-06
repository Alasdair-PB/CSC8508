#include "FileExplorer.h"
#include "IComponent.h"
#include <tuple>
#include <string>
#include "ComponentManager.h"
#include "GameWorld.h"
#include "EditorWindowManager.h"
#include "../EditorGame.h"

FileExplorer::FileExplorer() : 
	editorManager(EditorWindowManager::Instance()), 
	gameWorld(GameWorld::Instance()),
	loadObjectFlag(false),
	flaggedAsset(new std::string())
{
	InitFileExplorer();
}

FileExplorer::~FileExplorer() = default;

void FileExplorer::OnSetFocus(GameObject* focus) {}
void FileExplorer::OnFocusEnd() {}
void FileExplorer::OnInit() {}

void FileExplorer::OnRenderFocus(GameObject* focus)
{
	if (loadObjectFlag) {
		// Change to enum flag to load multiple asset types
		loadObjectFlag = false;
		GameObject* loaded = new GameObject();
		loaded->Load(*flaggedAsset);
		gameWorld.AddGameObject(loaded);
		editorManager.SetFocus(loaded);
		//gameWorld.Load(*flaggedAsset);
	}
	window->ClearAllElements();
	std::string* path = EditorWindowManager::Instance().GetFolderPath();
	PushDirectory(path);
}

void FileExplorer::InitFileExplorer() {
	window = new UIElementsGroup(
		ImVec2(0.2f, 0.7f),
		ImVec2(0.5f, 0.3f),
		1.0f,
		"Explorer",
		0.0f
	);
}

void FileExplorer::PushDirectory(std::string* path) {
	for (const auto& entry : std::filesystem::directory_iterator(*path)) {
		if (entry.is_directory())
			PushOpenFolder(path, entry);
		else if (entry.is_regular_file())
			PushOpenAsset(path, entry);
	}
	if ((*path) == editorManager.GetAssetRootDir()) return;
	PushFolderBack(path);
}

void FileExplorer::PushOpenAsset(std::string* path, const std::filesystem::directory_entry entry) {
	std::string ext = entry.path().extension().string();
	if (extensions.find(ext) != extensions.end()) {

		std::string asset = entry.path().filename().string();
		window->PushStatelessButtonElement(ImVec2(), asset,
			[path, asset, this]() {
				editorManager.MarkWorldToClearWorld();
				*flaggedAsset = (*path) + asset;
				loadObjectFlag = true;
			});
	}
}

void FileExplorer::PushOpenFolder(std::string* path, const std::filesystem::directory_entry entry) {
	std::string assetPath = entry.path().filename().string();
	window->PushStatelessButtonElement(ImVec2(), assetPath,
		[path, assetPath]() {
			(*path) += assetPath + "/";
		});
}

void FileExplorer::PushFolderBack(std::string* path){
	window->PushStatelessButtonElement(ImVec2(), "Return",
		[path, this]() {
			(*path) = GetParentDirectory((*path));
		});
}

std::string FileExplorer::GetParentDirectory(std::string& path) {
	size_t pos = path.find_last_of('/', path.length() - 2);
	if (pos != std::string::npos) return path.substr(0, pos + 1);
	return "";
}