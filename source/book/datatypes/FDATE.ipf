Date data structure for file-system functions. 
:xmp.
typedef struct _FDATE {
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     day&colon.5;    /*  Binary day for directory entry. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     month&colon.4;  /*  Binary month for directory entry. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     year&colon.7;   /*  The number of years since 1980 for this directory entry. */
} FDATE;

typedef FDATE *PFDATE;
:exmp.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
