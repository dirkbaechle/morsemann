all:
	sgml2latex -l german -o tex anleit;

ps: anleit.tex
	latex anleit;
	latex anleit;
	latex anleit;
	dvips -D300 -o anleit.ps anleit.dvi
	
