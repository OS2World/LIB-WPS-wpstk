..******************************* Module Header *******************************
..*
..* Module Name: wtkset.im
..*
..* Hypertext/2 include file for programmers guide section on
..* Settings and Details Manager
..*
..* Copyright (c) WPS Toolkit Project - Christian Langanke 2000
..*
..* $Id: wtkset.inc,v 1.12 2009-04-12 13:21:27 cla Exp $
..*
..* ===========================================================================
..*
..* This file is part of the WPS Toolkit package and is free software.  You can
..* redistribute it and/or modify it under the terms of the GNU Library General
..* Public License as published by the Free Software Foundation, in version 2
..* as it comes in the "COPYING.LIB" file of the WPS Toolkit main distribution.
..* This library is distributed in the hope that it will be useful, but WITHOUT
..* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
..* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
..* License for more details.
..*
..*****************************************************************************

.. ----------------------------------------
.1 Settings and Details Manager
.. ----------------------------------------
.an IDPNL_APISET
.
[=TOPICS]
.su V30 1

.. ----------------------------------------
.2 Settings and Details Manager Overview
.. ----------------------------------------
.an IDPNL_APISET_OVERVIEW
.
The Settings and Details Manager API of the [=TITLE]
encapsulates the complexity of handling object settings and details by
automating the following tasks of a Workplace Shell object class:
.ul
- handling settings strings (like "NAME1=value1;NAME2=value21,value22;") per object class
- handling values for all of the settings per object intance.
- automatic transporting object instance setting values to the settings
  notebookpages of the object instance and vice versa.
- providing an automatic translation of setup strings sent to __wpSetup into
  changes of the settings values of an object instance, with automatic update
  of open setting notebook pages, if desired.
- querying the complete settings implemented by the WPS class for an object instance
  in a settings string, greatly simplifying the code for __wpSaveState and __wpRestoreState.
  While __wpSaveState can save the settings as one string, the __wpRestoreState just has to
  send the previously saved settings string to __wpSetup to initialize the object
  on next Workplace Shell startup.

.. ----------------------------------------
.2 Handling of object settings in general
.. ----------------------------------------
.an IDPNL_APISET_GEN_SETTINGS
.
The general handling of object settings is divided into several parts:

_*creation/awakening of an object*_
.. ----------------------------------------
.pl tsize=10
..
- __wpclsInitData - Initializing the meta class data
.
= Here variables used by all object instances are being set up.
..
- __wpclsQueryIconData - providing an icon for all instances of the class
.
= Here the icon information for the replacement icon is provided
..
- __wpclsQueryDetailsInfo - providing data for organization of details data
.
= Here the layout and offset information about the details data is provided
..
- __wpInitData - Initializing the object instance data
.
= Here the variables are initnalized, of which every object instance has its own copy.
..
- __wpRestoreState - restoring previously settings values
.
= get previously saved values for all settings and store them
  in the instance variables of the object.
..
- __wpQueryDetailsData - providing data for details view
.
= Here the information for the details view for the object instance is provided,
  according to the layout information provided in __wpclsQueryDetailsInfo earlier
..


_*opening of the settings notebook / selecting a notebook page / performing undo*_
.. ----------------------------------------
.pl tsize=10
..
- __wpAddSettingsPages - add own setting notebook pages
.
= a WPS class adds setting notebook pages implementing
  GUI controls for each value of the supported settings. When adding new
  notebook pages, a window message procedure has to be specified for each page.
  When using the Settings and Details Manager API of the [=TITLE],
  mostly one window message procedure is sufficient for all settings notebook pages
  that a WPS class implements.
.
.
[=NOTE]
.ul compact
- The Settings and Details Manager API of the [=TITLE]
  does not provide a function for this. This part has to be coded conventionally and
  is enlisted here for clarity.
.el
- WM__INITDLG in window message procedure - initialize dialog
.
= fill all GUI controls with the appropriate values that are stored
  in the instance variables of the object.
..
- WM__COMMAND for undo pushbutton in window message procedure - restore to current values
.
= fill all GUI controls with the appropriate values that are stored
  in the instance variables of the object.


