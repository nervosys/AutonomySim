// autonomysim_settings_parser.cpp

#include "autonomysim_settings_parser.h"

AutonomySimSettingsParser::AutonomySimSettingsParser(const std::string &host_ip) : host_ip_(host_ip) {
    success_ = initializeSettings();
}

bool AutonomySimSettingsParser::success() { return success_; }

bool AutonomySimSettingsParser::getSettingsText(std::string &settings_text) const {
    nervosys::autonomylib::RpcLibClientBase autonomysim_client(host_ip_);
    autonomysim_client.confirmConnection();

    settings_text = autonomysim_client.getSettingsString();

    return !settings_text.empty();
}

std::string AutonomySimSettingsParser::getSimMode() {
    const auto &settings_json = nervosys::autonomylib::Settings::loadJSonString(settings_text_);
    return settings_json.getString("SimMode", "");
}

// mimics void ASimHUD::initializeSettings()
bool AutonomySimSettingsParser::initializeSettings() {
    if (getSettingsText(settings_text_)) {
        AutonomySimSettings::initializeSettings(settings_text_);

        AutonomySimSettings::singleton().load(std::bind(&AutonomySimSettingsParser::getSimMode, this));
        std::cout << "SimMode: " << AutonomySimSettings::singleton().simmode_name << std::endl;

        return true;
    }

    return false;
}
