@echo off

if not exist nsis-3.08.zip (
  echo 'NSISをダウンロードしています。'
  powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://suika2.com/dl/nsis-3.08.zip', (Convert-Path '.') + '\nsis-3.08.zip')"
)

if not exist nsis-3.08 (
  echo 'NSISを展開しています。'
  powershell -Command "Expand-Archive -Path nsis-3.08.zip -DestinationPath ."
)

powershell -Command "[void][System.Reflection.Assembly]::Load('Microsoft.VisualBasic, Version=8.0.0.0, Culture=Neutral, PublicKeyToken=b03f5f7f11d50a3a');[Microsoft.VisualBasic.Interaction]::InputBox('ゲームの日本語タイトルを入力してください', '日本語タイトル') | Out-File GAME.TXT -Encoding utf8"
powershell -Command "[void][System.Reflection.Assembly]::Load('Microsoft.VisualBasic, Version=8.0.0.0, Culture=Neutral, PublicKeyToken=b03f5f7f11d50a3a');[Microsoft.VisualBasic.Interaction]::InputBox('ゲームの半角アルファベットのみの名前を入力してください', 'ゲーム識別名(半角アルファベット)') | Out-File GAME_ASCII.TXT -Encoding utf8"

powershell -Command "$MyFile = (Get-Content 'GAME.TXT');[System.IO.File]::WriteAllLines('GAME.TXT', $MyFile, (New-Object System.Text.UTF8Encoding($False)))"
powershell -Command "$MyFile = (Get-Content 'GAME_ASCII.TXT');[System.IO.File]::WriteAllLines('GAME_ASCII.TXT', $MyFile, (New-Object System.Text.UTF8Encoding($False)))"

chcp 65001

SET /P GAME=<GAME.TXT
SET /P GAME_ASCII=<GAME_ASCII.TXT

powershell "(Get-Content 'install-script.nsi').Replace('REPLACE_GAME_NAME_JAPANESE', '%GAME%') | Out-File install-script.nsi"
powershell "(Get-Content 'install-script.nsi').Replace('REPLACE_GAME_NAME_ASCII', '%GAME_ASCII%') | Out-File install-script.nsi"

chcp 932

nsis-3.08\makensis.exe /INPUTCHARSET UTF8 install-script.nsi
copy *-installer.exe ..\

cd ..
start .
exit
