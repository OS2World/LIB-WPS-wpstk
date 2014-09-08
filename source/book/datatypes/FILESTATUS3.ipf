Level 1 (32-bit) (FIL_STANDARD) information. 
:xmp.
 typedef struct _FILESTATUS3 {
   :link reftype=hd viewport refid=FDATE.FDATE:elink.     fdateCreation;    /*  Date of file creation. */
   :link reftype=hd viewport refid=FTIME.FTIME:elink.     ftimeCreation;    /*  Time of file creation. */
   :link reftype=hd viewport refid=FDATE.FDATE:elink.     fdateLastAccess;  /*  Date of last access. */
   :link reftype=hd viewport refid=FTIME.FTIME:elink.     ftimeLastAccess;  /*  Time of last access. */
   :link reftype=hd viewport refid=FDATE.FDATE:elink.     fdateLastWrite;   /*  Date of last write. */
   :link reftype=hd viewport refid=FTIME.FTIME:elink.     ftimeLastWrite;   /*  Time of last write. */
   :link reftype=hd viewport refid=ULONG.ULONG:elink.     cbFile;           /*  File size (end of data). */
   :link reftype=hd viewport refid=ULONG.ULONG:elink.     cbFileAlloc;      /*  File allocated size. */
   :link reftype=hd viewport refid=ULONG.ULONG:elink.     attrFile;         /*  Attributes of the file. */
 } FILESTATUS3;
 
 typedef FILESTATUS3 *PFILESTATUS3;
:exmp.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
