//
//  gridprefs.cc
//
//  Preferences page for grid options.
//
//  Copyright (C) 2002 Mark R. Shinwell
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "gridprefs.h"
#include "message.h"

#include <wx/statline.h>

static const wxWindowID ID_GRID_PREFS = 1002;
static const wxWindowID ID_GRID_SHOW = 2000;

GridPrefs::GridPrefs(wxWindow* parent) : PanelDlgPage(parent, ID_GRID_PREFS)
{
    wxCheckBox* show_grid = new wxCheckBox(this, ID_GRID_SHOW, msg(/*Draw a grid*/361));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(show_grid, 0 /* not vertically stretchable */, wxALIGN_TOP | wxBOTTOM, 0);

    SetAutoLayout(true);
    SetSizer(sizer);
}

GridPrefs::~GridPrefs()
{

}

const wxString GridPrefs::GetName()
{
    return "Grid";
}

const wxBitmap GridPrefs::GetIcon()
{
    return wxGetApp().LoadPreferencesIcon("grid");
}
