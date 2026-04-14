//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
 //========= Copyright � 1996-2003, Valve LLC, All rights reserved. ============
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================


#include <stdio.h>
#define PROTECTED_THINGS_DISABLE

#include "utldict.h"

#include <vgui/KeyCode.h>
#include <vgui/Cursor.h>
#include <vgui/MouseCode.h>
#include <KeyValues.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>

#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageBox.h>
#include "FileSystem.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui2;

//-----------------------------------------------------------------------------
// Handle table
//-----------------------------------------------------------------------------
IMPLEMENT_HANDLES( BuildGroup, 20 )


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
BuildGroup::BuildGroup(Panel *parentPanel, Panel *contextPanel)
{
	CONSTRUCT_HANDLE( );

	_enabled=false;
	_snapX=1;
	_snapY=1;
	_cursor_sizenwse = dc_sizenwse;
	_cursor_sizenesw = dc_sizenesw;
	_cursor_sizewe = dc_sizewe;
	_cursor_sizens = dc_sizens;
	_cursor_sizeall = dc_sizeall;
	_currentPanel = nullptr;
	_dragging=false;
	m_pResourceName=NULL;
	m_pResourcePathID = NULL;
	m_hBuildDialog=NULL;
	m_pParentPanel=parentPanel;
	for (int i=0; i<4; ++i)
		_rulerNumber[i] = NULL;
	SetContextPanel(contextPanel);
	_showRulers = false;

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
BuildGroup::~BuildGroup()
{
	if (m_hBuildDialog)
		delete m_hBuildDialog.Get();
	m_hBuildDialog = NULL;

	delete [] m_pResourceName;
	delete [] m_pResourcePathID;

	for (int i=0; i <4; ++i)
	{
		if (_rulerNumber[i])
		{
			delete _rulerNumber[i];
			_rulerNumber[i]= NULL;
		}
	}
	
	DESTRUCT_HANDLE();
}

//-----------------------------------------------------------------------------
// Purpose: Toggles build mode on/off
// Input  : state - new state
//-----------------------------------------------------------------------------
void BuildGroup::SetEnabled(bool state)
{
	if(_enabled != state)
	{
		_enabled = state;
		_currentPanel = NULL;

		if ( state )
		{
			ActivateBuildDialog();
		}
		else
		{
			// hide the build dialog
			if ( m_hBuildDialog )
			{
				m_hBuildDialog->OnCommand("Close");
			}

			// request focus for our main panel
			m_pParentPanel->RequestFocus();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check if buildgroup is enabled
//-----------------------------------------------------------------------------
bool BuildGroup::IsEnabled()
{
	return _enabled;
}

//-----------------------------------------------------------------------------
// Purpose: Get the list of panels that are currently selected
//-----------------------------------------------------------------------------
CUtlVector<PHandle> *BuildGroup::GetControlGroup()
{
   return &_controlGroup;
}

//-----------------------------------------------------------------------------
// Purpose:	Check if ruler display is activated
//-----------------------------------------------------------------------------
bool BuildGroup::HasRulersOn()
{
   return _showRulers;
}

//-----------------------------------------------------------------------------
// Purpose:	Toggle ruler display 
//-----------------------------------------------------------------------------
void BuildGroup::ToggleRulerDisplay()
{
	_showRulers = !_showRulers;

	if (_rulerNumber[0] == NULL) // rulers haven't been initialized
	{
		_rulerNumber[0] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[1] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[2] = new Label(m_pBuildContext, NULL, "");
		_rulerNumber[3] = new Label(m_pBuildContext, NULL, "");
	}
    SetRulerLabelsVisible(_showRulers);

   m_pBuildContext->Repaint();
}


//-----------------------------------------------------------------------------
// Purpose:	Tobble visibility of ruler number labels
//-----------------------------------------------------------------------------
void BuildGroup::SetRulerLabelsVisible(bool state)
{
	_rulerNumber[0]->SetVisible(state);
	_rulerNumber[1]->SetVisible(state);
	_rulerNumber[2]->SetVisible(state);
	_rulerNumber[3]->SetVisible(state);
}

void BuildGroup::ApplySchemeSettings( IScheme *pScheme )
{
	DrawRulers();
}

//-----------------------------------------------------------------------------
// Purpose:	Draw Rulers on screen if conditions are right
//-----------------------------------------------------------------------------
void BuildGroup::DrawRulers()
{		
	// don't draw if visibility is off
	if (!_showRulers)
	{
		return;
	}
	
	// no drawing if we selected the context panel
	if (m_pBuildContext == _currentPanel)
	{
		SetRulerLabelsVisible(false);
		return;
	}
	else
		SetRulerLabelsVisible(true);
	
	int x, y, wide, tall;
	// get base panel's postition
	m_pBuildContext->GetBounds(x, y, wide, tall);
	m_pBuildContext->ScreenToLocal(x,y);
	
	int cx, cy, cwide, ctall;
	_currentPanel->GetBounds (cx, cy, cwide, ctall);
	
	surface()->PushMakeCurrent(m_pBuildContext->GetVPanel(), false);	
	
	// draw rulers
	surface()->DrawSetColor(255, 255, 255, 255);	// white color
	
	surface()->DrawFilledRect(0, cy, cx, cy+1);           //top horiz left
	surface()->DrawFilledRect(cx+cwide, cy, wide, cy+1);  //top horiz right
	
	surface()->DrawFilledRect(0, cy+ctall-1, cx, cy+ctall);   //bottom horiz left
	surface()->DrawFilledRect(cx+cwide, cy+ctall-1, wide, cy+ctall);   //bottom	 horiz right
	
	surface()->DrawFilledRect(cx,0,cx+1,cy);         //top vert left
	surface()->DrawFilledRect(cx+cwide-1,0, cx+cwide, cy);  //top vert right
	
	surface()->DrawFilledRect(cx,cy+ctall, cx+1, tall); //bottom vert left
	surface()->DrawFilledRect(cx+cwide-1, cy+ctall, cx+cwide, tall); //bottom vert right   
	
	surface()->PopMakeCurrent(m_pBuildContext->GetVPanel());
	
	// now let's put numbers with the rulers
	char textstring[20];
	Q_snprintf (textstring, sizeof( textstring ), "%d", cx);
	_rulerNumber[0]->SetText(textstring);
	int twide, ttall;
	_rulerNumber[0]->GetContentSize(twide,ttall);
	_rulerNumber[0]->SetSize(twide,ttall);
	_rulerNumber[0]->SetPos(cx/2-twide/2, cy-ttall+3);
	
	Q_snprintf (textstring, sizeof( textstring ), "%d", cy);
	_rulerNumber[1]->SetText(textstring);
	_rulerNumber[1]->GetContentSize(twide,ttall);
	_rulerNumber[1]->SetSize(twide,ttall);
	_rulerNumber[1]->GetSize(twide,ttall);
	_rulerNumber[1]->SetPos(cx-twide + 3, cy/2-ttall/2);
	
	Q_snprintf (textstring, sizeof( textstring ), "%d", cy);
	_rulerNumber[2]->SetText(textstring);
	_rulerNumber[2]->GetContentSize(twide,ttall);
	_rulerNumber[2]->SetSize(twide,ttall);
	_rulerNumber[2]->SetPos(cx+cwide+(wide-cx-cwide)/2 - twide/2,  cy+ctall-3);
	
	Q_snprintf (textstring, sizeof( textstring ), "%d", cy);
	_rulerNumber[3]->SetText(textstring);
	_rulerNumber[3]->GetContentSize(twide,ttall);
	_rulerNumber[3]->SetSize(twide,ttall);
	_rulerNumber[3]->SetPos(cx+cwide, cy+ctall+(tall-cy-ctall)/2 - ttall/2);
	
}

//-----------------------------------------------------------------------------
// Purpose: respond to cursor movments
//-----------------------------------------------------------------------------
bool BuildGroup::CursorMoved(int x, int y, Panel *panel)
{
	Assert(panel);

	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->CursorMoved( x, y, panel );
				}
			}
		}
		return false;
	}

	// no moving uneditable panels
	// commented out because this has issues with panels moving 
	// to front and obscuring other panels
	//if (!panel->IsBuildModeEditable())
	//	return;

	if (_dragging)
	{
		input()->GetCursorPos(x, y);
		
		if (_dragMouseCode == MOUSE_RIGHT)
		{
			int newW = max( 1, _dragStartPanelSize[ 0 ] + x - _dragStartCursorPos[0] );
			int newH = max( 1, _dragStartPanelSize[ 1 ] + y - _dragStartCursorPos[1] );

			bool shift = ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) );
			bool ctrl = ( input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL) );

			if ( shift )
			{
				newW = _dragStartPanelSize[ 0 ];
			}
			if ( ctrl )
			{
				newH = _dragStartPanelSize[ 1 ];
			}

			panel->SetSize( newW, newH );
			ApplySnap(panel);
		}
		else
		{
			for (int i=0; i < _controlGroup.Count(); ++i)
			{
				// now fix offset of member panels with respect to the one we are dragging
				Panel *groupMember = _controlGroup[i].Get();
			   	groupMember->SetPos(_dragStartPanelPos[0] + _groupDeltaX[i] +(x-_dragStartCursorPos[0]), _dragStartPanelPos[1] + _groupDeltaY[i] +(y-_dragStartCursorPos[1]));
				ApplySnap(groupMember);				
			}
		}

		// update the build dialog
		if (m_hBuildDialog)
		{
			KeyValues *keyval = new KeyValues("UpdateControlData");
			keyval->SetPtr("panel", GetCurrentPanel());
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);

			keyval = new KeyValues("EnableSaveButton");	
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);	
		}
		
		panel->Repaint();
		panel->CallParentFunction(new KeyValues("Repaint"));
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool BuildGroup::MousePressed(MouseCode code, Panel *panel)
{
	Assert(panel);

	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->MousePressed( code, panel );
				}
			}
		}
		return false;
	}

	// if people click on the base build dialog panel.
	if (panel == m_hBuildDialog)
	{
		// hide the click menu if its up
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("HideNewControlMenu"), NULL);
		return true;
	}

	// don't select unnamed items
	if (strlen(panel->GetName()) < 1)
		return true;
	
	bool shift = ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) );	
	if (!shift)
	{
		_controlGroup.RemoveAll();	
	}

	// Show new ctrl menu if they click on the bg (not on a subcontrol)
	if ( code == MOUSE_RIGHT && panel == GetContextPanel())
	{		
		// trigger a drop down menu to create new controls
		ivgui()->PostMessage (m_hBuildDialog->GetVPanel(), new KeyValues("ShowNewControlMenu"), NULL);	
	}	
	else
	{	
		// don't respond if we click on ruler numbers
		if (_showRulers) // rulers are visible
		{
			for ( int i=0; i < 4; i++)
			{
				if ( panel == _rulerNumber[i])
					return true;
			}
		}

		_dragging = true;
		_dragMouseCode = code;
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("HideNewControlMenu"), NULL);
		
		int x, y;
		input()->GetCursorPos(x, y);
		
		_dragStartCursorPos[0] = x;
		_dragStartCursorPos[1] = y;
	
		
		input()->SetMouseCapture(panel->GetVPanel());
		
		_groupDeltaX.RemoveAll();
		_groupDeltaY.RemoveAll();

		// basepanel is the panel that all the deltas will be calculated from.
		// it is the last panel we clicked in because if we move the panels  as a group
		// it would be from that one
		Panel *basePanel = NULL;
		// find the panel we clicked in, that is the base panel
		// it might already be in the group
		for (int i=0; i< _controlGroup.Count(); ++i)	
		{
			if (panel == _controlGroup[i].Get())
			{
				basePanel = panel;
				break;
			}
		}

		// if its not in the group we just added this panel. get it in the group 
		if (basePanel == NULL)
		{
			PHandle temp;
			temp = panel;
			_controlGroup.AddToTail(temp);
			basePanel = panel;
		}
		
		basePanel->GetPos(x,y);
		_dragStartPanelPos[0]=x;
		_dragStartPanelPos[1]=y;

		basePanel->GetSize( _dragStartPanelSize[ 0 ], _dragStartPanelSize[ 1 ] );

		// figure out the deltas of the other panels from the base panel
		for (int i=0; i<_controlGroup.Count(); ++i)
		{
			int cx, cy;
			_controlGroup[i].Get()->GetPos(cx, cy);
			_groupDeltaX.AddToTail(cx - x);
			_groupDeltaY.AddToTail(cy - y);
		}
						
		// if this panel wasn't already selected update the buildmode dialog controls to show its info
		if(_currentPanel != panel)
		{			
			_currentPanel = panel;
			
			if ( m_hBuildDialog )
			{
				// think this is taken care of by SetActiveControl.
				//ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("ApplyDataToControls"), NULL);
				
				KeyValues *keyval = new KeyValues("SetActiveControl");
				keyval->SetPtr("PanelPtr", GetCurrentPanel());
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);
			}		
		}		

		// store undo information upon panel selection.
		ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("StoreUndo"), NULL);

		panel->RequestFocus();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool BuildGroup::MouseReleased(MouseCode code, Panel *panel)
{
	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->MouseReleased( code, panel );
				}
			}
		}
		return false;
	}

	Assert(panel);

	_dragging=false;
	input()->SetMouseCapture(NULL_HANDLE);
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool BuildGroup::MouseDoublePressed(MouseCode code, Panel *panel)
{
	Assert(panel);
	return MousePressed( code, panel );
}

