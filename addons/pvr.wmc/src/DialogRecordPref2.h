#pragma once

/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "client.h"
#include "pvr2wmc.h"
#include "platform/util/StdString.h"

// version 2 of custom recording pref dialog

class CDialogRecordPref2
{

public:
  CDialogRecordPref2(bool isSeries, bool recSeries, int runtype, bool anyChannel, bool anyTime,
					int days, int recDay,
					vector<CStdString> preDefPaddings, int preDefIndex, vector<CStdString> postDefPaddings, int postDefIndex,
					bool isPrePadForced, bool isPostPadForced,
					vector<CStdString> keepLengths, int keepLengthIndex, vector<CStdString> maxEpisodesAmounts, int maxEpisodeIndex,
					CStdString currentChannelName, CStdString currentAirTime, CStdString showName);
	virtual ~CDialogRecordPref2();

	bool Show();
	void Close();
	int DoModal();						// returns -1=> load failed, 0=>canceled, 1=>confirmed
	

	// value returned to caller
	//bool RecSeries;						// values returned
	//int RunType;
	//bool AnyChannel;
	//bool AnyTime;
	bool GetRecSeries();
	int GetRunType();
	bool GetAnyChannel();
	bool GetAnyTime();
	int GetDaysOfWeek();
	int GetPrePaddingIndex();
	int GetPostPaddingIndex();
	bool GetForcePrePad();
	bool GetForcePostPad();
	int GetKeepLengthIndex();
	int GetMaxEpisodeIndex();

private:
	CStdString _currentChannel;			// these are just used for dialog display
	CStdString _currentAirTime;
	CStdString _showName;
private:
	void DaysOfWeekVisible(bool visable);
	void DaysOfWeekSetSelected(bool val);
	void SetDaysOfWeek(int days);
	void SetDefaults();
	
private:
	CAddonGUIRadioButton *_radioRecEpisode;
	CAddonGUIRadioButton *_radioRecSeries;
	CAddonGUIRadioButton *_radioAnyDay;
	CAddonGUIRadioButton *_radioRecDay;
	CAddonGUIRadioButton *_radioForcePrePad;
	CAddonGUIRadioButton *_radioForcePostPad;
	CAddonGUISpinControl *_spinRunType;
	CAddonGUISpinControl *_spinChannel;
	CAddonGUISpinControl *_spinAirTime;
	CAddonGUISpinControl *_spinPrePadding;
	CAddonGUISpinControl *_spinPostPadding;
	CAddonGUISpinControl *_spinKeepLength;
	CAddonGUISpinControl *_spinMaxEpisode;


  // following is needed for every dialog
private:
  CAddonGUIWindow *_window;
  int _confirmed;		//-1=> load failed, 0=>canceled, 1=>confirmed

  bool OnClick(int controlId);
  bool OnFocus(int controlId);
  bool OnInit();
  bool OnAction(int actionId);

  static bool OnClickCB(GUIHANDLE cbhdl, int controlId);
  static bool OnFocusCB(GUIHANDLE cbhdl, int controlId);
  static bool OnInitCB(GUIHANDLE cbhdl);
  static bool OnActionCB(GUIHANDLE cbhdl, int actionId);


};

