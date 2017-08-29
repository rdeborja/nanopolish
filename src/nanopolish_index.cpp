//---------------------------------------------------------
// Copyright 2017 Ontario Institute for Cancer Research
// Written by Jared Simpson (jared.simpson@oicr.on.ca)
//
// nanopolish index - build an index from FASTA/FASTQ file
// and the associated signal-level data
//
//---------------------------------------------------------
//

//
// Getopt
//
#define SUBPROGRAM "index"

#include <iostream>
#include <sstream>
#include <getopt.h>

#include "nanopolish_index.h"
#include "nanopolish_common.h"
#include "fs_support.hpp"
#include "logger.hpp"
#include "fast5.hpp"

static const char *INDEX_VERSION_MESSAGE =
SUBPROGRAM " Version " PACKAGE_VERSION "\n"
"Written by Jared Simpson.\n"
"\n"
"Copyright 2017 Ontario Institute for Cancer Research\n";

static const char *INDEX_USAGE_MESSAGE =
"Usage: " PACKAGE_NAME " " SUBPROGRAM " [OPTIONS] -d nanopore_raw_file_directory reads.fastq\n"
"Build an index mapping from basecalled reads to the signals measured by the sequencer\n"
"\n"
"      --help                           display this help and exit\n"
"      --version                        display version\n"
"  -v, --verbose                        display verbose output\n"
"  -d, --directory                      path to the directory containing the raw ONT signal files\n"
"\nReport bugs to " PACKAGE_BUGREPORT "\n\n";

namespace opt
{
    static unsigned int verbose = 0;
    static std::string raw_file_directory;
    static std::string reads_file;
}
static std::ostream* os_p;

void index_process_file(const std::string& fn)
{
    fprintf(stdout, "Processing %s\n", fn.c_str());
} // process_file

void index_process_path(const std::string& path)
{
    LOG(info) << path << "\n";
    if (is_directory(path)) {
        auto dir_list = list_directory(path);
        for (const auto& fn : dir_list) {
            if(fn == "." or fn == "..") {
                continue;
            }

            std::string full_fn = path + "/" + fn;
            if(is_directory(full_fn)) {
                // recurse
                index_process_path(full_fn);
            } else {
                index_process_file(full_fn);
            }
        }
    }
} // process_path

static const char* shortopts = "vd:";

enum {
    OPT_HELP = 1,
    OPT_VERSION,
    OPT_LOG_LEVEL,
};

static const struct option longopts[] = {
    { "help",               no_argument,       NULL, OPT_HELP },
    { "version",            no_argument,       NULL, OPT_VERSION },
    { "log-level",          required_argument, NULL, OPT_LOG_LEVEL },
    { "verbose",            no_argument,       NULL, 'v' },
    { "directory",          required_argument, NULL, 'd' },
    { NULL, 0, NULL, 0 }
};

void parse_index_options(int argc, char** argv)
{
    bool die = false;
    std::vector< std::string> log_level;
    for (char c; (c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1;) {
        std::istringstream arg(optarg != NULL ? optarg : "");
        switch (c) {
            case OPT_HELP:
                std::cout << INDEX_USAGE_MESSAGE;
                exit(EXIT_SUCCESS);
            case OPT_VERSION:
                std::cout << INDEX_VERSION_MESSAGE;
                exit(EXIT_SUCCESS);
            case OPT_LOG_LEVEL:
                log_level.push_back(arg.str());
                break;
            case 'v': opt::verbose++; break;
            case 'd': arg >> opt::raw_file_directory; break;
        }
    }

    // set log levels
    auto default_level = (int)logger::warning + opt::verbose;
    logger::Logger::set_default_level(default_level);
    logger::Logger::set_levels_from_options(log_level, &std::clog);

    opt::reads_file = argv[optind];
    
    if (argc - optind < 1) {
        std::cerr << SUBPROGRAM ": not enough arguments\n";
        die = true;
    }

    if (argc - optind > 1) {
        std::cerr << SUBPROGRAM ": too many arguments\n";
        die = true;
    }
    
    if (die) 
    {
        std::cout << "\n" << INDEX_USAGE_MESSAGE;
        exit(EXIT_FAILURE);
    }

    opt::reads_file = argv[optind++];
}

int index_main(int argc, char** argv)
{
    parse_index_options(argc, argv);
    std::ofstream ofs;
    
    // this will recurse into subdirectories as needed
    index_process_path(opt::raw_file_directory);

    return 0;
}