bool BuildGroup::KeyTyped( wchar_t unichar, Panel *panel )
{
	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->KeyTyped( unichar, panel );
				}
			}
		}
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool BuildGroup::KeyCodeTyped(KeyCode code, Panel *panel)
{
	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->KeyCodeTyped( code, panel );
				}
			}
		}
		return false;
	}

	Assert(panel);

	int dx=0;
	int dy=0;

	bool shift = ( input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT) );
	bool ctrl = ( input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL) );
	bool alt = (input()->IsKeyDown(KEY_LALT) || input()->IsKeyDown(KEY_RALT));

	
	if ( ctrl && shift && alt && code == KEY_B)
	{
		// enable build mode
		EditablePanel *ep = dynamic_cast< EditablePanel * >( panel );
		if ( ep )
		{
			ep->ActivateBuildMode();
		}
		return true;
	}

	switch (code)
	{
		case KEY_LEFT:
		{
			dx-=_snapX;
			break;
		}
		case KEY_RIGHT:
		{
			dx+=_snapX;
			break;
		}
		case KEY_UP:
		{
			dy-=_snapY;
			break;
		}
		case KEY_DOWN:
		{
			dy+=_snapY;
			break;
		}
		case KEY_DELETE:
		{
			// delete the panel we have selected 
			ivgui()->PostMessage (m_hBuildDialog->GetVPanel(), new KeyValues ("DeletePanel"), NULL);
			break;
		}

	}

	if (ctrl)
	{
		switch (code)
		{
		case KEY_Z:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Undo"), NULL);
				break;
			}

		case KEY_C:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Copy"), NULL);
				break;
			}
		case KEY_V:
			{
				ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("Paste"), NULL);
				break;
			}
		}
	}

	if(dx||dy)
	{
		//TODO: make this stuff actually snap

		int x,y,wide,tall;

		panel->GetBounds(x,y,wide,tall);

		if(shift)
		{
			panel->SetSize(wide+dx,tall+dy);
		}
		else
		{
			panel->SetPos(x+dx,y+dy);
		}

		ApplySnap(panel);

		panel->Repaint();
		if (panel->GetVParent() != 0)
		{
			panel->PostMessage(panel->GetVParent(), new KeyValues("Repaint"));
		}


		// update the build dialog
		if (m_hBuildDialog)
		{
			// post that it's active
			KeyValues *keyval = new KeyValues("SetActiveControl");
			keyval->SetPtr("PanelPtr", GetCurrentPanel());
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);

			// post that it's been changed
			ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), new KeyValues("PanelMoved"), NULL);
		}
	}

	// If holding key while dragging, simulate moving cursor so shift/ctrl key changes take effect
	if ( _dragging && panel != GetContextPanel() )
	{
		int x, y;
		input()->GetCursorPos( x, y );
		CursorMoved( x, y, panel );
	}

	return true;
}