_*closing the settings notebook / saving the current values*_
.. ----------------------------------------
.pl tsize=10
..
- WM__DESTROY in window message procedure - destroy the dialog
.
= get values from all GUI controls, validate them and store them
  in the instance variables of the object.
..
- __wpSaveState - saving current settings values
.
= save all values of the instance variables of the object.


_*external setup of an object*_
.. ----------------------------------------
.pl tsize=10
..
- __wpSetup (__wpSetupOnce) - modifying the setttings values via settings strings
.
= evaluate all settings strings. If they apply to the WPS class and contain
  valid values, store them in the instance variables of the object.


_*deletion/making dormant of an object*_
.. ----------------------------------------
.pl tsize=10
..
- __wpUnInitData - Deinitializing the object instance data
.
= Free all resources being used by the object instance, e.g memory.
..
- __wpclsUnInitData - Initializing the meta class data
.
= Free all resources being used together by all object instance, e.g memory.

.. ----------------------------------------
.2 Handling of object settings with the [=BASETITLE]
.. ----------------------------------------
.an IDPNL_APISET_API_SETTINGS
.
Use the [=TITLE] API as follows to automate the
settings handling:

_*creation/awakening of an object*_
.. ----------------------------------------
.pl tsize=10
- __wpclsInitData - Initializing the meta class data
.
= Create a settings table and fill it with definitions for all unsupported settings
.pl tsize=10
- [.WtkCreateClassSettingsTable WtkCreateClassSettingsTable]
= create the class settings table and specify the address of a callback procedure
- [.WtkAddClassSetting WtkAddClassSetting]
= call this once for each setting that the WPS class should support.
- [.WtkAddClassDetail WtkAddClassDetail]
= call this once for each detail not being a setting that the WPS class should support.
- [.WtkCloseClassSettingsTable WtkCloseClassSettingsTable]
= close the table for usage by object instances
.el
..
- __wpInitData - Initializing the object instance data
.
= initialize all instance variables
.
  Create a value table for the object instance. This table contains pointers to
  the instance variables maintained by the WPS class.
.pl tsize=10
- [.WtkCreateObjectValueTable WtkCreateObjectValueTable]
= create the object value table
.el
..
- __wpclsQueryIconData - providing an icon for all instances of the class
.
= call [.WtkQueryIconData WtkQueryIconData] to properly report the icon data
  for an icon being read from the WPS class DLL or another resource DLL
..
- __wpclsQueryDetailsInfo - providing data for organization of details data
.
= call [.WtkQueryClassDetailsInfo WtkQueryClassDetailsInfo] to get all details
  previously registered by [.WtkAddClassSetting WtkAddClassSetting] or
  [.WtkAddClassDetail WtkAddClassDetail] automatically added to the details
  provided by the class.
..
- __wpRestoreState - restoring previously settings values
.
= get previously saved values for all settings and store them
  in the instance variables of the object by calling __wpSetup.
.
 This can be done in two different ways, corresponding to the method chosen
 within __wpSaveState (see below!):
.pl tsize=10
- [.WtkRestoreObjectState WtkRestoreObjectState]
= directly reads all settings to the internal object state in OS2.INI,
  using their settings identifier. This requires that the settings have
  been previously stored with [.WtkSaveObjectState WtkSaveObjectState].
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings]
= use this API to setup the object settings that have been previously
  queried with [.WtkQueryObjectSettings WtkQueryObjectSettings] and been
  stored at any place.
.el
..
- __wpQueryDetailsData - providing data for details view
.
= call [.WtkQueryObjectDetailsData WtkQueryObjectDetailsData] to automatically add
  the current values of the details reported earlier by
  [.WtkQueryClassDetailsInfo WtkQueryClassDetailsInfo] for the object instance

_*opening of the settings notebook / selecting a notebook page / performing undo*_
.. ----------------------------------------
.pl tsize=10
- __wpAddSettingsPages - add own setting notebook pages
.
= a WPS class adds setting notebook pages implementing
  GUI conrols for each value of the supported settings.
