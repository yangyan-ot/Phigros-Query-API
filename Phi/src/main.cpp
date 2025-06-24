//#define DEBUG
#ifdef DEBUG
#include <test/bing/test_project.hpp>

int main(int argc, char* argv[]) {
    std::cout << "\033[44m--* DEBUG START *--\033[0m\n";

    init();

    TestProject::SongAllData();

    std::cout << "\033[44m--*  DEBUG END  *--\033[0m" << std::endl;
    return 0;
}
#endif // DEBUG

#ifndef DEBUG
#include "main.h"

int main(int argc, char* argv[])
{
    Global::ExecutableFilePath = argv[0];
    std::string port{ "65536" }, concurrency{ "" }, sid{ "0" };
    argument_init(argc, std::move(argv), port, concurrency, sid);
    init();
    start(port, concurrency, sid);
    return 0;
}

#endif // !DEBUG

