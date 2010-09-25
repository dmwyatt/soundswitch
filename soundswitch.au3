#cs ----------------------------------------------------------------------------

	AutoIt Version: 3.3.0.0
	Author:         Dustin Wyatt

#ce ----------------------------------------------------------------------------
;~ AutoItSetOption("TrayIconDebug", 1)
#include <Color.au3>
#include <Array.au3>
#include <Constants.au3>


;~ --------------------------------------------------------------------------------------------------------
;~ Initialization
;~ --------------------------------------------------------------------------------------------------------
$hotkeys = IniReadSection("sound_switch.ini", "HotKeys")

If @error Then
	$err = "sound_switch.ini cannot be located in "
	MsgBox(16, "SoundSwitch Error", StringFormat("%s%s", $err, @ScriptDir))
	Terminate()
EndIf

Global $global_os = ""
GetOS()
Global $title = "Sound"
Global $text = "Playback"
Global $ctrl = "SysListView321"
Global $ContextClass = StringFormat("[CLASS:#%s]", 32768)
Global $Source1 = IniRead("sound_switch.ini", "Sound Devices", "Source1", "error")
Global $Source2 = IniRead("sound_switch.ini", "Sound Devices", "Source2", "error")
Global $Set1 = IniRead("sound_switch.ini", "Speakers", "Set1", "error")
Global $Set2 = IniRead("sound_switch.ini", "Speakers", "Set2", "error")
Global $icon_hide = Int(IniRead("sound_switch.ini", "Options", "HideIcon", 0))
Global $tray_click = Int(IniRead("sound_switch.ini", "Options", "TrayClickMode", 1))

For $key = 1 To $hotkeys[0][0]
	$msg = StringFormat("Setting %s to %s", $hotkeys[$key][1], $hotkeys[$key][0])
	$err = HotKeySet($hotkeys[$key][1], $hotkeys[$key][0])
	If $err == 0 Then
		$errmsg_fmt = "Error setting %s to %s."
		MsgBox(16, "SoundSwitch Error", StringFormat($errmsg_fmt, $hotkeys[$key][0], $hotkeys[$key][1]))
		Terminate()
	EndIf
Next

If $icon_hide Then
	Opt("TrayIconHide", $icon_hide)
Else
	Opt("TrayMenuMode",1)
EndIf

If $tray_click Then
	Opt("TrayOnEventMode",1)
	Opt("TrayIconHide", 0)
	TraySetOnEvent($TRAY_EVENT_PRIMARYDOWN, "SetDefaultSource1")
	TraySetOnEvent($TRAY_EVENT_SECONDARYDOWN, "SetDefaultSource2")
EndIf

While 1
	Sleep(10)
WEnd

#region Action Functions
Func SwitchSpeakerCount()
	OpenSound()
	$states = ItemStates()

	For $item = 0 To UBound($states) - 1
		If $states[$item][0] Then
			$device = $item
		EndIf
	Next
	ControlListView($title, $text, $ctrl, "Select", $device)
	If ControlCommand($title, $text, "Button1", "IsEnabled") Then
		ControlClick($title, $text, "Button1")
		WinWait("Speaker Setup", "Select the speaker setup below")
		$sel = ControlCommand("Speaker Setup", "Select the speaker setup below", "ListBox1", "GetCurrentSelection")
		If $sel == $Set1 Then
			$select_this = $Set2
		Else
			$select_this = $Set1
		EndIf
		ControlCommand("Speaker Setup", "Select the speaker setup below", "ListBox1", "SelectString", $select_this)
		While WinExists("Speaker Setup", "")
			ControlClick("Speaker Setup", "", "Button1")
			Sleep(50)
		WEnd
	EndIf
	CloseSound()
EndFunc   ;==>SwitchSpeakerCount

Func SwitchDefault()
	OpenSound()
	$states = ItemStates()
	$source_indexes = SourceIndexes($states)
	$curr_def = GetDefault($states)

	Select
		Case $curr_def = -1
			;If no current default then just use Source1
			SetAsDefault($source_indexes[0])

		Case ($curr_def <> $source_indexes[0]) And ($curr_def <> $source_indexes[1])
			;If current default not Source1 or Source2 we'll just use Source1
			SetAsDefault($source_indexes[0])

		Case $curr_def = $source_indexes[0]
			;If current default is Source1 make it Source2...
			SetAsDefault($source_indexes[1])

		Case $curr_def = $source_indexes[1]
			;...or vice-versa
			SetAsDefault($source_indexes[0])
	EndSelect
	CloseSound()
EndFunc   ;==>SwitchDefault

Func SwitchDevice()
	OpenSound()
	$states = ItemStates()
	$source_indexes = SourceIndexes($states)
	$curr_def = GetDefaultDevice($states)

	Select
		Case $curr_def = -1
