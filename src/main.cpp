#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

// custom libraries
#include <argparse/argparse.hpp>
using namespace std;

int main(int argc, char *argv[]) {
    /**
     * @brief
     * Build directory recycle bin in home user
     */

    // set home directory to environment or based on user's home directory
    char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    char* recycledir = strcat(homedir, "/recyclebin");

    // build home directory
    int status = mkdir(recycledir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

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

}