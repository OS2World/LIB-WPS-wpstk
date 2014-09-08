Time data structure for file-system functions. 
:xmp.
typedef struct _FTIME {
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     twosecs&colon.5;  /*  Binary number of two-second increments. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     minutes&colon.6;  /*  Binary number of minutes. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     hours&colon.5;    /*  Binary number of hours. */
} FTIME;
 
typedef FTIME *PFTIME;
:exmp. 
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
