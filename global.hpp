//
// Created by Rend on 2020/12/6.
//
#pragma once
#include <mozart++/process>
#include <fstream>
#include <cstdlib>
#include <string>
#include <regex>
#include <iostream>

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
    void show_init();   //为了兼容现在的输出代码而追加的输出前格式化
    void show_reinit(); //和输出后回到之前的状态, 有必要的话后续再改
    void write_config();
public:
    std::string COVSCRIPT_HOME, CS_IMPORT_PATH, CS_DEV_PATH;
    std::string csman_path, pac_repo;
    std::string config_path, sources_idx_path;
    std::string notes[4]; //0 = csman_path's notes, 1 = pac_repo, 2 = sour_idx_path...
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

    notes[0] = notes[1] = notes[2] = "";
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
     * 配置文件支持文件注释, 要求:
     * 1.变量与等号与值之间均有空格分隔, 可以乱序
     * 2.注释以# (#与至少一个空格)开头, 一次注释一行, 不支持多行注释
     * 3.每个变量后最多跟一行注释, 整个文件最多有三行注释, 更多注释将被舍弃,
     * 
     *  to do: 支持更多注释需要添加vec<string> notes
     * 
     * 一个正确的格式:
     * csman_path = default
     * # test notes1 asd asd asd
     * pac_repo = default
     * # test notes2 e
     * sources_idx_path = D;\\test\\test\\test\\idxpath
     * max_reconnect_time = 6
     * # test notes3 lisudfbILDFN XKLJcvn;dsjfn;SDF ;djsnv;DVN
     * 
     * 
     */
    
    std::string key, value, buffer;
    int flag = -1;
    while (!ifs.eof())
    {
        ifs >> key;
        //csmanconfig_path是固定的因此不必判断
        if (key == "csman_path"){
            ifs >> /*去掉=号*/value >> /*读入数据*/value;
            csman_path = value;
            flag = 0;
        }
        if (key == "pac_repo"){
            ifs >> value >> value;
            pac_repo = value;
            flag = 1;
        }
        if(key=="sources_idx_path"){
            ifs >> value >> value;
            sources_idx_path = value;
            flag = 2;
        }
        if(key=="max_reconnect_time"){
            ifs >> value >> value;
            max_reconnect_time = std::stoi(value);
            flag = 3;
        }
        if (key[0] == '#' && flag >= 0 && flag <=3 ){ //注释 读一行
            std::getline(ifs, notes[flag], '\n');
        }
    }
    ifs.close();
    return ;
}
void context::show_init() {
    if(notes[0] != "")
        csman_path.append("\n").append(notes[0]);
    if(notes[1] != "")
        pac_repo.append("\n").append(notes[1]);
    if(notes[2] != "")
        sources_idx_path.append("\n").append(notes[2]);
    //max_reconnect_time 不输出注释
}
void context::show_reinit() {
    int temp = std::string::npos;
    if(notes[0] != "" && (temp = csman_path.find(notes[0])) != std::string::npos)
        csman_path.erase(csman_path.find("\n"), csman_path.size());
    if(notes[1] != "" && (temp = pac_repo.find(notes[1])) != std::string::npos)
        pac_repo.erase(pac_repo.find("\n"), pac_repo.size());
    if(notes[2] != "" && (temp = sources_idx_path.find(notes[2])) != std::string::npos)
        sources_idx_path.erase(sources_idx_path.find("\n"), pac_repo.size());
}

void context::show_all() {
    show_init();
    std::cout<<"CovScript COVSCRIPT_HOME = \""<<COVSCRIPT_HOME<<"\""<<std::endl
            <<"CovScript CS_IMPORT_PATH = \""<<CS_IMPORT_PATH<<"\""<<std::endl
            <<"CovScript CS_DEV_PATH = \""<<CS_DEV_PATH<<"\""<<std::endl
            <<"csman_path = \""<<csman_path<<"\""<<std::endl
            <<"csman pac_repo = \""<<pac_repo<<"\""<<std::endl
            <<"csman config_path = \""<<config_path<<"\""<<std::endl
            <<"CovScript sources_idx_path = \""<<sources_idx_path<<"\""<<std::endl
            <<"max reconnect time = "<<max_reconnect_time<<std::endl;
    show_reinit();
}
void context::show(const std::string &key){
    show_init();
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
    show_reinit();
}

void context::write_config(){
    std::ofstream ofs(config_path);
    
    //如果需要这个函数的话我再补充
    // ofs <<"csman_path = "<< csman_path<< " "<<std::endl
    //     <<"csman pac_repo = \""<<pac_repo<<"\""<<std::endl
    //     <<"csman config_path = \""<<config_path<<"\""<<std::endl
    //     <<"CovScript sources_idx_path = \""<<sources_idx_path<<"\""<<std::endl
    //     <<"max reconnect time = "<<max_reconnect_time<<std::endl;

    ofs.close();
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