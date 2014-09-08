Date and time data structure.
:xmp.
typedef struct _DATETIME {
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      hours;       /*  Current hour, using values 0 through 23. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      minutes;     /*  Current minute, using values 0 through 59. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      seconds;     /*  Current second, using values 0 through 59. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      hundredths;  /*  Current hundredths of a second, using values 0 through 99. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      day;         /*  Current day of the month, using values 1 through 31. */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      month;       /*  Current month of the year, using values 1 through 12. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.     year;        /*  Current year. */
  :link reftype=hd viewport refid=SHORT.SHORT:elink.      timezone;    /*  The difference in minutes between the current time zone and Greenwich Mean Time (GMT). */
  :link reftype=hd viewport refid=UCHAR.UCHAR:elink.      weekday;     /*  Current day of the week, using values 0 through 6. */
} DATETIME;

typedef DATETIME *PDATETIME;
:exmp.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
