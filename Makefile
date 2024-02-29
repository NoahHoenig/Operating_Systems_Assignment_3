# CXX Make variable for compiler
CC=g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb 
CCFLAGS=-std=c++11 -Wall -g3 -c

# object files
OBJS = main.o vaddr_tracereader.o bit_operations.o pagetable.o print_helpers.o

# Program name
PROGRAM = mmuwithtlb

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.cpp
	$(CC) $(CCFLAGS) main.cpp

vaddr_tracereader.o : vaddr_tracereader.c vaddr_tracereader.h
	$(CC) $(CCFLAGS) vaddr_tracereader.c

bit_operations.o : bit_operations.cpp bit_operations.h
	$(CC) $(CCFLAGS) bit_operations.cpp

pagetable.o : pagetable.cpp pagetable.h
	$(CC) $(CCFLAGS) pagetable.cpp

print_helpers.o : print_helpers.cpp print_helpers.h
	$(CC) $(CCFLAGS) print_helpers.cpp


	


# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -f *.o *~ $(PROGRAM)

