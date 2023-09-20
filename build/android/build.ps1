$wsobj = new-object -comobject wscript.shell

if (!(Test-Path 'openjdk-17.zip')) {
  $result = $wsobj.popup('OpenJDK 17をダウンロードします。', 0, 'Suika2 Pro', 1)
  if ($result -eq 1) {
    (New-Object System.Net.WebClient).DownloadFile('https://aka.ms/download-jdk/microsoft-jdk-17.0.8.1-windows-x64.zip', (Convert-Path '.') + '\openjdk-17.zip')
  } else {
    exit
  }
}
if (!(Test-Path 'jdk-17.0.8.1+1')) {
  $wsobj.popup('OpenJDK 17のZIPを展開します。', 0, 'Suika2 Pro', 0)
  Expand-Archive -Path 'openjdk-17.zip' -DestinationPath "."
}
$env:JAVA_HOME = (Convert-Path '.\jdk-17.0.8.1+1')

if (!(Test-Path 'cmdline-tools.zip')) {
  $result = $wsobj.popup('Android Commandline Toolsをダウンロードします。', 0, 'Suika2 Pro', 1)
  if ($result -eq 1) {
    (New-Object System.Net.WebClient).DownloadFile('https://dl.google.com/android/repository/commandlinetools-win-10406996_latest.zip', (Convert-Path '.') + '\cmdline-tools.zip')
  } else {
    exit
  }
}
if ((Test-Path 'cmdline-tools')) {
  Remove-item -Force -Recurse 'cmdline-tools'
}
$wsobj.popup('Android Commandline ToolsのZIPを展開します。', 0, 'Suika2 Pro', 0)
Expand-Archive -Path 'cmdline-tools.zip' -DestinationPath "."
$env:ANDROID_HOME = ''
$env:ANDROID_SDK_ROOT = (Convert-Path '.\cmdline-tools')

$PWD = (Convert-Path '.')
Write-Output 'y' | ./cmdline-tools/bin/sdkmanager 'cmdline-tools;latest' --sdk_root=$PWD
$env:ANDROID_SDK_ROOT = (Convert-Path '.')

Write-Output y,y,y,y,y,y,y,y,y,y,y,y,y | ./cmdline-tools/bin/sdkmanager --licenses --sdk_root="$env:ANDROID_SDK_ROOT"
Write-Output y,y,y,y,y,y,y,y,y,y,y,y,y | ./cmdline-tools/bin/sdkmanager --install "ndk;25.2.9519653" --sdk_root="$env:ANDROID_SDK_ROOT"

./gradlew.bat build

Copy-Item '.\app\build\outputs\apk\debug\app-debug.apk' '.\suika.apk'
