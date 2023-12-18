!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "Suika2"
OutFile "suika2-installer.exe"
InstallDir "$APPDATA\Local\Suika2"

SetCompressor /SOLID /FINAL lzma

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section ""
  SetOutPath "$INSTDIR"
  File "suika-pro.exe"
  File /r "games"
  File /r "tools"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Suika2"
  SetOutPath "$INSTDIR"
  CreateShortcut "$SMPROGRAMS\Suika2\Suika2.lnk" "$INSTDIR\suika-pro.exe" ""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2" "DisplayName" "Suika2"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Desktop Shortcut" SectionX
  SetShellVarContext current
  CreateShortCut "$DESKTOP\Suika2.lnk" "$INSTDIR\suika-pro.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\suika-pro.exe"
  Delete "$INSTDIR\games"
  Delete "$INSTDIR\tools"
  RMDir /r "$INSTDIR"
  Delete "$SMPROGRAMS\Suika2\Suika2.lnk"
  RMDir "$SMPROGRAMS\Suika2"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2"
SectionEnd

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"
