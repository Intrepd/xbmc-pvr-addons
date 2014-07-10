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

//#define RADIO_BUTTON_EPISODE			10
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
#define SPIN_CONTROL_PRIORITY			117

#define LABEL_TITLEBAR					19
#define LABEL_INFOSHOW1					23
#define LABEL_INFOSHOW2					24  // air info
#define LABEL_INFOSHOW3					25  // channel info
#define LABEL_INFOSHOW4					26  // channel info

#define DAYS_CONTROLS_START				170
#define DAYS_CONTROLS_ANY				177

// these values are used to store the original value of the param, its used to reset default
bool _createNewTimer;
int _runType;
bool _anyChannel;
bool _anyTime;
int _days;
int _recDay;		// int for day the recording happens on, its a DateTime day (starts at 0=>Sun, 1->Mon)
bool _recSeries;
bool _isSeries;
int _preDefValue, _postDefValue, _priorityValue;
int _keepLengthValue, _maxEpisodeValue;
vector<CStdString> _preDefPaddings;
vector<CStdString> _postDefPaddings;
vector<CStdString> _keepLengths;
vector<CStdString> _maxEpisodes;
vector<CStdString> _priorityValues;
bool _isPrePadForced, _isPostPadForced;
// most of these are just used for info display
CStdString _currentChannel;			
CStdString _currentAirTime;
CStdString _showName;
CStdString _shortDescription;
CStdString _airDateTime;


