//
// Created by Rend on 2020/12/9.
//
#include<string>
// #ifdef _WIN32
	// #include <direct.h>
	// #include <io.h>
// #elif __linux__
	#include <unistd.h>
	#include <sys/stat.h>
	#include <dirent.h>
// #endif

void create_dir(const std::string &path){
    int exist;
    #ifdef _WIN32
    	exist = _access(path.c_str(),0);
	#ifdef __linux__
		exist = access(path.c_str(),0);
	if(exist==0)
	    return ;
	else{
	    std::string buf;
	    for(int i=0;i<path.size();i++){
	        if(path[i]=='/' || path[i]=='\\'){
                #ifdef _WIN32
                exist = _access(buf.c_str(),0);
                #ifdef __linux__
                exist = access(buf.c_str(),0);
                if(exist==0){
                    #ifdef _WIN32
                    _mkdir(buf.c_str());
                    #ifdef __linux__
                    mkdir(buf.c_str(),0777);
                }
	        }
	        buf += path[i];
	    }
	}
	
}

int main(){
	create_dir("/1/2/3");
	return 0;
}