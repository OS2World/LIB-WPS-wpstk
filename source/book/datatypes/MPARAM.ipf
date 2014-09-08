A 4-byte message-dependent parameter structure.
:xmp.
typedef VOID *MPARAM;
:exmp.
:p.
Certain elements of information, placed into the parameters of a message,
have data types that do not use all four bytes of this data type.
The rules governing these cases are&colon.
:parml compact tsize=10 break=none.
:pt.:link reftype=hd viewport refid=BOOL.BOOL:elink.
:pd.The value is contained in the low word and the high word is 0.
:pt.:link reftype=hd viewport refid=SHORT.SHORT:elink.
:pd.The value is contained in the low word and its sign is extended into the high word.
:pt.:link reftype=hd viewport refid=USHORT.USHORT:elink.
:pd.The value is contained in the low word and the high word is 0.
:pt.NULL
:pd.The entire four bytes are 0.
:eparml.
:p.
The structure of this data type depends on the message. For details,
see the description of the particular message.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