bool BuildGroup::KeyCodeReleased(KeyCode code, Panel *panel )
{
	if ( !m_hBuildDialog.Get() )
	{
		if ( panel->GetParent() )
		{
			EditablePanel *ep = dynamic_cast< EditablePanel * >( panel->GetParent() );
			if ( ep )
			{
				BuildGroup *bg = ep->GetBuildGroup();
				if ( bg && bg != this )
				{
					bg->KeyCodeTyped( code, panel );
				}
			}
		}
		return false;
	}

	// If holding key while dragging, simulate moving cursor so shift/ctrl key changes take effect
	if ( _dragging && panel != GetContextPanel() )
	{
		int x, y;
		input()->GetCursorPos( x, y );
		CursorMoved( x, y, panel );
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Searches for a BuildModeDialog in the hierarchy
//-----------------------------------------------------------------------------
Panel *BuildGroup::CreateBuildDialog( void )
{
	// request the panel
	Panel *buildDialog = NULL;
	KeyValues *data = new KeyValues("BuildDialog");
	data->SetPtr("BuildGroupPtr", this);
	if (m_pBuildContext->RequestInfo(data))
	{
		buildDialog = (Panel *)data->GetPtr("PanelPtr");
	}

	// initialize the build dialog if found
	if ( buildDialog )
	{
		input()->ReleaseAppModalSurface();
	}

	return buildDialog;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the build mode settings dialog
//-----------------------------------------------------------------------------
void BuildGroup::ActivateBuildDialog( void )
{
	// create the build mode dialog first time through
	if (!m_hBuildDialog.Get())
	{
		m_hBuildDialog = CreateBuildDialog();

		if (!m_hBuildDialog.Get())
			return;
	}

	m_hBuildDialog->SetVisible( true );

	// send a message to set the initial dialog controls info
	_currentPanel = m_pParentPanel;
	KeyValues *keyval = new KeyValues("SetActiveControl");
	keyval->SetPtr("PanelPtr", GetCurrentPanel());
	ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
HCursor BuildGroup::GetCursor(Panel *panel)
{
	Assert(panel);
	
	int x,y,wide,tall;
	input()->GetCursorPos(x,y);
	panel->ScreenToLocal(x,y);
	panel->GetSize(wide,tall);

	if(x < 2)
	{
		if(y < 4)
		{
			return _cursor_sizenwse;
		}
		else
		if(y<(tall-4))
		{
			return _cursor_sizewe;
		}
		else
		{
			return _cursor_sizenesw;
		}
	}

	return _cursor_sizeall;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void BuildGroup::ApplySnap(Panel *panel)
{
	Assert(panel);
	
	int x,y,wide,tall;
	panel->GetBounds(x,y,wide,tall);

	x=(x/_snapX)*_snapX;
	y=(y/_snapY)*_snapY;
	panel->SetPos(x,y);
	
	int xx,yy;
	xx=x+wide;
	yy=y+tall;
	
	xx=(xx/_snapX)*_snapX;
	yy=(yy/_snapY)*_snapY;
	panel->SetSize(xx-x,yy-y);
}

//-----------------------------------------------------------------------------
// Purpose:	Return the currently selected panel
//-----------------------------------------------------------------------------
Panel *BuildGroup::GetCurrentPanel()
{
	return _currentPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Add panel the list of panels that are in the build group
//-----------------------------------------------------------------------------
void BuildGroup::PanelAdded(Panel *panel)
{
	Assert(panel);

	PHandle temp;
	temp = panel;
	int c = _panelDar.Count();
	for ( int i = 0; i < c; ++i )
	{
		if ( _panelDar[ i ] == temp )
		{
			return;
		}
	}
	_panelDar.AddToTail(temp);
}

//-----------------------------------------------------------------------------
// Purpose: loads the control settings from file
//-----------------------------------------------------------------------------
void BuildGroup::LoadControlSettings(const char *controlResourceName, const char *pathID, KeyValues *pPreloadedKeyValues, KeyValues *pConditions)
{
#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings entry resource='%s' pathID='%s' parent=%p preloaded=%p\n",
		controlResourceName ? controlResourceName : "<null>", pathID ? pathID : "<null>",
		(void *)m_pParentPanel, (void *)pPreloadedKeyValues );
#endif
	// make sure the file is registered
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings before RegisterControlSettingsFile resource='%s'\n",
		controlResourceName ? controlResourceName : "<null>" );
	#endif
	RegisterControlSettingsFile(controlResourceName, pathID);
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings after RegisterControlSettingsFile resource='%s'\n",
		controlResourceName ? controlResourceName : "<null>" );
	#endif

	// Use the keyvalues they passed in or load them.
	KeyValues *rDat = pPreloadedKeyValues;
	if ( !rDat )
	{
		// load the resource data from the file
		rDat  = new KeyValues(controlResourceName);

		// check the skins directory first, if an explicit pathID hasn't been set
		bool bSuccess = false;
		if (!pathID)
		{
			bSuccess = rDat->LoadFromFile(g_pFullFileSystem, controlResourceName, "SKIN");
		}
		if (!bSuccess)
		{
			bSuccess = rDat->LoadFromFile(g_pFullFileSystem, controlResourceName, pathID);
		}

#ifdef __EMSCRIPTEN__
		printf("[BG] after LoadFromFile success=%d name='%s' firstSubKey='%s' firstSubSubKey='%s' resource='%s' rDat=%p\n",
			bSuccess ? 1 : 0,
			rDat && rDat->GetName() ? rDat->GetName() : "<null>",
			(rDat && rDat->GetFirstSubKey() && rDat->GetFirstSubKey()->GetName()) ? rDat->GetFirstSubKey()->GetName() : "<null>",
			(rDat && rDat->GetFirstSubKey() && rDat->GetFirstSubKey()->GetFirstSubKey() && rDat->GetFirstSubKey()->GetFirstSubKey()->GetName())
				? rDat->GetFirstSubKey()->GetFirstSubKey()->GetName()
				: "<null>",
			controlResourceName ? controlResourceName : "<null>",
			(void *)rDat);
#endif

		// GoldSrc: Maybe posible to port?
#if 0
		if ( bSuccess )
		{
			if ( IsX360() )
			{
				rDat->ProcessResolutionKeys( surface()->GetResolutionKey() );
			}
			if ( IsPC() )
			{
				ConVarRef cl_hud_minmode( "cl_hud_minmode", true );
				if ( cl_hud_minmode.IsValid() && cl_hud_minmode.GetBool() )
				{
					rDat->ProcessResolutionKeys( "_minmode" );
				}
			}

			if ( pConditions && pConditions->GetFirstSubKey() )
			{
				ProcessConditionalKeys( rDat, pConditions );			
			}
		}
#endif
	}
	#ifdef __EMSCRIPTEN__
	else
	{
		printf( "[BUILDGROUP] LoadControlSettings using preloaded KeyValues resource='%s' rDat=%p firstSubKey=%p\n",
			controlResourceName ? controlResourceName : "<null>",
			(void *)rDat,
			rDat ? (void *)rDat->GetFirstSubKey() : NULL );
	}
	#endif

	// save off the resource name
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings before resource-name copy resource='%s'\n",
		controlResourceName ? controlResourceName : "<null>" );
	#endif
	delete [] m_pResourceName;
	m_pResourceName = new char[strlen(controlResourceName) + 1];
	strcpy(m_pResourceName, controlResourceName);

	if (pathID)
	{
		delete [] m_pResourcePathID;
		m_pResourcePathID = new char[strlen(pathID) + 1];
		strcpy(m_pResourcePathID, pathID);
	}

	// delete any controls not in both files
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings before DeleteAllControlsCreatedByControlSettingsFile resource='%s'\n",
		controlResourceName ? controlResourceName : "<null>" );
	#endif
	DeleteAllControlsCreatedByControlSettingsFile();
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings after DeleteAllControlsCreatedByControlSettingsFile resource='%s'\n",
		controlResourceName ? controlResourceName : "<null>" );
	#endif

	// loop through the resource data sticking info into controls
#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings before ApplySettings resource='%s' firstSubKey=%p\n",
		controlResourceName ? controlResourceName : "<null>", rDat ? (void *)rDat->GetFirstSubKey() : NULL );
