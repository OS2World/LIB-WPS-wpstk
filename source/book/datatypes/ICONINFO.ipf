Icon information data structure. 
:xmp.
typedef struct _ICONINFO {
  :link reftype=hd viewport refid=ULONG.ULONG:elink.       cb;           /*  Length of the ICONINFO structure. */
  :link reftype=hd viewport refid=ULONG.ULONG:elink.       fFormat;      /*  Indicates where the icon resides. */
  :link reftype=hd viewport refid=PSZ.PSZ:elink.         pszFileName;  /*  Name of the file containing icon data. */
  :link reftype=hd viewport refid=HMODULE.HMODULE:elink.     hmod;         /*  Module containing the icon resource. */
  :link reftype=hd viewport refid=ULONG.ULONG:elink.       resid;        /*  Identity of the icon resource. */
  :link reftype=hd viewport refid=ULONG.ULONG:elink.       cbIconData;   /*  Length of the icon data in bytes. */
  :link reftype=hd viewport refid=PVOID.PVOID:elink.       pIconData;    /*  Pointer to the buffer containing icon data. */
} ICONINFO;

typedef ICONINFO *PICONINFO;
:exmp. 
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
