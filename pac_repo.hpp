#include<unordered_map>
#include<iostream>
#include<fstream>
#include<set>

#include "global.hpp"
#include "dir.hpp"
class pac_repo{
private:
    context *cxt;

public:/*公开类*/
    class pac_data{
    public:
        std::string available;
        std::set<std::string>ver;
    };
    std::unordered_map<std::string, pac_data>local_pac;
public:/*公开接口*/
    pac_repo(const context *&cxt){
        std::ifstream ifs(cxt->pac_repo);
        if(!ifs.is_open())
            throw std::runtime_error("opening \"pac_repo\" failed.");

        ifs.close();
        return;
    }
    ~pac_repo(){
        std::ofstream ofs(cxt->pac_repo);
        if(!ofs.is_open())
            throw std::runtime_error("saving \"pac_repo\" failed.");

        return;
    }
    void update_install(const std::string &name, const std::string &ver, bool is_available){
        if(local_pac.count(name)==0)
            local_pac[name] = pac_data();
        if(is_available)
            local_pac[name].available = ver;
        local_pac[name].ver.insert(ver);

        return;
    }
    void update_uninstall(const std::string &name, const std::string &ver){
        auto it = local_pac[name].ver.find(ver);
        if(local_pac.count(name)==0 || it==local_pac[name].ver.end())
            throw std::invalid_argument("package \""+name+" "+ver+"\" is not existed.");
        if(local_pac[name].available==ver)
            local_pac[name].available.clear();
        local_pac[name].ver.erase(ver);
        return;
    }
    void update_checkout(){

    }
    inline std::set<std::string> query_contains_ver(const std::string &name){
        std::set<std::string> query_reasult;
        if(local_pac.count(name) != 0){
            for(auto ver : local_pac[name].ver)
                query_reasult.insert(ver);
        }
        return query_reasult;
    }
    inline std::string query_using_ver(const std::string &name){
        return local_pac.count(name) == 0 ? "" : /*likely*/ local_pac[name].available;
    }
    inline std::string get_current_runtime_ver(){
     return cxt->runtime_ver;
    }
};