#cs ----------------------------------------------------------------------------

	AutoIt Version: 3.3.0.0
	Author:         Dustin Wyatt

#ce ----------------------------------------------------------------------------

#include <Color.au3>
#include <Array.au3>

;~ --------------------------------------------------------------------------------------------------------
;~ Initialization
;~ --------------------------------------------------------------------------------------------------------
$hotkeys = IniReadSection("sound_switch.ini", "HotKeys")

If @error Then
	$err = "sound_switch.ini cannot be located in "
	MsgBox(16, "SoundSwitch Error", StringFormat("%s%s", $err, @ScriptDir))
	Terminate()
EndIf

Global $global_soundstate[1][1]
Global $global_os = ""
GetOS()
Global $title = "Sound"
Global $text = "Playback"
Global $ctrl = "SysListView321"
Global $ContextClass = StringFormat("[CLASS:#%s]", 32768)
Global $Source1 = Int(IniRead("sound_switch.ini", "Sound Devices", "Source1", "error")) - 1
Global $Source2 = Int(IniRead("sound_switch.ini", "Sound Devices", "Source2", "error")) - 1
Global $Set1 = IniRead("sound_switch.ini", "Speakers", "Set1", "error")
Global $Set2 = IniRead("sound_switch.ini", "Speakers", "Set2", "error")
Global $icon_hide = Int(IniRead("sound_switch.ini", "Options", "HideIcon", 0))


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
EndIf

While 1
	Sleep(10)
WEnd

#Region Action Functions
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
	Switcher($states, 0)
	CloseSound()
EndFunc   ;==>SwitchDefault

Func SwitchDevice()
	OpenSound()
	$states = ItemStates()
	Switcher($states, 2)
	CloseSound()
EndFunc   ;==>SwitchDevice

Func SwitchComm()
	OpenSound()
	$states = ItemStates()
	Switcher($states, 1)
	CloseSound()
EndFunc   ;==>SwitchComm

Func ScrollDefault()
	OpenSound()
	$states = ItemStates()
	Scroller($states, 0)
	CloseSound()
EndFunc   ;==>ScrollDefault

Func ScrollDevice()
	OpenSound()
	$states = ItemStates()
	Scroller($states, 2)
	CloseSound()
EndFunc   ;==>ScrollDevice

Func ScrollComm()
	OpenSound()
	$states = ItemStates()
	Scroller($states, 1)
	CloseSound()
EndFunc   ;==>ScrollComm

Func OpenSound()
	Run("control.exe /name Microsoft.AudioDevicesAndSoundThemes")
	WinWait($title, $text)
EndFunc   ;==>OpenSound

Func CloseSound()
	If WinExists($title, $text) Then
		ControlSend($title, $text, "", "{ESC}")
	EndIf
EndFunc   ;==>CloseSound

Func Scroller($states, $scrolling)
	;$scrolling:	0 - Default
	;				1 - CommDevice
	;				2 - Device
	$curr = "notfound"
	Select
		Case $scrolling == 0
			For $item = 0 To UBound($states) - 1
				If $states[$item][0] And $states[$item][1] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
			If $curr == "notfound" Then $curr = 0
		Case $scrolling == 1
			For $item = 0 To UBound($states) - 1
				If $states[$item][1] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
		Case $scrolling == 2
			For $item = 0 To UBound($states) - 1
				If $states[$item][0] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
		Case Else
			Return -1
	EndSelect

	If $curr == "notfound" Then Return -1

	If $curr == UBound($states) - 1 Then
		$next = 0
	Else
		$next = $curr + 1
	EndIf

	Select
		Case $scrolling == 0
			SetAsDefault($next)
		Case $scrolling == 1
			SetAsDefaultComm($next)
		Case $scrolling == 2
			SetAsDefaultDevice($next)
		Case Else
			Return -1
	EndSelect
	Return 0
EndFunc   ;==>Scroller

