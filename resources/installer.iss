; Softvim Browser — Inno Setup Script
; Creates a lightweight, modern Windows installer for Softvim Browser.

#define MyAppName "Softvim Browser"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Softvim Solutions"
#define MyAppURL "https://softvim.com"
#define MyAppExeName "SoftvimBrowser.exe"

[Setup]
AppId={{9B7F1B3E-4B02-4D9A-B0F7-2D98D5E5F611}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\Softvim Solutions\Softvim Browser
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
OutputDir=..\dist\Installer
OutputBaseFilename=SoftvimBrowser_Setup_x64
SetupIconFile=softvim.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\dist\SoftvimBrowser\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

