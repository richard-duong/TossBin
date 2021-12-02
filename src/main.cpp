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

class toss_exception {
private:
    string msg;
public:
    toss_exception(string msg):msg(msg){}
    const char* what() const { return msg.c_str(); }
};

struct TossFile {
    string path;
    long change_time;
    uintmax_t size;
    TossFile(string p, long c, uintmax_t s):path(p), change_time(c), size(s){}
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

bool isRelativePath(const string& str) {
    return str.rfind("/", 0) != 0 && str.rfind("~", 0) != 0 && str.rfind("\\", 0) != 0;
}

bool startsWith(const string& str, const string& prefix) {
    return str.substr(0, prefix.size()) == prefix;
}

int main(int argc, char *argv[]) {

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

    program.add_argument("-f", "--force")
        .help("force toss or force recover files from recycle bin")
        .default_value(false)
        .implicit_value(true);
    
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
    vector<TossFile> files; 
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

        // list all files in recycle bin
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

    // catch file arguments 
    vector<string> inputs;
    try {
        inputs = program.get<vector<string>>("files");
    } catch (std::logic_error& err) {
        cerr << "No files provided" << endl;
        cerr << program << endl;
        exit(1);
    }

    /* prepare source and destination file pairs
        - if recovering, src moves from recycle bin to actual file
        - if tossing, src moves from actual file to recycle bin 
    */
    
    vector<pair<string, string>> src_dest_files;
    try {
        for (unsigned int i = 0; i < inputs.size(); ++i) {
            string src = "";
            string dest = "";

            // do not include recycledir path in file input
            if (startsWith(inputs[i], recycledir)) {
                throw toss_exception("do not include recycle directory: \"" + recycledir + "\" in the filename");
            } 

            /** If recovering, source = ~/recyclebin, dest = actual path **/
            if (program["--recover"] == true) {

                // modify for relative path
                if (isRelativePath(inputs[i])) {
                    dest = filesystem::current_path().string() + "/" + inputs[i];
                } else {
                    dest = inputs[i];
                }
                src = recycledir + dest;
            }

            else if (program["--recover"] == false) {

                // modify for relative path
                if (isRelativePath(inputs[i])) {
                    src = filesystem::current_path().string() + "/" + inputs[i];
                } else {
                    src = inputs[i];
                }
                dest = recycledir + src;
            }

            /**
             * Push back final source and destination files for tossing or recovery
             * 1. push all non-directory files 
             * 2. throw error if pushing directory recursively without flag
             * 3. push regular files recursively from directory
             */
            if (filesystem::is_directory(src) == false) {
                src_dest_files.push_back({src, dest});
            } else if (program["--recursive"] == false) {
                throw toss_exception(src + " is a directory. Use --recursive flag to include directories");
            } else if (program["--recursive"] == true) {
                for (const auto& entry: filesystem::recursive_directory_iterator(src)) {

                    // if recovering recursively, pick recycledir as src, actual path as dest
                    if (entry.is_regular_file() == false && startsWith(entry.path().string(), recycledir)) {
                        string src_ = entry.path().string();
                        src_dest_files.push_back({entry.path().string(), entry.path().string().substr(recycledir.size())});
                    }

                    // if tossing recursively, pick actual path as src, recycledir as src
                    else if (entry.is_regular_file()) {
                        src_dest_files.push_back({entry.path().string(), recycledir + entry.path().string()});
                    }
                }
            }
        }
    } catch (toss_exception& err) {
        cerr << "toss error: " << err.what() << endl;
        exit(1);
    }

    cout << "all caught files" << endl;
    for (auto& v: src_dest_files) {
        cout << "src: " << v.first << " ----- " << "dest: " << v.second << endl;
    }

    /*
     * Handle remaining toss / recover operations
     * 
     */
    for (auto& file: src_dest_files) {
        filesystem::path src = file.first;
        filesystem::path dest = file.second;
        
        try {

            // throw error if item doesn't exist in recycle bin to recover
            if (filesystem::exists(src) == false && program["--recover"] == true) {
                throw toss_exception("failed to recover - file not found in recycle bin: " + src.string());
            }

            // throw error if item doesn't exist in system to toss
            else if (filesystem::exists(src) == false && program["--recover"] == false) {
                throw toss_exception("failed to toss - file not found " + src.string());
            }
            
            // confirm recovery if file already exists at destination
            else if (filesystem::exists(dest) && program["--recover"] == true && program["--force"] == false) {
                cout << "There currently exists a file you want to replace: " << dest << endl;
                cout << "Are you sure you want to replace this? (y/n)" << endl;
                string input;
                cin >> input; 
                if (input == "y" || input == "Y" || input == "yes" || input == "Yes" || input == "YES") {
                    filesystem::create_directory(dest.parent_path(), src.parent_path());
                    filesystem::rename(src, dest);
                } else {
                    cout << "toss operation canceled" << endl;
                    exit(1);
                }
            } 

            // otherwise continue if tossing or forced flag enabled or file does not exist at destination
            else {
                
                filesystem::create_directory(dest.parent_path(), src.parent_path());
                filesystem::rename(src, dest);
            }
            
        } catch(const toss_exception& err) {
            cerr << "toss error: " << err.what() << endl;
            exit(1);
        } catch (const filesystem::filesystem_error& err) {
            cerr << "filesystem error: " << err.what() << endl;
            exit(1);
        }
    }


}