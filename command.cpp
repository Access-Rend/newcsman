//
// Created by Rend on 2020/12/3.
//
#include"idx.hpp"
#include"global.hpp"
#include"http.hpp"
#include"zip.hpp"
#include<iostream>
#include<string>
#include<vector>
#include<set>

class parser {
private:
    context *cxt;   // 上下文，存取全局变量与信息
    idx_file idx;   // sources_idx，可下载包信息，负责以来查询，支持查询
    std::vector<std::string> args;  // 用户命令的参数，例如install xxx
    std::set<std::string> opt;  // 从args里分离出的可选参数，例如-r
    std::string predicate, object;  // 谓语，宾语

    inline bool y_or_n() {
        static char c = '#';
        std::cout << '>';
        while (c != 'y' && c != 'Y' && c != 'n' && c != 'N') {
            std::cout << "please type \'y\' or \'n\'." << std::endl;
            std::cin >> c;
        }
        return (c == 'y' || c == 'Y');
    }

    /*从args分离opt的filter*/
    inline void opt_filter() {
        for (auto it = args.begin(); it != args.end(); it++) {
            if (it->size() == 2 && it->operator[](0) == '-')
                opt.insert(*it),
                        args.erase(it);
        }
    }

    struct {
        std::string title;

        inline void first_sentence(const std::string &_title, const std::string &_content) {
            title = _title;
            std::cout << title << '\t' << _content << std::endl;
        }

        inline void content(const std::string &_content) {
            std::cout << '\t' << _content << std::endl;
        }
    } message;

public:
    parser(context *cxt, const std::vector<std::string> &a) : cxt(cxt), idx(cxt), args(a) {}

    void parse() {
        try {
            opt_filter();
            predicate = args[0];
            object = args[1];
            if (predicate == "install")
                install();
            else if (predicate == "uninstall")
                uninstall();
            else if (predicate == "config")
                config();
            else
                throw std::invalid_argument("syntax error!");
        }
        catch (std::exception &e) {
            throw e;
        }
    }

    void install() {
        std::string ver;
        if (args.size() <= 2) ver = idx.get_stable_ver(object);
        else if (args[2] == "stable") ver = idx.get_stable_ver(object);
        else if (args[2] == "unstable") ver = idx.get_unstable_ver(object);
        else if (!is_ver(args[2]))throw std::invalid_argument("wrong package version.");
        else ver = args[2];

        try {
            // 获取依赖
            auto dep_set = idx.get_depend_set(object, ver);
            message.first_sentence("csman:", "installing " + object + " " + ver +
                                             " needs to install these packages all because of dependencies:");
            for (auto x: dep_set)
                message.content(x.name + " " + x.ver);
            message.first_sentence("do you want to install them all?", "[y/n]");
            // 是否安装?
            if (!y_or_n()) { // 否
                message.first_sentence("csman:", "operation interrupted by user.");
                return;
            }
            // 是 进入安装
            for (auto x : dep_set) {
                message.content("installing " + x.name + " " + x.ver + "...");
                std::string dir_path = cxt->pac_repo + "/" + x.name + "/" + x.ver + "/";
                std::string zip_path = dir_path + "pac.zip";
                http_get(x.url, zip_path, cxt->max_reconnect_time);
                cov::zip_extract(zip_path, dir_path);
            }
            message.first_sentence("csman: install", object + " " + ver + " and it's dependencies successfully.");
        }
        catch(std::exception &e){
            throw e;
        }
    }

    void uninstall() {
        std::string ver;
//        if (args.size() <= 2) ver = idx.get_stable_ver(object); 以下寻找包版本,均应替换为pac_repo的接口,例: pac_repo.get_current_runtime_ver()
//        else if (args[2] == "stable") ver = idx.get_stable_ver(object);
//        else if (args[2] == "unstable") ver = idx.get_unstable_ver(object);
//        else if (!is_ver(args[2]))throw std::invalid_argument("wrong package version.");
//        else ver = args[2];
    }

    void config() {

    }
};

void REPL() {
    while (true) {
        return;
    }
}

void cmd() {

}

int main(int argc, char **argv) {
    try {
        std::vector<std::string> args(&argv[1], argv + argc);

        context *cxt = new context();

        parser ps(cxt, args);

        ps.parse();

        return 0;
    }
    catch (std::exception &e) {
        std::cout << e.what();
    }
    return 0;
}