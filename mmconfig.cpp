#include "mmconfig.h"
#include "INIReader.h"

#include <iostream>
#include <fstream>

using std::string;

int MMConfig::readFromFile(const string &filepath)
{
  INIReader reader(filepath);

  if (reader.ParseError() > 0)
  {
    std::cerr << "Unable to parse INI file '" << filepath << "'\n";
    return 1;
  }

  // Accessing values 
  istr = reader.Get("user", "name", "dirk"); 
  ival = reader.GetInteger("user", "age", 8);

  return 0;
}

int MMConfig::writeFile(const string &filepath)
{
  std::ofstream iniFile(filepath);

  if (iniFile.is_open())
  {
    iniFile << "[user]\n";
    iniFile << "name=" << istr << "\n";
    iniFile << "age=" << ival << "\n";
    iniFile.close();
  } 
  else 
  {
    std::cerr << "Unable to open file '" << filepath << "' for writing\n";
    return 1;
  }
  return 0;
}
