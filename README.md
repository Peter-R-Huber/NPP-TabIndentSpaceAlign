NPP-TabIndentSpaceAlign
=======================

Purpose
-------

This Notepad++ plugin is for those who care about the distinction between indentation and formatting in source code and who want to indent with TAB and format with SPACE.

The original plugin has been developed by someone else in 2012. Only a 32-bit version is available. I created this repository, because I adopted the original files for 64-bit and thought that maybe someone else might be interested in the results.

Unfortunately I don't know, who the original developer is. The original files are hosted on Google Code and can be obtained here: [https://code.google.com/archive/p/kereds-notepad-plus-plus-plugins/](https://code.google.com/archive/p/kereds-notepad-plus-plus-plugins/).


Features
--------

When the TabIndentSpaceAlign plugin is enabled, Notepad++ changes its behavior concerning auto-indent and TAB handling in the following manner:

* When pressing ENTER, the new line gets the same initial whitespace as the previous line. That means, whatever combination of TAB and SPACE is at the beginning of the previous line, up to the first character that in not TAB or SPACE, is duplicated in the new line.

  Without the plugin, when auto-indent is enabled, the new line also gets indented as the previous line, but the whitespace characters are either all converted to SPACE or to TAB (plus maybe some SPACES if the indentation goes in between tab positions).

* When pressing TAB before any non-TAB character in a line, then a TAB character is inserted. After the first non-TAB character in a line, a number of SPACES is inserted to fill up to the next tab position.


Improvements
------------

In addition to making the source code 64-bit compatible, I changed a few other things:

* When pressing ENTER, in some cases not only the new line got indented, but also the previous line got additional indention. I don't know, whether that was on purpose. I changed it anyway, so that the previous line never gets changed.

* If "TAB to SPACE" is configured in Notepad++ (globally or for the current language), pressing the TAB key has the same effect as if the plugin were disabled. In the original version the "TAB to SPACE" setting was ignored.

  That change is useful when we want TAB for indention in general, but not for specific languages, e.g. Python.


Issues
------

A lot (-:

My version does not differ much from the original version yet. But maybe improvements are possible now, that were not possible in 2012, when the original was implemented. The API for the communication between plugins and Notepad++ and the underlying Scintilla library have been extended since then.


Installation
------------

The plugin can not (yet) be installed via the Notepad++ Plugin Admin. A subdirectory must be created below the plugin directory of Notepad++ and the plugin DLL must copied into it.

Creating the DLL is easy with The Microsoft Developer Studio. Just open the solution file "TabIndentSpaceAlign.sln" and build the configuration "Unicode Release / 64bit".


License
-------

The original developer chose [Gnu GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html) as copyright license, so I keep it like that.
