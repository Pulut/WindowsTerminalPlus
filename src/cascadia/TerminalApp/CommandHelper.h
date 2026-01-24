#pragma once
#include "pch.h"
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
//#include <json/json.h>
#include "til/string.h"

namespace TerminalApp
{
    struct CommonCommand
    {
        std::wstring Name;
        std::wstring CommandText;
    };

    class CommandHelper
    {
    public:
        static std::vector<CommonCommand> Load()
        {
            // TEMPORARY DEBUG: Force return defaults to identify if JSON parsing is causing the crash.
            return GetDefaults();
            
            /*
            auto path = GetJsonPath();
            
            // 1. 优先读取本地 JSON
            if (std::filesystem::exists(path))
            {
                try
                {
                    return ParseJson(path);
                }
                catch (...) {}
            }

            // 2. 只有当 JSON 不存在时，才使用内置默认值
            return GetDefaults();
            */
        }

    private:
        static std::filesystem::path GetJsonPath()
        {
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
            std::filesystem::path path(exePath);
            return path.parent_path() / L"common_commands.json";
        }

        static std::vector<CommonCommand> GetDefaults()
        {
            return {
                { L"List Files", L"dir" },
                { L"Check IP", L"ipconfig" },
                { L"Clear Screen", L"cls" },
                { L"Ping Google", L"ping 8.8.8.8" },
                { L"Debug Mode", L"echo Works!" } 
            };
        }

        /*
        static std::vector<CommonCommand> ParseJson(const std::filesystem::path& path)
        {
            std::vector<CommonCommand> results;
            std::ifstream file(path);
            Json::Value root;
            Json::CharReaderBuilder builder;
            std::string errs;

            if (Json::parseFromStream(builder, file, &root, &errs))
            {
                const auto& items = root["commands"];
                if (items.isArray())
                {
                    for (const auto& item : items)
                    {
                        CommonCommand cmd;
                        auto name = item["name"].asString();
                        auto text = item["cmd"].asString();
                        
                        cmd.Name = til::u8u16(name);
                        cmd.CommandText = til::u8u16(text);
                        
                        if (!cmd.Name.empty())
                        {
                            results.push_back(cmd);
                        }
                    }
                }
            }
            return results;
        }
        */
    };
}