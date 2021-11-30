#include <iostream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <cmath>
#include <regex>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

// custom libraries
#include <argparse/argparse.hpp>
using namespace std;

struct File {
    string path;
    long change_time;
    uintmax_t size;
    File(string p, long c, uintmax_t s):path(p), change_time(c), size(s){}
};

struct HumanReadable {
    std::uintmax_t size {};
 
    template <typename Os> friend Os& operator<< (Os& os, HumanReadable hr)
    {
        int i{};
        double mantissa = hr.size;
        for (; mantissa >= 1024.; ++i) {
            mantissa /= 1024.;
        }
        mantissa = std::ceil(mantissa * 10.) / 10.;
        os << mantissa << "BKMGTPE"[i];
        return i == 0 ? os : os << "B (" << hr.size << ')';
    }
};

int isDirectory(const char* path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

long getChangeTimeLong(const char* path) {
    struct stat fileInfo;
    if (stat(path, &fileInfo) != 0) {
        cerr << "Error: " << strerror(errno) << endl;
        exit(1);
    }
    return fileInfo.st_ctime;
}

string getChangeTimeString(const char* path) {
    struct stat fileInfo;
    if (stat(path, &fileInfo) != 0) {
        cerr << "Error: " << strerror(errno) << endl;
        exit(1);
    }

    return ctime(&fileInfo.st_ctime);
}

string getChangeTimeString(const string& path) {
    return getChangeTimeString(path.c_str());
}

long getChangeTimeLong(const string& path) {
    return getChangeTimeLong(path.c_str());
}

int main(int argc, char *argv[]) {

    /**
     * @brief
     * Build directory recycle bin in home user
     */

    // set home directory to environment or based on user's home directory
    string homedir, recycledir;
    if ((homedir = getenv("HOME")) == "") {
        homedir = getpwuid(getuid())->pw_dir;
        // recycledir = getpwuid(getuid())->pw_dir;
    }
    recycledir = homedir + "/recyclebin";

    // build home directory
    int status = mkdir(recycledir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // EEXIST is the error that throws if already exists, otherwise some other error    
    if (status && errno != EEXIST) {
        cerr << "Cannot create recyclebin for unknown reason: " << strerror(errno) << endl;
        throw std::runtime_error(strerror(errno));
    }

    /**
     * nothing = files
     * -r = recursive
     * --recover = recovery
     * -ls, --list = see old items 
     * -h, --help = help 
     */
    argparse::ArgumentParser program("toss");
    
    program.add_argument("-l", "--list", "--list-recent")
        .help("list items in recycle bin by most recent")
        .default_value(false)
        .implicit_value(true);
    
    program.add_argument("-ls", "--list-size")
        .help("list items in recycle bin by size")
        .default_value(false)
        .implicit_value(true);
    
    program.add_argument("-ln", "--list-name")
        .help("list items in recycle bin by name")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-g", "--regex", "--reg")
        .help("enable regex matching for files to toss/recover")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-r", "--recursive")
        .help("recursively toss directories into the recycle bin")
        .default_value(false)
        .implicit_value(true);
    
    program.add_argument("-c", "--recover", "--restore")
        .help("recover or restore a file")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("files")
        .help("files or directories to toss into recycle bin")
        .remaining();
    
    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        cerr << err.what() << endl;
        cerr << program;
        exit(1);
    }

    /** List Recycle Bin **/
    vector<File> files; 
    if (program["--list"]  == true || program["--list-name"]  == true || program["--list-size"] == true) { 

        // list header
        cout << left << setw(30) << "Date Tossed" << left << setw(50) << "Filename" << right << "Size" << endl;
        cout << string(90, '=') << endl;


        // load all files in recycle bin
        for (const auto& entry: filesystem::recursive_directory_iterator(recycledir.c_str())) {
            if (entry.is_regular_file()) {
                string path = entry.path().string();
                long change_time = getChangeTimeLong(entry.path().string());
                uintmax_t size = entry.file_size();
                files.push_back({path, change_time, size});    
            }
        }

        // sort by recent
        if (program["--list"] == true) {      
            sort(files.begin(), files.end(), [](const auto &x, const auto &y) {return x.change_time > y.change_time;}); 
        }

        // sort by name
        else if (program["--list-name"] == true) {
            sort(files.begin(), files.end(), [](const auto &x, const auto &y) {return x.path < y.path;});
        }

        // sort by size
        else if (program["--list-size"] == true) {
            sort(files.begin(), files.end(), [](const auto &x, const auto &y) {return x.size > y.size;});
        }
        
        /*
        else if (program["--list-expiring"] == true) {
            sort(files.begin(), files.end(), [](const auto &x, const auto &y) {return x.change_time < y.change_time;});

        }
        */

        // list all files
        for (const auto& file: files) {
            string path = file.path;
            string change_time = ctime(&file.change_time);
            uintmax_t size = file.size;
            path.erase(0, recycledir.size()); 
            change_time = change_time.substr(0, change_time.size() - 1);  
            cout << left << setw(30) << change_time << left << setw(50) << path << right << HumanReadable{size} << endl;
        }

        exit(1);           
    }

    // catch empty file arguments 
    vector<string> input_files;
    try {
        input_files = program.get<vector<string>>("files");
    } catch (std::logic_error& err) {
        cerr << "No files provided" << endl;
        cerr << program << endl;
        exit(1);
    }
    
    // recover files from the recycle bin
    /*
    if (program["--recover"] == true) {
        for (auto& file: input_files) {
            string dest = "";
            string src = "";

            // relative path search
            if (file.rfind("/", 0) != 0 && file.rfind("~", 0) != 0 && file.rfind("\\", 0) != 0) {
                dest = filesystem::current_path().string() + "/" + file;
                cout << "relative: ";
            } else {
                cout << "absolute: ";
            }
            src = recycledir + dest;
            cout << src << " -> " << dest << endl;

            // check for file existence

            if (program["--force"] && filesystem::exists(dest)) {
                filesystem::remove(dest);
            }
            else if (filesystem::exists(dest)) {
                cout << "There currently exists a file for: " << dest << endl;
                cout << "Are you sure you want to replace this? (y/n)" << endl;
                string input;
                cin >> input; 
                if (input == "y" || input == "Y" || input == "yes" || input == "Yes" || input == "YES") {
                    filesystem::remove(dest);
                }
            }

        }
    }
    */
}