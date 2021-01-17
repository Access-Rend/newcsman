//
// Created by Rend on 2020/12/6.
//
#pragma once

#include <mozart++/process>
#include <fstream>
#include <cstdlib>
#include <string>
#include <regex>

bool is_abi(const std::string &str) {
    static const std::regex reg("^ABI[0-9]{4}[0-9A-F]{2}$");
    return std::regex_match(str, reg);
}

bool is_std(const std::string &str) {
    static const std::regex reg("^STD[0-9]{4}[0-9A-F]{2}$");
    return std::regex_match(str, reg);
}

bool is_ver(const std::string &str) {
    static const std::regex reg("^([0-9]+\\.){0,3}[0-9]+\\w*$");
    return std::regex_match(str, reg);
}

bool is_pac(const std::string &str) {
    static const std::regex reg("^(\\w+\\.)+\\w+$");
    return std::regex_match(str, reg);
}

class context {
private:
    void initialize_var();
    void get_covscript_env();
    void read_config();
public:
    std::string COVSCRIPT_HOME, CS_IMPORT_PATH, CS_DEV_PATH;
    std::string csman_path, pac_repo;
    std::string config_path, idx_path;
    std::string ABI, STD, runtime_ver;
    int max_reconnect_time;
    context() {
        try{
            initialize_var();
            get_covscript_env();
            read_config();
        }catch (std::exception e){
            throw e;
        }
    }
};

void context::initialize_var() {
    if (std::getenv("COVSCRIPT_HOME") == nullptr ||
        std::getenv("CS_IMPORT_PATH") == nullptr ||
        std::getenv("CS_DEV_PATH") == nullptr)
        throw std::runtime_error("CovScript has not installed yet or maybe it has been broken.");
    /*CovScript env var*/
    COVSCRIPT_HOME = std::getenv("COVSCRIPT_HOME");
    CS_IMPORT_PATH = std::getenv("CS_IMPORT_PATH");
    CS_DEV_PATH = std::getenv("CS_DEV_PATH");
    /*csman client var*/
    const std::string home_path = std::getenv("HOME");
    config_path = home_path + "/.csman_config";

    csman_path = home_path + "/.csman/";
    pac_repo = COVSCRIPT_HOME + "/pac_repo";
    idx_path = csman_path + "/sources.idx";
    max_reconnect_time = 5;
}
void context::get_covscript_env() {
    mpp::process_builder builder;
    builder.command("cs")
            .arguments(std::vector<std::string>{"-v"})
            .merge_outputs(true);
    auto p = builder.start();
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
        throw std::runtime_error("CovScript has not installed yet or maybe it has been broken.");
    return;
}
void context::read_config() {
    std::ifstream ifs(config_path);
    if(!ifs.is_open()){
        ifs.close();
        std::ofstream ofs(config_path);
        ofs.close();
        return ;
    }
    /*
     * 配置文件暂时不支持识别注释
     */
    std::string key,var;
    while(ifs>>key && ifs>>var){
        if(var == "default") continue;
        if(key=="csman_path") this->csman_path = var;
        else if(key=="pac_repo")pac_repo = var;
        else if(key=="idx_path") idx_path = var;
        else if(key=="max_reconnect_time") max_reconnect_time = std::stoi(var);
    }
    return ;
}