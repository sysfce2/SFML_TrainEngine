#include "FileManager.h"
#include <dirent.h>
#include <iostream>
#include <SFML/System.hpp>
#include "FileManager.h"
#if defined(SFML_SYSTEM_ANDROID)
    #include <android/asset_manager.h>
    #include <android/native_activity.h>
    #include <SFML/System/NativeActivity.hpp>
#endif

int FileManager::GetFileCount(const std::string& directory)
{
    DIR* pDirectoryStream = opendir(directory.c_str());
    dirent* pCurrentFile;
    int fileCount = -2;

    if (pDirectoryStream == nullptr)
    {
        std::cout << "Failed to open directory: " << directory << '\n';
        return 0;
    }
    
    while ((pCurrentFile = readdir(pDirectoryStream)))
    {
        ++fileCount;
    }
    
    closedir(pDirectoryStream);
    return fileCount;
}

std::vector<std::string> FileManager::GetFilenamesInDirectory(const std::string& directory)
{
    std::vector<std::string> filenames;
    DIR* pDirectoryStream = opendir(directory.c_str());
    dirent* pCurrentFile;
    
    if (pDirectoryStream == nullptr)
    {
        std::cout << "Failed to open directory: " << directory << '\n';
        return filenames;
    }
    
    while ((pCurrentFile = readdir(pDirectoryStream)))
    {
        filenames.emplace_back(pCurrentFile->d_name);
    }
    
    closedir(pDirectoryStream);
    return filenames;
}

#if defined(SFML_SYSTEM_ANDROID)
/// Read a given compressed text file from the assets directory on Android
std::string FileManager::ReadTxtFromAssets(const std::string& filename)
{
    ANativeActivity* pNativeActivity = sf::getNativeActivity();
    AAssetManager* pAssetManager = pNativeActivity->assetManager;

    AAsset* pFile = AAssetManager_open(pAssetManager, filename.c_str(), AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(pFile);

    char* buffer = new char[fileLength + 1];

    AAsset_read(pFile, buffer, fileLength);
    buffer[fileLength] = '\0';

    std::string text = buffer;
    delete[] buffer;

    return text;
}
#endif