Func Switcher($states, $switching)
	;$scrolling:	0 - Default
	;				1 - CommDevice
	;				2 - Device
	$curr = "notfound"
	Select
		Case $switching == 0
			For $item = 0 To UBound($states) - 1
				If $states[$item][0] And $states[$item][1] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
			If $curr == "notfound" Then $curr = 0
		Case $switching == 1
			For $item = 0 To UBound($states) - 1
				If $states[$item][1] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
		Case $switching == 2
			For $item = 0 To UBound($states) - 1
				If $states[$item][0] Then
					$curr = $item
					ExitLoop
				EndIf
			Next
		Case Else
			Return -1
	EndSelect

	If $curr == "notfound" Then Return -1

	Select
		Case $switching == 0
			If $curr == $Source1 Then
				SetAsDefault($Source2)
			ElseIf $curr == $Source2 Then
				SetAsDefault($Source1)
			Else
				SetAsDefault($Source1)
			EndIf
		Case $switching == 1
			If $curr == $Source1 Then
				SetAsDefaultComm($Source2)
			ElseIf $curr == $Source2 Then
				SetAsDefaultComm($Source1)
			Else
				SetAsDefaultComm($Source1)
			EndIf
		Case $switching == 2
			If $curr == $Source1 Then
				SetAsDefaultDevice($Source2)
			ElseIf $curr == $Source2 Then
				SetAsDefaultDevice($Source1)
			Else
				SetAsDefaultDevice($Source1)
			EndIf
	EndSelect

	Return 0

EndFunc   ;==>Switcher

Func SetAsDefault($item)
	ControlListView($title, $text, $ctrl, "Select", $item)
	ControlClick($title, $text, "Button2", "primary")
EndFunc   ;==>SetAsDefault

Func SetAsDefaultComm($item)
	ControlListView($title, $text, $ctrl, "Select", $item)
	If GetOS() = "7" Then
		ControlClick($title, $text, "Button2", "primary", 1, 89, 22)
		$hWND = WinGetHandle($ContextClass)
		ControlSend($hWND, "", "", "c")
	ElseIf GetOS() = "Vista" Then
		SetAsDefault($item)
	EndIf
	Return
EndFunc   ;==>SetAsDefaultComm

Func SetAsDefaultDevice($item)
	ControlListView($title, $text, $ctrl, "Select", $item)
	If GetOS() = "7" Then
		ControlClick($title, $text, "Button2", "primary", 1, 89, 22)
		$hWND = WinGetHandle($ContextClass)
		ControlSend($hWND, "", "", "d")
	ElseIf GetOS() = "Vista" Then
		SetAsDefault($item)
	EndIf
EndFunc   ;==>SetAsDefaultDevice

Func ToggleDiscMenuItem()
	ControlClick($title, $text, $ctrl, "secondary", 1, 1, 1)
	$hWND = WinGetHandle($ContextClass)
	ControlSend($hWND, "", "", "{DOWN 2}{ENTER}")
EndFunc   ;==>ToggleDiscMenuItem

Func ToggleDisabledMenuItem()
	ControlClick($title, $text, $ctrl, "secondary", 1, 1, 1)
	$hWND = WinGetHandle($ContextClass)
	ControlSend($hWND, "", "", "{DOWN}{ENTER}")
EndFunc   ;==>ToggleDisabledMenuItem

#EndRegion Action Functions

#Region Info functions
Func GetOS()
	If $global_os Then Return $global_os

	$a = Regread("HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion","ProductName")
	$OS = "XP"

	If StringInStr($a, "Windows 7") Then
		$OS = "7"
	ElseIf StringInStr($a, "Windows Vista") Then
		$OS = "Vista"
	EndIf
	$global_os = $OS
	Return $OS
EndFunc

Func ItemStates()
	If Not WinExists($title, $text) Then
		OpenSound()
		$close_Sound = True
	Else
		$close_Sound = False
	EndIf
	$item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	Dim $item_states[$item_count][6]

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
	_ArrayDisplay($item_states)
	$global_soundstate = $item_states

EndFunc   ;==>ItemStates


Func ShowingDisconnected()
	; Returns -1 if there are no disconnected devices in system
	$orig_item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	ToggleDiscMenuItem()
	$curr_item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	If $orig_item_count > $curr_item_count Then
		$state = True
	ElseIf $orig_item_count < $curr_item_count Then
		$state = False
	Else
		$state = -1
	EndIf
	ToggleDiscMenuItem()
	Return $state
EndFunc   ;==>ShowingDisconnected

Func ShowingDisabled()
	; Returns -1 if there are no disabled devices in system
	$orig_item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	ToggleDisabledMenuItem()
	$curr_item_count = ControlListView($title, $text, $ctrl, "GetItemCount")
	If $orig_item_count > $curr_item_count Then
		$state = True
	ElseIf $orig_item_count < $curr_item_count Then
		$state = False
	Else
		$state = -1
	EndIf
	ToggleDisabledMenuItem()
	Return $state
EndFunc   ;==>ShowingDisabled
#EndRegion Info functions

#Region Helper functions
Func out($msg)
	ConsoleWrite(String($msg))
	ConsoleWrite(@LF)
EndFunc   ;==>out

Func Terminate()
	Exit
EndFunc   ;==>Terminate
#EndRegion Helper functions