;~ 			;If no current default device then use first Ready device
			;THIS SHOULDNT HAPPEN
			SetAsDefaultDevice(GetReady($states))

		Case ($curr_def <> $source_indexes[0]) And ($curr_def <> $source_indexes[1])
			;If current default device isn't either of our Sources, just use Source1
			SetAsDefaultDevice($source_indexes[0])

		Case $curr_def = $source_indexes[0]
			;If current default is Source1 make it Source2...
			SetAsDefaultDevice($source_indexes[1])

		Case $curr_def = $source_indexes[1]
			;...or vice-versa
			SetAsDefaultDevice($source_indexes[0])
	EndSelect
	CloseSound()
EndFunc   ;==>SwitchDevice

Func SwitchComm()
	OpenSound()
	$states = ItemStates()
	$source_indexes = SourceIndexes($states)
	$curr_def = GetDefaultCommDevice($states)
;~ 	MsgBox(0, "curr def", $curr_def)
;~ 	_ArrayDisplay($source_indexes)
;~ 	_ArrayDisplay($states)

	Select
		Case $curr_def = -1
;~ 			;If no current default device then use first Ready device
			;THIS SHOULDNT HAPPEN
			SetAsDefaultComm(GetReady($states))

		Case ($curr_def <> $source_indexes[0]) And ($curr_def <> $source_indexes[1])
			;If current default device isn't either of our Sources, just use Source1
			SetAsDefaultComm($source_indexes[0])

		Case $curr_def = $source_indexes[0]
			;If current default is Source1 make it Source2...
			SetAsDefaultComm($source_indexes[1])

		Case $curr_def = $source_indexes[1]
			;...or vice-versa
			SetAsDefaultComm($source_indexes[0])
	EndSelect

	CloseSound()
EndFunc   ;==>SwitchComm

Func ScrollDefault()
	OpenSound()
	$states = ItemStates()
	;Find current default
	For $i = 0 To UBound($states) - 1
		If $states[$i][4] = "Default Device" Then
			$curr_def = $i
			ExitLoop
		EndIf
	Next

	$next = $curr_def
	For $i = 0 To UBound($states) - 1
		$next = Scroll($next, UBound($states) - 1)
		If IsReady($next, $states) Then
			SetAsDefault($next)
			Return
		EndIf
	Next

	CloseSound()
EndFunc   ;==>ScrollDefault

Func ScrollDevice()
	OpenSound()
	$states = ItemStates()
	;Find current default
	For $i = 0 To UBound($states) - 1
		If $states[$i][4] = "Default Device" Then
			$curr_def = $i
			ExitLoop
		EndIf
	Next

	$next = $curr_def
	For $i = 0 To UBound($states) - 1
		$next = Scroll($next, UBound($states) - 1)
		If IsReady($next, $states) Then
			SetAsDefaultDevice($next)
			Return
		EndIf
	Next

	CloseSound()
EndFunc   ;==>ScrollDevice

Func ScrollComm()
	OpenSound()
	$states = ItemStates()
	;Find current default
	$curr_def = False
	For $i = 0 To UBound($states) - 1
		If $states[$i][4] = "Default Communications Device" Then
			$curr_def = $i
			ExitLoop
		EndIf
	Next
	If Not $curr_def Then
		For $i = 0 To UBound($states) - 1
			If $states[$i][4] = "Default Device" Then
				$curr_def = $i
				ExitLoop
			EndIf
		Next
	EndIf

	$next = $curr_def
	For $i = 0 To UBound($states) - 1
		$next = Scroll($next, UBound($states) - 1)
		If IsReady($next, $states) Then
			SetAsDefaultComm($next)
			Return
		EndIf
	Next

	CloseSound()
EndFunc   ;==>ScrollComm

Func OpenSound()
	Run("control.exe /name Microsoft.AudioDevicesAndSoundThemes")
	WinWait($title, $text)
	WinMove($title, $text, -500, -500)
EndFunc   ;==>OpenSound

Func CloseSound()
	If WinExists($title, $text) Then
		ControlSend($title, $text, "", "{ESC}")
	EndIf
EndFunc   ;==>CloseSound

Func SetAsDefault($item)
	If IsReady($item) Then
		ControlListView($title, $text, $ctrl, "Select", $item)
		ControlClick($title, $text, "Button2", "primary")
		$states = ItemStates()
		TraySetToolTip($states[$item][5])
	Else
		MsgBox(0, "Soundswitch", "Device not in 'Ready' state")
	EndIf
EndFunc   ;==>SetAsDefault

Func SetAsDefaultComm($item)
	If IsReady($item) Then
		ControlListView($title, $text, $ctrl, "Select", $item)
		If GetOS() = "7" Then
			ControlSend($title, $text, "Button2", "{DOWN}c")
;~ 			ControlClick($title, $text, $ctrl, "secondary")
;~ 			ControlSend($title, $text, $ctrl, "c")
		ElseIf GetOS() = "Vista" Then
			SetAsDefault($item)
		EndIf
	Else
		MsgBox(0, "Soundswitch", "Device not in 'Ready' state")
	EndIf
