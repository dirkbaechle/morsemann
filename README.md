# Morsemann
Ein Programm für das Lernen und Üben des Hörens von Morsezeichen (CW)

Copyright (C) 2003-2024 by Dirk Bächle (dl9obn@darc.de)

https://github.com/dirkbaechle/morsemann


## Disclaimer

```
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the 

Free Software Foundation, Inc.
675 Mass Ave
Cambridge
MA 02139
USA
```

(siehe auch die Datei "COPYING")


## Kompilieren des Programmes

### Linux

Wechseln sie in das erzeugte Verzeichnis `morsemann`.

Geben Sie das Kommando

```
make
```

ein. Nach kurzer Zeit sollte der Kompilier-Vorgang stoppen und eine
ausführbare Datei namens `morsemann` erzeugt haben. Denken Sie bitte
daran, dass sie Root-Rechte brauchen um den "Morsemann" zu starten
(nicht für das Kompilieren!).
Wechseln Sie also vor dem Start des Programmes mit

```
su
```

in den Superuser-Modus. 
Falls man nicht immer wieder `su' und das Passwort eingeben
möchte, kann man für die Datei `morsemann' auch das entsprechende
Ausführungs-Bit setzen. Dazu wechselt man in den Superuser-Modus und
ruft:

```
make allusers
```

auf. Anschliessend kann man den "Morsemann" als normaler Benutzer/User
starten (er wird aber im Root-Modus ausgeführt)...


Beschwert sich `make' darüber, dass es den Kompiler `gcc' nicht finden
konnte, so haben Sie zwei Möglichkeiten: 

1. Den `g++` installieren.
2. Sie editieren das `Makefile` und setzen die Variable `CXX` auf den
C++-Kompiler, der in Ihrer Linux-Distribution installiert ist.

Falls der Linker die Library `ncurses` nicht finden kann, so haben Sie
diese wahrscheinlich nicht installiert. Erneut haben Sie zwei Optionen:

1. `ncurses` nachträglich installieren.
2. Sie kompilieren den "Morsemann" mit dem Aufruf

```
make curses
```

      oder, falls sich der Kompiler immer noch beschwert, ohne Farben mit
```
make nocolor
```

## Quellcode-Dokumentation

