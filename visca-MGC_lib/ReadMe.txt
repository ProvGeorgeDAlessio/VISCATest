==============================
   MGC CAMERA CONTROL CODE
  (PROJECT NAME: VISCA_MGC).
==============================

This project implements the Sony VISCA camera control protocol with 
a multi-threaded serial library.  Various portions are licensed under
different terms.  Specifically:

1. TSERIAL_EVENT Library is Copyright (C) 2001-2002 by Thierry Schneider
   and is licenced under freeware terms that require only maintaining 
   copyright notice in source code files.  

   AFFECTED FILES: TSERIAL_EVENT.H and TSERIAL_EVENT.CPP


2. Some library constants in the VISCA_CONSTS.H file are from the Linux 
   LIBVISCA project and are licensed under the FSF's Lesser General
   Public License (LGPL).  

3. VISCA_CAMERA.H and VISCA_CAMERA.CPP implement the VISCA protocol, 
   and are ground-up creations of MGC Works Inc.  They are licensed 
   under the FSF's Lesser General Public License (LGPL).

4. VISCA_MGC.CPP is a small test program that calls the library functions.
   It is now licensed under FSF's Lesser General  Public Lincse (LGPL).

Please note that all LGPL code can be freely incorporated into proptietery
applications without violating the license.  Therefore, no licensing issues
arise from the use of LGPL code in future applications using these libraries, 
provided that the LGPL terms are adhered to strictly.

Any revision or modification of the LGPL code MUST be made freely available
under the terms of the LGPL license.  Any distribution of binary versions of
the LGPL code not in strict adherence to the LGPL terms is strictly prohibited.





