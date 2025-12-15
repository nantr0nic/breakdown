#include <toml++/toml.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>
#include <string_view>
#include <string>
#include <vector>

void ConfigManager::loadConfig(std::string_view configID, std::string_view filepath)
{
    if (m_ConfigFiles.contains(configID))
    {
        // if configID is the same then return (don't re-load)
        // not really a "warning" but I wanted to see something yellow in the log window lol
        logger::Warn(std::format("Config ID \"{}\" already loaded.", configID));
        return;
    }

    toml::parse_result configFile = toml::parse_file(filepath);

    if (!configFile)
    {
        logger::Error(std::format(
            "Error parsing config file --> {}", configFile.error().description()
        ));
        return;
    }

    m_ConfigFiles.insert_or_assign(std::string(configID), std::move(configFile.table()));

    logger::Info(std::format("Config ID \"{}\" loaded from: {}", configID, filepath));
}

const toml::table* ConfigManager::getConfigTable(std::string_view configID) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("Config file ID [{}] not found.", configID));
        return nullptr;
    }

    return &it->second;
}

std::vector<std::string> ConfigManager::getStringArray(
    std::string_view configID, std::string_view section, std::string_view key, 
    const std::source_location& loc) const
{
    std::vector<std::string> result;

    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("File: {}({}:{}) -> Config file ID [{}] not found.", 
            logger::formatPath(loc.file_name()), loc.line(), loc.column(), configID));
        return result; // return empty
    }

    auto node = it->second[section][key];

    if (!node.is_array())
    {
        logger::Warn(std::format(
            "Key [{}] in Section [{}] of Config [{}] is not an array.", key, section, configID));
        return result;
    }

    const auto& arr = *node.as_array();
    for (const auto& elem : arr)
    {
        // value_or("") ensures we get a string or empty if type is wrong
        result.push_back(elem.value_or<std::string>(""));
    }

    return result;
}