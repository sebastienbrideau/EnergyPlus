; DO NOT EDIT THIS FILE DIRECTLY.
; Changes may be lost if the file is updated from the Git repository.
; Make a copy of the file and name it 'config.iss'.
; Then, insert the information relevant to your addin.

#define VERSION "${COOLPROP_VERSION_MAJOR}.${COOLPROP_VERSION_MINOR}"                ; The version number
#define LONGVERSION "${COOLPROP_VERSION_MAJOR}.${COOLPROP_VERSION_MINOR}.${COOLPROP_VERSION_PATCH}.0" ; The version in four-number format
#define YEARSPAN "${COOLPROP_YEAR}"         ; The year(s) of publication
                                     ; (e.g., 2014-2015)
#define PRODUCT "CoolProp for Windows"
#define COMPANY "${COOLPROP_PUBLISHER}"

#define SOURCEDIR "source"           ; The folder with the addin files
                                     ; (relative path)

#define DEFINSDIR "{userappdata}\CoolProp"
#define DLLINSDIR "{userappdata}\CoolProp"
#define EXAMPLDIR "{userdesktop}"
#define EESINSDIR "C:\EES32\Userlib\COOLPROP_EES"

#define LOGFILE "INST-LOG.TXT"       ; The name of the log file. 

AppPublisherURL=http://www.coolprop.org
AppSupportURL=https://github.com/CoolProp/CoolProp/issues
AppUpdatesURL=http://www.coolprop.org
OutputBaseFilename=CoolProp_v{#LONGVERSION}
OutputDir=deploy

; If you want to display a license file, uncomment the following
; line and put in the correct file name.
; LicenseFile=license.rtf

; Icons and banners
; The install icon and banner do not need to be included
; in the setup package; they are only required during compilation
; of the InnoSetup script.
SetupIconFile=images/logo.ico
WizardImageFile=images/logo.bmp
WizardSmallImageFile=images/logo_small.bmp
WizardImageStretch=false
WizardImageBackColor=clWhite

; The uninstall icon must be included in the setup package
; and placed in the output folder.
UninstallDisplayIcon={#DEFINSDIR}/logo.ico

; Specific AppID
; Use InnoSetup's Generate UID command from the Tools menu
; to generate a unique ID. Make sure to have this ID start
; with TWO curly brackets.
; Never change this UID after the addin has been deployed.
AppId={{64551448-9F95-4571-BD89-F2132B3FBA8E}}

; vim: set ts=2 sts=2 sw=2 noet tw=60 fo+=lj cms=;%s 