.
  Specify a single window message procedure for all settings notebook pages
  that a WPS class implements. Also, pass the handle to the object value table
  as a parameter to the window procedure, so that it can be stored with
  WinSetWindowULong to position QWL__USER. This handle is required within the
  window procedure to call any of the Wtk APIs for object handling.
.
.
  If desired, call [.WtkRegisterSettingsNotebook WtkRegisterSettingsNotebook]
  to register the notbook for automatic validation of all notebookpages, when the
  when the notebook is closed. Of course onle these pages can be validated, which have been
  registered with [.WtkRegisterSettingsDialog WtkRegisterSettingsDialog]
  during their creation.
..
- WM__INITDLG in window message procedure - initialize dialog
.
= prepare the dialog for automatic update by the settings manager API
  and setup all GUI controls
.pl tsize=10
- [.WtkRegisterSettingsDialog WtkRegisterSettingsDialog]
= register the dialog for automatic update of GUI controls and object instance values
- [.WtkReadObjectValueTable WtkReadObjectValueTable]
= fill all GUI controls in this dialog with the appropriate values that are stored
  in the instance variables of the object. You may also use WinSendMsg with
  WM__COMMAD for the undo button to accomplish this task, as this also calls
  *WtkReadObjectValueTable*, see below.
.el
..
- WM__COMMAND for undo pushbutton in window message procedure - restore to current values
.
= refill all GUI controls in this dialog with the appropriate values that are stored
  in the instance variables of the object.
.pl tsize=10
- [.WtkReadObjectValueTable WtkReadObjectValueTable]
= fill all GUI controls with the appropriate values that are stored
in the instance variables of the object.
.el

_*closing the settings notebook / saving the current values*_
.. ----------------------------------------
.pl tsize=10
- WM__DESTROY in window message procedure - destroy the dialog
.
= get values from all GUI controls, validate them and store them
  in the instance variables of the object.
.pl tsize=10
- [.WtkWriteObjectValueTable WtkWriteObjectValueTable]
= fill all GUI controls with the appropriate values of the value table.
.el
..
- __wpSaveState - saving current settings values
.
= save all values of the instance variables of the object.
.
 This can be done in two different ways:
.pl tsize=10
- [.WtkSaveObjectState WtkSaveObjectState]
= directly store all settings to the internal object state in OS2.INI,
  using their settings identifier. This method is very quick, and recommended,
  if your WPS application has to deal with a lot of instances of your class.
- [.WtkQueryObjectSettings WtkQueryObjectSettings]
= query all settings as one readable string that can easily be saved and further
  be restored by sending it to __wpSetup
  (see *external setup of an object*. below). This method is slower and
  recommended only if your WPS class is instanciated only once. As a benefit
  of that approach, the resulting string can be stored in readable form.

_*external setup of an object*_
.. ----------------------------------------
.pl tsize=10
- __wpSetup (__wpSetupOnce) - modifying the setttings values via settings strings
.
= evaluate all settings strings. If they apply to the WPS class and contain
  valid values, store them in the instance variables of the object.
.pl tsize=10
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings]
= validates settings values and updates the object value table.


_*deletion/making dormant of an object*_
.. ----------------------------------------
.pl tsize=10
- __wpUnInitData - Deinitializing the object instance data
.
= Free all resources being used by the object instance.
.pl tsize=10
- [.WtkDestroyObjectValueTable WtkDestroyObjectValueTable]
= frees all memory being used by the settings manager API for the object instance
.el
..
- __wpclsUnInitData - Initializing the meta class data
.
= Free all resources being used together by all object instance.
.pl tsize=10
- [.WtkDestroyClassSettingsTable WtkDestroyClassSettingsTable]
= frees all memory being used by the settings manager API for the object class.

.. ----------------------------------------
.2 Callback function for the Settings Manager API
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK
..
.. This resource id is used to link from doc comment of WtkCreateClassSettingsTable
.res 1000
..
.
You need to specify a callback procedure when creating a class settings and details table
with [.WtkCreateClassSettingsTable WtkCreateClassSettingsTable]. This function needs
to process certain callbacks in order to provide more information on settings to the
Settings Manager API, and perform tasks that cannot be covered by this API.

