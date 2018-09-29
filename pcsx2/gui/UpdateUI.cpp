/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "MainFrame.h"
#include "GSFrame.h"

// General Notes:
//  * It's very important that we re-discover menu items by ID every time we change them,
//	because the modern era of configurable GUIs means that we can't be assured the IDs
//	exist anymore.


// This is necessary because this stupid wxWidgets thing has implicit debug errors
// in the FindItem call that asserts if the menu options are missing.  This is bad
// mojo for configurable/dynamic menus. >_<
void MainEmuFrame::EnableMenuItem(int id, bool enable)
{
	if (wxMenuItem *item = m_menubar.FindItem(id))
		item->Enable(enable);
}

void MainEmuFrame::SetMenuItemLabel(int id, wxString str)
{
	if (wxMenuItem *item = m_menubar.FindItem(id))
		item->SetItemLabel(str);
}

static void _SaveLoadStuff(bool enabled)
{
	sMainFrame.EnableMenuItem(MenuId_Sys_LoadStates, enabled);
	sMainFrame.EnableMenuItem(MenuId_Sys_SaveStates, enabled);

#ifdef USE_NEW_SAVESLOTS_UI
	for (int i = 0; i < 10; i++)
	{
		int load_menu_item = MenuId_State_Load01 + i + 1;
		int save_menu_item = MenuId_State_Save01 + i + 1;

		// If the cache is out of sync with the actual files, we need to update things. First update, the cache'll be blank, and this will populate everything.
		if (saveslot_cache[i].empty == saveslot_cache[i].isUsed())
		{
			// If there is actually a file there, or the cache was for a different game, we force an update.
			// If the cache says there's a saveslot for the current game that there isn't a file for, writing it is done in a different thread,
			// so it might not be written yet. Which is why I cache to begin with. 
			if (saveslot_cache[i].isUsed() || (saveslot_cache[i].crc != ElfCRC)) 
			{
				saveslot_cache[i].UpdateCache();
			}
		}

		sMainFrame.EnableMenuItem(load_menu_item, !saveslot_cache[i].empty);
		sMainFrame.SetMenuItemLabel(load_menu_item, saveslot_cache[i].SlotName());
		sMainFrame.SetMenuItemLabel(save_menu_item, saveslot_cache[i].SlotName());
	}
	Sstates_updateLoadBackupMenuItem(false);
#endif
}

// Updates the enable/disable status of all System related controls: menus, toolbars,
// etc.  Typically called by SysEvtHandler whenever the message pump becomes idle.
void UI_UpdateSysControls()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_UpdateSysControls))
		return;

	sApp.PostAction(CoreThreadStatusEvent(CoreThread_Indeterminate));

	_SaveLoadStuff(SysHasValidState());
}

void UI_DisableSysShutdown()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_DisableSysShutdown))
		return;

	sMainFrame.EnableMenuItem(MenuId_Sys_Shutdown, false);
	sMainFrame.EnableMenuItem(MenuId_IsoBrowse, !g_Conf->AskOnBoot);
	wxGetApp().GetRecentIsoManager().EnableItems(!g_Conf->AskOnBoot);
}

void UI_EnableSysShutdown()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_EnableSysShutdown))
		return;

	sMainFrame.EnableMenuItem(MenuId_Sys_Shutdown, true);
}


void UI_DisableSysActions()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_DisableSysActions))
		return;

	sMainFrame.EnableMenuItem(MenuId_Sys_Shutdown, false);

	_SaveLoadStuff(false);
}

void UI_EnableSysActions()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_EnableSysActions))
		return;

	sMainFrame.EnableMenuItem(MenuId_Sys_Shutdown, true);
	sMainFrame.EnableMenuItem(MenuId_IsoBrowse, true);
	wxGetApp().GetRecentIsoManager().EnableItems(true);

	_SaveLoadStuff(true);
}

void UI_DisableStateActions()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_DisableStateActions))
		return;

	_SaveLoadStuff(false);
}

void UI_EnableStateActions()
{
	if (wxGetApp().Rpc_TryInvokeAsync(&UI_EnableStateActions))
		return;

	_SaveLoadStuff(true);
}
