#include <string>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>      //mkdir(), rmdir()
#include <io.h>         //access()
#define ACCESS _access
#define MKDIR(path) _mkdir((path))
#define RMDIR(path) _rmdir((path))
#endif

#ifdef __linux__
#include <unistd.h>     //mkdir(), rmdir()
#include <sys/stat.h>   //access()
#define ACCESS access
#define MKDIR(path) mkdir((path),0777)
#define RMDIR(path) rmdir((path))
#endif

bool create_dir(const std::string &path) {
    for (int i = 0; i < path.size(); i++) {
        if(path[i]!='/' && path[i]!='\\')
            continue;
        std::string tmp = path.substr(0,i+1).c_str();
        int status = ACCESS(tmp.c_str(), 0);
        if (status != 0){
            status = MKDIR(tmp.c_str());
            if (status != 0)
                return false;
        }

    }
    return true; //success
}