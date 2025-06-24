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

#include <main.h>

int main(int argc, char* argv[])
{
    init();
    start();

    return 0;
}

#endif // !DEBUG