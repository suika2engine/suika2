!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "Suika2 Pro Desktop"
OutFile "suika2-installer.exe"
InstallDir "$APPDATA\Local\Suika2 Pro Desktop"

RequestExecutionLevel user
SetCompressor /SOLID /FINAL lzma

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section "Install"
  Call UninstallPrevious

  SetOutPath "$INSTDIR"
  File "suika-pro.exe"
  File /r "games"
  File /r "tools"
  File "plaintext.code-snippets.en"
  File "plaintext.code-snippets.jp"
  File "icon.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Suika2 Pro Desktop"
  CreateShortcut "$SMPROGRAMS\Suika2 Pro Desktop\Suika2 Pro Desktop.lnk" "$INSTDIR\suika-pro.exe" ""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "InstDir" '"$INSTDIR"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "DisplayName" "Suika2 Pro Desktop"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "DisplayIcon" '"$INSTDIR\icon.ico"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "DisplayVersion" "15"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "Publisher" "Keiichi Tabata"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "Contact" "tabata@luxion.jp"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  SetShellVarContext current
  CreateShortCut "$DESKTOP\Suika2 Pro Desktop.lnk" "$INSTDIR\suika-pro.exe"
SectionEnd

Function UninstallPrevious
  ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "UninstallString"
  ${If} $R0 == ""
    DetailPrint "No previous installation."
    Goto Done
  ${EndIf}
  DetailPrint "Removing previous installation."
  ExecWait "$R0 /S"
  Done:
FunctionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\suika-pro.exe"
  Delete "$INSTDIR\games"
  Delete "$INSTDIR\tools"
  Delete "$INSTDIR\plaintext.code-snippets.en"
  Delete "$INSTDIR\plaintext.code-snippets.jp"
  RMDir /r "$INSTDIR"
  Delete "$SMPROGRAMS\Suika2 Pro Desktop\Suika2 Pro Desktop.lnk"
  RMDir "$SMPROGRAMS\Suika2 Pro Desktop"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop"
  Delete "$DESKTOP\Suika2 Pro Desktop.lnk"
SectionEnd

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"
