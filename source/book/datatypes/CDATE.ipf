Structure that contains date information for a data element in the details view of 
a container control. 
:xmp.
typedef struct _CDATE {
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      day;    /*  Current day. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      month;  /*  Current month. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     year;   /*  Current year. */
} CDATE;

typedef CDATE *PCDATE;
:exmp. 
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
