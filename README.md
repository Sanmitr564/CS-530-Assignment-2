Group Members:                  Aaron Alegre, Patrick Stewart, Edwin Vega
Class Accounts (Respectively):  cssc2101, cssc2138, cssc2141
RedIDs (Respectively):          828222103, 130969511, 827746186
CS 530 Fall 2025
Assignment 2 - SIC/XE Two-Pass Assembler
README

Files in this Project:
    -main.c 
    -assembler.c 
    -assembler.h 
    -instructions.c 
    -instructions.h 
    -linkedlist.c 
    -linkedlist.h 
    -Makefile
    -README (this)
    
TO COMPILE:
    -Make sure project files are in current directory in Edoras
    -Enter command, "make" in terminal
    -This should return an executable, asx, which you can then run to test it out

Instructions for Use:
    -Takes in one or more SIC/XE source files with the .sic extension
    -Only .sic files are accepted, others will throw out an error
    -For every input there will be two outputs. 
        -One .l file which is the listing 
        -One .st file which is the symbol and literal tables


Significant Design Decisions
    -Two pass assembler
        -Pass 1 creates and intermediate representation, SYMTAB and LITTAB. This also calculates the programs length.
        -Pass 2 Produces the listing and tables. 
    -We use a singly linked list for intermediate representation, SYMTAB and LITTAB. 
    -Supports formats 1, 2, 3, and 4. PC relative or BASE relative when enabled
    -For validation there are opcode and format checks, range checks and a fixed column parsing. Errors will terminate with a message. 

Deficiencies/Bugs
    -No algebra evaluation for operands
    -Some format 2 instructions which don’t take registers won’t process correctly
    -Won’t automatically print literals before EOF unless signalled by * in source code
    

Lessons Learned
    -How a two pass asembler works and what each pass should do
    -The specific decisions and functionallity of both passes and how they come together
    -How to connect what we've seen in class with our coding skills
    -What an SDD is and how it helps in starting a project
    -Organization of a project with an SDD
    -Helped imporve our teamwork skills so everything is done and divided evenly with the team 
