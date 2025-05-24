#ifndef MMCONFIG_H
#define MMCONFIG_H

#include <string>

struct MMConfig
{

    //
    // Morse
    //

    /** Geschwindigkeit in Buchstaben pro Minute (bpm) */
    int speed;
    /** Pausenfaktor */
    int farnsworthFactor;
    /** Gesamtzahl der zu gebenden Buchstaben */
    unsigned long int totalLength;
    /** Bestätigung/Abfrage der einzelnen Wörter? (0=nein, 1=ja) */
    int confirmWords;
    /** Modus für das Erzeugen neuer Wörter. (0=zufällig, 1=aus Datei, 2=PARIS)*/
    int wordMode;

    // random
    /** Auswahl der Zeichenmenge (1-8) */
    int charGroup;
    /** Zeichenmenge als String (Auswahl Zeichenmenge = 8) */
    std::string charSet;
    /** Art der Wortgruppen (0=fest, 1=variabel) */
    int variableWordLength;
    /** Länge der festen Wortgruppen (2-8) */
    int fixedWordLength;

    // file
    /** Name der aktuellen Datei, aus der neue Wörter entnommen werden sollen. */
    std::string fileName;
    /** Enthält die Datei einzelne Worte pro Zeile, die zufällig ausgegeben werden
     * sollen, oder ist es ein Text der zusammenhängend gegeben wird (0=nein/text, 1=ja/zufällig)?
     */
    int fileModeWords;
    /** Letzte Wort-Position innerhalb der Datei (zur Fortsetzung). */
    unsigned long int filePosition;
    /** Sollen bei der Augabe aus einer Datei alle bekannten Zeichen gegeben werden,
     * oder nur die DTP-relevanten (0=nein/dtp, 1=ja/alle)?
     */
    int fileUseAllChars;

    //
    // Common
    //

    /** Soll die Tonhöhe zufällig gewählt werden? (0=nein, 1=ja) */
    int randomFrequency;
    /** Tonhöhe in Hz falls die Tonhöhe fest gesetzt ist. */
    int fixedMorseFrequency;
    /** Methode zum Abrunden der Zeichenkanten (0-3) */
    int soundShaping;
    /** Werden Fehler pro Wort gezählt? (0=nein jeder Buchstabe einzeln, 1=ja) */
    int errorsPerWord;
    /** Sollen die Optionen/Einstellungen in der INI Datei gespeichert
     * werden? (0=nein, 1=ja)
     */
    int saveOptions;

    MMConfig();
    int readFromFile(const std::string &filepath);
    int writeFile(const std::string &filepath);
    void setDefaultValues();
    std::string toString();
};

#endif
