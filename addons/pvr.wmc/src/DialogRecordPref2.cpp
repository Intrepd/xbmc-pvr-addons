/*
 *      Copyright (C) 2005-2011 Team XBMC
 *      http://xbmc.org
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

#include "DialogRecordPref2.h"
#include "libXBMC_gui.h"

#define BUTTON_OK                       1
#define BUTTON_CANCEL                   2
#define BUTTON_CLOSE					22
#define BUTTON_DEFAULTS					180

#define RADIO_BUTTON_EPISODE			10
#define RADIO_BUTTON_SERIES				11
#define RADIO_FORCE_PREPAD				113
#define RADIO_FORCE_POSTPAD				115

#define SPIN_CONTROLRunType				12
#define SPIN_CONTROL_CHANNEL			13
#define SPIN_CONTROL_AIRTIME			14
#define SPIN_CONTROL_MAXEPISODE			15
#define SPIN_CONTROL_PREPAD				112
#define SPIN_CONTROL_POSTPAD			114
#define SPIN_CONTROL_KEEPLENGTH			116

#define LABEL_SHOWNAME					20

#define DAYS_CONTROLS_START				170
#define DAYS_CONTROLS_ANY				177

// used to save default values
//bool _defRecSeries;
//int _defRunType;
//bool _defAnyChannel;
//bool _defAnyTime;

int _runType;
bool _anyChannel;
bool _anyTime;
int _days;
int _recDay;		// hold the index of day of week the series records on Note its a DateTime day of week (starts at 0=>Sunday)
bool _recSeries;
bool _isSeries;
int _preDefIndex, _postDefIndex;
int _keepLengthIndex, _maxEpisodeIndex;
vector<CStdString> _preDefPaddings;
vector<CStdString> _postDefPaddings;
vector<CStdString> _keepLengths;
vector<CStdString> _maxEpisodes;
bool _isPrePadForced, _isPostPadForced;


CDialogRecordPref2::CDialogRecordPref2(
					bool isSeries, bool recSeries, int runtype, bool anyChannel, bool anyTime,
					int days, int recDay,
					vector<CStdString> preDefPaddings, int preDefIndex, vector<CStdString> postDefPaddings, int postDefIndex,
					bool isPrePadForced, bool isPostPadForced,
					vector<CStdString> keepLengths, int keepLengthIndex, vector<CStdString> maxEpisodesAmounts, int maxEpisodeIndex,
					CStdString currentChannelName, CStdString currentAirTime, CStdString showName) 
{
	// set result fields and save defaults
	_isSeries = isSeries;
	if (!_isSeries)
		_recSeries = false;							// show is not a series, recSeries is never true
	else
		_recSeries = recSeries;						// set whether rec series or episode is to be set by default
	_runType = runtype;
	_anyChannel = anyChannel;
	_anyTime = anyTime;
	// these fields are not accessible
	_currentChannel = currentChannelName;
	_currentAirTime = currentAirTime;
	_showName = showName;
	_days = days;
	_recDay = recDay;
	_preDefPaddings = preDefPaddings;
	_postDefPaddings = postDefPaddings;
	_preDefIndex = preDefIndex;
	_postDefIndex = postDefIndex;
	_isPrePadForced = isPrePadForced;
	_isPostPadForced = isPostPadForced;
	_keepLengthIndex = keepLengthIndex;
	_maxEpisodeIndex = maxEpisodeIndex;
	_keepLengths = keepLengths;
	_maxEpisodes = maxEpisodesAmounts;

	// needed for every dialog
	_confirmed = -1;				// init to failed load value (due to xml file not being found)
	_window = GUI->Window_create("DialogWMCRecordPrefs.xml", "Confluence", false, true);
	_window->m_cbhdl = this;
	_window->CBOnInit = OnInitCB;
	_window->CBOnFocus = OnFocusCB;
	_window->CBOnClick = OnClickCB;
	_window->CBOnAction = OnActionCB;
}

CDialogRecordPref2::~CDialogRecordPref2()
{
  GUI->Window_destroy(_window);
}

bool CDialogRecordPref2::OnInit()
{
	// display the show name in the window
	_window->SetControlLabel(LABEL_SHOWNAME, _showName.c_str());

	CStdString str; 

	// init radio buttons
	_radioRecEpisode = GUI->Control_getRadioButton(_window, RADIO_BUTTON_EPISODE);
	_radioRecSeries = GUI->Control_getRadioButton(_window, RADIO_BUTTON_SERIES);
	_radioRecEpisode->SetText("Record Episode"/*XBMC->GetLocalizedString(30101)*/);
	_radioRecSeries->SetText("Record Series"/*XBMC->GetLocalizedString(30102)*/);
	_radioRecEpisode->SetSelected(!_recSeries || !_isSeries);
	if (!_isSeries)
		_radioRecSeries->SetVisible(false);
	_radioRecSeries->SetSelected(_recSeries);  

	// force pre/post pad radio buttons
	_radioForcePrePad = GUI->Control_getRadioButton(_window, RADIO_FORCE_PREPAD);
	_radioForcePrePad->SetSelected(_isPrePadForced);
	_radioForcePostPad = GUI->Control_getRadioButton(_window, RADIO_FORCE_POSTPAD);
	_radioForcePostPad->SetSelected(_isPostPadForced);


	// init pre-padding spinner
	_spinPrePadding = GUI->Control_getSpin(_window, SPIN_CONTROL_PREPAD);
	_spinPrePadding->SetText(XBMC->GetLocalizedString(30123));		// set spinner text 
	for (int i=0; i<_preDefPaddings.size(); i++)
	{
		int minutes = atoi(_preDefPaddings[i]);
		if (minutes / 60 > 0 && minutes % 60 == 0)
		{
			if (minutes / 60 == 1)
				str.Format(XBMC->GetLocalizedString(30126), minutes / 60);
			else
				str.Format(XBMC->GetLocalizedString(30127), minutes / 60);
		}
		else
		{
			if (minutes == 1)
				str.Format(XBMC->GetLocalizedString(30124), minutes);
			else
				str.Format(XBMC->GetLocalizedString(30125), minutes);
		}
		_spinPrePadding->AddLabel(str, i);			// set spinner label
	}
	_spinPrePadding->SetValue(_preDefIndex);		// set spinner default
	
	// init post-padding spinner
	_spinPostPadding = GUI->Control_getSpin(_window, SPIN_CONTROL_POSTPAD);
	_spinPostPadding->SetText(XBMC->GetLocalizedString(30128));		// set spinner text 
	for (int i=0; i<_postDefPaddings.size(); i++)
	{
		int minutes = atoi(_postDefPaddings[i]);
		if (minutes / 60 > 0 && minutes % 60 == 0)
		{
			if (minutes / 60 == 1)  
				str.Format(XBMC->GetLocalizedString(30131), minutes / 60);
			else
				str.Format(XBMC->GetLocalizedString(30132), minutes / 60);
		}
		else
		{
			if (minutes == 1)
				str.Format(XBMC->GetLocalizedString(30129), minutes);
			else
				str.Format(XBMC->GetLocalizedString(30130), minutes);
		}
		_spinPostPadding->AddLabel(str, i);			// set spinner label 
	}
	_spinPostPadding->SetValue(_postDefIndex);		// set spinner default 

	// keep length spinner
	_spinKeepLength = GUI->Control_getSpin(_window, SPIN_CONTROL_KEEPLENGTH);
	_spinKeepLength->SetText(XBMC->GetLocalizedString(30133));		// set spinner text 
	for (int i=0; i<_keepLengths.size(); i++)
	{
		_spinKeepLength->AddLabel(XBMC->GetLocalizedString(30134 + i),  i);			// set spinner labels
	}
	_spinKeepLength->SetValue(_keepLengthIndex);		// set spinner default 

	
	// max episode spinner
	_spinMaxEpisode = GUI->Control_getSpin(_window, SPIN_CONTROL_MAXEPISODE);
	_spinMaxEpisode->SetText(XBMC->GetLocalizedString(30138));				// set spinner text 
	for (int i=0; i<_maxEpisodes.size(); i++)
	{
		if (_maxEpisodes[i] == "1")
			str.Format(XBMC->GetLocalizedString(30139), _maxEpisodes[i]);
		else if (_maxEpisodes[i] == "-1")
			str = XBMC->GetLocalizedString(30141);
		else 
			str.Format(XBMC->GetLocalizedString(30140), _maxEpisodes[i]);

		_spinMaxEpisode->AddLabel(str,  i);				// set spinner labels
	}
	_spinMaxEpisode->SetValue(_maxEpisodeIndex);		// set spinner default 


	// init runtype spin control
	_spinRunType = GUI->Control_getSpin(_window, SPIN_CONTROLRunType);
	_spinRunType->SetText(XBMC->GetLocalizedString(30103));		// set spinner text (show type)
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30104), 0); // any show type
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30105), 1);	// first run only
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30106), 2);	// live only
	_spinRunType->SetValue(_runType);

	// init channel spin control
	_spinChannel = GUI->Control_getSpin(_window, SPIN_CONTROL_CHANNEL);
	_spinChannel->SetText(XBMC->GetLocalizedString(30107));		// set spinner text (channel)
	_spinChannel->AddLabel(_currentChannel.c_str(), 0);			// add current channel
	_spinChannel->AddLabel(XBMC->GetLocalizedString(30108), 1);	// add "Any channel"
	_spinChannel->SetValue(_anyChannel ? 1:0);
	
	// init airtime spin control
	_spinAirTime = GUI->Control_getSpin(_window, SPIN_CONTROL_AIRTIME);
	_spinAirTime->SetText(XBMC->GetLocalizedString(30110));		// set spinner text (airtime)
	_spinAirTime->AddLabel(_currentAirTime.c_str(), 0);			// current air time
	_spinAirTime->AddLabel(XBMC->GetLocalizedString(30111), 1);	// "Anytime" 
	_spinAirTime->SetValue(_anyTime ? 1:0);

	// init days of week radio buttons
	_radioAnyDay = GUI->Control_getRadioButton(_window, DAYS_CONTROLS_ANY);
	_radioRecDay = GUI->Control_getRadioButton(_window, DAYS_CONTROLS_START + _recDay);	// get the control that corresponds to the recording Day
	SetDaysOfWeek(_days);
	
	// set visibility of spin controls based on whether dialog is set to rec a series
	_spinRunType->SetVisible(_recSeries);
	_spinChannel->SetVisible(_recSeries);
	_spinAirTime->SetVisible(_recSeries);
	_spinMaxEpisode->SetVisible(_recSeries);
	DaysOfWeekVisible(_recSeries);
	
  return true;
}

