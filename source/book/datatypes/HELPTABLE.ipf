Help table. 
:p.
This is a collection of help table entries, each of which has the structure defined below, 
the last entry of the collection being a NULL structure. 
:xmp.
typedef struct _HELPTABLE {
  :link reftype=hd viewport refid=USHORT.USHORT:elink.            idAppWindow;       /*  Application window identity. */
  :link reftype=hd viewport refid=HELPSUBTABLE.PHELPSUBTABLE:elink.     phstHelpSubTable;  /*  Help subtable for this application window. */
  :link reftype=hd viewport refid=USHORT.USHORT:elink.            idExtPanel;        /*  Identity of the extended help panel for the application window. */
} HELPTABLE;

typedef HELPTABLE *PHELPTABLE;
:exmp. 
