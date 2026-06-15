; Inno Setup script for Vial Effects
; --------------------------------
; Build with:  iscc installer\VialEffects.iss
;   (or `iscc /DConfiguration=Release installer\VialEffects.iss` for Release)
;
; Requires Inno Setup 6+: https://jrsoftware.org/isinfo.php

#ifndef Configuration
  #define Configuration "Debug"
#endif

#define AppName        "Vial Effects"
#define AppPublisher   "buchenberg"
#define AppURL         "https://github.com/buchenberg/vial-effects"
#define AppVersion     GetVersionNumbersString("..\build\VialEffects_artefacts\" + Configuration + "\Standalone\Vial Effects.exe")

[Setup]
AppId={{ACF81B98-D123-48BD-BA00-E83CEF31EB4B}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\buchenberg\{#AppName}
DefaultGroupName={#AppPublisher}\{#AppName}
DisableProgramGroupPage=auto
OutputDir=..\build\installer
OutputBaseFilename=VialEffects-{#AppVersion}-win64
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\Vial Effects.exe
LicenseFile=..\LICENSE

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full";    Description: "Full installation (VST3 + Standalone)"
Name: "vst3";    Description: "VST3 plugin only"
Name: "standalone"; Description: "Standalone application only"
Name: "custom";  Description: "Custom"; Flags: iscustom

[Components]
Name: "vst3";       Description: "VST3 Plugin (64-bit)"; Types: full vst3 custom; Flags: disablenouninstallwarning
Name: "standalone"; Description: "Standalone Application"; Types: full standalone custom

[Files]
; --- VST3 Plugin (folder bundle) ---
Source: "..\build\VialEffects_artefacts\{#Configuration}\VST3\Vial Effects.vst3\*"; \
    DestDir: "{commoncf64}\VST3\Vial Effects.vst3"; \
    Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs

; --- Standalone executable ---
Source: "..\build\VialEffects_artefacts\{#Configuration}\Standalone\Vial Effects.exe"; \
    DestDir: "{app}"; Components: standalone; Flags: ignoreversion

; --- License ---
Source: "..\LICENSE"; DestDir: "{app}"; Components: standalone; \
    Flags: ignoreversion; DestName: "LICENSE.txt"

[Icons]
Name: "{group}\Vial Effects"; Filename: "{app}\Vial Effects.exe"; \
    Components: standalone
Name: "{group}\Uninstall Vial Effects"; Filename: "{uninstallexe}"; \
    Components: standalone
Name: "{group}\License"; Filename: "{app}\LICENSE.txt"; \
    Components: standalone

[Run]
Filename: "{app}\Vial Effects.exe"; Description: "Launch {#AppName}"; \
    Flags: nowait postinstall skipifsilent; Components: standalone
