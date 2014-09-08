Structure that contains time information for a data element in the details view of a container control.
:xmp.
typedef struct _CTIME {
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.     hours;       /*  Current hour. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.     minutes;     /*  Current minute. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.     seconds;     /*  Current second. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.     ucReserved;  /*  Reserved. */
} CTIME;

typedef CTIME *PCTIME;
:exmp.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