#endif
	ApplySettings(rDat);
	#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings after ApplySettings resource='%s' parent=%p\n",
		controlResourceName ? controlResourceName : "<null>", (void *)m_pParentPanel );
	#endif

	if (m_pParentPanel)
	{
		#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] LoadControlSettings before parent invalidate/repaint resource='%s' parent=%p\n",
			controlResourceName ? controlResourceName : "<null>", (void *)m_pParentPanel );
		#endif
		m_pParentPanel->InvalidateLayout();
		m_pParentPanel->Repaint();
		#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] LoadControlSettings after parent invalidate/repaint resource='%s' parent=%p\n",
			controlResourceName ? controlResourceName : "<null>", (void *)m_pParentPanel );
		#endif
	}

	if ( rDat != pPreloadedKeyValues )
	{
		#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] LoadControlSettings before deleteThis resource='%s' rDat=%p\n",
			controlResourceName ? controlResourceName : "<null>", (void *)rDat );
		#endif
		rDat->deleteThis();
		#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] LoadControlSettings after deleteThis resource='%s'\n",
			controlResourceName ? controlResourceName : "<null>" );
		#endif
	}

#ifdef __EMSCRIPTEN__
	printf( "[BUILDGROUP] LoadControlSettings exit resource='%s' parent=%p\n",
		controlResourceName ? controlResourceName : "<null>", (void *)m_pParentPanel );
