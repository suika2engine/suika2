!define GAME_NAME "REPLACE_GAME_NAME_JAPANESE"
!define GAME_NAME_ASCII "REPLACE_GAME_NAME_ASCII"

!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "${GAME_NAME}"
OutFile "${GAME_NAME_ASCII}-installer.exe"
InstallDir "$APPDATA\Local\${GAME_NAME}"

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section ""
  SetOutPath "$INSTDIR"
  File "suika.exe"
  File "data01.arc"
  File /nonfatal /r "mov"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\${GAME_NAME}"
  SetOutPath "$INSTDIR"
  CreateShortcut "$SMPROGRAMS\${GAME_NAME}\${GAME_NAME}.lnk" "$INSTDIR\suika.exe" ""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME_ASCII}" "DisplayName" "${GAME_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME_ASCII}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
SectionEnd

Section "Desktop Shortcut" SectionX
  SetShellVarContext current
  CreateShortCut "$DESKTOP\${GAME_NAME}.lnk" "$INSTDIR\suika.exe"
SectionEnd

!define MUI_FINISHPAGE_RUN "$INSTDIR\suika.exe"
!insertmacro MUI_PAGE_FINISH

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\suika.exe"
  RMDir /r "$INSTDIR"
  Delete "$SMPROGRAMS\${GAME_NAME}\${GAME_NAME}.lnk"
  RMDir "$SMPROGRAMS\${GAME_NAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GAME_NAME_ASCII}"
SectionEnd

!insertmacro MUI_LANGUAGE "Japanese"