[=TOPICS]
.su H50

.. ----------------------------------------
.3 The callback procedure
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_PARMS
.
The callback procedure is of type [.FNCB FNCB] and thus must look like this:
.fo off
BOOL MyCallbackProc( ULONG ulAction, PVOID pvData, PVOID pvObjectInstance, PVOID pvObjectData);
.fo on

The parameters are used as follows:
.pl tsize=10
- ulAction
= defines the action of the callback as well as the type of data that pvData points to
- pvData
= for each callback action points to a different data structure, providing
  information on how the callback routine should process the callback (outgoing parameters),
  and, if required, passing results back within the structure (incoming parameters).
- pvObjectInstance
= is a SOM pointer to an instance of the WPS class. It can be used to call
  methods of the WPS class (not from the meta class!). Therefore for callbacks
  being sent from the SOM metaclass code part this parameter is NULL.
- pvObjectData
= is a SOM pointer to the data of an instance of the WPS class. It can be used to
  access data of the WPS class thatis, member variables of the class (not from
  the meta class!). Therefore for callbacks being sent from the SOM metaclass code
  part this parameter is NULL.
- return value
= The return value has to be set depending on the callback type.

The instance object methods and variables can be accessed as follows:
.fo off
// include the internal header of WPS class
##include "myclass.ih"

// get the pointer to the instance
MyClass     **somSelf = (MyClass**)pvObjectInstance;

// get the pointer to the instance data
MyClassData **somThis = (MyClassData**) pvObjectData;

// call a method
// (implicitely using somSelf)
__myClassMethod();

// access a clss member variable
// (implicitely uses somThis)
__ulMemberVar = 1;
.fo on

.. ----------------------------------------
.3 STM__CALLBACK__REPORTINIT
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_REPORTINIT
.
For the callback STM__CALLBACK__REPORTINIT pvData points to [.CBREPORTINIT PCBREPORTINIT].

The callback procedure initializes all data required by the callback. It may need to
read initialization files or other data being required to fill GUI values etc..

This callback is being invoked on [.WtkCreateClassSettingsTable WtkCreateClassSettingsTable],
which is in turn to be called on __wpclsInitData. Therfore the parameters *pvObjectInstance*
and *pvObjectData* are NULL for this callback !

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__QUERYVALUEINFO
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYVALUEINFO
.
For the callback STM__CALLBACK__QUERYVALUEINFO pvData points to [.CBQUERYVALUEINFO PCBQUERYVALUEINFO].

For each (sub)value of a setting the callback procedure must pass information to the API about
the setting and the GUI dialog and control IDs for automatic transport of the
settings (sub)value. If such information is not provided, the GUI control handling is not
automated for a value.

This callback is being invoked on [.WtkAddClassSetting WtkAddClassSetting],
which is in turn to be called on __wpclsInitData. Therfore the parameters *pvObjectInstance*
and *pvObjectData* are NULL for this callback !

Most important is the value type for the settings value. See also
.sl compact
- [.IDPNL_APISET_TYPES]
.el

*Return*
.pl
- TRUE
= let the setting be added to the class settings table with the provided information
- FALSE
= don't add the setting to the class settings table

.. ----------------------------------------
.3 STM__CALLBACK__QUERYDETAILINFO
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYDETAILINFO
.
For the callback STM__CALLBACK__QUERYDETAILINFO pvData points to [.CBQUERYDETAILINFO PCBQUERYDETAILINFO].

For each (sub)value of a setting that is used as a detail and is of the type
.ul compact
- STM__VALUETYPE__INDEX
- STM__VALUETYPE__TRUEFALSE
- STM__VALUETYPE__YESNO
- STM__VALUETYPE__ONOFF
- STM__VALUETYPE__INDEXITEM
.el
the callback procedure must pass information to the API about the maximum length of the
translated strings, being displayed in the details view.