void CDialogRecordPref2::SetDefaults()
{
	//restore controls with default values
	OnInit();
}

bool CDialogRecordPref2::OnClick(int controlId)
{
	switch(controlId)
	{
		case BUTTON_OK:				// save value from GUI, then FALLS THROUGH TO CANCEL
			//RecSeries = _radioRecSeries->IsSelected();
			//RunType = _spinRunType->GetValue();
			//AnyChannel = _spinChannel->GetValue() == 1;
			//AnyTime = _spinAirTime->GetValue() == 1;
		case BUTTON_CANCEL:
		case BUTTON_CLOSE:
			if (_confirmed == -1)		// if not previously confirmed, set to cancel value
				_confirmed = 0;			
			_window->Close();
			//GUI->Control_releaseRadioButton(_radioRecEpisode);  // not sure if these releases are needed
			//GUI->Control_releaseRadioButton(_radioRecSeries);
			//GUI->Control_releaseSpin(_spinRunType);
			//GUI->Control_releaseSpin(_spinChannel);
			//GUI->Control_releaseSpin(_spinAirTime);
			break;
		case BUTTON_DEFAULTS:
			SetDefaults();
			break;
		case RADIO_BUTTON_EPISODE:
		case RADIO_BUTTON_SERIES:
			_recSeries = (controlId == RADIO_BUTTON_SERIES); 
			_radioRecEpisode->SetSelected(!_recSeries);
			_radioRecSeries->SetSelected(_recSeries);
			_spinRunType->SetVisible(_recSeries);
			_spinChannel->SetVisible(_recSeries);
			_spinAirTime->SetVisible(_recSeries);
			_spinMaxEpisode->SetVisible(_recSeries);
			DaysOfWeekVisible(_recSeries);
			break;
		case DAYS_CONTROLS_ANY:
			if (_radioAnyDay->IsSelected())
				DaysOfWeekSetSelected(true);
			else
			{
				DaysOfWeekSetSelected(false);
				_radioRecDay->SetSelected(true);
			}
			break;
	}
	if (controlId >= DAYS_CONTROLS_START && controlId < DAYS_CONTROLS_ANY)							// if a day radio button was clicked
	{
		CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, controlId);				// get the button clicked

		if (!radio->IsSelected())					// if the day was deselected
		{
			if (_radioAnyDay->IsSelected())			// make 'any' day deselected too
				_radioAnyDay->SetSelected(false);
		}
		else										// a day was selected
		{
			// see if all days are now selected, if they are then select 'Any' day too
			bool allDaysSelected = true;
			for (int i = DAYS_CONTROLS_START; i<DAYS_CONTROLS_ANY; i++)
			{
				CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
				if (!radio->IsSelected())
				{
					allDaysSelected = false;
					break;
				}
			}
			if (allDaysSelected)					// all days are selected, so set 'Any' day true
				_radioAnyDay->SetSelected(true);
		}

	}

  return true;
}

