!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "Suika2 Pro Desktop"
OutFile "suika2-installer.exe"
InstallDir "$APPDATA\Local\Suika2 Pro Desktop"

SetCompressor /SOLID /FINAL lzma

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section ""
  SetOutPath "$INSTDIR"
  File "suika-pro.exe"
  File /r "games"
  File /r "tools"
  File "plaintext.code-snippets.en"
  File "plaintext.code-snippets.jp"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\Suika2 Pro Desktop"
  SetOutPath "$INSTDIR"
  CreateShortcut "$SMPROGRAMS\Suika2 Pro Desktop\Suika2 Pro Desktop.lnk" "$INSTDIR\suika-pro.exe" ""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "DisplayName" "Suika2 Pro Desktop"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Desktop Shortcut" SectionX
  SetShellVarContext current
  CreateShortCut "$DESKTOP\Suika2 Pro Desktop.lnk" "$INSTDIR\suika-pro.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\suika-pro.exe"
  Delete "$INSTDIR\games"
  Delete "$INSTDIR\tools"
  RMDir /r "$INSTDIR"
  Delete "$SMPROGRAMS\Suika2 Pro Desktop\Suika2.lnk"
  RMDir "$SMPROGRAMS\Suika2 Pro Desktop"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Suika2 Pro Desktop"
SectionEnd

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"
