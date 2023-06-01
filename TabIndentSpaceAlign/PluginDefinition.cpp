//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <shlwapi.h>
#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <strsafe.h>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading
bool enable;
TCHAR iniFilePath[MAX_PATH];
void pluginInit(HANDLE hModule)
{
	GetModuleFileName((HMODULE)hModule, iniFilePath, sizeof(iniFilePath));

    // remove the module name : get plugins directory path
    PathRemoveFileSpec(iniFilePath);

	PathAppend(iniFilePath, TEXT("Config"));

	PathAppend(iniFilePath, NPP_PLUGIN_NAME);
	StringCchCat(iniFilePath, MAX_PATH, TEXT(".ini"));

	enable = GetPrivateProfileInt(TEXT("Settings"), TEXT("Enable"), 0, iniFilePath) != 0 ? true : false;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	WritePrivateProfileString(TEXT("Settings"), TEXT("Enable"), enable ? TEXT("1") : TEXT("0"), iniFilePath);
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
ShortcutKey *Tab;
ShortcutKey *STab;
void commandMenuInit()
{
	setCommand(0, TEXT("Enable"), toggle, NULL, enable);

	Tab = new ShortcutKey;
    Tab->_isAlt      = false;
    Tab->_isCtrl     = false;
    Tab->_isShift    = false;
    Tab->_key        = VK_TAB;
	setCommand(1, TEXT("Indent"), indent, Tab);

    STab = new ShortcutKey;
    STab->_isAlt     = false;
    STab->_isCtrl    = false;
    STab->_isShift   = true;
    STab->_key       = VK_TAB;
    setCommand(2, TEXT("Unindent"), unindent, STab);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete(Tab);
	delete(STab);
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit)
{
    if (index >= nbFunc)
	{
        return false;
	}

    if (!pFunc)
	{
        return false;
	}

    StringCchCopy(funcItem[index]._itemName, sizeof(funcItem[index]._itemName), cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

void basicAutoIndent()
{
	if(enable)
	{
		HWND curScintilla = getCurrentScintilla();

		SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

		auto position    = SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
		auto line_number = SendMessage(curScintilla, SCI_LINEFROMPOSITION, position, 0);
		auto line_start  = SendMessage(curScintilla, SCI_POSITIONFROMLINE, line_number, 0);

		auto prevline = line_number - 1;
		auto prevline_start = SendMessage(curScintilla, SCI_POSITIONFROMLINE, prevline, 0);
		auto prevline_indent = SendMessage(curScintilla, SCI_GETLINEINDENTPOSITION, prevline, 0);

		/* The indentation is split into two parts, copy both parts.
		   Note that either part may be empty. */
		char indent1[9999];
		SendMessage(curScintilla, SCI_SETSEL, prevline_start, prevline_indent);
		SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&indent1);

		/* Combine the parts and duplicate them on both lines. */
		SendMessage(curScintilla, SCI_INSERTTEXT, line_start, (LPARAM)&indent1);

		/* Place cursor at end of indentation. */
		auto new_position = line_start + (prevline_indent - prevline_start);
		SendMessage(curScintilla, SCI_SETSEL, new_position, new_position);

		SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	}
}


void toggle()
{
	enable = !enable;

	HWND curScintilla = getCurrentScintilla();
	HMENU hMenu = GetMenu(nppData._nppHandle);
    if (hMenu)
    {
        CheckMenuItem(hMenu, funcItem[0]._cmdID, MF_BYCOMMAND | (enable ? MF_CHECKED : MF_UNCHECKED));
    }
}

/* Conceptually, each line is divided into three parts: Indentation, alignment, and text.
	Indentation consists of hard tabs, alignment of spaces, and text of any character after
	alignment. */
void indent()
{
	HWND curScintilla = getCurrentScintilla();

	/* Override indent behavior only if we're using hard tabs. */
	if(enable)
	{
		SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

		auto start = SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
		auto end   = SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);

		/* If we have a selection, the last character is not included. */
		if(start < end)
		{
			end--;
		}

		auto start_line = SendMessage(curScintilla, SCI_LINEFROMPOSITION, start, 0);
		auto end_line   = SendMessage(curScintilla, SCI_LINEFROMPOSITION, end, 0);

		if(start_line < end_line)
		{
			auto position = SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
			auto anchor   = SendMessage(curScintilla, SCI_GETANCHOR, 0, 0);

			for(auto line_number = start_line; line_number <= end_line; line_number++)
			{
				auto line_start = SendMessage(curScintilla, SCI_POSITIONFROMLINE, line_number, 0);

				/* Insert tab. */
				SendMessage(curScintilla, SCI_INSERTTEXT, line_start, (LPARAM)&"\t");
			}

			start = SendMessage(curScintilla, SCI_POSITIONFROMLINE, start_line, 0);
			end   = SendMessage(curScintilla, SCI_GETLINEENDPOSITION, end_line, 0) + 1;

			if(position > anchor)
			{
				SendMessage(curScintilla, SCI_SETSEL, start, end);
			}
			else
			{
				SendMessage(curScintilla, SCI_SETSEL, end, start);
			}
		}
		else
		{
			char line[9999];

			auto line_start     = SendMessage(curScintilla, SCI_POSITIONFROMLINE, start_line, 0);
			auto line_alignment = SendMessage(curScintilla, SCI_GETLINEINDENTPOSITION, start_line, 0);
			SendMessage(curScintilla, SCI_GETLINE, start_line, (LPARAM)&line);

			/* Find the position where indent ends and alignment begins. */
			auto line_indent = line_alignment - line_start;
			char space = ' ';
			while(line_indent > 0 && line[line_indent-1] == space)
			{
				line_indent--;
			}
			line_indent += line_start;

			if(start <= line_indent)
			{
				/* Insert tab. */
				SendMessage(curScintilla, SCI_INSERTTEXT, line_start, (LPARAM)&"\t");

				/* Place caret at end of indentation. */
				SendMessage(curScintilla, SCI_SETSEL, line_indent + 1, line_indent + 1);
			}
			else
			{
				auto tabWidth = SendMessage(curScintilla, SCI_GETTABWIDTH, 0, 0);

				decltype(tabWidth) numSpaces;
				if(start <= line_alignment)
				{
					auto col = SendMessage(curScintilla, SCI_GETCOLUMN, line_alignment, 0);
					numSpaces = tabWidth - (col % tabWidth);
				}
				else
				{
					auto col = SendMessage(curScintilla, SCI_GETCOLUMN, start, 0);
					numSpaces = tabWidth - (col % tabWidth);
				}

				SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)&"");

				SendMessage(curScintilla, SCI_SETSEL, start, start);
				for(int i = 0; i < numSpaces; i++)
				{
					SendMessage(curScintilla, SCI_INSERTTEXT, start, (LPARAM)&" ");
				}
				SendMessage(curScintilla, SCI_SETSEL, start + numSpaces, start + numSpaces);
			}
		}

		SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	}
	else
	{
		SendMessage(curScintilla, SCI_TAB, 0, 0);
	}
}

