To start test AMarginalizeCompareMatLab you need to add path to your Matlab directories and add path to BNT in your MatLab and save it here.
All tests can't compile before it!
So, you should:
1) At Microsoft Visual Studio at Tools\Options\Directories:
	at Include files add path: <MatLab>\extern\include
	at Library files add path: <MatLab>\EXTERN\LIB\WIN32\MICROSOFT\MSVC60 (if you work in Windows)
2) At MatLab: File\SetPath
	"Add with subfolders" path to BNT on your computer.
Now you can compile and start all tests