
------------------------------------------------------------------
 PNL -- Probabilistic Networks Library. Beta 1.0. 17-Febrary-2004
------------------------------------------------------------------

Requirements
  Operating system:
    MS Windows 98/Me/2000/XP
    Linux
  Compiler:
    Visual C++ 6.0 (Intel Compiler 7.0 may used as compiler
    for a Visual Studio Environment)
    gcc 3.x.x
    icc 8.x

Directory tree.
---------------
        bin(*)             -- executable files and DLLs
        lib(*)             -- static libraries and stub libraries (for using DLLs)
        c_pgmtk            -- root folder for C++ version of PNL
           examples        -- example applications
              data         -- folder containing data files, which is used in examples
           include         -- include files for external interface
           make            -- project definition files 
           src             -- source code of library
              include      -- internal header files
           tests           -- set of tests
              include      -- internal header files for tests
              src          -- source code for tests
              make         -- project definition files for tests        
              testdata     -- data files used by tests 
              !readme!.txt -- read it before you start building tests
        cxcore             -- openCV core. Used for operation with sparse matrices
        doc                -- users guide and reference manual           
        trs                -- TRS test system
           include         -- header files
           make            -- project definition files
           src             -- source files

(*) The directory and its content are generated during the build
    process.

Building the library, examples and tests for C/C++ version from Developer Studio 6.0
--------------------------------------------------------

To build the library and utilities from Developer Studio 6.0 do the following:

  1. Start Microsoft Developer Studio 6.0.

  2. Open workspace "c_pgmtk/make/pnl.dsw". It contains the following 
        projects:

     Project...             For...
     ------------------------------------------------------------
     _build_all             All components provided by workspace
     ex_param               Example of using evidence class
     gibbs                  Example of using Gibbs inference sampling
     inf_learn_bnet         Example of using inference and learning classes for BNets
     inf_learn_dbn          Example of using inference and learning classes for DBNs
     learn_param            Example of using learn class
     mixture_gaussian_bnet  Example of mixture gaussian bnet creation
     pnl                    C++ version of PNL 
     test_pnl_c             Tests for C++ version of PGMTk 
     testSL                 Test on structure learning of BNet
     trial                  Example of working with junction tree inference engine
     trs                    TRS test system
     use_matrix             Example of operating with matricies

   4. Build project _build_all to build library, examples and tests.

Notes:
  (a) Configurations "Win32 Debug" and "Win32 Release" build DLL version 
      of the library, examples and tests that link this DLL.

  (b) Debug variants of library, examples and tests have the suffix "d",
      for example: "pnld.dll", "triald.exe".


--------------------------------------------------------
Building the library, examples and tests for
C/C++ version from Linux with gcc
--------------------------------------------------------
   1. Go to the root directory (it contain this file and changes.txt)
   2. Run './configure.gcc'
   3. Run 'make'

Notes:
   Step 2 (Run './configure.gcc') should be run on initial
or on compiler changing

   If you have some error during compiling or if you want to view compiling
message later, run 'make 2>&1 | tee compiling.log' instead of 'make'


--------------------------------------------------------
Building the library, examples and tests for
C/C++ version from Linux with icc (Intel compiler)
--------------------------------------------------------
   1. Go to the root directory (it contain this file and changes.txt)
   2. Run './configure.icc'
   3. Run 'make'

Notes:
   Step 2 (Run './configure.icc') should be run on initial
or on compiler changing

   If you have some error during compiling or if you want to view compiling
message later, run 'make 2>&1 | tee compiling.log' instead  of 'make'