void unindent()
{
	HWND curScintilla = getCurrentScintilla();

	/* Override unindent behavior only if we're using hard tabs. */
	if(enable)
	{
		SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

		auto position = SendMessage(curScintilla, SCI_GETCURRENTPOS, 0, 0);
		auto anchor   = SendMessage(curScintilla, SCI_GETANCHOR, 0, 0);

		auto start = SendMessage(curScintilla, SCI_GETSELECTIONSTART, 0, 0);
		auto end   = SendMessage(curScintilla, SCI_GETSELECTIONEND, 0, 0);

		/* If we have a selection, the last character is not included. */
		if(start < end)
		{
			end--;
		}

		auto start_line = SendMessage(curScintilla, SCI_LINEFROMPOSITION, start, 0);
		auto end_line   = SendMessage(curScintilla, SCI_LINEFROMPOSITION, end, 0);

		bool multiline = start_line < end_line;

		for(auto line_number = start_line; line_number <= end_line; line_number++)
		{
			char line[9999];

			/* line_start <= line_indent <= line_alignment <= line_end */
			auto line_start     = SendMessage(curScintilla, SCI_POSITIONFROMLINE, line_number, 0);
			auto line_alignment = SendMessage(curScintilla, SCI_GETLINEINDENTPOSITION, line_number, 0);
			auto line_end       = SendMessage(curScintilla, SCI_GETLINEENDPOSITION, line_number, 0);
			SendMessage(curScintilla, SCI_GETLINE, line_number, (LPARAM)&line);

			/* Find the position where indent ends and alignment begins. */
			auto line_indent = line_alignment - line_start;
			char space = ' ';

			while(line_indent > 0 && line[line_indent-1] == space)
			{
				line_indent--;
			}
			line_indent += line_start;

			/* If multiline or start is within indent, remove one level of indentation. */
			if(multiline || start <= line_indent)
			{
				/* Ensure that we have indentation to remove. */
				if(line_start < line_indent)
				{
					/* Delete the last character of indent. (Should be a tab) */
					SendMessage(curScintilla, SCI_SETSEL, line_indent - 1, line_indent);
					SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)&"");

					/* Place caret between indent and alignment. */
					SendMessage(curScintilla, SCI_SETSEL, line_indent - 1, line_indent - 1);
				}
			}
			/* If start is within alignment or there is no indent, remove alignment. */
			else if(start <= line_alignment)
			{
				SendMessage(curScintilla, SCI_SETSEL, line_indent, line_alignment);
				SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)&"");

				/* Place caret between indent and text. */
				SendMessage(curScintilla, SCI_SETSEL, line_indent, line_indent);
			}
			/* If start is within text, use normal behavior.
				Note that multi-line selection cannot begin in text. */
			else
			{
				SendMessage(curScintilla, SCI_BACKTAB, 0, 0);
			}
		}

		/* Set selection after multi-line unindent to all lines. */
		if(multiline)
		{
			start = SendMessage(curScintilla, SCI_POSITIONFROMLINE, start_line, 0);
			end = SendMessage(curScintilla, SCI_GETLINEENDPOSITION, end_line, 0) + 1;

			if(position > anchor)
			{
				SendMessage(curScintilla, SCI_SETSEL, start, end);
			}
			else
			{
				SendMessage(curScintilla, SCI_SETSEL, end, start);
			}
		}

		SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	}
	else
	{
		SendMessage(curScintilla, SCI_BACKTAB, 0, 0);
	}
}

/* Helper functions. */
HWND getCurrentScintilla()
{
    int which = -1;
    SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return NULL;
    if (which == 0)
    {
        return nppData._scintillaMainHandle;
    }
    else
    {
        return nppData._scintillaSecondHandle;
    }
    return nppData._scintillaMainHandle;
}