This callback is being invoked on [.WtkAddClassSetting WtkAddClassSetting],
which is in turn to be called on __wpclsInitData. Therfore the parameters *pvObjectInstance*
and *pvObjectData* are NULL for this callback !

Most important is the value type for the settings value. See also
.sl compact
- [.IDPNL_APISET_TYPES]
.el

*Return*
.pl
- TRUE
= let the setting be added to the class settings table with the provided information
- FALSE
= don't add the setting to the class settings table

.. ----------------------------------------
.3 STM__CALLBACK__QUERYTARGETBUF
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYTARGETBUF
.
For the callback STM__CALLBACK__QUERYTARGETBUF pvData points to [.CBQUERYTARGETBUF PCBQUERYTARGETBUF].
.
For each (sub)value of a setting the callback procedure must pass the pointer to the
corresponding memory holding the value for this instance. In general this memory is provided
by an object instance variable. If the target buffer is not provided for a given value, the
corresponding setting is ignored.

This callback is being invoked on [.WtkCreateObjectValueTable WtkCreateObjectValueTable]
which is in turn to be called on __wpInitData.

*Return*
.pl
- TRUE
= let the provided memory be added to the object value table, so that it can store data
- FALSE
= skip this setting within the object value table

.. ----------------------------------------
.3 STM__CALLBACK__VALIDATE
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_VALIDATE
.
For the callback STM__CALLBACK__VALIDATE pvData points to [.CBVALIDATE PCBVALIDATE].

For each (sub)value of a setting the callback procedure may validate the provided value.
If that validation is not performed by the callback routine, the standard validation for
the type of the value takes place.

This callback is being invoked either on
.ul compact
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings], being called on __wpSetup or __wpSetupOnce  or
- [.WtkValidateObjectValueTable WtkValidateObjectValueTable], being called on close of the notebook
  (assuming that [.WtkRegisterSettingsNotebook WtkRegisterSettingsNotebook] has been called within
   the __wpAddSettingsPages method)

Return
.pl tsize=8 break=none
- TRUE
= The result of the validation is returned in *fResult* of *PCBQUERYVALIDATE*
- FALSE
= The value is to be validated with the standard validation scheme for the given
  value type. If no standard validation scheme exists, the value is always valid.
.el
.
See also
.sl compact
- [.IDPNL_APISET_TYPES]
.el

.. ----------------------------------------
.3 STM__CALLBACK__REPORTCHANGED
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_REPORTCHANGED
.
For the callback STM__CALLBACK__REPORTCHANGED pvData points to [.CBREPORTCHANGED PCBREPORTCHANGED].

For each setting the callback procedure may take an appropriate action, when a setting
has been changed. One example is to store multiple values per setting, by copying
the contents of the target buffer to the buffer for the respective value instance
(see also callback [.STM_CALLBACK_QUERYVALUE] for how to report the stored values).

This callback is being invoked for each modified setting either on
.ul compact
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings] for passed settings strings
- [.WtkWriteObjectValueTable WtkWriteObjectValueTable] for modified values in GUI controls

In addition to that, on exit of either function a last change report is generated with no name
and the id set to zero in order to allow any action required after all changes have been saved.

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__QUERYVALUE
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYVALUE
.
For the callback STM__CALLBACK__QUERYVALUE pvData points to [.CBQUERYVALUE PCBQUERYVALUE].
.
This callback is being invoked, wherever a setting is to be supported with
multiple values, and all these values have to be queried.
This callback is being invoked within
.ul compact
- [.WtkQueryObjectSettings WtkQueryObjectSettings], called on __wpSaveState
- [.WtkSaveObjectState WtkSaveObjectState], called on __wpSaveState
- [.WtkRestoreObjectState WtkRestoreObjectState], called on __wpRestoreState

This callback is being invoked n times (amount of times that this setting is
supported), where n is equal to the parameter [.ulQueryCount_WtkAddClassSetting ulQueryIndex]
of [.WtkAddClassSetting WtkAddClassSetting]. ulQueryCount is normally set to one,
and thus this callback needs not to be handled.

