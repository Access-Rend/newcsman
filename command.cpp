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
    context *cxt;
    idx_file idx;
    std::vector<std::string> args;
    std::set<std::string> opt;
    std::string predicate, object;

    inline bool input_yes_or_no(){
        static char c = '?';
        std::cout << '>';
        while (c != 'y' && c != 'Y' && c != 'n' && c != 'N')
            std::cin >> c;
        return (c == 'y' || c == 'Y');
    }

    inline void opt_filter() {
        for (auto it = args.begin(); it != args.end(); it++) {
            if (it->size() == 2 && it->operator[](0) == '-')
                opt.insert(*it),
                        args.erase(it);
        }
    }

    struct {
        std::string title;
        inline void first(const std::string &_title, const std::string &_content) {
            title = _title;
            std::cout << title << '\t' << _content << std::endl;
        }
        inline void content(const std::string &_content) {
            std::cout<<'\t' << _content << std::endl;
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
        if (args.size() <= 2)
            ver = idx.un_stable_ver[object].second;
        else if (!is_ver(args[2]))throw std::invalid_argument("wrong package version.");
        else ver = args[2];

        int id = idx.node_id[object][ver];
        if (id <= 0 || id >= idx.G.size)throw std::invalid_argument("this version of package has not found.");

        auto dep_set = idx.G.depend_data(id);
        message.first("csman:", "installing " + object + " " + ver +
                                " needs to install these packages all because of dependencies:");
        for (auto x: dep_set)
            message.content(x.name + " " + x.ver);
        message.first("do you want to install them all?","[y/n]");
        if(!input_yes_or_no()){
            message.first("csman:", "operation interrupted by user.");
            return ;
        }


        for (auto x : dep_set) {
            message.content("installing " + x.name + " " + x.ver + "...");
            std::string dir_path = cxt->get_val(context::key::csman_pac_repo) + "/" + x.name + "/" + x.ver + "/";
            std::string zip_path = dir_path + "pac.zip";
            http_get(x.url, zip_path, std::stoi(cxt->get_val(context::key::max_reconnect_time)));
            cov::zip_extract(zip_path, dir_path);
        }
        message.first("csman: install",object + " " + ver + " and it's dependencies successfully.");
    }

    void uninstall() {
        std::string ver;
        if (args.size() < 2)
            ver = idx.un_stable_ver[object].second; //应该换成从local_pac找已装包而不是去找stable
        else if (!is_ver(args[2]))throw std::invalid_argument("wrong package version.");
        else ver = args[2];

        int id = idx.node_id[object][ver]; //应该换成从local_pac找已装包而不是去找stable
        if (id <= 0 || id >= idx.G.size)throw std::invalid_argument("this version of package has not found.");
        auto sup_set = idx.G.depend_data(id);

        message.first("csman:", "uninstalling " + object + " " + ver +
                                " needs to uninstall these packages all because of dependencies:");
        for (auto x: sup_set)
            message.content(x.name + " " + x.ver);
        message.first("do you want to uninstall them all?","[y/n]");
        if(!input_yes_or_no()){
            message.first("csman:", "operation interrupted by user.");
            return ;
        }

        for (auto x : sup_set) {
            message.content("uninstalling " + x.name + " " + x.ver + "...");
            std::string del_path = cxt->get_val(context::key::csman_pac_repo) + "/" + x.name + "/" + x.ver;
            std::remove(del_path.c_str());
        }
        message.first("csman: install",object + " " + ver + " and it's support successfully.");
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