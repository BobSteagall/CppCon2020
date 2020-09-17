#include <string>
#include <thread>

void    test_tx(FILE* fp,
                size_t item_count,
                size_t thread_count,
                size_t tx_count,
                size_t refs_count,
                size_t mode,
                int loglvl);

void
usage()
{}

int main(int argc, char* argv[])
{
    size_t  item_count   = 5'000'000;
    size_t  thread_count = std::thread::hardware_concurrency();
    size_t  tx_count     = 2'000'000;
    size_t  refs_count   = 10;
    size_t  mode         = 1;
    FILE*   fp_out       = stdout;
    int     loglvl       = 1;

    for (int i = 1;  i < argc;  ++i)
    {
        std::string     arg = argv[i];

        if (arg == "-i")
        {
            if (++i < argc)
            {
                item_count = std::stoul(argv[i]);
            }
        }
        else if (arg == "-t")
        {
            if (++i < argc)
            {
                thread_count = std::stoul(argv[i]);
            }
        }
        else if (arg == "-x")
        {
            if (++i < argc)
            {
                tx_count = std::stoul(argv[i]);
            }
        }
        else if (arg == "-r")
        {
            if (++i < argc)
            {
                refs_count = std::stoul(argv[i]);
            }
        }
        else if (arg == "-o")
        {
            if (++i < argc)
            {
                fp_out = fopen(argv[i], "wb");

                if (fp_out == nullptr)
                {
                    fp_out = stdout;
                }
            }
        }
        else if (arg == "-m")
        {
            if (++i < argc)
            {
                mode = std::stoul(argv[i]);
            }
        }
        else if (arg == "-l")
        {
            if (++i < argc)
            {
                loglvl = std::stoul(argv[i]);
            }
        }
        else if (arg == "-h")
        {
            usage();
            return 0;
        }
    }

    test_tx(fp_out, item_count, thread_count, tx_count, refs_count, mode, loglvl);

    return 0;
}