If a setting is to be supported for n times, the callback has to make sure that
the current value(s) of the n-t instance of the setting are copied to the
target buffer (once reported via [.IDPNL_APISET_CALLBACK_QUERYTARGETBUF STM__CALLBACK__QUERYTARGETBUF]).
The values for the instances of the setting can be hold in an array, where the items can be addressed
by the index provided by this callback.

If the values are not transferred with this this callback, the WPS object will act as
if all instances of the setting have the same value(s).

In order to have multiple values stored per setting, the
callback [.IDPNL_APISET_CALLBACK_REPORTCHANGED] has to be processed, in order
to store a value instance of the setting.

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__QUERYINDEX
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYINDEX
.
For the callback STM__CALLBACK__QUERYINDEX pvData points to [.CBQUERYINDEX PCBQUERYINDEX].

For each (sub)value of a setting that is of type STM__VALUETYPE__INDEX
or STM__VALUETYPE__INDEXITEM, a string is passed, which must be translated by the
callback routine to a numeric value.

For STM__VALUETYPE__INDEX, the string may come from either a GUI control or a settings
string, for STM__VALUETYPE__INDEXITEM the string can come only from a settings string.
The index value is stored in the target buffer reported on callback
STM__CALLBACK__QUERYTARGETBUF (mostly the object member variable).

If the string/index translation is not performed by the callback routine, value updates to the
coressponding setting are ignored and the value of the setting cannot be saved/restored
or shown in a related GUI control.

This callback is being invoked within
.ul compact
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings], called on _wpSetup or _wpSetupOnce
- [.WtkWriteObjectValueTable WtkWriteObjectValueTable], called on close of the properties notebook

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__QUERYSTRING
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYSTRING
.
For the callback STM__CALLBACK__QUERYSTRING pvData points to [.CBQUERYSTRING CBQUERYSTRING].

For each (sub)value of a setting that is of type STM__VALUETYPE__INDEX
or STM__VALUETYPE__INDEXITEM, a numeric value is passed, which must be translated by the
callback procedure to a string representing the numeric value.

For STM__VALUETYPE__INDEX, the string is used to either update a GUI control or be
added to a settings string, for STM__VALUETYPE__INDEXITEM the string is only to be used to be added
to a settings string. The index value is taken from the target buffer reported on callback
STM__CALLBACK__QUERYTARGETBUF (mostly the object member variable).

If the string/index translation is not performed by the callback routine, value updates to the
coressponding setting are ignored and the value of the setting cannot be saved/restored
or shown in a related GUI control.

This callback is being invoked within
.ul compact
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings], being called on __wpSetup or __wpSetupOnce  or
- [.WtkReadObjectValueTable WtkReadObjectValueTable], being called in the notebook page dialog window procedure
  for the messages
  - WM__INITDLG message  or
  - WM__COMMAND message for the undo pushbutton
- [.WtkEvaluateObjectSettings WtkEvaluateObjectSettings], being called on __wpSetup or __wpSetupOnce  or
- [.WtkValidateObjectValueTable WtkValidateObjectValueTable], being called on close of the notebook
  (assuming that [.WtkRegisterSettingsNotebook WtkRegisterSettingsNotebook] has been called within
   the __wpAddSettingsPages method)

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__QUERYDETAILSTRING
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_QUERYDETAILSTRING
.
For the callback STM__CALLBACK__QUERYDETAILSTRING pvData points to [.CBQUERYDETAILSTRING CBQUERYDETAILSTRING].

For each (sub)value of a setting that is used as a detail and is of the type
.ul compact
- STM__VALUETYPE__INDEX
- STM__VALUETYPE__TRUEFALSE
- STM__VALUETYPE__YESNO
- STM__VALUETYPE__ONOFF
- STM__VALUETYPE__INDEXITEM
.el
a numeric value is passed, which must be translated by the
callback procedure to a string representing the numeric value in the details information.

If the string/index translation is not performed by the callback routine, the field is
filled up qith question marks to indicate an error.

