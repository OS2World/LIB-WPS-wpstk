Pointer to a window procedure.
:p.
This is the standard function definition for window procedures.
:xmp.
typedef FNWP *PFNWP;
:exmp.
:p.
The first argument (:link reftype=hd viewport refid=HWND.HWND:elink.) is the handle of the window receiving the message.
.br
The second argument (:link reftype=hd viewport refid=ULONG.ULONG:elink.) is a message identifier.
.br
The third argument (:link reftype=hd viewport refid=MPARAM.MPARAM:elink.) is the first message parameter (mp1).
.br
The fourth argument (:link reftype=hd viewport refid=MPARAM.MPARAM:elink.) is the second message parameter (mp2).
.br
The function returns an :link reftype=hd viewport refid=MRESULT.MRESULT:elink.. Each message has a specific set of possible return codes.
The window procedure must return a value that is appropriate for the message being processed.

In the header file, this is a two-part definition as shown below:
:xmp.
typedef MRESULT (EXPENTRY FNWP)(HWND, ULONG, MPARAM, MPARAM);
typedef FNWP *PFNWP;
:exmp.
:p.
This data type is being introduced by the OS/2 Warp Toolkit.
