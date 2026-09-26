### CS452 Homework 2

## Project Description
This project creates a memory allocation system using the format of a Buddy System. 

## Building and Running
I updated the lower level make file taken from homework 1 to match my current project. In order to use the following commands, you must be in the HW-2-2 directory. You can make all files using "make all" and can also run "make clean" to delete all initialized files. I also added separate commands to make tests in both main and from the wrapper tests. To make the main file run "make main" and "make wrappertest" to make the tests designed using the wrapper and deq. You can then show the results of these tests by calling ./main or ./wrappertest respectively. For future projects, balloc.c will be the main method of using my memory allocation system. 

## My Experience
I wish I had researched more about the Buddy System before I started my project as I had to often stop what I was doing and research the correct way to handle certain events using the Buddy System. This mainly occurred when testing edge cases and how to handle the top level/box of the system. My print functions greatly helped me better understand where my errors were coming from and how to resolve them. I worked through this project by testing each new implementation I added before moving onto the next implementation. This project took me a long time to complete, but I'm happy with how the final product turned out. Also, I separated my testing using the wrapper and my deq from homework 1 into a separate file than the other tests for this project to prevent any potential issues with differing allocation methods. You can find my tests for deq in the wrappertest.c file and run them using my previously stated commands. 

## Known Issues
There are no known issues with my current implementation. 