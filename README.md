# Privilege escalation using the XAML diagnostics API (CVE-2023-36003)

This is a POC (Proof of Concept) of a privilege escalation vulnerability using
the XAML diagnostics API. The vulnerability was patched in December's Patch
Tuesday, and the CVE assigned to it is
[CVE-2023-36003](https://msrc.microsoft.com/update-guide/vulnerability/CVE-2023-36003).

## Usage

The POC is a C++ project that can be compiled using Visual Studio. After
compiling, the POC can be run without arguments to look for an inaccessible
process and then run the exploit against it. Alternatively, a process id can be
passed as an argument, and the exploit will be run against that process.

## Vulnerability details

More details about the vulnerability can be found in the following blog post:

[Privilege escalation using the XAML diagnostics API
(CVE-2023-36003)](https://m417z.com/Privilege-escalation-using-the-XAML-diagnostics-API-CVE-2023-36003/)
