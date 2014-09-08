Help subtable. 
:p.
A help subtable is an array of records, preceded by a value that specifies the size of each help-subtable record. 
:xmp.
typedef USHORT _HELPSUBTABLE {
  USHORT     usSubitemSize;        /*  Size of each record of the help subtable. */
  USHORT     HelpSubTableEntry[];  /*  Help subtable records. */
};
:exmp.
The first entry in the help subtable indicates the size of the records that follow in the subtable. Each of the following 
entries in the help subtable is a record that consists of a Field ID parameter, a Help Panel ID parameter, and an 
optional array of application-related USHORT integers. The minimum number of words in the record is two: the Field ID 
and the Help Panel ID. The last record in the subtable must be a NULL entry. 
:p.
The Field ID is the symbolic constant for a field from which the user can request help. The Field ID can identify a 
control, a menu item, or a message box, and must be unique across the help subtable. The value 0xFFFF is reserved 
for use by the Help Manager. 
:p.
The Help Panel ID is the resource ID (res) of the contextual help panel to be associated with the field in the Field ID 
parameter. This is the panel to be displayed when the user requests help for the field. 
:p.
The optional array of USHORT integers is ignored by the Help Manager and can be used to store information of 
relevance to the application. 
:p.
There can be a maximum of 16,000 help subtables for a given help instance and each subtable can have a maximum 
of 64K bytes of data. 
:p.
The following figure contains the declaration of a help subtable that contains only Field IDs and Help Panel IDs. In 
this subtable, each of the records after the size entry consists of 1 Field ID and 1 Help Panel ID for a size of 2.  
Note that the last record is filled with NULLs (0) to indicate the end of the array. 
:xmp.
HELPSUBTABLE HelpSubTable[] =
{
   2,                           /* Size of each record */
   FIELD_ID_1, IDRES_HELP1,     /* The first record    */
   FIELD_ID_2, IDRES_HELP2,     /* The second record   */
   FIELD_ID_3, IDRES_HELP3,     /* The third record    */
   FIELD_ID_4, IDRES_HELP4,     /* The fourth record   */
   FIELD_ID_5, IDRES_HELP5,     /* The fifth record    */
   FIELD_ID_6, IDRES_HELP6,     /* The sixth record    */
   0,          0                /* NULL record == end of the array */
}
:exmp.