// return values of controls to caller (this is how caller gets result if dialog was confirmed)
bool CDialogRecordPref2::GetRecSeries() { return _radioRecSeries->IsSelected(); }
int CDialogRecordPref2::GetRunType() { return _spinRunType->GetValue(); };
bool CDialogRecordPref2::GetAnyChannel() { return _spinChannel->GetValue() == 1; };
bool CDialogRecordPref2::GetAnyTime() { return _spinAirTime->GetValue() == 1; };
int CDialogRecordPref2::GetPrePaddingIndex() { return _spinPrePadding->GetValue(); }
int CDialogRecordPref2::GetPostPaddingIndex() { return _spinPostPadding->GetValue(); }
bool CDialogRecordPref2::GetForcePrePad() { return _radioForcePrePad->IsSelected(); };
bool CDialogRecordPref2::GetForcePostPad() { return _radioForcePostPad->IsSelected(); };
int CDialogRecordPref2::GetKeepLengthIndex() { return _spinKeepLength->GetValue(); };
int CDialogRecordPref2::GetMaxEpisodeIndex() { return _spinMaxEpisode->GetValue(); };

// set days of week selected state based on wmc DaysOfWeek input value.  
// Uses bit-wise selection
void CDialogRecordPref2::SetDaysOfWeek(int days)
{
	int bit = 1;
	for (int i=DAYS_CONTROLS_START; i<DAYS_CONTROLS_ANY; i++)
	{
		CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
		radio->SetSelected( (bit & days) != 0);		// set radio button as selected depending on bit value
		bit = bit<<1;								// shift day bit to left
	}
	if (days == 0x7f)
		_radioAnyDay->SetSelected(true); 
}

