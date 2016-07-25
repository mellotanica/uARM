all: manual.pdf

manual.pdf: $(wildcard *.tex) $(wildcard tex/*.tex) $(wildcard tex/tables/*.tex)
	pdflatex manual.tex
	pdflatex manual.tex

clean:
	-rm -f manual.pdf
