#include "mmconfig.h"
#include "INIReader.h"
#include "global.h"

#include <iostream>
#include <fstream>
#include <sstream>

using std::string;

MMConfig::MMConfig()
{
  setDefaultValues();
}

int MMConfig::readFromFile(const string &filepath)
{
  INIReader reader(filepath);

  if (reader.ParseError() > 0)
  {
    std::cerr << "Unable to parse INI file '" << filepath << "'\n";
    return MM_FALSE;
  }

  // Accessing values 
  speed = reader.GetInteger("morse", "speed", 60);
  farnsworthFactor = reader.GetInteger("morse", "farnsworthFactor", 1);
  totalLength = reader.GetUnsigned64("morse", "totalLength", 200);
  confirmWords = reader.GetInteger("morse", "confirmWords", MM_FALSE);
  wordMode = reader.GetInteger("morse", "wordMode", MM_WM_RANDOM);

  // random
  charGroup = reader.GetInteger("morse", "charGroup", 1);
  charSet = reader.Get("morse", "charSet", ""); 
  variableWordLength = reader.GetInteger("morse", "variableWordLength", MM_FALSE);
  fixedWordLength = reader.GetInteger("morse", "fixedWordLength", 5);

  // file
  fileName = reader.Get("morse", "fileName", ""); 
  fileModeWords = reader.GetInteger("morse", "fileModeWords", MM_TRUE);
  filePosition = reader.GetUnsigned64("morse", "filePosition", 0);
  fileUseAllChars = reader.GetInteger("morse", "fileUseAllChars", MM_FALSE);

  //
  // Common
  //

  randomFrequency = reader.GetInteger("common", "randomFrequency", MM_TRUE);
  fixedMorseFrequency = reader.GetInteger("common", "fixedMorseFrequency", 800);
  soundShaping = reader.GetInteger("common", "soundShaping", 3);
  errorsPerWord = reader.GetInteger("common", "errorsPerWord", MM_TRUE);
  countCharsInFileMode = reader.GetInteger("common", "countCharsInFileMode", MM_TRUE);
  saveOptions = reader.GetInteger("common", "saveOptions", MM_TRUE);

  return MM_TRUE;
}

int MMConfig::writeFile(const string &filepath)
{
  std::ofstream iniFile(filepath, std::ofstream::out | std::ofstream::trunc);

  if (iniFile.is_open())
  {
    iniFile << toString();
    iniFile.flush();
  } 
  else 
  {
    std::cerr << "Unable to open file '" << filepath << "' for writing\n";
    return MM_FALSE;
  }
  return MM_TRUE;
}

void MMConfig::setDefaultValues()
{
  //
  // Morse
  //

  speed = 60;
  farnsworthFactor = 1;
  totalLength = 200;
  confirmWords = MM_FALSE;
  wordMode = MM_WM_RANDOM;

  // random
  charGroup = 1;
  charSet = "";
  variableWordLength = MM_FALSE;
  fixedWordLength = 5;

  // file
  fileName = "";
  fileModeWords = MM_TRUE;
  filePosition = 0;
  fileUseAllChars = MM_FALSE;

  //
  // Common
  //

  randomFrequency = MM_TRUE;
  fixedMorseFrequency = 800;
  soundShaping = 3;
  errorsPerWord = MM_TRUE;
  countCharsInFileMode = MM_TRUE;
  saveOptions = MM_TRUE;
}

string MMConfig::toString()
{
  std::ostringstream iniFile;

  // morse
  iniFile << "[morse]\n";
  iniFile << "speed=" << speed << "\n";
  iniFile << "farnsworthFactor=" << farnsworthFactor << "\n";
  iniFile << "totalLength=" << totalLength << "\n";
  iniFile << "confirmWords=" << confirmWords << "\n";
  iniFile << "wordMode=" << wordMode << "\n";
  // random
  iniFile << "charGroup=" << charGroup << "\n";
  iniFile << "charSet=" << charSet << "\n";
  iniFile << "variableWordLength=" << variableWordLength << "\n";
  iniFile << "fixedWordLength=" << fixedWordLength << "\n";
  // file
  iniFile << "fileName=" << fileName << "\n";
  iniFile << "fileModeWords=" << fileModeWords << "\n";
  iniFile << "filePosition=" << filePosition << "\n";
  iniFile << "fileUseAllChars=" << fileUseAllChars << "\n\n";
 
  iniFile << "[common]\n";
  iniFile << "randomFrequency=" << randomFrequency << "\n";
  iniFile << "fixedMorseFrequency=" << fixedMorseFrequency << "\n";
  iniFile << "soundShaping=" << soundShaping << "\n";
  iniFile << "errorsPerWord=" << errorsPerWord << "\n";
  iniFile << "countCharsInFileMode=" << countCharsInFileMode << "\n";
  iniFile << "saveOptions=" << saveOptions << "\n\n";
 
  return iniFile.str();
}

bool operator==(const MMConfig& lhs, const MMConfig& rhs)
{
  if (lhs.randomFrequency != rhs.randomFrequency)
    return false;
  if (lhs.speed != rhs.speed)
    return false;
  if (lhs.farnsworthFactor != rhs.farnsworthFactor)
    return false;
  if (lhs.totalLength != rhs.totalLength)
    return false;
  if (lhs.confirmWords != rhs.confirmWords)
    return false;
  if (lhs.wordMode != rhs.wordMode)
    return false;
  if (lhs.charGroup != rhs.charGroup)
    return false;
  if (lhs.charSet != rhs.charSet)
    return false;
  if (lhs.variableWordLength != rhs.variableWordLength)
    return false;
  if (lhs.fixedWordLength != rhs.fixedWordLength)
    return false;
  if (lhs.fileName != rhs.fileName)
    return false;
  if (lhs.fileModeWords != rhs.fileModeWords)
    return false;
  if (lhs.filePosition != rhs.filePosition)
    return false;
  if (lhs.fileUseAllChars != rhs.fileUseAllChars)
    return false;
  if (lhs.randomFrequency != rhs.randomFrequency)
    return false;
  if (lhs.fixedMorseFrequency != rhs.fixedMorseFrequency)
    return false;
  if (lhs.soundShaping != rhs.soundShaping)
    return false;
  if (lhs.errorsPerWord != rhs.errorsPerWord)
    return false;
  if (lhs.countCharsInFileMode != rhs.countCharsInFileMode)
    return false;
  if (lhs.saveOptions != rhs.saveOptions)
    return false;

  return true;
}

bool operator!=(const MMConfig& lhs, const MMConfig& rhs)
{
  return !(lhs == rhs);
}