This callback is being invoked within
.ul compact
- [.WtkQueryObjectDetailsData WtkQueryObjectDetailsData]

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__INITCONTROL
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_INITCONTROL
.
For the callback STM__CALLBACK__INITCONTROL pvData points to [.CBINITCONTROL CBINITCONTROL].

This callback is invoked once for each
.ul compact
- GUI control being associated to a (sub)value of a setting. This callback
  can initialize the GUI control to your needs, like e.g. filling listboxes etc..
- dialog registered. This is called after all GUI control related callbacks
  for a dialog have been processed, in order to allow initialization of additional
  GUI controls not related to any setting or detail. When called, the usDialogId field
  is set to -1 to indicate that this is the dialog related callback.
.el

The code to initialize a given control needs not to set the value of the control
(like the text for an entryfield or the selection for a listbox). Instead it
configures the control so that the value of a setting can be set for the control
automatically (like filling a listbox or initializing a combo box).

[=NOTE]
.ul compact
- The textlimit of entryfields is automatically set to the limit of the associated
  target buffer, before this callback is invoked.
- any requested subclassing is executed after this callback has returned
.el

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__REPORTERROR
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_REPORTERROR
.
For the callback STM__CALLBACK__REPORTERROR pvData points to [.CBREPORTERROR CBREPORTERROR].

This callback is invoked once for each GUI control not containing a valid
value, when [.WtkValidateObjectValueTable WtkValidateObjectValueTable]
was called. This enables the WPS class to popup a message box.

[=NOTE]
.ul compact
- The WPS class normally does not call
  [.WtkValidateObjectValueTable WtkValidateObjectValueTable] directly.
  Instead, to let the Settings and Details Manager API automatically validate all setting
  notebook dialog pages registered with [.WtkRegisterSettingsDialog WtkRegisterSettingsDialog],
  when the settings notebookis closed. In order to ensure automatic validation, call
  [.WtkRegisterSettingsNotebook WtkRegisterSettingsNotebook]
   within the __wpAddSettingsPages method.
.el

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__REPORTSAVED
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_REPORTSAVED
.
For the callback STM__CALLBACK__REPORTSAVED pvData points to [.CBREPORTSAVED CBREPORTSAVED].

This callback is invoked, when the object setting values have
been saved with [.WtkWriteObjectValueTable WtkWriteObjectValueTable], and
allows the callback to take appropriate action (like saving a timestamp).

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.3 STM__CALLBACK__REPORTDESTROYED
.. ----------------------------------------
.an IDPNL_APISET_CALLBACK_REPORTDESTROYED
.
For the callback STM__CALLBACK__REPORTDESTROYED pvData points to [.CBREPORTDESTROYED CBREPORTDESTROYED].

This callback is invoked, when a class is being deregistered. This allows
to free all resources being held by the callback.

This callback is being invoked on [.WtkDestroyClassSettingsTable WtkDestroyClassSettingsTable],
which is in turn to be called on __wpclsUnInitData. Therfore the parameters *pvObjectInstance*
and *pvObjectData* are NULL for this callback !

*Return*
.pl
- FALSE
= Reserved value

.. ----------------------------------------
.2 Setting value types
.. ----------------------------------------
.an IDPNL_APISET_TYPES
.
The Settings and Details Manager API of the [=TITLE]
supports several types of values.

For each setting to be handled the value type must be specified in
[.CBQUERYVALUEINFO PCBQUERYVALUEINFO].ulValueType when processing the callback
[.IDPNL_APISET_CALLBACK_QUERYVALUEINFO].

The standard validation scheme can always be overridden by
processing the callback

The following value types are supported:
.table cols='23 17 8 25' rules=both frame=box
- type
= standard validation provided
= target buffer
= supported GUI controls
..------------
- STM__VALUETYPE__STRING
= No, always valid.
= CHAR[]
= <all supported> except WC__**SLIDER. WC__COMBOBOX and WC__SPINBUTTON only as listbox
..------------
- STM__VALUETYPE__INDEX
= Yes, the translation from string to index and vice versa may not fail.
= LONG
= <all supported>
..------------
- STM__VALUETYPE__INDEXITEM
= Yes, the translation from string to index and vice versa may not fail.
= LONG
= WC__LISTBOX and WC__COMBOBOX
.
.
  Can currently not be used for details !!!