Der Quellcode (zumindest der in `morsemann.cpp`) ist mit speziellen
C-Kommentaren versehen. 
Mit dem Programm "Doxygen" (http://www.stack.nl/~dimitri/doxygen) kann
bei Bedarf leicht eine Dokumentation in HTML oder LaTeX erstellt
werden. Lesen Sie bitte zu diesem Zweck die Anleitung von "Doxygen".

## Credits

Die Dateien `beep.h`, `beepLinux.c` und `alarm.[ch]` wurden 
unverändert dem Programm `morse` von Joe Dellinger entnommen. 
Jeglicher Dank, dafür dass man unter Linux die Morsezeichen 
im PC-Speaker auch hört und nicht nur sieht, gebührt daher
Joe Dellinger, Scott Seligman und John Paul Morrison.

Für die Verwaltung des Config-Files wurde eine Version des
Paketes "inih" von Ben Hoyt (https://github.com/benhoyt/inih,
7914ad7f4f43, 2025-03-17 02:28:26) benutzt.

Dirk Bächle, 2025-05-18


## Anleitung

Ein kleines Programm zum Lernen und Üben des Hörens
von Morsezeichen (CW). Es kennt die Zeichen `a-z`, `0-9` und `.,=?/`.

*Es tutet leis' der "Morsemann"*,<br>
*bis man schneller hören kann*... (frei nach Loriot)


### Hauptfenster

Nach dem Start des Programmes wird das Hauptfenster angezeigt.
Die Auswahl von Menüpunkten erfolgt im gesamten
Programm mit den Pfeil-Tasten
(`CursorUp` und `CursorDown`), bestätigt wird
mit der `RETURN`-Taste. Falls man nicht gerade eine 
"Texteingabe"
macht, kommt man mit der `BACKSPACE`-Taste zurück zum
vorherigen Fenster.

Wählt man den Punkt "Start", so beginnt der *Morsemann*
mit der Ausgabe von Morsezeichen entsprechend den eingestellten Optionen.
Wie man diese ändert wird im Abschnitt "Optionen"
beschrieben.

Mit der `BACKSPACE`-Taste kann die Ausgabe der Zeichen
unterbrochen werden. Ansonsten hält das Programm nicht eher
an, bis die gewünschte Anzahl von Zeichen erreicht ist.
Durch Betätigen einer
beliebigen Taste gelangt man dann wieder in das Hauptfenster.

Bei Auswahl des Punktes "Beenden" wird das Programm ... naja,
beendet.

### Optionen

Folgende Optionen sind verfügbar:

- **Zeichen**: Ein weiteres Fenster erscheint und man kann
die gewünschte Gruppe von Zeichen wählen. Mit dem Punkt
"Zeichen eingeben" kann man seinen eigenen Zeichenvorrat (maximal
75 Zeichen!) zusammenstellen, aus diesem wählt der 
*Morsemann* dann zufällig aus.
Die mehrfache Eingabe erhöht die Häufigkeit mit der das Zeichen
im Ausgabetext erscheint. Es sind nur die unterstützten Zeichen
zugelassen und es dürfen keine Leerzeichen eingegeben werden!
- **Geschwindigkeit**: In einem eigenen Fenster kann man mit
den Pfeil-Tasten die Geschwindigkeit im Bereich 10-250 BPM (Buchstaben
pro Minute) einstellen.
- **Pausenfaktor**: Der Pausenfaktor verlängert die
Abstände zwischen den einzelnen Morsezeichen und auch zwischen
kompletten Wörtern. Ist er z.B. auf "2" gestellt, so sind
die Pausen doppelt so lang wie normal. Er kann im Bereich 1-9 gewählt
werden. Hinweis: Der Faktor "4" bei einem Tempo von 45 BPM entspricht
den Einstellungen für Prüfungen nach der Farnsworth-Methode. 
- **Zeichenanzahl**: Die Zeichenanzahl ist von 5-9999 per
"Texteingabe" einstellbar.
- **Feste Wortgruppen**: Steht diese Option auf "Ja", so erzeugt
der *Morsemann* alle Wörter mit der gleichen Länge.
Diese steht hinter der Option in Klammern und kann durch Betätigen
der Links- und Rechts-Pfeil-Tasten (CursorLeft und CursorRight) im Bereich
2-8 verändert werden. Ist "Nein" angewählt, so variiert
die Wortlänge zufällig von 3 bis 8.
- **Zeichen bestätigen**: Die Standardeinstellung ist
hier "Nein". Der *Morsemann* gibt dann Zeichen um Zeichen, ohne
auf eine Rückmeldung vom Benutzer zu warten. Ist "Buchstaben"
gewählt, muss jedes einzelne Zeichen bestätigt werden.
Dazu tippen Sie auf der Tastatur einfach das Zeichen, das Sie meinen
gehört zu haben. War die Eingabe richtig, so erklingt sofort das
nächste Zeichen. Bei einem Fehler erklingt ein Fehler-Ton
und das richtige Zeichen erscheint
in roter Schrift auf dem Bildschirm. Am Ende der Ausgabe wird die
Gesamtanzahl der Fehler noch einmal angezeigt. Ähnlich 
verhält sich der *Morsemann* auch bei der Einstellung
"Worte". Jetzt müssen allerdings ganze Worte gehört und
dann durch "Texteingabe"
bestätigt werden. 

### Texteingaben

Bei Texteingaben verliert die `BACKSPACE`-Taste für
kurze Zeit ihre Bedeutung als "Zum-vorherigen-Fenster-zurück".
Sie dient dann dazu einzelne Buchstaben zu löschen. Texteingaben
im *Morsemann* können auf zwei Arten abgeschlossen werden:
Durch die `RETURN`- oder die `ESC`-Taste. Mit der
`RETURN`-Taste akzeptiert man die gemachten Änderungen, bei
`ESC` werden sie verworfen.


