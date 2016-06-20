#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#ifndef CONFIG_H
#define CONFIG_H

namespace rbc {
typedef std::map<std::string, std::string> ConfigInfo;
class Config{
public:
    ConfigInfo configValues;
    Config(){
        std::ifstream t("/etc/rbc/rbc.conf");
        std::stringstream fileStream;
        fileStream << t.rdbuf();

        std::string line;
        while (std::getline(fileStream, line))
        {
            std::string key;
            std::string value;
            size_t pos = line.find('=');
            key = line.substr(0, pos);
            value = line.substr(pos+1);
            if (key[0] == '#')
                continue;

            configValues[key] = value;
            //std::cout << "nes added key: " << key <<  "    value:" << configValues[key] << std::endl;
        }
    }
};

}

#endif
