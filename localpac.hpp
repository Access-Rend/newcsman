#pragma onece
#include "global.hpp"
#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <set>
class localpac_file{
private:
    context* cxt;
    std::unordered_map<std::string , std::vector<std::string> > pac;
    void initialize(){
        std::ofstream ofs(cxt->get_val(context::key::localpac_path));
        ofs.close();
    }
public:
    localpac_file(context *_cxt):cxt(cxt){
        std::ifstream ifs(cxt->get_val(context::key::localpac_path));
        if(!ifs.is_open()){
            ifs.close();
            initialize();
        }
        std::string name;
        int n;
        while(ifs>>name){
            ifs>>n;
            std::string ver;
            for(int i=0;i<n;i++){
                ifs>>ver;
                pac[name].push_back(ver);
            }
        }
        ifs.close();
    }
};