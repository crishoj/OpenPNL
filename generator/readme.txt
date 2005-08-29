We created a generator, which will help users to create new projects, that links to PNL or PNLHigh.

What is generator?
==================

Generator is an executable file, which can be found in PNL/generator folder. To create a new empty project you have to run this file. Then you have to enter the name of the new project and the type of it. 

Two types are available now. They are “PNL” and “PNLHigh”. The project of “PNL” type contains all necessary libraries and includes to work with the pnl.dll directly. The project of “PNLHigh” type contains all necessary project settings to work with PNL with the help of wrappers.
The new project is created in PNL/Applications folder. You can move the folder with your project (folder with the name that you specified while working with generator) everywhere you want. There are three files of the project in this folder. They are *.dsw file of Visual Studio workspace, *.dsp file of Visual Studio project and main.cpp file. To start work with your project you need to open the *.dsw file using “OpenWorkspace” command in “File” menu of Visual Studio.

Implementation 
==============

Generator creates a subfolder in PNL/Applications directory. Then it copies the files from PNL/Applications/Template directory to the new folder and rename them according to the project name, which was specified by user. Then, it adds some necessary information to the lines in *.dsp file that specifies the additional include paths, input libraries and paths to the *.lib files. 


Warning
=======

Do not remove the generator.exe from this folder!!
Otherwize it won't work properly!!