CDialogRecordPref2::CDialogRecordPref2(
					bool createNewTimer,
					bool isSeries, bool recSeries, int runtype, bool anyChannel, bool anyTime,
					int days, int recDay,
					vector<CStdString> preDefPaddings, int preDefValue, vector<CStdString> postDefPaddings, int postDefValue,
					bool isPrePadForced, bool isPostPadForced,
					vector<CStdString> keepLengths, int keepLengthValue,
					vector<CStdString> maxEpisodesAmounts, int maxEpisodeValue,
					vector<CStdString> priorityValues, int priorityValue,
					PVR_TIMER_STATE tmrState,
					CStdString currentChannelName, CStdString currentAirTime, CStdString airDateTime,
					CStdString showName, CStdString shortDescription) 
{
	_createNewTimer = false;//createNewTimer;
	// set result fields and save defaults
	_isSeries = isSeries;						// true if the program is actually part of a tv series
	if (!_isSeries)
		_recSeries = false;						// show is not a series, recSeries option is never true
	else
		_recSeries = recSeries;					// set whether rec series or episode is to be set by default
	_runType = runtype;
	_anyChannel = anyChannel;
	_anyTime = anyTime;  
	_days = days;
	_recDay = recDay;
	_preDefPaddings = preDefPaddings;
	_postDefPaddings = postDefPaddings;
	_preDefValue = preDefValue;
	_postDefValue = postDefValue;
	_isPrePadForced = isPrePadForced;
	_isPostPadForced = isPostPadForced;
	_keepLengthValue = keepLengthValue;
	_maxEpisodeValue = maxEpisodeValue;
	_keepLengths = keepLengths;
	_maxEpisodes = maxEpisodesAmounts;
	_priorityValues = priorityValues;
	_priorityValue = priorityValue;
	
	_showName = showName;
	_shortDescription = shortDescription;
	_currentChannel = currentChannelName;
	_currentAirTime = currentAirTime;
	_airDateTime = airDateTime;

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

// called to initialize dialog controls after window is created.  Also this method is
// called to reset default values if user his the 'defaults' button.
bool CDialogRecordPref2::OnInit()
{
	CStdString str; 

	str = _showName;
	_window->SetControlLabel(LABEL_TITLEBAR, str.c_str());

	// display the show name in the window
	str = /*_showName + "[CR]  " +*/ _shortDescription /*+ "[CR]Airs: " + _airDateTime + " on " + _currentChannel*/;
	_window->SetControlLabel(LABEL_INFOSHOW1, str.c_str());
	
	str = "Airs: " + _airDateTime ;
	_window->SetControlLabel(LABEL_INFOSHOW2, str.c_str());

	str = "Channel: " + _currentChannel;
	_window->SetControlLabel(LABEL_INFOSHOW3, str.c_str());

	// show the recording status if existing timer is being edited
	if (!_createNewTimer)
	{
		str.Format(XBMC->GetLocalizedString(30148), "there are some conflicts");
		_window->SetControlLabel(LABEL_INFOSHOW4, str.c_str());
	}
	else
		_window->SetControlLabel(LABEL_INFOSHOW4, "");

	// init series radio button
	_radioRecSeries = GUI->Control_getRadioButton(_window, RADIO_BUTTON_SERIES);
	_radioRecSeries->SetText(XBMC->GetLocalizedString(30102));
	
	// if episode is not part of a series, disable series radio button
	_window->SetProperty("isSeries", (_isSeries) ? "true" : "false");
	_radioRecSeries->SetSelected(_recSeries);  

	// force pre/post pad radio buttons
	_radioForcePrePad = GUI->Control_getRadioButton(_window, RADIO_FORCE_PREPAD);
	_radioForcePrePad->SetSelected(_isPrePadForced);
	_radioForcePostPad = GUI->Control_getRadioButton(_window, RADIO_FORCE_POSTPAD);
	_radioForcePostPad->SetSelected(_isPostPadForced);

	// init the prioriy spinner
	_spinPriority = GUI->Control_getSpin(_window, SPIN_CONTROL_PRIORITY);
	_spinPriority->Clear();		// clear out old values (for when set defaults call this method)
	_spinPriority->SetText(XBMC->GetLocalizedString(30142));		// set spinner text 
	for (int i=0; i<_priorityValues.size(); i++)
	{
		str = XBMC->GetLocalizedString(atoi(_priorityValues[i]));	// priority values contains string ids
		_spinPriority->AddLabel(str, atoi(_priorityValues[i]));		// set spinner label
	}
	_spinPriority->SetValue(_priorityValue);		// set spinner default
	
	// init pre-padding spinner
	_spinPrePadding = GUI->Control_getSpin(_window, SPIN_CONTROL_PREPAD);
	_spinPrePadding->Clear();		// clear out old values (for when set defaults call this method)
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
		_spinPrePadding->AddLabel(str, minutes);			// set spinner label
	}
	_spinPrePadding->SetValue(_preDefValue);		// set spinner default
	
	// init post-padding spinner
	_spinPostPadding = GUI->Control_getSpin(_window, SPIN_CONTROL_POSTPAD);
	_spinPostPadding->Clear();		// clear out old values (for when set defaults call this method)
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
		_spinPostPadding->AddLabel(str, minutes);			// set spinner label 
	}
	_spinPostPadding->SetValue(_postDefValue);		// set spinner default 

	// keep length spinner
	_spinKeepLength = GUI->Control_getSpin(_window, SPIN_CONTROL_KEEPLENGTH);
	_spinKeepLength->Clear();		// clear out old values (for when set defaults call this method)
	_spinKeepLength->SetText(XBMC->GetLocalizedString(30133));		// set spinner text 
	for (int i=0; i<_keepLengths.size(); i++)
	{
		_spinKeepLength->AddLabel(XBMC->GetLocalizedString(30134 + i),  atoi(_keepLengths[i]));			// set spinner labels
	}
	_spinKeepLength->SetValue(_keepLengthValue);		// set spinner default 

	
	// max episode spinner
	_spinMaxEpisode = GUI->Control_getSpin(_window, SPIN_CONTROL_MAXEPISODE);
	_spinMaxEpisode->Clear();		// clear out old values (for when set defaults call this method)
	_spinMaxEpisode->SetText(XBMC->GetLocalizedString(30138));				// set spinner text 
	for (int i=0; i<_maxEpisodes.size(); i++)
	{
		if (_maxEpisodes[i] == "1")
			str.Format(XBMC->GetLocalizedString(30139), _maxEpisodes[i]);
		else if (_maxEpisodes[i] == "-1")
			str = XBMC->GetLocalizedString(30141);
		else 
			str.Format(XBMC->GetLocalizedString(30140), _maxEpisodes[i]);

		_spinMaxEpisode->AddLabel(str,  atoi(_maxEpisodes[i]));		// set spinner labels
	}
	_spinMaxEpisode->SetValue(_maxEpisodeValue);					// set spinner default 


	// init runtype spin control
	_spinRunType = GUI->Control_getSpin(_window, SPIN_CONTROLRunType);
	_spinRunType->Clear();		// clear out old values (for when set defaults call this method)
	_spinRunType->SetText(XBMC->GetLocalizedString(30103));		// set spinner text (show type)
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30104), 0); // any show type
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30105), 1);	// first run only
	_spinRunType->AddLabel(XBMC->GetLocalizedString(30106), 2);	// live only
	_spinRunType->SetValue(_runType);

	// init channel spin control
	_spinChannel = GUI->Control_getSpin(_window, SPIN_CONTROL_CHANNEL);
	_spinChannel->Clear();		// clear out old values (for when set defaults call this method)
	_spinChannel->SetText(XBMC->GetLocalizedString(30107));		// set spinner text (channel)
	_spinChannel->AddLabel(_currentChannel.c_str(), 0);			// add current channel
	_spinChannel->AddLabel(XBMC->GetLocalizedString(30108), 1);	// add "Any channel"
	_spinChannel->SetValue(_anyChannel ? 1:0);
	
	// init airtime spin control
	_spinAirTime = GUI->Control_getSpin(_window, SPIN_CONTROL_AIRTIME);
	_spinAirTime->Clear();		// clear out old values (for when set defaults call this method)
	_spinAirTime->SetText(XBMC->GetLocalizedString(30110));		// set spinner text (airtime)
	_spinAirTime->AddLabel(_currentAirTime.c_str(), 0);			// current air time
	_spinAirTime->AddLabel(XBMC->GetLocalizedString(30111), 1);	// "Anytime" 
	_spinAirTime->SetValue(_anyTime ? 1:0);

	// init days of week radio buttons
	_radioAnyDay = GUI->Control_getRadioButton(_window, DAYS_CONTROLS_ANY);
	_radioRecDay = GUI->Control_getRadioButton(_window, DAYS_CONTROLS_START + _recDay);	// get the control that corresponds to the recording Day
	SetDaysOfWeek(_days);
	
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
		case BUTTON_OK:					// _confirmed previously set to 1, then FALLS THROUGH TO CANCEL
		case BUTTON_CANCEL:
		case BUTTON_CLOSE:
			if (_confirmed == -1)		// if not previously set to 1, set to cancel value
				_confirmed = 0;			
			_window->Close();
			break;
		case BUTTON_DEFAULTS:
			SetDefaults();
			break;
		case RADIO_BUTTON_SERIES:
			_recSeries = _radioRecSeries->IsSelected();
			break;
		case DAYS_CONTROLS_ANY:
			if (_radioAnyDay->IsSelected())			// if 'any day' was set true, turn on all day radio buttons
				DaysOfWeekSetSelected(true);
			else
			{
				DaysOfWeekSetSelected(false);		// user turned off 'any day' so turn off all the day button, except for 
				_radioRecDay->SetSelected(true);	// the day the show is aired (since it makes no sense to have all days turned off)
			}
			break;
	}
	// handle invidual day presses here
	if (controlId >= DAYS_CONTROLS_START && controlId < DAYS_CONTROLS_ANY)							// if a day radio button was clicked
	{
		CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, controlId);				// get the button clicked

		if (!radio->IsSelected())					// if the day was deselected
		{
			if (_radioAnyDay->IsSelected())			// make 'any day' deselected too
				_radioAnyDay->SetSelected(false);
		}
		else										// a day was selected
		{
			// see if all days are now selected, if they are then select 'Any day' too
			bool allDaysSelected = true;
			for (int i = DAYS_CONTROLS_START; i<DAYS_CONTROLS_ANY; i++)
			{
				// check if all days are now selected
				CAddonGUIRadioButton *radio = GUI->Control_getRadioButton(_window, i);
				if (!radio->IsSelected())
				{
					allDaysSelected = false;
					break;
				}
			}
			_radioAnyDay->SetSelected(allDaysSelected);			// all days are selected, so set 'Any day' true
		}

	}

  return true;
}

// return values of controls to caller (this is how caller gets result if dialog was confirmed)
bool CDialogRecordPref2::GetRecSeries() { return _radioRecSeries->IsSelected(); }
int CDialogRecordPref2::GetRunType() { return _spinRunType->GetValue(); };
bool CDialogRecordPref2::GetAnyChannel() { return _spinChannel->GetValue() == 1; };
bool CDialogRecordPref2::GetAnyTime() { return _spinAirTime->GetValue() == 1; };
int CDialogRecordPref2::GetPrePadding() { return _spinPrePadding->GetValue(); }
int CDialogRecordPref2::GetPostPadding() { return _spinPostPadding->GetValue(); }
bool CDialogRecordPref2::GetForcePrePad() { return _radioForcePrePad->IsSelected(); };
bool CDialogRecordPref2::GetForcePostPad() { return _radioForcePostPad->IsSelected(); };
int CDialogRecordPref2::GetKeepLength() { return _spinKeepLength->GetValue(); };
int CDialogRecordPref2::GetMaxEpisode() { return _spinMaxEpisode->GetValue(); };
int CDialogRecordPref2::GetPriority() { return _spinPriority->GetValue(); };

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