#endif
}

void BuildGroup::ProcessConditionalKeys( KeyValues *pData, KeyValues *pConditions )
{
	// for each condition, look for it in keys
	// if its a positive condition, promote all of its children, replacing values

	if ( pData )
	{
		KeyValues *pSubKey = pData->GetFirstSubKey();
		if ( !pSubKey )
		{
			// not a block
			return;
		}

		for ( ; pSubKey != NULL; pSubKey = pSubKey->GetNextKey() )
		{
			// recursively descend each sub block
			ProcessConditionalKeys( pSubKey, pConditions );

			KeyValues *pCondition = pConditions->GetFirstSubKey();
			for ( ; pCondition != NULL; pCondition = pCondition->GetNextKey() )
			{
				// if we match any conditions in this sub block, copy up
				KeyValues *pConditionBlock = pSubKey->FindKey( pCondition->GetName() );
				if ( pConditionBlock )
				{
					KeyValues *pOverridingKey;
					for ( pOverridingKey = pConditionBlock->GetFirstSubKey(); pOverridingKey != NULL; pOverridingKey = pOverridingKey->GetNextKey() )
					{
						KeyValues *pExistingKey = pSubKey->FindKey( pOverridingKey->GetName() );
						if ( pExistingKey )
						{
							pExistingKey->SetStringValue( pOverridingKey->GetString() );
						}
						else
						{
							KeyValues *copy = pOverridingKey->MakeCopy();
							pSubKey->AddSubKey( copy );
						}
					}				
				}
			}			
		}		
	}
}

