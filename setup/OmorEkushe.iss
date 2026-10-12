; =============================================================================
; Omor Ekushe - Modern Inno Setup Script
; Generates standalone setup installer for Omor Ekushe with seamless update
; support, application auto-close, start menu & desktop shortcuts, and
; automatic uninstallation.
; =============================================================================

#ifndef MyAppVersion
#define MyAppVersion "1.1.0"
#endif

#define MyAppName "Omor Ekushe"
#define MyAppPublisher "Sabbir Legacy"
#define MyAppURL "https://github.com/sabbir28/OmorEkushe"
#define MyAppUpdatesURL "https://github.com/sabbir28/OmorEkushe/releases"
#define MyAppExeName "OmorEkushe.exe"

[Setup]
AppId={{8B8A62FE-B26C-4A2E-9FA1-4E67E7E36C2F}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} v{#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppUpdatesURL}
DefaultDirName={autopf}\Omor Ekushe
DefaultGroupName=Omor Ekushe
AllowNoIcons=yes
OutputDir=..\release_stage
OutputBaseFilename=OmorEkushe-Setup
SetupIconFile=..\build\data\Icons\Phonetic.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog commandline
CloseApplications=yes
CloseApplicationsFilter=*OmorEkushe*.exe
RestartApplications=yes
UninstallDisplayIcon={app}\bin\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Start Omor Ekushe automatically when Windows starts"; GroupDescription: "System Integration:"; Flags: unchecked

[Files]
; Core Binaries
Source: "..\build\bin\OmorEkushe.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\build\bin\LayoutEditor.exe"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\build\bin\SpellingChecker.exe"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\build\bin\*.dll"; DestDir: "{app}\bin"; Flags: ignoreversion skipifsourcedoesntexist

; Data Directory (Layouts, Icons, Assets)
Source: "..\build\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; IconFilename: "{app}\bin\{#MyAppExeName}"
Name: "{group}\Layout Editor"; Filename: "{app}\bin\LayoutEditor.exe"; IconFilename: "{app}\bin\LayoutEditor.exe"; Flags: createonlyiffileexists
Name: "{group}\Spelling Checker"; Filename: "{app}\bin\SpellingChecker.exe"; IconFilename: "{app}\bin\SpellingChecker.exe"; Flags: createonlyiffileexists
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\bin\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Auto-start with Windows (if selected during setup)
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "OmorEkushe"; ValueData: """{app}\bin\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\bin\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

