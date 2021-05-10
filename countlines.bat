@echo off
@setlocal enabledelayedexpansion
@set testl=%cmdcmdline:"=%
@set testr=!testl:%~nx0=!
set cmdcloc=cloc src include --skip-uniqueness --exclude-dir=fonts

if /I not "%testl%" == "%testr%" (
	%cmdcloc% --by-file
	%cmdcloc%
	pause
) else (
	%cmdcloc% %*
)