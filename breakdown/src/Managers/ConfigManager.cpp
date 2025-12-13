#include <toml++/toml.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>
#include <string_view>

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
