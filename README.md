# Paste Special

A Visual Studio Code extension to paste different clipboard formats like text, HTML, RTF, CSV and file lists.

## Features

This extension makes it possible to paste data present on the clipboard in different supported data formats into the editor window.
At the moment, supported formats are:

### Windows
- Plain Text
- HTML source
- Rich Text Format
- Comma-separated values
- A list of filenames

### Linux
- Plain Text
- HTML source
- Rich Text Format
- A list of URIs (e.g. filenames)
- PNG images (as base64 endoded data url)
- JPEG images (as base64 endoded data url) 
- TIFF images (as base64 endoded data url) 

### macOS
- Plain Text

The extension integrates a *Paste Special...* command in the context menu of the editor window 
and implements the usual platform-specific keyboard shortcuts for *Paste Special*.

## Requirements

The extension uses native code modules to access the different clipboard formats. 
As of now, there are only native code modules for Windows and Linux. 

The extension should work on any newer Windows version, both 32-bit and 64-bit. 

The extension should work on 64-bit Linux versions and was tested with Debian and Ubuntu. GTK+ 3 must be installed.

When using macOS, only the plain text format is available at the moment, duplicating the normal *Paste* functionality.

## Known Issues

None. 

## Release Notes

### 0.9.1

Bug fixed regarding Windows registered clipboard formats

### 0.9.0

Initial release


