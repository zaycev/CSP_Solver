all:
	@g++ csp.cpp -O3
	@mv a.out KenKenPuzzleSolver
	@./KenKenPuzzleSolver input9.txt otput_example1.txt
	@cat output_q3.txt

clean:
	@rm KenKenPuzzleSolver