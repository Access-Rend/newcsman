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
    void initialize_val();
    void get_covscript_env();
    void read_config();
public:
    std::string COVSCRIPT_HOME, CS_IMPORT_PATH, CS_DEV_PATH;
    std::string csman_path, pac_repo;
    std::string config_path, sources_idx_path;
    std::string ABI, STD, runtime_ver;
    int max_reconnect_time;
    void show(const std::string &key);
    void show_all();
    void set(const std::string &key,const std::string &val);
    context() {
        try{
            initialize_val();
            get_covscript_env();
            read_config();
        }catch (std::exception e){
            throw e;
        }
    }
};
void context::initialize_val() {
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
    sources_idx_path = csman_path + "/sources.idx";
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
        else if(key=="sources_idx_path") sources_idx_path = var;
        else if(key=="max_reconnect_time") max_reconnect_time = std::stoi(var);
    }
    return ;
}
void context::show_all() {
    std::cout<<"CovScript COVSCRIPT_HOME = \""<<COVSCRIPT_HOME<<"\""<<std::endl
            <<"CovScript CS_IMPORT_PATH = \""<<CS_IMPORT_PATH<<"\""<<std::endl
            <<"CovScript CS_DEV_PATH = \""<<CS_DEV_PATH<<"\""<<std::endl
            <<"csman_path = \""<<csman_path<<"\""<<std::endl
            <<"csman pac_repo = \""<<pac_repo<<"\""<<std::endl
            <<"csman config_path = \""<<config_path<<"\""<<std::endl
            <<"CovScript sources_idx_path = \""<<sources_idx_path<<"\""<<std::endl
            <<"max reconnect time = "<<max_reconnect_time<<std::endl;
}
void context::show(const std::string &key){
    if(key=="COVSCRIPT_HOME")
        std::cout<<"CovScript COVSCRIPT_HOME = \""<<COVSCRIPT_HOME<<"\""<<std::endl;
    else if(key=="CS_IMPORT_PATH")
        std::cout<<"CovScript CS_IMPORT_PATH = \""<<CS_IMPORT_PATH<<"\""<<std::endl;
    else if(key=="CS_DEV_PATH")
        std::cout<<"CovScript CS_DEV_PATH = \""<<CS_DEV_PATH<<"\""<<std::endl;
    else if(key=="csman_path")
        std::cout<<"csman_path = \""<<csman_path<<"\""<<std::endl;
    else if(key=="pac_repo")
        std::cout<<"csman pac_repo = \""<<pac_repo<<"\""<<std::endl;
    else if(key=="config_path")
        std::cout<<"csman config_path = \""<<config_path<<"\""<<std::endl;
    else if(key=="sources_idx_path")
        std::cout<<"CovScript sources_idx_path = \""<<sources_idx_path<<"\""<<std::endl;
    else if(key=="max_reconnect_time")
        std::cout<<"max reconnect time = "<<max_reconnect_time<<std::endl;
    else
        std::cout<<"no argument named \""<<key<<"\"!"<<std::endl;
}

void context::set(const std::string &key,const std::string &val){
    if(key=="COVSCRIPT_HOME"){
    }
    else if(key=="CS_IMPORT_PATH"){

    }
    else if(key=="CS_DEV_PATH"){

    }
    else if(key=="csman_path"){

    }
    else if(key=="pac_repo"){

    }
    else if(key=="config_path"){

    }
    else if(key=="sources_idx_path"){

    }
    else if(key=="max_reconnect_time"){

    }
    else
        std::cout<<"no argument named \""<<key<<"\"!"<<std::endl;
}