//-----------------------------------------------------------------------------
// Purpose: registers that a control settings file may be loaded
//			use when the dialog may have multiple states and the editor will need to be able to switch between them
//-----------------------------------------------------------------------------
void BuildGroup::RegisterControlSettingsFile(const char *controlResourceName, const char *pathID)
{
	// add the file into a list for build mode
	CUtlSymbol sym(controlResourceName);
	if (!m_RegisteredControlSettingsFiles.IsValidIndex(m_RegisteredControlSettingsFiles.Find(sym)))
	{
		m_RegisteredControlSettingsFiles.AddToTail(sym);
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor / iterator
//-----------------------------------------------------------------------------
int BuildGroup::GetRegisteredControlSettingsFileCount()
{
	return m_RegisteredControlSettingsFiles.Count();
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *BuildGroup::GetRegisteredControlSettingsFileByIndex(int index)
{
	return m_RegisteredControlSettingsFiles[index].String();
}

//-----------------------------------------------------------------------------
// Purpose: reloads the control settings from file
//-----------------------------------------------------------------------------
void BuildGroup::ReloadControlSettings()
{
	delete m_hBuildDialog.Get(); 
	m_hBuildDialog = NULL;

	// loop though objects in the current control group and remove them all
	// the 0th panel is always the contextPanel which is not deletable 
	for( int i = 1; i < _panelDar.Count(); i++ )
	{	
		if (!_panelDar[i].Get()) // this can happen if we had two of the same handle in the list
		{
			_panelDar.Remove(i);
			--i;
			continue;
		}
		
		// only delete deletable panels, as the only deletable panels
		// are the ones created using the resource file
		if ( _panelDar[i].Get()->IsBuildModeDeletable())
		{
			delete _panelDar[i].Get();
			_panelDar.Remove(i);
			--i;
		}		
	}	

	if (m_pResourceName)
	{
		EditablePanel *edit = dynamic_cast<EditablePanel *>(m_pParentPanel);
		if (edit)
		{
			edit->LoadControlSettings(m_pResourceName, m_pResourcePathID);
		}
		else
		{
			LoadControlSettings(m_pResourceName, m_pResourcePathID);
		}
	}

	_controlGroup.RemoveAll();	

	ActivateBuildDialog();	
}

//-----------------------------------------------------------------------------
// Purpose: changes which control settings are currently loaded
//-----------------------------------------------------------------------------
void BuildGroup::ChangeControlSettingsFile(const char *controlResourceName)
{
	// clear any current state
	_controlGroup.RemoveAll();
	_currentPanel = m_pParentPanel;

	// load the new state, via the dialog if possible
	EditablePanel *edit = dynamic_cast<EditablePanel *>(m_pParentPanel);
	if (edit)
	{
		edit->LoadControlSettings(controlResourceName, m_pResourcePathID);
	}
	else
	{
		LoadControlSettings(controlResourceName, m_pResourcePathID);
	}

	// force it to update
	KeyValues *keyval = new KeyValues("SetActiveControl");
	keyval->SetPtr("PanelPtr", GetCurrentPanel());
	ivgui()->PostMessage(m_hBuildDialog->GetVPanel(), keyval, NULL);
}

//-----------------------------------------------------------------------------
// Purpose: saves control settings to file
//-----------------------------------------------------------------------------
bool BuildGroup::SaveControlSettings( void )
{
	bool bSuccess = false;
	if ( m_pResourceName )
	{
		KeyValues *rDat = new KeyValues( m_pResourceName );

		// get the data from our controls
		GetSettings( rDat );

		// save the data out to a file
		bSuccess = rDat->SaveToFile( g_pFullFileSystem, m_pResourceName, m_pResourcePathID);
		if (!bSuccess)
		{
			MessageBox *dlg = new MessageBox("BuildMode - Error saving file", "Error: Could not save changes.  File is most likely read only.");
			dlg->DoModal();
		}

		rDat->deleteThis();
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Purpose: Deletes all the controls not created by the code
//-----------------------------------------------------------------------------
void BuildGroup::DeleteAllControlsCreatedByControlSettingsFile()
{
	// loop though objects in the current control group and remove them all
	// the 0th panel is always the contextPanel which is not deletable 
	for ( int i = 1; i < _panelDar.Count(); i++ )
	{	
		if (!_panelDar[i].Get()) // this can happen if we had two of the same handle in the list
		{
			_panelDar.Remove(i);
			--i;
			continue;
		}
		
		// only delete deletable panels, as the only deletable panels
		// are the ones created using the resource file
		if ( _panelDar[i].Get()->IsBuildModeDeletable())
		{
			delete _panelDar[i].Get();
			_panelDar.Remove(i);
			--i;
		}		
	}

	_currentPanel = m_pBuildContext;
	_currentPanel->InvalidateLayout();
    m_pBuildContext->Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: serializes settings from a resource data container
//-----------------------------------------------------------------------------
void BuildGroup::ApplySettings( KeyValues *resourceData )
{
#ifdef __EMSCRIPTEN__
	printf("[BUILDGROUP] ApplySettings ENTRY resourceData=%p name='%s' firstSubKey='%s' panelDar.Count=%d parent=%p parentName='%s' context=%p contextName='%s'\n",
		(void *)resourceData,
		(resourceData && resourceData->GetName()) ? resourceData->GetName() : "<null>",
		(resourceData && resourceData->GetFirstSubKey() && resourceData->GetFirstSubKey()->GetName()) ? resourceData->GetFirstSubKey()->GetName() : "<null>",
		_panelDar.Count(),
		(void *)m_pParentPanel,
		(m_pParentPanel && m_pParentPanel->GetName()) ? m_pParentPanel->GetName() : "<null>",
		(void *)m_pBuildContext,
		(m_pBuildContext && m_pBuildContext->GetName()) ? m_pBuildContext->GetName() : "<null>");

	for (int i = 0; i < _panelDar.Count(); ++i)
	{
		Panel *p = _panelDar[i].Get();
		printf("[BUILDGROUP] panelDar[%d] ptr=%p name='%s'\n",
			i,
			(void *)p,
			(p && p->GetName()) ? p->GetName() : "<null>");
	}
#endif

	// loop through all the keys, applying them wherever
	for (KeyValues *controlKeys = resourceData->GetFirstSubKey(); controlKeys != NULL; controlKeys = controlKeys->GetNextKey())
	{
		bool bFound = false;

		// Skip keys that are atomic..
		if (controlKeys->GetDataType() != KeyValues::TYPE_NONE)
			continue;

		char const *keyName = controlKeys->GetName();
#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] ApplySettings key='%s' control='%s'\n",
			keyName ? keyName : "<null>",
			controlKeys->GetString( "ControlName", "<none>" ) );
#endif

		// check to see if any buildgroup panels have this name
		for ( int i = 0; i < _panelDar.Count(); i++ )
		{
			Panel *panel = _panelDar[i].Get();

			if (!panel) // this can happen if we had two of the same handle in the list
			{
				_panelDar.Remove(i);
				--i;
				continue;
			}


			Assert (panel);

			// make the control name match CASE INSENSITIVE!
			char const *panelName = panel->GetName();

			if (!Q_stricmp(panelName, keyName))
			{
				// apply the settings
				panel->ApplySettings(controlKeys);
				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			// the key was not found in the registered list, check to see if we should create it
			if ( keyName /*controlKeys->GetInt("AlwaysCreate", false)*/ )
			{
				// create the control even though it wasn't registered
				NewControl( controlKeys );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create a new control in the context panel
// Input:	name: class name of control to create
//			controlKeys: keyvalues of settings for the panel.
//			name OR controlKeys should be set, not both.  
//			x,y position relative to base panel
// Output: Panel *newPanel, NULL if failed to create new control.
//-----------------------------------------------------------------------------
Panel *BuildGroup::NewControl( const char *name, int x, int y)
{
	Assert (name);
	
	Panel *newPanel = NULL;
	// returns NULL on failure
	newPanel = static_cast<EditablePanel *>(m_pParentPanel)->CreateControlByName(name);
	
	if (newPanel)
	{
		// panel successfully created
		newPanel->SetParent(m_pParentPanel);	
		newPanel->SetBuildGroup(this);
		newPanel->SetPos(x, y);

		char newFieldName[255];
		GetNewFieldName(newFieldName, sizeof(newFieldName), newPanel);
		newPanel->SetName(newFieldName);
		
		newPanel->AddActionSignalTarget(m_pParentPanel);
		newPanel->SetBuildModeEditable(true);
		newPanel->SetBuildModeDeletable(true);	
		
		// make sure it gets freed
		newPanel->SetAutoDelete(true);
	}	

	return newPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Create a new control in the context panel
// Input:	controlKeys: keyvalues of settings for the panel only works when applying initial settings.
// Output:	Panel *newPanel, NULL if failed to create new control.
//-----------------------------------------------------------------------------
Panel *BuildGroup::NewControl( KeyValues *controlKeys, int x, int y)
{
	Assert (controlKeys);
	
	Panel *newPanel = NULL;
	if (controlKeys)
	{
		printf("[BUILDGROUP] NewControl request field='%s' class='%s' x=%d y=%d parent=%p parentName='%s' context=%p contextName='%s'\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			controlKeys->GetString("ControlName", "<none>"),
			x,
			y,
			(void *)m_pParentPanel,
			m_pParentPanel && m_pParentPanel->GetName() ? m_pParentPanel->GetName() : "<null>",
			(void *)m_pBuildContext,
			m_pBuildContext && m_pBuildContext->GetName() ? m_pBuildContext->GetName() : "<null>");
//		Warning( "Creating new control \"%s\" of type \"%s\"\n", controlKeys->GetString( "fieldName" ), controlKeys->GetString( "ControlName" ) );
		KeyValues *keyVal = new KeyValues("ControlFactory", "ControlName", controlKeys->GetString("ControlName"));
		printf("[BUILDGROUP] sending RequestInfo name='%s' ControlName='%s' context=%p contextName='%s'\n",
			keyVal->GetName() ? keyVal->GetName() : "<null>",
			keyVal->GetString("ControlName", "<none>"),
			(void *)m_pBuildContext,
			m_pBuildContext && m_pBuildContext->GetName() ? m_pBuildContext->GetName() : "<null>");
		m_pBuildContext->RequestInfo(keyVal);
		// returns NULL on failure
		newPanel = (Panel *)keyVal->GetPtr("PanelPtr");
		printf("[BUILDGROUP] NewControl after RequestInfo field='%s' class='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			controlKeys->GetString("ControlName", "<none>"),
			(void *)newPanel);
		keyVal->deleteThis();
	}
	else
	{
		return NULL;
	}

	if (newPanel)
	{
#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] NewControl created field='%s' class='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			controlKeys->GetString("ControlName", "<none>"), (void *)newPanel );
#endif
		// panel successfully created
	#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] NewControl before SetParent field='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>", (void *)newPanel );
	#endif
		newPanel->SetParent(m_pParentPanel);	
	#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] NewControl after SetParent field='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>", (void *)newPanel );
	#endif
		newPanel->SetBuildGroup(this);
		newPanel->SetPos(x, y);

		newPanel->SetName(controlKeys->GetName()); // name before applysettings :)
#ifdef __EMSCRIPTEN__
		{
			unsigned char *rawPanel = reinterpret_cast<unsigned char *>(newPanel);
			void *fontOverridePtr = rawPanel ? *reinterpret_cast<void **>(rawPanel + 368) : NULL;
			unsigned char *fontBytes = rawPanel ? rawPanel + 368 : NULL;
			printf("[BUILDGROUP] NewControl pre-ApplySettings field='%s' class='%s' panel=%p _fontOverrideName=%p raw368=%02x %02x %02x %02x %02x %02x %02x %02x\n",
				controlKeys->GetName() ? controlKeys->GetName() : "<null>",
				controlKeys->GetString("ControlName", "<none>"),
				(void *)newPanel,
				fontOverridePtr,
				fontBytes ? fontBytes[0] : 0,
				fontBytes ? fontBytes[1] : 0,
				fontBytes ? fontBytes[2] : 0,
				fontBytes ? fontBytes[3] : 0,
				fontBytes ? fontBytes[4] : 0,
				fontBytes ? fontBytes[5] : 0,
				fontBytes ? fontBytes[6] : 0,
				fontBytes ? fontBytes[7] : 0);
		}
#endif
	#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] NewControl before ApplySettings field='%s' class='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			controlKeys->GetString("ControlName", "<none>"), (void *)newPanel );
	#endif
		newPanel->ApplySettings(controlKeys);
	#ifdef __EMSCRIPTEN__
		printf( "[BUILDGROUP] NewControl after ApplySettings field='%s' class='%s' panel=%p\n",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			controlKeys->GetString("ControlName", "<none>"), (void *)newPanel );
	#endif

		int xpos = 0, ypos = 0, wide = 0, tall = 0;
		newPanel->GetBounds(xpos, ypos, wide, tall);
		printf("[VGUI_RES] Created: %s \"%s\" at xpos=%d ypos=%d wide=%d tall=%d visible=%d enabled=%d parent=\"%s\" parentPtr=%p resource=\"%s\"\n",
			controlKeys->GetString("ControlName", "<none>"),
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			xpos, ypos, wide, tall,
			newPanel->IsVisible() ? 1 : 0,
			newPanel->IsEnabled() ? 1 : 0,
			(m_pParentPanel && m_pParentPanel->GetName()) ? m_pParentPanel->GetName() : "<null>",
			(void *)m_pParentPanel,
			m_pResourceName ? m_pResourceName : "<null>");

		newPanel->AddActionSignalTarget(m_pParentPanel);
		newPanel->SetBuildModeEditable(true);
		newPanel->SetBuildModeDeletable(true);	
		
		// make sure it gets freed
		newPanel->SetAutoDelete(true);
	}
	else
	{
		printf("[VGUI_RES] MISSING FACTORY: %s in %s - SKIPPED field=\"%s\" parent=\"%s\" parentPtr=%p\n",
			controlKeys->GetString("ControlName", "<none>"),
			m_pResourceName ? m_pResourceName : "<null>",
			controlKeys->GetName() ? controlKeys->GetName() : "<null>",
			(m_pParentPanel && m_pParentPanel->GetName()) ? m_pParentPanel->GetName() : "<null>",
			(void *)m_pParentPanel);
	}

	return newPanel;
}

//-----------------------------------------------------------------------------
// Purpose: Get a new unique fieldname for a new control
//-----------------------------------------------------------------------------
void BuildGroup::GetNewFieldName(char *newFieldName, int newFieldNameSize, Panel *newPanel)
{
	int fieldNameNumber=1;
	char defaultName[25];
	
	Q_strncpy( defaultName, newPanel->GetClassName(), sizeof( defaultName ) );

	while (1)
	{
		Q_snprintf (newFieldName, newFieldNameSize, "%s%d", defaultName, fieldNameNumber);
		if ( FieldNameTaken(newFieldName) == NULL)
			break;
		++fieldNameNumber;
	}	
}

//-----------------------------------------------------------------------------
// Purpose: check to see if any buildgroup panels have this fieldname
// Input  : fieldName, name to check
// Output : ptr to a panel that has the name if it is taken
//-----------------------------------------------------------------------------
Panel *BuildGroup::FieldNameTaken(const char *fieldName)
{	 	
	for ( int i = 0; i < _panelDar.Count(); i++ )
	{
		Panel *panel = _panelDar[i].Get();
		if ( !panel )
			continue;

		if (!stricmp(panel->GetName(), fieldName) )
		{
			return panel;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: serializes settings to a resource data container
//-----------------------------------------------------------------------------
void BuildGroup::GetSettings( KeyValues *resourceData )
{
	// loop through all the objects getting their settings
	for( int i = 0; i < _panelDar.Count(); i++ )
	{
		Panel *panel = _panelDar[i].Get();
		if (!panel)
			continue;

		bool isRuler = false;
		// do not get setting for ruler labels.
		if (_showRulers) // rulers are visible
		{
			for (int i = 0; i < 4; i++)
			{
				if (panel == _rulerNumber[i])
				{
					isRuler = true;
					break;
				}
			}
			if (isRuler)
			{
				isRuler = false;
				continue;
			}
		}

		// Don't save the setting of the buildmodedialog
		if (!stricmp(panel->GetName(), "BuildDialog"))
			continue;

		// get the keys section from the data file
		if (panel->GetName() && *panel->GetName())
		{
			KeyValues *datKey = resourceData->FindKey(panel->GetName(), true);

			// get the settings
			panel->GetSettings(datKey);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: loop though objects in the current control group and remove them all
//-----------------------------------------------------------------------------
void BuildGroup::RemoveSettings()
{	
	// loop though objects in the current control group and remove them all
	int i;
	for( i = 0; i < _controlGroup.Count(); i++ )
	{		
		// only delete delatable panels
		if ( _controlGroup[i].Get()->IsBuildModeDeletable())
		{
			delete _controlGroup[i].Get();
			_controlGroup.Remove(i);
			--i;
		}		
	}
	
	// remove deleted panels from the handle list
	for( i = 0; i < _panelDar.Count(); i++ )
	{
		if ( !_panelDar[i].Get() )	
		{	
		  _panelDar.Remove(i);
		  --i;
		}
	}

	_currentPanel = m_pBuildContext;
	_currentPanel->InvalidateLayout();
    m_pBuildContext->Repaint();
}

//-----------------------------------------------------------------------------
// Purpose: sets the panel from which the build group gets all it's object creation info
//-----------------------------------------------------------------------------
void BuildGroup::SetContextPanel(Panel *contextPanel)
{
	m_pBuildContext = contextPanel;
}

//-----------------------------------------------------------------------------
// Purpose: gets the panel from which the build group gets all it's object creation info
//-----------------------------------------------------------------------------
Panel *BuildGroup::GetContextPanel() 
{
	return m_pBuildContext;
}

//-----------------------------------------------------------------------------
// Purpose: get the list of panels in the buildgroup
//-----------------------------------------------------------------------------
CUtlVector<PHandle> *BuildGroup::GetPanelList() 
{
	return &_panelDar;
}

//-----------------------------------------------------------------------------
// Purpose: dialog variables
//-----------------------------------------------------------------------------
KeyValues *BuildGroup::GetDialogVariables()
{
	EditablePanel *edit = dynamic_cast<EditablePanel *>(m_pParentPanel);
	if (edit)
	{
		return edit->GetDialogVariables();
	}

	return NULL;
}
