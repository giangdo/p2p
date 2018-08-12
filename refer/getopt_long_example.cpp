#include <getopt.h>
#include <iostream>

int num = -1;
bool is_beep = false;
float sigma = 2.034;
std::string write_file = "default_file.txt";

void PrintHelp()
{
    std::cout <<
            "--num <n>:           Set number of program\n"
            "--beep:              Beep the user\n"
            "--sigma <val>:       Set sigma of program\n"
            "--writeFile <fname>: File to write to\n"
            "--help:              Show help\n";
    exit(1);
}

void ProcessArgs(int argc, char** argv)
{
    const char* const short_opts = "n:bs:w:h";
    const option long_opts[] = {
            {"num", required_argument, nullptr, 'n'},
            {"beep", no_argument, nullptr, 'b'},
            {"sigma", required_argument, nullptr, 's'},
            {"writeFile", required_argument, nullptr, 'w'},
            {"help", no_argument, nullptr, 'h'},
            {nullptr, no_argument, nullptr, 0}
    };

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case 'n':
            num = std::stoi(optarg);
            std::cout << "Num set to: " << num << std::endl;
            break;

        case 'b':
            is_beep = true;
            std::cout << "Beep is set to true\n";
            break;

        case 's':
            sigma = std::stof(optarg);
            std::cout << "Sigma set to: " << sigma << std::endl;
            break;

        case 'w':
            write_file = std::string(optarg);
            std::cout << "Write file set to: " << write_file << std::endl;
            break;

        case 'h': // -h or --help
        case '?': // Unrecognized option
        default:
            PrintHelp();
            break;
        }
    }
}

int main(int argc, char **argv)
{
    ProcessArgs(argc, argv);
    return 0;
}

/*
$ g++ -std=c++11 opt.cpp

$ ./a.out --help
--num <n>:           Set number of program
--beep:              Beep the user
--sigma <val>:       Set sigma of program
--writeFile <fname>: File to write to
--help:              Show help

$ ./a.out --num 10
Num set to: 10

$ ./a.out --num 10 --writeFile haha.txt --sigma 3.45 --beep
Num set to: 10
Write file set to: haha.txt
Sigma set to: 3.45
Beep is set to true
*/
