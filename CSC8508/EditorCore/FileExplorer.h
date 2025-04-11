#ifndef FILEEXPLORE_H
#define FILEEXPLORE_H
#include "imgui.h"
#include "UIElementsGroup.h"
#include "GameObject.h"
#include "ComponentAssemblyDefiner.h"
#include "EditorWindow.h"
#include "GameWorld.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <set>

using namespace NCL;
using namespace CSC8508;
using namespace UI;

class EditorWindowManager;

class FileExplorer : public EditorWindow {
public:
	FileExplorer();
	~FileExplorer();

	void OnSetFocus(GameObject* focus) override;
	void OnRenderFocus(GameObject* focus) override;
	void OnFocusEnd() override;
	void OnInit() override;
	std::string GetName() const override { return "FileExplorer"; }

private:
	std::string* flaggedAsset;
	enum LoadFlag {NoFlag, Object, World};

	LoadFlag loadFlag;
	EditorWindowManager& editorManager;
	GameWorld& gameWorld;
	void InitFileExplorer();
	void PushDirectory(std::string* path);
	std::string GetParentDirectory(std::string& path);
	void PushOpenAsset(std::string* path, const std::filesystem::directory_entry entry);
	void PushOpenFolder(std::string* path, const std::filesystem::directory_entry entry);
	void PushFolderBack(std::string* path);
	void OnLoadFlag();
	const std::set<std::string>& extensions = { ".Pfab", ".pfab", ".wrld", ".Wrld"};
};


#endif // FILEEXPLORE_H