//set all days of week (including 'Any') visible or now visible
void CDialogRecordPref2::DaysOfWeekVisible(bool show)
{
	for (int i=DAYS_CONTROLS_START; i <= DAYS_CONTROLS_ANY; i++)
	{
		CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
		radio->SetVisible(show);
	}
}

// set all days of week selected or not selected (excluding 'Any' day)
void CDialogRecordPref2::DaysOfWeekSetSelected(bool val)
{
	for (int i=DAYS_CONTROLS_START; i<DAYS_CONTROLS_ANY; i++)
	{
		CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
		radio->SetSelected(val);
	}
}

// return wmc DaysOfWeek value as set by days radio buttons
int CDialogRecordPref2::GetDaysOfWeek()
{
	if (_radioAnyDay->IsSelected())
		return 0x7f;
	else
	{
		int retValue = 0;
		int bit = 1;
		for (int i=DAYS_CONTROLS_START; i<DAYS_CONTROLS_ANY; i++)
		{
			CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
			if (radio->IsSelected())
				retValue = retValue | bit;
			bit = bit<<1;							// shift day bit to left
		}
		if (retValue == 0)							// if user for some reason switched off all the day flags
			retValue = _recDay + 1;					// so force rec day (+1 to convert from DateTime day to wmc DaysOfWeek enum)
		return retValue;
	}
}


bool CDialogRecordPref2::OnInitCB(GUIHANDLE cbhdl)
{
  CDialogRecordPref2* dialog = static_cast<CDialogRecordPref2*>(cbhdl);
  return dialog->OnInit();
}

bool CDialogRecordPref2::OnClickCB(GUIHANDLE cbhdl, int controlId)
{
	CDialogRecordPref2* dialog = static_cast<CDialogRecordPref2*>(cbhdl);
	if (controlId == BUTTON_OK)
		dialog->_confirmed = 1;
	return dialog->OnClick(controlId);
}

bool CDialogRecordPref2::OnFocusCB(GUIHANDLE cbhdl, int controlId)
{
  CDialogRecordPref2* dialog = static_cast<CDialogRecordPref2*>(cbhdl);
  return dialog->OnFocus(controlId);
}

bool CDialogRecordPref2::OnActionCB(GUIHANDLE cbhdl, int actionId)
{
  CDialogRecordPref2* dialog = static_cast<CDialogRecordPref2*>(cbhdl);
  return dialog->OnAction(actionId);
}

bool CDialogRecordPref2::Show()
{
  if (_window)
    return _window->Show();

  return false;
}

void CDialogRecordPref2::Close()
{
  if (_window)
    _window->Close();
}

int CDialogRecordPref2::DoModal()
{
  if (_window)
    _window->DoModal();
  return _confirmed;		// return true if user didn't cancel dialog
}

bool CDialogRecordPref2::OnFocus(int controlId)
{
  return true;
}

bool CDialogRecordPref2::OnAction(int actionId)
{
  if (actionId == ADDON_ACTION_CLOSE_DIALOG || actionId == ADDON_ACTION_PREVIOUS_MENU || actionId == 92/*back*/)
    return OnClick(BUTTON_CANCEL);
  else
    return false;
}