..------------
- STM__VALUETYPE__LONG
= No, always valid.
= LONG
= <all supported>
..------------
- STM__VALUETYPE__TRUEFALSE
= Yes, the value must be either #TRUE# or #FALSE# (case insensitive).
= BOOL
= <all supported> except WC__**SLIDER
..------------
- STM__VALUETYPE__YESNO
= Yes, the value must be either #YES# or #NO# (case insensitive).
= BOOL
= <all supported> except WC__**SLIDER
..------------
- STM__VALUETYPE__ONOFF
= Yes, the value must be either #ON# or #OFF# (case insensitive).
= BOOL
= <all supported> except WC__**SLIDER
.el

The following value types are reserved for using with details only:
.ul compact
- STM__VALUETYPE__CDATE
- STM__VALUETYPE__CTIME

.. ----------------------------------------
.2 Handling of object details in general
.. ----------------------------------------
.an IDPNL_APISET_GEN_DETAILS
.
The general handling of object details is divided into several parts:

_*initializing the meta class*_

.pl tsize=10
- __wpclsQueryDetailsInfo - Retrieving information about provided details
.
= Here the Workplace Shell queries, what details are provided by the
  meta class. This includes type information, layout information for the details
  view and the title for the column of every detail. This method is mostly called
  during the initialization of the meta class.


_*updating the details view of a folder that contains an object*_

.pl tsize=10
- __wpQueryDetailsData - Querying the current details data
.
= Here the Workplace Shell queries the current set of data of an object instance,
  whenever it has changed.

.. ----------------------------------------
.2 Handling of object details with the Worplace Shell Toolkit
.. ----------------------------------------
.an IDPNL_APISET_API_DETAILS
.
Use the [=TITLE] API as follows to automate the
details handling:

_*initializing the meta class*_

.pl tsize=10
- __wpclsQueryDetailsInfo - Retrieving information about provided details
.
= Call [.WtkQueryClassDetailsInfo WtkQueryClassDetailsInfo]
  in order to let the Settings and Details Manager report all provided details for the given
  WPS class. Note that when __wpclsQueryDetailsInfo is being called, your WPS class already
  has done the following things:
.ul compact
- it has registered all details by either calling
  - [.WtkAddClassSetting WtkAddClassSetting] or
  - [.WtkAddClassDetail WtkAddClassDetail]
.el
.
  during the initialization of the meta class, where the order of
  of the registration determines the order, in that the details appear in the
  details view.
- it has called the [.CBQUERYVALUEINFO STM__CALLBACK__QUERYVALUEINFO] callback
  within your [.IDPNL_APISET_CALLBACK callback procedure for the Settings
  Manager API] in order to get a value for *pszDetailsTitle*, which can be used
  as the title for the column of a detail within the details view.


_*updating the details view of a folder that contains an object*_

.pl tsize=10
- __wpQueryDetailsData - Querying the current details data
.
= Call [.WtkQueryObjectDetailsData.WtkQueryObjectDetailsData]
  in order to let the Settings and Details Manager report all current detail data for
  the given WPS class.

.. ----------------------------------------
.2 Parsing settings setup strings
.. ----------------------------------------
.an IDPNL_APISET_SETUPSTRINGS
.
Although the Settings and Details Manager API provides functions
which automatically parse and evaluate settings strings, in certain
circumstances it may still be neccessary to scan setup strings for
certain settings or process all settings within a settings string.

The following APIs serve for this purpose:
.pl tsize=10
- [.WtkScanSetupString WtkScanSetupString]
= scan a setup string for a certain setting and return the value
- [.WtkExtractSetupString WtkExtractSetupString]
= works as WtkScanSetupString, but removes a found setting from
  the setup string
- [.WtkSplitSetupString WtkSplitSetupString]
= separates the next setting from a given settings string.