EndFunc   ;==>SetAsDefaultComm

Func SetAsDefaultDevice($item)
	If IsReady($item) Then
		ControlListView($title, $text, $ctrl, "Select", $item)
		If GetOS() = "7" Then
			ControlSend($title, $text, "Button2", "{DOWN}d")
		ElseIf GetOS() = "Vista" Then
			SetAsDefault($item)
		EndIf
	Else
		MsgBox(0, "Soundswitch", "Device not in 'Ready' state")
	EndIf
EndFunc   ;==>SetAsDefaultDevice
#endregion Action Functions

#region Info functions
Func Scroll($curr, $limit)
	$curr += 1
	if $curr > $limit Then
		$curr = 0
	EndIf
	Return $curr
EndFunc

Func GetReady($items)
;~ Pick first device with Ready status from $items
	For $i = 0 To UBound($items) - 1
		If $items[$i][4] = "Ready" Then Return $i
	Next
	Return -1
EndFunc   ;==>GetReady

Func GetOS()
	If $global_os Then Return $global_os

	$a = RegRead("HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion", "ProductName")
	$OS = "XP"

	If StringInStr($a, "Windows 7") Then
		$OS = "7"
	ElseIf StringInStr($a, "Windows Vista") Then
		$OS = "Vista"
	EndIf
	$global_os = $OS
	Return $OS
EndFunc   ;==>GetOS

Func IsReady($item, $states = False)
	If Not $states Then $states = ItemStates()
	If $states[$item][4] = "Ready" Then Return True
	If StringInStr($states[$item][4], "Default") Then Return True
	Return False
EndFunc   ;==>IsReady

Func ItemStates()
	If Not WinExists($title, $text) Then
		OpenSound()
		$close_Sound = True
	Else
		$close_Sound = False
	EndIf
	$item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	Dim $item_states[$item_count][7]

	$found_comm = False
	For $i = 0 To $item_count - 1
		$device_type = ControlListView($title, $text, $ctrl, "GetText", $i, 0)
		$device_name = ControlListView($title, $text, $ctrl, "GetText", $i, 1)
		$device_status = ControlListView($title, $text, $ctrl, "GetText", $i, 2)
		$device_matcher = $device_type & " " & $device_name

		$item_states[$i][2] = $device_type
		$item_states[$i][3] = $device_name
		$item_states[$i][4] = $device_status
		$item_states[$i][5] = $device_matcher

		If StringInStr($device_matcher, $Source1) Then
			$item_states[$i][6] = "Source1"
		EndIf

		If StringInStr($device_matcher, $Source2) Then
			$item_states[$i][6] = "Source2"
		EndIf

		If $device_status = "Default Device" Then
			$item_states[$i][0] = True
			$found_def = $i
		Else
			$item_states[$i][0] = False
		EndIf

		If $device_status = "Default Communications Device" Then
			$item_states[$i][1] = True
			$found_comm = True
		Else
			$item_states[$i][1] = False
		EndIf
	Next

	If Not $found_comm Then
		$item_states[$found_def][1] = True
	EndIf
	If $close_Sound Then
		CloseSound()
	EndIf
;~ 	_ArrayDisplay($item_states)
	Return $item_states

EndFunc   ;==>ItemStates

Func SourceIndexes($items)
	Dim $indexes[2]
	For $i = 0 To UBound($items) - 1
		If $items[$i][6] = "Source1" Then
			$indexes[0] = $i
		ElseIf $items[$i][6] = "Source2" Then
			$indexes[1] = $i
		EndIf
	Next
	Return $indexes
EndFunc   ;==>SourceIndexes

Func GetDefault($items)
	For $i = 0 To UBound($items) - 1
		If $items[$i][0] And $items[$i][1] Then Return $i
	Next
	Return -1
EndFunc   ;==>GetDefault

Func GetDefaultDevice($items)
	For $i = 0 To UBound($items) - 1
		If $items[$i][0] Then Return $i
	Next
	Return -1
EndFunc   ;==>GetDefaultDevice

Func GetDefaultCommDevice($items)
	For $i = 0 To UBound($items) - 1
		If $items[$i][1] Then Return $i
	Next
	Return -1
EndFunc   ;==>GetDefaultCommDevice

Func SetDefaultSource1()
	OpenSound()
	$states = ItemStates()
	$source_indexes = SourceIndexes($states)
	SetAsDefault($source_indexes[0])
	CloseSound()
EndFunc

Func SetDefaultSource2()
	OpenSound()
	$states = ItemStates()
	$source_indexes = SourceIndexes($states)
	SetAsDefault($source_indexes[1])
	CloseSound()
EndFunc
#endregion Info functions

#region Helper functions
Func out($msg)
	ConsoleWrite(String($msg))
	ConsoleWrite(@LF)
EndFunc   ;==>out

Func Terminate()
	Exit
EndFunc   ;==>Terminate
#endregion Helper functions