//
// Created by Rend on 2020/12/6.
//
#pragma once

#include <mozart++/process>
#include <fstream>
#include <cstdlib>
#include <string>
#include <regex>

bool is_abi(const std::string &str){
    static const std::regex reg("^ABI[0-9]{4}[0-9A-F]{2}$");
    return std::regex_match(str, reg);
}
bool is_std(const std::string &str){
    static const std::regex reg("^STD[0-9]{4}[0-9A-F]{2}$");
    return std::regex_match(str, reg);
}
bool is_ver(const std::string &str){
    static const std::regex reg("^([0-9]+\\.){0,3}[0-9]+\\w*$");
    return std::regex_match(str, reg);
}
bool is_pac(const std::string &str)
{
    static const std::regex reg("^(\\w+\\.)+\\w+$");
    return std::regex_match(str, reg);
}

class context {
private:
    std::string COVSRIPT_HOME = "/mnt/d/CovScript", CS_IMPORT_PATH = "/mnt/d/CovScript/imports", CS_DEV_PATH;
    std::string csman_path="home/rend/csman", csman_pac_repo = "/mnt/d/CovScript/pac_repo";
    std::string config_path, idx_path = "/mnt/d/mywork/newcsman/idx", localpac_path = "/mnt/d/mywork/newcsman/localpath";
    std::string ABI, STD, runtime_ver;
    std::string max_reconnect_time="5";

public:
    enum class key {
        COVSRIPT_HOME, CS_IMPORT_PATH, CS_DEV_PATH,
        csman_path, csman_pac_repo,
        config_path, idx_path, localpac_path,
        ABI, STD, runtime_ver,
        max_reconnect_time
    };

    context() {

//        if (std::getenv("COVSRIPT_HOME") == nullptr ||
//            std::getenv("CS_IMPORT_PATH") == nullptr ||
//            std::getenv("CS_DEV_PATH") == nullptr)
//            throw std::runtime_error("未安装CovScript");
//
//        COVSRIPT_HOME = std::getenv("COVSRIPT_HOME");
//        CS_IMPORT_PATH = std::getenv("CS_IMPORT_PATH");
//        CS_DEV_PATH = std::getenv("CS_DEV_PATH");

        config_path = std::getenv("HOME") + std::string("/.csman_config");

        mpp::process_builder builder;
        builder
                .command("cs")
                .arguments(std::vector<std::string>{"-v"})
                .merge_outputs(true);
        auto p = builder.start();
        /*缺少：进程启动失败，还未安装covscript*/
        auto &out = p.out();
        std::regex regVersion("Version: ([0-9\\.]+)"),
                regSTD("STD Version: ([0-9]{4}[0-9A-F]{2})"),
                regABI("ABI Version: ([0-9]{4}[0-9A-F]{2})"),
                regAPI("API Version: ([0-9]{4}[0-9A-F]{2})"),
                regBuild("Build ([0-9]+)");
        std::string _build, _version;
        while (out) {
            std::string line;
            std::getline(out, line);
            std::smatch std_match;

            if (std::regex_search(line, std_match, regSTD))
                this->STD = std_match[1];
            else if (std::regex_search(line, std_match, regABI))
                this->ABI = std_match[1];
            else if (std::regex_search(line, std_match, regAPI))
                /*do nothing*/;
            else if (std::regex_search(line, std_match, regVersion))
                _version = std_match[1];

            if (std::regex_search(line, std_match, regBuild)) {
                _build = std_match[1];
                continue;
            }
        }
        this->runtime_ver = _version + _build;
        if (this->ABI.empty() || this->STD.empty() || this->runtime_ver.empty())
            throw std::runtime_error("CovScript has not installed yet.");
        std::ifstream ifs(config_path);
        ifs >> csman_path >> csman_pac_repo >> idx_path >> localpac_path >> max_reconnect_time;
    }

    auto get_val(key k) {
        switch (k) {
            case key::COVSRIPT_HOME:
                return COVSRIPT_HOME;
            case key::CS_IMPORT_PATH:
                return CS_IMPORT_PATH;
            case key::CS_DEV_PATH:
                return CS_DEV_PATH;
            case key::csman_path:
                return csman_path;
            case key::csman_pac_repo:
                return csman_pac_repo;
            case key::config_path:
                return config_path;
            case key::idx_path:
                return idx_path;
            case key::localpac_path:
                return localpac_path;
            case key::ABI:
                return ABI;
            case key::STD:
                return STD;
            case key::runtime_ver:
                return runtime_ver;
            case key::max_reconnect_time:
                return max_reconnect_time;
        }
    }
};