//
//  gfxcore.cc
//
//  Core drawing code for Aven.
//
//  Copyright (C) 2000-2003 Mark R. Shinwell
//  Copyright (C) 2001-2003 Olly Betts
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <float.h>

#include "aven.h"
#include "gfxcore.h"
#include "mainfrm.h"
#include "message.h"
#include "useful.h"
#include "guicontrol.h"

#include <wx/confbase.h>
#include <wx/image.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c) ((a) > (b) ? MAX(a, c) : MAX(b, c))

// Values for m_SwitchingTo
#define PLAN 1
#define ELEVATION 2

#ifdef _WIN32
static const int FONT_SIZE = 8;
#else
static const int FONT_SIZE = 9;
#endif
static const int CROSS_SIZE = 3;
static const Double COMPASS_SIZE = 24.0;
static const int COMPASS_OFFSET_X = 60;
static const int COMPASS_OFFSET_Y = 80;
static const int INDICATOR_BOX_SIZE = 60;
static const int INDICATOR_GAP = 2;
static const int INDICATOR_MARGIN = 5;
static const int INDICATOR_OFFSET_X = 15;
static const int INDICATOR_OFFSET_Y = 15;
static const int CLINO_OFFSET_X = 6 + INDICATOR_OFFSET_X +
                                  INDICATOR_BOX_SIZE + INDICATOR_GAP;
static const int DEPTH_BAR_OFFSET_X = 16;
static const int DEPTH_BAR_EXTRA_LEFT_MARGIN = 2;
static const int DEPTH_BAR_BLOCK_WIDTH = 20;
static const int DEPTH_BAR_BLOCK_HEIGHT = 15;
static const int DEPTH_BAR_MARGIN = 6;
static const int DEPTH_BAR_OFFSET_Y = 16 + DEPTH_BAR_MARGIN;
static const int TICK_LENGTH = 4;
static const int SCALE_BAR_OFFSET_X = 15;
static const int SCALE_BAR_OFFSET_Y = 12;
static const int SCALE_BAR_HEIGHT = 12;
static const int HIGHLIGHTED_PT_SIZE = 2;
static const int TEXT_COLOUR = 7;
static const int HERE_COLOUR = 5;

const ColourTriple COLOURS[] = {
    { 0, 0, 0 },       // black
    { 100, 100, 100 }, // grey
    { 180, 180, 180 }, // light grey
    { 140, 140, 140 }, // light grey 2
    { 90, 90, 90 },    // dark grey
    { 255, 255, 255 }, // white
    { 0, 100, 255},    // turquoise
    { 0, 255, 40 },    // green
    { 150, 205, 224 }, // indicator 1
    { 114, 149, 160 }, // indicator 2
    { 255, 255, 0 },   // yellow
    { 255, 0, 0 },     // red
};

static const int NAME_COLOUR_INDEX = 7;

#define DELETE_ARRAY(A) do { assert((A)); delete[] (A); } while (0)

#define HITTEST_SIZE 20

// vector for lighting angle
static const Vector3 light(.577, .577, .577);

#ifdef AVENGL
BEGIN_EVENT_TABLE(GfxCore, wxGLCanvas)
#else
BEGIN_EVENT_TABLE(GfxCore, wxWindow)
#endif
    EVT_PAINT(GfxCore::OnPaint)
    EVT_LEFT_DOWN(GfxCore::OnLButtonDown)
    EVT_LEFT_UP(GfxCore::OnLButtonUp)
    EVT_MIDDLE_DOWN(GfxCore::OnMButtonDown)
    EVT_MIDDLE_UP(GfxCore::OnMButtonUp)
    EVT_RIGHT_DOWN(GfxCore::OnRButtonDown)
    EVT_RIGHT_UP(GfxCore::OnRButtonUp)
    EVT_MOTION(GfxCore::OnMouseMove)
    EVT_SIZE(GfxCore::OnSize)
    EVT_IDLE(GfxCore::OnIdle)
    EVT_CHAR(GfxCore::OnKeyPress)
END_EVENT_TABLE()

GfxCore::GfxCore(MainFrm* parent, wxWindow* parent_win, GUIControl* control) :
    GLACanvas(parent_win, 100, wxDefaultPosition, wxSize(640, 480)),
    m_Font(FONT_SIZE, wxSWISS, wxNORMAL, wxNORMAL, FALSE, "Helvetica",
           wxFONTENCODING_ISO8859_1),
    m_HaveData(false)
{
    m_Control = control;
    m_ScaleBar.offset_x = SCALE_BAR_OFFSET_X;
    m_ScaleBar.offset_y = SCALE_BAR_OFFSET_Y;
    m_ScaleBar.width = 0;
    m_Parent = parent;
    m_ParentWin = parent_win;
    m_RotationOK = true;
    m_DoneFirstShow = false;
    m_RedrawOffscreen = false;
    m_Crosses = false;
    m_Legs = true;
    m_Names = false;
    m_OverlappingNames = false;
    m_Compass = true;
    m_Clino = true;
    m_Depthbar = true;
    m_Scalebar = true;
    m_LabelGrid = NULL;
    m_Rotating = false;
    m_SwitchingTo = 0;
    m_Entrances = false;
    m_FixedPts = false;
    m_ExportedPts = false;
    m_Tubes = false;
    m_Grid = false;
    wxConfigBase::Get()->Read("metric", &m_Metric, true);
    wxConfigBase::Get()->Read("degrees", &m_Degrees, true);
    m_here.x = DBL_MAX;
    m_there.x = DBL_MAX;
    clipping = false;
    vertices = NULL;
    surface_vertices = NULL;
    num_segs = NULL;
    surface_num_segs = NULL;
    m_Lists.underground_legs = 0;
    m_Lists.surface_legs = 0;
    m_Lists.names = 0;
    m_Lists.indicators = 0;

    // Create pens and brushes for drawing.
    int num_colours = (int) col_LAST;
    m_Pens = new GLAPen[num_colours];
    for (int col = 0; col < num_colours; col++) {
        m_Pens[col].SetColour(COLOURS[col].r / 255.0,
                              COLOURS[col].g / 255.0,
                              COLOURS[col].b / 255.0);
    }

    // Initialise grid for hit testing.
    m_PointGrid = new list<LabelInfo*>[HITTEST_SIZE * HITTEST_SIZE];
}

GfxCore::~GfxCore()
{
    TryToFreeArrays();

    DELETE_ARRAY(m_Pens);

    delete[] m_PointGrid;
}

void GfxCore::TryToFreeArrays()
{
    // Free up any memory allocated for arrays.

    if (vertices) DELETE_ARRAY(vertices);
    if (num_segs) DELETE_ARRAY(num_segs);
    if (surface_vertices) DELETE_ARRAY(surface_vertices);
    if (surface_num_segs) DELETE_ARRAY(surface_num_segs);

    if (m_LabelGrid) {
	DELETE_ARRAY(m_LabelGrid);
	m_LabelGrid = NULL;
    }
}

//
//  Initialisation methods
//

void GfxCore::Initialise()
{
    // Initialise the view from the parent holding the survey data.

    TryToFreeArrays();

    m_DoneFirstShow = false;

    m_Bands = m_Parent->GetNumDepthBands();
    
    vertices = new Point[m_Parent->GetNumPoints()];
    num_segs = new int[m_Parent->GetNumLegs()];
    surface_vertices = new Point[m_Parent->GetNumPoints()];
    surface_num_segs = new int[m_Parent->GetNumLegs()];

    m_HitTestGridValid = false;
    m_here.x = DBL_MAX;
    m_there.x = DBL_MAX;

    // Apply default parameters.
    DefaultParameters();

    // If there are no legs (e.g. after loading a .pos file), turn crosses on.
    if (m_Parent->GetNumLegs() == 0) {
        m_Crosses = true;
    }

    // Check for flat/linear/point surveys.
    m_RotationOK = true;

    m_Lock = lock_NONE;
    if (m_Parent->GetXExtent() == 0.0) m_Lock = LockFlags(m_Lock | lock_X);
    if (m_Parent->GetYExtent() == 0.0) m_Lock = LockFlags(m_Lock | lock_Y);
    if (m_Parent->GetZExtent() == 0.0) m_Lock = LockFlags(m_Lock | lock_Z);

    // Scale the survey to a reasonable initial size.
    switch (m_Lock) {
	case lock_POINT:
	    m_RotationOK = false;
	    m_InitialScale = 1.0;
	    break;
	case lock_YZ:
	    m_InitialScale = Double(m_XSize) / m_Parent->GetXExtent();
	    break;
	case lock_XZ:
	    m_InitialScale = Double(m_YSize) / m_Parent->GetYExtent();
	    break;
	case lock_XY:
	    m_RotationOK = false;
	    m_InitialScale = Double(m_YSize) / m_Parent->GetZExtent();
	    break;
	case lock_X:
	    m_RotationOK = false;
	    m_InitialScale = min(Double(m_YSize) / m_Parent->GetZExtent(),
				 Double(m_XSize) / m_Parent->GetYExtent());
	    break;
	case lock_Y:
	    m_RotationOK = false;
	    m_InitialScale = min(Double(m_YSize) / m_Parent->GetZExtent(),
				 Double(m_XSize) / m_Parent->GetXExtent());
	    break;
	default:
	    m_InitialScale = min(Double(m_XSize) / m_Parent->GetXExtent(),
				 Double(m_YSize) / m_Parent->GetYExtent());
    }

    m_InitialScale *= .85;

    SetScaleInitial(m_InitialScale);

    m_HaveData = true;
    DefaultParameters();
}

void GfxCore::FirstShow()
{
    GLACanvas::FirstShow();

    SetBackgroundColour(0.0, 0.0, 0.0);
	
    // Set viewing volume coordinates.
    double extent = MAX3(m_Parent->GetXExtent(),
		         m_Parent->GetYExtent(),
			 m_Parent->GetZExtent()) / 2.0;
    SetVolumeCoordinates(-extent, extent, -extent, extent, -extent, extent);

    // Update our record of the client area size and centre.
    GetClientSize(&m_XSize, &m_YSize);
    m_XCentre = m_XSize / 2;
    m_YCentre = m_YSize / 2;

    // Set the scale correctly.
    SetScale(m_Params.scale);

    m_DoneFirstShow = true;
}

//
//  Recalculating methods
//

void GfxCore::SetScaleInitial(Double scale)
{
    m_Params.scale = scale;

    Point* pt = vertices;
    assert(pt);
    int* count = num_segs;
    assert(count);
    Point* spt = surface_vertices;
    assert(spt);
    int* scount = surface_num_segs;
    assert(scount);
    count--;
    scount--;

    m_Polylines = 0;
    m_SurfacePolylines = 0;

    Double x, y, z;

    list<PointInfo*>::const_iterator pos = m_Parent->GetPoints();
    list<PointInfo*>::const_iterator end = m_Parent->GetPointsEnd();
    bool first_point = true;
    bool last_was_move = true;
    bool current_polyline_is_surface = false;
    
    while (pos != end) {
	PointInfo* pti = *pos++;

	if (pti->IsLine()) {
	    // We have a leg.

	    assert(!first_point); // The first point must always be a move.
	    bool changing_ug_state =
                (current_polyline_is_surface != pti->IsSurface());

	    // Record new underground/surface state.
	    current_polyline_is_surface = pti->IsSurface();

	    if (changing_ug_state || last_was_move) {
		// Start a new polyline if we're switching
		// underground/surface state or if the previous point
		// was a move.

		Point** dest;

		if (current_polyline_is_surface) {
		    m_SurfacePolylines++;
		    // initialise number of vertices for next polyline
		    *(++scount) = 1;
		    dest = &spt;
		}
		else {
		    m_Polylines++;
		    // initialise number of vertices for next polyline
		    *(++count) = 1;
		    dest = &pt;
		}

		(*dest)->x = x;
		(*dest)->y = y;
		(*dest)->z = z;

		// Advance the relevant coordinate pointer to the next
		// position.
		(*dest)++;
	    }

	    // Add the leg onto the current polyline.
	    Point** dest = &(current_polyline_is_surface ? spt : pt);

	    (*dest)->x = x = pti->GetX();
	    (*dest)->y = y = pti->GetY();
	    (*dest)->z = z = pti->GetZ();

	    // Advance the relevant coordinate pointer to the next position.
	    (*dest)++;

	    // Increment the relevant vertex count.
	    if (current_polyline_is_surface) {
		(*scount)++;
	    }
	    else {
		(*count)++;
	    }

	    last_was_move = false;
	}
	else {
	    first_point = false;
	    last_was_move = true;

	    // Save the current coordinates for the next time around the loop.
	    x = pti->GetX();
	    y = pti->GetY();
	    z = pti->GetZ();
	}
    }
}

void GfxCore::SetScale(Double scale)
{
    // Fill the plot data arrays with screen coordinates, scaling the survey
    // to a particular absolute scale.

    clipping = false;
    if (scale < m_InitialScale / 20) {
        scale = m_InitialScale / 20;
    } else {
        if (scale > 1000.0) scale = 1000.0;
        Double max_scale =
            32767.0 / MAX(m_Parent->GetXExtent(), m_Parent->GetYExtent());
        if (scale > max_scale) clipping = true;
    }

    m_Params.scale = scale;
    m_HitTestGridValid = false;

    GLACanvas::SetScale(scale);

    UpdateIndicators();
    UpdateBlobs();
}

void GfxCore::UpdateBlobs()
{
//    DeleteList(m_Lists.blobs);
//    m_Lists.blobs = CreateList(this, &GfxCore::GenerateBlobsDisplayList);
}

void GfxCore::UpdateLegs()
{
    if (m_Lists.underground_legs) {
        DeleteList(m_Lists.underground_legs);
    }

    m_Lists.underground_legs = CreateList(this, &GfxCore::GenerateDisplayList);
}

void GfxCore::UpdateNames()
{
    if (m_Names) {
        DeleteList(m_Lists.names);
        m_Lists.names = CreateList(this, &GfxCore::DrawNames);
    }
}

//
//  Event handlers
//

void GfxCore::OnIdle(wxIdleEvent& event)
{
    // Handle an idle event.

    if (Animate(&event)) {
        ForceRefresh();
    }
}

void GfxCore::OnPaint(wxPaintEvent& event)
{
    // Redraw the window.

    // Get a graphics context.
    wxPaintDC dc(this);

    assert(GetContext());

    StartDrawing();

    // Clear the background.
    Clear();

    // Make sure we're initialised.
    bool first_time = !m_DoneFirstShow;
    if (first_time) {
        FirstShow();
    }

    if (m_HaveData) {
        // Set up model transformation matrix.
        SetDataTransform();

        if (first_time) {
            m_Lists.underground_legs =
                CreateList(this, &GfxCore::GenerateDisplayList);
            
            m_Lists.surface_legs =
                CreateList(this, &GfxCore::GenerateDisplayListSurface);
            
            m_Lists.names = CreateList(this, &GfxCore::DrawNames);

            m_Lists.indicators =
                CreateList(this, &GfxCore::GenerateIndicatorDisplayList);
        }

        if (m_Legs) {
            // Draw the underground legs.
            
            if (m_Tubes) {
                EnableSmoothPolygons();
            }

            SetColour(m_Pens[7]);
            DrawList(m_Lists.underground_legs);

            if (m_Tubes) {
                DisableSmoothPolygons();
            }
        }

        if (m_Surface) {
            // Draw the surface legs.

            if (m_SurfaceDashed) {
                EnableDashedLines();
            }

            DrawList(m_Lists.surface_legs);

            if (m_SurfaceDashed) {
                DisableDashedLines();
            }
        }
        
        // Draw station names.
        if (m_Names && !m_Control->MouseDown() && !m_Rotating &&
            !m_SwitchingTo) {
            DrawList(m_Lists.names);
        }
/*
        //DrawList(m_Lists.blobs);
        GenerateBlobsDisplayList();*/
/*
        if (m_Grid) {
            // Draw the grid.
            DrawList(m_Lists.grid);
        }
*/

        if (!m_Rotating && !m_SwitchingTo) {
            Double size = SurveyUnitsAcrossViewport() * HIGHLIGHTED_PT_SIZE /
                           m_XSize;
    
            // Draw "here" and "there".
            if (m_here.x != DBL_MAX) {
                DrawSphere(m_Pens[HERE_COLOUR],
                           m_here.x, m_here.y, m_here.z, size, 16);
            }
            if (m_there.x != DBL_MAX) {
                if (m_here.x != DBL_MAX) {
                    BeginLines();
                    PlaceVertex(m_here.x, m_here.y, m_here.z);
		    PlaceVertex(m_there.x, m_there.y, m_there.z);
		    EndLines();
                }
                DrawSphere(m_Pens[HERE_COLOUR],
                           m_there.x, m_there.y, m_there.z, size, 16);
            }
        }
        
        // Draw indicators.
        SetIndicatorTransform();
        DrawList(m_Lists.indicators);
    }

    FinishDrawing();
}

Double GfxCore::GridXToScreen(Double x, Double y, Double z) const
{
    return 0.0;

    /*
    x += m_Params.translation.x;
    y += m_Params.translation.y;
    z += m_Params.translation.z;

    return (XToScreen(x, y, z) * m_Params.scale) + m_XCentre;*/
}

Double GfxCore::GridYToScreen(Double x, Double y, Double z) const
{
    return 0.0;

    /*
    x += m_Params.translation.x;
    y += m_Params.translation.y;
    z += m_Params.translation.z;

    return m_YCentre - ((ZToScreen(x, y, z) * m_Params.scale));*/
}

void GfxCore::DrawGrid()
{
    // Draw the grid.
/*
    SetColour(col_RED);

    // Calculate the extent of the survey, in metres across the screen plane.
    Double m_across_screen = Double(m_XSize / m_Params.scale);
    // Calculate the length of the scale bar in metres.
    //--move this elsewhere
    Double size_snap = pow(10.0, floor(log10(0.75 * m_across_screen)));
    Double t = m_across_screen * 0.75 / size_snap;
    if (t >= 5.0) {
        size_snap *= 5.0;
    }
    else if (t >= 2.0) {
        size_snap *= 2.0;
    }

    Double grid_size = size_snap / 10.0;
    Double edge = grid_size * 2.0;
    Double grid_z = -m_Parent->GetZExtent()/2.0 - grid_size;
    Double left = -m_Parent->GetXExtent()/2.0 - edge;
    Double right = m_Parent->GetXExtent()/2.0 + edge;
    Double bottom = -m_Parent->GetYExtent()/2.0 - edge;
    Double top = m_Parent->GetYExtent()/2.0 + edge;
    int count_x = (int) ceil((right - left) / grid_size);
    int count_y = (int) ceil((top - bottom) / grid_size);
    Double actual_right = left + count_x*grid_size;
    Double actual_top = bottom + count_y*grid_size;

    for (int xc = 0; xc <= count_x; xc++) {
        Double x = left + xc*grid_size;
        
        m_DrawDC.DrawLine((int) GridXToScreen(x, bottom, grid_z), (int) GridYToScreen(x, bottom, grid_z),
                          (int) GridXToScreen(x, actual_top, grid_z), (int) GridYToScreen(x, actual_top, grid_z));
    }

    for (int yc = 0; yc <= count_y; yc++) {
        Double y = bottom + yc*grid_size;
        m_DrawDC.DrawLine((int) GridXToScreen(left, y, grid_z), (int) GridYToScreen(left, y, grid_z),
                          (int) GridXToScreen(actual_right, y, grid_z),
                          (int) GridYToScreen(actual_right, y, grid_z));
    }
*/
}

glaCoord GfxCore::GetClinoOffset() const
{
    return m_Compass ? CLINO_OFFSET_X : INDICATOR_OFFSET_X;
}

wxPoint GfxCore::CompassPtToScreen(Double x, Double y, Double z) const
{
    /*return wxPoint(long(-XToScreen(x, y, z)) + m_XSize - COMPASS_OFFSET_X,
                   long(ZToScreen(x, y, z)) + m_YSize - COMPASS_OFFSET_Y);*/

    return wxPoint(0, 0);
}

GLAPoint GfxCore::IndicatorCompassToScreenPan(int angle) const
{
    Double theta = (angle * M_PI / 180.0) + m_PanAngle;
    glaCoord length = (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2) / 2;
    glaCoord x = glaCoord(length * sin(theta));
    glaCoord y = glaCoord(length * cos(theta));

    return GLAPoint(m_XSize/2 - INDICATOR_OFFSET_X - INDICATOR_BOX_SIZE/2 - x,
                    -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE/2 + y, 0.0);
}

GLAPoint GfxCore::IndicatorCompassToScreenElev(int angle) const
{
    Double theta = (angle * M_PI / 180.0) + m_TiltAngle + M_PI_2;
    glaCoord length = (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2) / 2;
    glaCoord x = glaCoord(length * sin(-theta));
    glaCoord y = glaCoord(length * cos(-theta));

    return GLAPoint(m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE/2 - x,
                    -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE/2 + y, 0.0);
}

void GfxCore::DrawTick(wxCoord cx, wxCoord cy, int angle_cw)
{
    Double theta = angle_cw * M_PI / 180.0;
    wxCoord length1 = (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2) / 2;
    wxCoord length0 = length1 + TICK_LENGTH;
    wxCoord x0 = wxCoord(length0 * sin(theta));
    wxCoord y0 = wxCoord(length0 * -cos(theta));
    wxCoord x1 = wxCoord(length1 * sin(theta));
    wxCoord y1 = wxCoord(length1 * -cos(theta));

    BeginLines();
    PlaceIndicatorVertex(cx + x0, cy - y0);
    PlaceIndicatorVertex(cx + x1, cy - y1);
    EndLines();
}

void GfxCore::Draw2dIndicators()
{
    // Draw the "traditional" elevation and compass indicators.

    // Indicator backgrounds.
    if (m_Compass && m_RotationOK) {
        DrawCircle(m_Pens[col_LIGHT_GREY_2], m_Pens[col_GREY],
                   m_XSize/2 - INDICATOR_OFFSET_X - INDICATOR_BOX_SIZE + INDICATOR_MARGIN,
                   -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE - INDICATOR_MARGIN,
                   (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2)/2);
    }
    if (m_Clino && m_Lock == lock_NONE) {
        glaCoord tilt = (glaCoord) (m_TiltAngle * 180.0 / M_PI);
	glaCoord start = fabs(-tilt - 90.0);
        DrawSemicircle(m_Pens[col_LIGHT_GREY_2], m_Pens[col_GREY],
		       m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE + INDICATOR_MARGIN,
                       -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE - INDICATOR_MARGIN,
                       (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2)/2, start);

        SetColour(m_Pens[col_GREY]);
        BeginLines();
        PlaceIndicatorVertex(m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE/2,
                             -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_MARGIN);
        
        PlaceIndicatorVertex(m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE/2,
                             -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE - INDICATOR_MARGIN);

        PlaceIndicatorVertex(m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE/2,
                             -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE/2);
        
        PlaceIndicatorVertex(m_XSize/2 - GetClinoOffset() - INDICATOR_MARGIN,
                             -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE/2);
        EndLines();
    }


    // Ticks
    bool white = false;
    /* FIXME m_Control->DraggingMouseOutsideCompass();
    m_DraggingLeft && m_LastDrag == drag_COMPASS && m_MouseOutsideCompass; */
    glaCoord pan_centre_x = m_XSize/2 - INDICATOR_OFFSET_X - INDICATOR_BOX_SIZE/2;
    glaCoord centre_y = -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE/2;
    glaCoord elev_centre_x = m_XSize/2 - GetClinoOffset() - INDICATOR_BOX_SIZE/2;

    if (m_Compass && m_RotationOK) {
        int deg_pan = (int) (m_PanAngle * 180.0 / M_PI);
        //--FIXME: bodge by Olly to stop wrong tick highlighting
        if (deg_pan) deg_pan = 360 - deg_pan;
        for (int angle = deg_pan; angle <= 315 + deg_pan; angle += 45) {
            if (deg_pan == angle) {
                SetAvenColour(col_GREEN);
            }
            else {
                SetAvenColour(white ? col_WHITE : col_LIGHT_GREY_2);
            }
            DrawTick((int) pan_centre_x, (int) centre_y, angle);
        }
    }

    if (m_Clino && m_Lock == lock_NONE) {
        white = false; /* FIXME m_DraggingLeft && m_LastDrag == drag_ELEV && m_MouseOutsideElev; */
        int deg_elev = (int) (m_TiltAngle * 180.0 / M_PI);
        for (int angle = 0; angle <= 180; angle += 90) {
            if (deg_elev == angle - 90) {
                SetAvenColour(col_GREEN);
            }
            else {
                SetAvenColour(white ? col_WHITE : col_LIGHT_GREY_2);
            }
            DrawTick((int) elev_centre_x, (int) centre_y, angle);
        }
    }

    // Pan arrow
    if (m_Compass && m_RotationOK) {
        GLAPoint p1 = IndicatorCompassToScreenPan(0);
        GLAPoint p2 = IndicatorCompassToScreenPan(150);
        GLAPoint p3 = IndicatorCompassToScreenPan(210);
        GLAPoint pc(pan_centre_x, centre_y, 0.0);
        GLAPoint pts1[3] = { p2, p1, pc };
        GLAPoint pts2[3] = { p3, p1, pc };
        
        DrawTriangle(m_Pens[col_LIGHT_GREY], m_Pens[col_INDICATOR_1], pts1);
        DrawTriangle(m_Pens[col_LIGHT_GREY], m_Pens[col_INDICATOR_2], pts2);
    }

    // Elevation arrow
    if (m_Clino && m_Lock == lock_NONE) {
        GLAPoint p1e = IndicatorCompassToScreenElev(0);
        GLAPoint p2e = IndicatorCompassToScreenElev(150);
        GLAPoint p3e = IndicatorCompassToScreenElev(210);
        GLAPoint pce(elev_centre_x, centre_y, 0.0);
        GLAPoint pts1e[3] = { p2e, p1e, pce };
        GLAPoint pts2e[3] = { p3e, p1e, pce };
        DrawTriangle(m_Pens[col_LIGHT_GREY], m_Pens[col_INDICATOR_2], pts1e);
        DrawTriangle(m_Pens[col_LIGHT_GREY], m_Pens[col_INDICATOR_1], pts2e);
    }

    // Text
    SetColour(m_Pens[TEXT_COLOUR]);

    glaCoord w, h;
    glaCoord width, height;
    wxString str;

    GetTextExtent(wxString("000"), &width, &h);
    height = -m_YSize/2 + INDICATOR_OFFSET_Y + INDICATOR_BOX_SIZE + INDICATOR_GAP + h;

    if (m_Compass && m_RotationOK) {
        if (m_Degrees) {
            str = wxString::Format("%03d", int(m_PanAngle * 180.0 / M_PI));
        } else {
            str = wxString::Format("%03d", int(m_PanAngle * 200.0 / M_PI));
        }       
        GetTextExtent(str, &w, &h);
        DrawIndicatorText(pan_centre_x + width / 2 - w, height, str);
        str = wxString(msg(/*Facing*/203));
        GetTextExtent(str, &w, &h);
        DrawIndicatorText(pan_centre_x - w / 2, height + h, str);
    }

    if (m_Clino && m_Lock == lock_NONE) {
        int angle;
        if (m_Degrees) {
            angle = int(-m_TiltAngle * 180.0 / M_PI);
        } else {
            angle = int(-m_TiltAngle * 200.0 / M_PI);
        }       
        str = angle ? wxString::Format("%+03d", angle) : wxString("00");
        GetTextExtent(str, &w, &h);
        DrawIndicatorText(elev_centre_x + width / 2 - w, height, str);
        str = wxString(msg(/*Elevation*/118));
        GetTextExtent(str, &w, &h);
        DrawIndicatorText(elev_centre_x - w / 2, height + h, str);
    }
}

// FIXME: either remove this, or make it an option...
void GfxCore::DrawCompass()
{
    // Draw the 3d compass.
#if 0
    wxPoint pt[3];

    SetColour(col_TURQUOISE);
    m_DrawDC.DrawLine(CompassPtToScreen(0.0, 0.0, -COMPASS_SIZE),
                      CompassPtToScreen(0.0, 0.0, COMPASS_SIZE));

    pt[0] = CompassPtToScreen(-COMPASS_SIZE / 3.0f, 0.0, -COMPASS_SIZE * 2.0f / 3.0f);
    pt[1] = CompassPtToScreen(0.0, 0.0, -COMPASS_SIZE);
    pt[2] = CompassPtToScreen(COMPASS_SIZE / 3.0f, 0.0, -COMPASS_SIZE * 2.0f / 3.0f);
    m_DrawDC.DrawLines(3, pt);

    m_DrawDC.DrawLine(CompassPtToScreen(-COMPASS_SIZE, 0.0, 0.0),
                      CompassPtToScreen(COMPASS_SIZE, 0.0, 0.0));

    SetColour(col_GREEN);
    m_DrawDC.DrawLine(CompassPtToScreen(0.0, -COMPASS_SIZE, 0.0),
                      CompassPtToScreen(0.0, COMPASS_SIZE, 0.0));

    pt[0] = CompassPtToScreen(-COMPASS_SIZE / 3.0f, -COMPASS_SIZE * 2.0f / 3.0f, 0.0);
    pt[1] = CompassPtToScreen(0.0, -COMPASS_SIZE, 0.0);
    pt[2] = CompassPtToScreen(COMPASS_SIZE / 3.0f, -COMPASS_SIZE * 2.0f / 3.0f, 0.0);
    m_DrawDC.DrawLines(3, pt);
#endif
}

void GfxCore::DrawNames()
{
    // Draw station names.
    
    SetColour(m_Pens[NAME_COLOUR_INDEX]);
    
    if (m_OverlappingNames) {
        SimpleDrawNames();
    } else {
        NattyDrawNames();
    }
}

void GfxCore::NattyDrawNames()
{
    // Draw station names, without overlapping.
    // FIXME: copied to OnSize()
   
    const int dv = 2;
    const int quantise = int(GetFontSize() / dv);
    const int quantised_x = m_XSize / quantise;
    const int quantised_y = m_YSize / quantise;
    const size_t buffer_size = quantised_x * quantised_y;
   
    if (m_LabelGrid) {
        delete[] m_LabelGrid;
    }
    m_LabelGrid = new char[buffer_size];

    memset((void*) m_LabelGrid, 0, buffer_size);

    list<LabelInfo*>::const_iterator label = m_Parent->GetLabels();

    while (label != m_Parent->GetLabelsEnd()) {
        Double x, y, z;
        
        Transform((*label)->x, (*label)->y, (*label)->z, &x, &y, &z);
        y += CROSS_SIZE - GetFontSize();

        wxString str = (*label)->GetText();

        Double tx, ty, tz;
        Transform(0, 0, 0, &tx, &ty, &tz);

        tx -= floor(tx / quantise) * quantise;
        ty -= floor(ty / quantise) * quantise;

        int ix = int(x - tx) / quantise;
        int iy = int(y - ty) / quantise;

        bool reject = true;

        if (ix >= 0 && ix < quantised_x && iy >= 0 && iy < quantised_y) {
            char* test = &m_LabelGrid[ix + iy * quantised_x];
            int len = str.Length() * dv + 1;
            reject = (ix + len >= quantised_x);
            int i = 0;

            while (!reject && i++ < len) {
                reject = *test++;
            }

            if (!reject) {
                DrawText((*label)->x, (*label)->y, (*label)->z, str);

                int ymin = (iy >= 2) ? iy - 2 : iy;
                int ymax = (iy < quantised_y - 2) ? iy + 2 : iy;
                for (int y0 = ymin; y0 <= ymax; y0++) {
                    assert((ix + y0 * quantised_x) < (quantised_x * quantised_y));
                    memset((void*) &m_LabelGrid[ix + y0 * quantised_x], 1, len);
                }
            }
        }
        ++label;
    }
}

void GfxCore::SimpleDrawNames()
{
    // Draw all station names, without worrying about overlaps
    list<LabelInfo*>::const_iterator label = m_Parent->GetLabels();
    while (label != m_Parent->GetLabelsEnd()) {
        DrawText((*label)->x, (*label)->y, (*label)->z, (*label)->GetText());
        ++label;
    }
}

void GfxCore::DrawDepthbar()
{
    if (m_Parent->GetZExtent() == 0.0) return;

    int y = m_YSize/2 -
            (DEPTH_BAR_BLOCK_HEIGHT * (m_Bands - 1) + DEPTH_BAR_OFFSET_Y);
    glaCoord size = 0;

    wxString* strs = new wxString[m_Bands];
    for (int band = 0; band < m_Bands; band++) {
        Double z = m_Parent->GetZMin() +
                   m_Parent->GetZExtent() * band / (m_Bands - 1);
        
        strs[band] = FormatLength(z, false);

        glaCoord x, y;
        GetTextExtent(strs[band], &x, &y);
        if (x > size) {
            size = x;
        }
    }

    glaCoord x_min = m_XSize/2 - DEPTH_BAR_OFFSET_X -
                     DEPTH_BAR_BLOCK_WIDTH - DEPTH_BAR_MARGIN - size;

    DrawRectangle(m_Pens[col_BLACK],
                  m_Pens[col_DARK_GREY],
                  m_Pens[col_DARK_GREY],
                  x_min - DEPTH_BAR_MARGIN - DEPTH_BAR_EXTRA_LEFT_MARGIN,
                  m_YSize/2 - (DEPTH_BAR_BLOCK_HEIGHT*(m_Bands-1)) -
                      DEPTH_BAR_OFFSET_Y - DEPTH_BAR_MARGIN*2,
                  DEPTH_BAR_BLOCK_WIDTH + size + DEPTH_BAR_MARGIN*3 +
                      DEPTH_BAR_EXTRA_LEFT_MARGIN,
                  DEPTH_BAR_BLOCK_HEIGHT*(m_Bands - 1) + DEPTH_BAR_MARGIN*4);

    for (int band = 0; band < m_Bands; band++) {
        if (band < m_Bands - 1) {
            DrawRectangle(m_Parent->GetPen(band),
                          m_Parent->GetPen(band),
                          m_Parent->GetPen(band+1),
                          x_min,
                          y,
                          DEPTH_BAR_BLOCK_WIDTH,
                          DEPTH_BAR_BLOCK_HEIGHT,
                          false);
        }

        SetColour(m_Pens[TEXT_COLOUR]);

        DrawIndicatorText(x_min + DEPTH_BAR_BLOCK_WIDTH + 5,
                          y - (FONT_SIZE / 2) - 1, strs[band]);

        y += DEPTH_BAR_BLOCK_HEIGHT;
    }

    delete[] strs;
}

wxString GfxCore::FormatLength(Double size_snap, bool scalebar)
{
    wxString str;
    bool negative = (size_snap < 0.0);

    if (negative) {
        size_snap = -size_snap;
    }

    if (size_snap == 0.0) {
        str = "0";
    } else if (m_Metric) {
#ifdef SILLY_UNITS
        if (size_snap < 1e-12) {
            str = wxString::Format("%.3gpm", size_snap * 1e12);
        } else if (size_snap < 1e-9) {
            str = wxString::Format("%.fpm", size_snap * 1e12);
        } else if (size_snap < 1e-6) {
            str = wxString::Format("%.fnm", size_snap * 1e9);
        } else if (size_snap < 1e-3) {
            str = wxString::Format("%.fum", size_snap * 1e6);
#else
        if (size_snap < 1e-3) {
            str = wxString::Format("%.3gmm", size_snap * 1e3);
#endif
        } else if (size_snap < 1e-2) {
            str = wxString::Format("%.fmm", size_snap * 1e3);
        } else if (size_snap < 1.0) {
            str = wxString::Format("%.fcm", size_snap * 100.0);
        } else if (size_snap < 1e3) {
            str = wxString::Format("%.fm", size_snap);
#ifdef SILLY_UNITS
        } else if (size_snap < 1e6) {
            str = wxString::Format("%.fkm", size_snap * 1e-3);
        } else if (size_snap < 1e9) {
            str = wxString::Format("%.fMm", size_snap * 1e-6);
        } else {
            str = wxString::Format("%.fGm", size_snap * 1e-9);
#else
        } else {
            str = wxString::Format(scalebar ? "%.fkm" : "%.2fkm", size_snap * 1e-3);
#endif
        }
    } else {
        size_snap /= METRES_PER_FOOT;
        if (scalebar) {
            Double inches = size_snap * 12;
            if (inches < 1.0) {
                str = wxString::Format("%.3gin", inches);
            } else if (size_snap < 1.0) {
                str = wxString::Format("%.fin", inches);
            } else if (size_snap < 5279.5) {
                str = wxString::Format("%.fft", size_snap);
            } else {
                str = wxString::Format("%.f miles", size_snap / 5280.0);
            }
        } else {
            str = wxString::Format("%.fft", size_snap);
        }
    }

    return negative ? wxString("-") + str : str;
}

void GfxCore::DrawScalebar()
{
    // Draw the scalebar.
    
    if (m_Lock == lock_POINT) return;

    // Calculate how many metres of survey are currently displayed across the screen.
    Double across_screen = SurveyUnitsAcrossViewport();

    // Convert to imperial measurements if required.
    Double multiplier = 1.0;
    if (!m_Metric) {
        across_screen /= METRES_PER_FOOT;
        multiplier = METRES_PER_FOOT;
        if (across_screen >= 5280.0 / 0.75) {
            across_screen /= 5280.0;
            multiplier *= 5280.0;
        }
    }

    // Calculate the length of the scale bar.
    Double size_snap = pow(10.0, floor(log10(0.65 * across_screen)));
    Double t = across_screen * 0.65 / size_snap;
    if (t >= 5.0) {
        size_snap *= 5.0;
    } else if (t >= 2.0) {
        size_snap *= 2.0;
    }

    if (!m_Metric) size_snap *= multiplier;

    // Actual size of the thing in pixels:
    int size = int((size_snap / SurveyUnitsAcrossViewport()) * m_XSize);
    m_ScaleBar.width = size;

    // Draw it...
    int end_x = -m_XSize/2 + m_ScaleBar.offset_x;
    int height = SCALE_BAR_HEIGHT;
    int end_y = -m_YSize/2 + m_ScaleBar.offset_y + height;
    int interval = size / 10;

    bool solid = true;
    for (int ix = 0; ix < 10; ix++) {
        int x = end_x + int(ix * ((Double) size / 10.0));
        GLAPen& pen = solid ? m_Pens[col_GREY] : m_Pens[col_WHITE];

        DrawRectangle(pen, pen, pen, x, end_y, interval + 2, height);

        solid = !solid;
    }

    // Add labels.
    wxString str = FormatLength(size_snap);

    SetColour(m_Pens[TEXT_COLOUR]);
    DrawIndicatorText(end_x, end_y - FONT_SIZE - 4, "0");

    glaCoord text_width, text_height;
    GetTextExtent(str, &text_width, &text_height);
    DrawIndicatorText(end_x + size - text_width, end_y - FONT_SIZE - 4, str);
}

bool GfxCore::CheckHitTestGrid(wxPoint& point, bool centre)
{
    if (point.x < 0 || point.x >= m_XSize ||
        point.y < 0 || point.y >= m_YSize) {
        return false;
    }

    SetDataTransform();

    if (!m_HitTestGridValid) {
        CreateHitTestGrid();
    }

    int grid_x = (point.x * (HITTEST_SIZE - 1)) / m_XSize;
    int grid_y = (point.y * (HITTEST_SIZE - 1)) / m_YSize;

    LabelInfo *best = NULL;
    int dist_sqrd = 25;
    int square = grid_x + grid_y * HITTEST_SIZE;
    list<LabelInfo*>::iterator iter = m_PointGrid[square].begin();

    while (iter != m_PointGrid[square].end()) {
        LabelInfo *pt = *iter++;

        Double cx, cy, cz;

        Transform(pt->GetX(), pt->GetY(), pt->GetZ(), &cx, &cy, &cz);

        cy = m_YSize - cy;

        int dx = point.x - int(cx);
        int ds = dx * dx;
        if (ds >= dist_sqrd) continue;
        int dy = point.y - int(cy);

        ds += dy * dy;
        if (ds >= dist_sqrd) continue;
 
        dist_sqrd = ds;
        best = pt;
        
        if (ds == 0) break;
    }
    
    if (best) {
        m_Parent->SetMouseOverStation(best);
        if (centre) {
            CentreOn(best->GetX(), best->GetY(), best->GetZ());
            SetThere(best->GetX(), best->GetY(), best->GetZ());
            m_Parent->SelectTreeItem(best);
        }
    } else {
        m_Parent->SetMouseOverStation(NULL);
    }

    return best;
}

void GfxCore::OnSize(wxSizeEvent& event)
{
    // Handle a change in window size.
    
//    wxGLCanvas::OnSize(event);//FIXME

    wxSize size = event.GetSize();

    m_XSize = size.GetWidth();
    m_YSize = size.GetHeight();
    if (m_XSize < 0 || m_YSize < 0) { //-- FIXME
        m_XSize = 640;
        m_YSize = 480;
    }
    m_XCentre = m_XSize / 2;
    m_YCentre = m_YSize / 2;

    if (m_DoneFirstShow) {
        // FIXME: copied from NattyDrawNames()
        const int dv = 2;
        const int quantise = int(GetFontSize() / dv);
        const int quantised_x = m_XSize / quantise;
        const int quantised_y = m_YSize / quantise;
        const size_t buffer_size = quantised_x * quantised_y;

        if (m_LabelGrid) delete[] m_LabelGrid;

        m_LabelGrid = new char[buffer_size];
        CreateHitTestGrid();

        UpdateIndicators();

        Refresh(false);
    }
}

void GfxCore::DefaultParameters()
{
    // Set default viewing parameters.

    // If there are no legs (e.g. after loading a .pos file), turn crosses on.
    if (m_Parent->GetNumLegs() == 0) {
	m_Crosses = true;
    }

    m_PanAngle = 0.0;
    m_TiltAngle = M_PI_2;
    switch (m_Lock) {
	case lock_X:
	{
	    // elevation looking along X axis (East)
	    m_PanAngle = M_PI * 1.5;
	    m_TiltAngle = 0.0;
	    break;
	}

	case lock_Y:
	case lock_XY: // survey is linearface and parallel to the Z axis => display in elevation.
	    // elevation looking along Y axis (North)
	    m_TiltAngle = 0.0;
	    break;

	case lock_Z:
	case lock_XZ: // linearface survey parallel to Y axis
	case lock_YZ: // linearface survey parallel to X axis
	{
	    // flat survey (zero height range) => go into plan view (default orientation).
	    break;
	}

	case lock_POINT:
	    m_Crosses = true;
	    break;

	case lock_NONE:
	    break;
    }

    m_Params.rotation.setFromEulerAngles(m_TiltAngle, 0.0, m_PanAngle);

    m_Params.translation.x = 0.0;
    m_Params.translation.y = 0.0;
    m_Params.translation.z = 0.0;

    SetRotation(m_Params.rotation);
    SetTranslation(0.0, 0.0, 0.0);

    m_Surface = false;
    m_SurfaceDepth = false;
    m_SurfaceDashed = true;
    m_RotationStep = M_PI / 400.0; // FIXME temp bodge (was 6.0)
    m_Rotating = false;
    m_SwitchingTo = 0;
    m_Entrances = false;
    m_FixedPts = false;
    m_ExportedPts = false;
    m_Grid = false;
    m_Tubes = false;
}

void GfxCore::Defaults()
{
    // Restore default scale, rotation and translation parameters.

    DefaultParameters();
    SetScale(m_InitialScale);
    ForceRefresh();
}

// the argument is a pointer to an idle event (to call RequestMore()) or NULL
// return: true if animation occured (and ForceRefresh() needs to be called)
bool GfxCore::Animate(wxIdleEvent*)
{
    if (!m_Rotating && !m_SwitchingTo) {
        return false;
    }

    static double last_t = 0;
    double t = timer.Time() * 1.0e-3;
    if (t == 0) t = 0.001;
    else if (t > 1.0) t = 1.0;
    if (last_t > 0) t = (t + last_t) / 2;
    last_t = t;

    // When rotating...
    if (m_Rotating) {
        TurnCave(m_RotationStep /* FIXME * t */);
    }

    if (m_SwitchingTo == PLAN) {
        // When switching to plan view...
        TiltCave(M_PI_2 * t);
        if (m_TiltAngle == M_PI_2) {
            m_SwitchingTo = 0;
            UpdateNames();
            ForceRefresh();
        }
    }
    else if (m_SwitchingTo == ELEVATION) {
        // When switching to elevation view...
        if (fabs(m_TiltAngle) < M_PI_2 * t) {
            m_SwitchingTo = 0;
            TiltCave(-m_TiltAngle);
            UpdateNames();
            ForceRefresh();
        }
        else if (m_TiltAngle < 0.0) {
            TiltCave(M_PI_2 * t);
        }
        else {
            TiltCave(-M_PI_2 * t);
        }
    }

    return true;
}

void GfxCore::RefreshLine(const Point &a1, const Point &b1,
                          const Point &a2, const Point &b2)
{
    ForceRefresh(); //--FIXME
#if 0
    // Calculate the minimum rectangle which includes the old and new
    // measuring lines to minimise the redraw time
    int l = INT_MAX, r = INT_MIN, u = INT_MIN, d = INT_MAX;
    if (a1.x != DBL_MAX) {
        int x = (int)GridXToScreen(a1);
        int y = (int)GridYToScreen(a1);
        l = x - HIGHLIGHTED_PT_SIZE * 4;
        r = x + HIGHLIGHTED_PT_SIZE * 4;
        u = y + HIGHLIGHTED_PT_SIZE * 4;
        d = y - HIGHLIGHTED_PT_SIZE * 4;
    }
    if (a2.x != DBL_MAX) {
        int x = (int)GridXToScreen(a2);
        int y = (int)GridYToScreen(a2);
        l = min(l, x - HIGHLIGHTED_PT_SIZE * 4);
        r = max(r, x + HIGHLIGHTED_PT_SIZE * 4);
        u = max(u, y + HIGHLIGHTED_PT_SIZE * 4);
        d = min(d, y - HIGHLIGHTED_PT_SIZE * 4);
    }
    if (b1.x != DBL_MAX) {
        int x = (int)GridXToScreen(b1);
        int y = (int)GridYToScreen(b1);
        l = min(l, x - HIGHLIGHTED_PT_SIZE * 4);
        r = max(r, x + HIGHLIGHTED_PT_SIZE * 4);
        u = max(u, y + HIGHLIGHTED_PT_SIZE * 4);
        d = min(d, y - HIGHLIGHTED_PT_SIZE * 4);
    }
    if (b2.x != DBL_MAX) {
        int x = (int)GridXToScreen(b2);
        int y = (int)GridYToScreen(b2);
        l = min(l, x - HIGHLIGHTED_PT_SIZE * 4);
        r = max(r, x + HIGHLIGHTED_PT_SIZE * 4);
        u = max(u, y + HIGHLIGHTED_PT_SIZE * 4);
        d = min(d, y - HIGHLIGHTED_PT_SIZE * 4);
    }
    const wxRect R(l, d, r - l, u - d);
    Refresh(false, &R);
#endif
}

void GfxCore::SetHere()
{
//    if (m_here.x == DBL_MAX) return;
    Point old = m_here;
    m_here.x = DBL_MAX;
    RefreshLine(old, m_there, m_here, m_there);
}

void GfxCore::SetHere(Double x, Double y, Double z)
{
    Point old = m_here;
    m_here.x = x;
    m_here.y = y;
    m_here.z = z;
    RefreshLine(old, m_there, m_here, m_there);
}

void GfxCore::SetThere()
{
//    if (m_there.x == DBL_MAX) return;
    Point old = m_there;
    m_there.x = DBL_MAX;
    RefreshLine(m_here, old, m_here, m_there);
}

void GfxCore::SetThere(Double x, Double y, Double z)
{
    Point old = m_there;
    m_there.x = x;
    m_there.y = y;
    m_there.z = z;
    RefreshLine(m_here, old, m_here, m_there);
}

void GfxCore::CreateHitTestGrid()
{
    // Clear hit-test grid.
    for (int i = 0; i < HITTEST_SIZE * HITTEST_SIZE; i++) {
        m_PointGrid[i].clear();
    }

    // Fill the grid.
    list<LabelInfo*>::const_iterator pos = m_Parent->GetLabels();
    list<LabelInfo*>::const_iterator end = m_Parent->GetLabelsEnd();
    while (pos != end) {
        LabelInfo* label = *pos++;

        if (!((m_Surface && label->IsSurface()) ||
	      (m_Legs && label->IsUnderground()) ||
	      (!label->IsSurface() && !label->IsUnderground()))) {
	    // if this station isn't to be displayed, skip to the next
	    // (last case is for stns with no legs attached)
            continue;
        }

        // Calculate screen coordinates.
        Double cx, cy, cz;
        Transform(label->GetX(), label->GetY(), label->GetZ(), &cx, &cy, &cz);
        if (cx < 0 || cx >= m_XSize) continue;
        if (cy < 0 || cy >= m_YSize) continue;

        cy = m_YSize - cy;

        // On-screen, so add to hit-test grid...
        int grid_x = int((cx * (HITTEST_SIZE - 1)) / m_XSize);
        int grid_y = int((cy * (HITTEST_SIZE - 1)) / m_YSize);

        m_PointGrid[grid_x + grid_y * HITTEST_SIZE].push_back(label);
    }

    m_HitTestGridValid = true;
}

void GfxCore::UpdateQuaternion()
{
    // Produce the rotation quaternion from the pan and tilt angles.
    
    Vector3 v1(0.0, 0.0, 1.0);
    Vector3 v2(1.0, 0.0, 0.0);
    Quaternion q1(v1, m_PanAngle);
    Quaternion q2(v2, m_TiltAngle);

    m_Params.rotation = q2 * q1; // care: quaternion multiplication
                                 //       is not commutative!
    SetRotation(m_Params.rotation);
}

void GfxCore::UpdateIndicators()
{
    if (m_Lists.indicators) {
        DeleteList(m_Lists.indicators);
        m_Lists.indicators = CreateList(this, &GfxCore::GenerateIndicatorDisplayList);
    }
}

//
//  Methods for controlling the orientation of the survey
//

void GfxCore::TurnCave(Double angle)
{
    // Turn the cave around its z-axis by a given angle.

    m_PanAngle += angle;
    if (m_PanAngle >= M_PI * 2.0) {
        m_PanAngle -= M_PI * 2.0;
    } else if (m_PanAngle < 0.0) {
        m_PanAngle += M_PI * 2.0;
    }

    m_HitTestGridValid = false;

    UpdateQuaternion();
    UpdateIndicators();
}

void GfxCore::TurnCaveTo(Double angle)
{
    // Turn the cave to a particular pan angle.
    TurnCave(angle - m_PanAngle);
}

void GfxCore::TiltCave(Double tilt_angle)
{
    // Tilt the cave by a given angle.
    if (m_TiltAngle + tilt_angle > M_PI_2) {
        tilt_angle = M_PI_2 - m_TiltAngle;
    } else if (m_TiltAngle + tilt_angle < -M_PI_2) {
        tilt_angle = -M_PI_2 - m_TiltAngle;
    }

    m_TiltAngle += tilt_angle;

    m_HitTestGridValid = false;

    UpdateQuaternion();
    UpdateIndicators();
}

void GfxCore::TranslateCave(int dx, int dy)
{
    AddTranslationScreenCoordinates(dx, dy);
    m_HitTestGridValid = false;

    ForceRefresh();
}

void GfxCore::DragFinished()
{
    UpdateNames();
    ForceRefresh();
}

void GfxCore::SetCoords(wxPoint point)
{
    // Update the coordinate or altitude display, given the (x, y) position in
    // window coordinates.  The relevant display is updated depending on
    // whether we're in plan or elevation view.

    Double cx, cy, cz;

    SetDataTransform();
    ReverseTransform(point.x, point.y, &cx, &cy, &cz);

    if (m_TiltAngle == M_PI_2) {
        m_Parent->SetCoords(cx + m_Parent->GetXOffset(), -cy + m_Parent->GetYOffset());
    }
    else if (m_TiltAngle == 0.0) {
        m_Parent->SetAltitude(-cz + m_Parent->GetZOffset());
    }
    else {
        m_Parent->ClearCoords();
    }
}

bool GfxCore::ChangingOrientation() const
{
    // Determine whether the cave is currently moving between orientations.

    return (m_SwitchingTo != 0);
}

bool GfxCore::ShowingCompass() const
{
    // Determine whether the compass is currently shown.

    return m_Compass;
}

bool GfxCore::ShowingClino() const
{
    // Determine whether the clino is currently shown.

    return m_Clino;
}

wxCoord GfxCore::GetCompassXPosition() const
{
    // Return the x-coordinate of the centre of the compass in window coordinates.
    
    return m_XSize - INDICATOR_OFFSET_X - INDICATOR_BOX_SIZE/2;
}

glaCoord GfxCore::GetClinoXPosition() const
{
    // Return the x-coordinate of the centre of the compass in window coordinates.

    return m_XSize - GetClinoOffset() - INDICATOR_BOX_SIZE/2;
}

wxCoord GfxCore::GetIndicatorYPosition() const
{
    // Return the y-coordinate of the centre of the indicators in window coordinates.

    return m_YSize - INDICATOR_OFFSET_Y - INDICATOR_BOX_SIZE/2;
}

wxCoord GfxCore::GetIndicatorRadius() const
{
    // Return the radius of each indicator.
    
    return (INDICATOR_BOX_SIZE - INDICATOR_MARGIN*2) / 2;
}

bool GfxCore::PointWithinCompass(wxPoint point) const
{
    // Determine whether a point (in window coordinates) lies within the compass.

    glaCoord dx = point.x - GetCompassXPosition();
    glaCoord dy = point.y - GetIndicatorYPosition();
    glaCoord radius = GetIndicatorRadius();
    
    return (dx * dx + dy * dy <= radius * radius);
}

bool GfxCore::PointWithinClino(wxPoint point) const
{
    // Determine whether a point (in window coordinates) lies within the clino.

    glaCoord dx = point.x - GetClinoXPosition();
    glaCoord dy = point.y - GetIndicatorYPosition();
    glaCoord radius = GetIndicatorRadius();
    
    return (dx * dx + dy * dy <= radius * radius);
}

bool GfxCore::PointWithinScaleBar(wxPoint point) const
{
    // Determine whether a point (in window coordinates) lies within the scale bar.

    return (point.x >= m_ScaleBar.offset_x &&
            point.x <= m_ScaleBar.offset_x + m_ScaleBar.width &&
            point.y <= m_YSize - m_ScaleBar.offset_y - SCALE_BAR_HEIGHT &&
            point.y >= m_YSize - m_ScaleBar.offset_y - SCALE_BAR_HEIGHT*2);
}

void GfxCore::SetCompassFromPoint(wxPoint point)
{
    // Given a point in window coordinates, set the heading of the survey.  If the point
    // is outside the compass, it snaps to 45 degree intervals; otherwise it operates as normal.

    wxCoord dx = point.x - GetCompassXPosition();
    wxCoord dy = point.y - GetIndicatorYPosition();
    wxCoord radius = GetIndicatorRadius();

    if (dx * dx + dy * dy <= radius * radius) {
        TurnCaveTo(atan2(dx, dy) - M_PI);
        m_MouseOutsideCompass = false;
    }
    else {
        TurnCaveTo(int(int((atan2(dx, dy) - M_PI) * 180.0 / M_PI) / 45) * M_PI_4);
        m_MouseOutsideCompass = true;
    }

    ForceRefresh();
}

void GfxCore::SetClinoFromPoint(wxPoint point)
{
    // Given a point in window coordinates, set the elevation of the survey.  If the point
    // is outside the clino, it snaps to 90 degree intervals; otherwise it operates as normal.

    glaCoord dx = point.x - GetClinoXPosition();
    glaCoord dy = point.y - GetIndicatorYPosition();
    glaCoord radius = GetIndicatorRadius();
    
    if (dx >= 0 && dx * dx + dy * dy <= radius * radius) {
        TiltCave(atan2(dy, dx) - m_TiltAngle);
        m_MouseOutsideElev = false;
    }
    else if (dy >= INDICATOR_MARGIN) {
        TiltCave(M_PI_2 - m_TiltAngle);
        m_MouseOutsideElev = true;
    }
    else if (dy <= -INDICATOR_MARGIN) {
        TiltCave(-M_PI_2 - m_TiltAngle);
        m_MouseOutsideElev = true;
    }
    else {
        TiltCave(-m_TiltAngle);
        m_MouseOutsideElev = true;
    }

    ForceRefresh();
}

void GfxCore::SetScaleBarFromOffset(wxCoord dx)
{
    // Set the scale of the survey, given an offset as to how much the mouse has
    // been dragged over the scalebar since the last scale change.

    SetScale((m_ScaleBar.width + dx) * m_Params.scale / m_ScaleBar.width);
    ForceRefresh();
}

void GfxCore::RedrawIndicators()
{
    // Redraw the compass and clino indicators.

    const wxRect r(m_XSize - INDICATOR_OFFSET_X - INDICATOR_BOX_SIZE*2 -
                     INDICATOR_GAP,
                   m_YSize - INDICATOR_OFFSET_Y - INDICATOR_BOX_SIZE,
                   INDICATOR_BOX_SIZE*2 + INDICATOR_GAP,
                   INDICATOR_BOX_SIZE);
                   
    m_RedrawOffscreen = true;
    Refresh(false, &r);
}

void GfxCore::StartRotation()
{
    // Start the survey rotating.
    
    m_Rotating = true;
    timer.Start(drawtime);
}

void GfxCore::ToggleRotation()
{
    // Toggle the survey rotation on/off.
    
    m_Rotating = !m_Rotating;

    if (m_Rotating) {
        timer.Start(drawtime);
    }
}

void GfxCore::StopRotation()
{
    // Stop the survey rotating.

    m_Rotating = false;
    UpdateNames();
    ForceRefresh();
}

bool GfxCore::CanRotate() const
{
    // Determine if the survey may be rotated.

    return m_RotationOK;
}

void GfxCore::ReverseRotation()
{
    // Reverse the direction of rotation.

    m_RotationStep = -m_RotationStep;
}

void GfxCore::RotateSlower(bool accel)
{
    // Decrease the speed of rotation, optionally by an increased amount.

    m_RotationStep /= accel ? 1.44 : 1.2;
/* FIXME    if (m_RotationStep < M_PI / 180.0) {
        m_RotationStep = (Double) M_PI / 180.0;
    } */
}

void GfxCore::RotateFaster(bool accel)
{
    // Increase the speed of rotation, optionally by an increased amount.

    m_RotationStep *= accel ? 1.44 : 1.2;
/*    if (m_RotationStep > 2.5 * M_PI) {
        m_RotationStep = (Double) 2.5 * M_PI;
    }*/
}

void GfxCore::SwitchToElevation()
{
    // Perform an animated switch to elevation view.

    switch (m_SwitchingTo) {
        case 0:
            timer.Start(drawtime);
            m_SwitchingTo = ELEVATION;
            break;
            
        case PLAN:
            m_SwitchingTo = ELEVATION;
            break;
            
        case ELEVATION:
            // a second order to switch takes us there right away
            TiltCave(-m_TiltAngle);
            m_SwitchingTo = 0;
            ForceRefresh();
    }
}

void GfxCore::SwitchToPlan()
{
    // Perform an animated switch to plan view.

    switch (m_SwitchingTo) {
        case 0:
            timer.Start(drawtime);
            m_SwitchingTo = PLAN;
            break;
            
        case ELEVATION:
            m_SwitchingTo = PLAN;
            break;

        case PLAN:
            // a second order to switch takes us there right away
            TiltCave(M_PI_2 - m_TiltAngle);
            m_SwitchingTo = 0;
            ForceRefresh();
    }
}

bool GfxCore::CanRaiseViewpoint() const
{
    // Determine if the survey can be viewed from a higher angle of elevation.
    
    return (m_TiltAngle < M_PI_2);
}

bool GfxCore::CanLowerViewpoint() const
{
    // Determine if the survey can be viewed from a lower angle of elevation.

    return (m_TiltAngle > -M_PI_2);
}

bool GfxCore::ShowingPlan() const
{
    // Determine if the survey is in plan view.
    
    return (m_TiltAngle == M_PI_2);
}

bool GfxCore::ShowingElevation() const
{
    // Determine if the survey is in elevation view.
    
    return (m_TiltAngle == 0.0);
}

bool GfxCore::ShowingMeasuringLine() const
{
    // Determine if the measuring line is being shown.
    
    return (m_there.x != DBL_MAX);
}

void GfxCore::ToggleFlag(bool* flag, bool refresh)
{
    *flag = !*flag;
    if (refresh) {
        ForceRefresh();
    }
}

int GfxCore::GetNumEntrances() const
{
    return m_Parent->GetNumEntrances();
}

int GfxCore::GetNumFixedPts() const
{
    return m_Parent->GetNumFixedPts();
}

int GfxCore::GetNumExportedPts() const
{
    return m_Parent->GetNumExportedPts();
}

void GfxCore::ClearTreeSelection()
{
    m_Parent->ClearTreeSelection();
}

void GfxCore::CentreOn(Double x, Double y, Double z)
{
    m_Params.translation.x = -x;
    m_Params.translation.y = -y;
    m_Params.translation.z = -z;
    
    SetTranslation(-x, -y, -z);
    m_HitTestGridValid = false;
    
    ForceRefresh();
}

void GfxCore::ForceRefresh()
{
    // Force a redraw of the offscreen bitmap.
    m_RedrawOffscreen = true;
    Refresh(false);
}

void GfxCore::GenerateDisplayListSurface()
{
    //assert(m_HaveData);
    //DrawPolylines(m_SurfaceDepth, false, m_SurfacePolylines, surface_num_segs, surface_vertices);
}

void GfxCore::GenerateDisplayList()
{
    // Generate the display list for the survey data.
    
    static wxPoint blob[10] = {
        wxPoint(-1,  2),
        wxPoint( 1,  2),
        wxPoint( 2,  1),
        wxPoint(-2,  1),
        wxPoint(-2,  0),
        wxPoint( 2,  0),
        wxPoint( 2, -1),
        wxPoint(-2, -1),
        wxPoint(-1, -2),
        wxPoint( 2, -2) // (1, -2) for any platform which draws the last dot
    };
    static wxPoint cross1[2] = {
        wxPoint(-CROSS_SIZE, -CROSS_SIZE),
        wxPoint(CROSS_SIZE + 1, CROSS_SIZE + 1) // remove +1 if last dot drawn
    };
    static wxPoint cross2[2] = {
        wxPoint(-CROSS_SIZE, CROSS_SIZE),
        wxPoint(CROSS_SIZE + 1, -CROSS_SIZE - 1) // remove +/-1 if last dot
    };

    timer.Start(); // reset timer

    // Invalidate hit-test grid.
    m_HitTestGridValid = false;

    if (m_HaveData) {
        bool grid_first = (m_TiltAngle >= 0.0);

        if (m_Grid && grid_first) {
            DrawGrid();
        }

        // Draw underground legs.
        if (m_Legs) {
	    DrawPolylines(true, m_Tubes, m_Polylines, num_segs, vertices); 
        }
    }
    
    drawtime = timer.Time();
}

void GfxCore::GenerateBlobsDisplayList()
{
    // Plot crosses and/or blobs.
    if (m_Crosses || m_Entrances || m_FixedPts || m_ExportedPts) {
        list<LabelInfo*>::const_reverse_iterator pos =
            m_Parent->GetRevLabels();
        while (pos != m_Parent->GetRevLabelsEnd()) {
            LabelInfo* label = *pos++;

            // When more than one flag is set on a point:
            // search results take priority over entrance highlighting
            // which takes priority over fixed point
            // highlighting, which in turn takes priority over exported
            // point highlighting.

            enum AvenColour col;
            enum {BLOB, CROSS} shape = BLOB;

            if (!((m_Surface && label->IsSurface()) ||
                  (m_Legs && label->IsUnderground()) ||
                  (!label->IsSurface() && !label->IsUnderground()))) {
                // if this station isn't to be displayed, skip to the next
                // (last case is for stns with no legs attached)
                continue;
            }

            if (label->IsHighLighted()) {
                col = col_YELLOW;
            }
            else if (m_Entrances && label->IsEntrance()) {
                col = col_GREEN;
            }
            else if (m_FixedPts && label->IsFixedPt()) {
                col = col_RED;
            }
            else if (m_ExportedPts && label->IsExportedPt()) {
                col = col_TURQUOISE;
            }
            else if (m_Crosses) {
                col = col_LIGHT_GREY;
                shape = CROSS;
            }
            else {
                continue;
            }

            Double size = SurveyUnitsAcrossViewport() * 3.0 / m_XSize;
/*
            Double x3 = label->GetX() + m_Params.translation.x;
            Double y3 = label->GetY() + m_Params.translation.y;
            Double z3 = label->GetZ() + m_Params.translation.z;

            // Calculate screen coordinates, and check if the point is
            // visible - this is faster, and avoids coordinate
            // wrap-around problems
            int x = (int) (x3 * m_00 + y3 * m_01 + z3 * m_02) + m_XCentre;
            if (x < -CROSS_SIZE || x >= m_XSize + CROSS_SIZE) continue;
            int y = -(int) (x3 * m_20 + y3 * m_21 + z3 * m_22) + m_YCentre;
            if (y < -CROSS_SIZE || y >= m_YSize + CROSS_SIZE) continue;

            SetColour(col);
            */
            if (shape == CROSS) {
               // m_DrawDC.DrawLines(2, cross1, x, y);
               // m_DrawDC.DrawLines(2, cross2, x, y);
            } else {
                DrawSphere(m_Pens[col], label->GetX(), label->GetY(), label->GetZ(), size, 16);
            }
        }
    }
/*
        if (m_Grid && !grid_first) DrawGrid();

        // Draw station names.
        if (m_Names) DrawNames();
*/
}

void GfxCore::GenerateIndicatorDisplayList()
{
    // Draw depthbar.
    if (m_Depthbar) {
        DrawDepthbar();
    }

    // Draw compass or elevation/heading indicators.
    if ((m_Compass && m_RotationOK) || (m_Clino && m_Lock == lock_NONE)) {
        Draw2dIndicators();
    }
    
    // Draw scalebar.
    if (m_Scalebar) {
        DrawScalebar();
    }
}

void GfxCore::SetColourFromHeight(Double z, Double factor)
{
    // Set the drawing colour based on an altitude.
   
    Double z_ext = m_Parent->GetZExtent();

    // points arising from tubes may be slightly outside the limits...
    if (z < -z_ext * 0.5) z = -z_ext * 0.5;
    if (z > z_ext * 0.5) z = z_ext * 0.5;

    Double z_offset = z + z_ext * 0.5;

    Double how_far = z_offset / z_ext;
    assert(how_far >= 0.0);
    assert(how_far <= 1.0);

    int band = int(floor(how_far * (m_Bands - 1)));
    int next_band = (band == (m_Bands - 1)) ? band : band+1;
    
    GLAPen pen1 = m_Parent->GetPen(band);
    const GLAPen& pen2 = m_Parent->GetPen(next_band);
    
    Double interval = z_ext / (m_Bands - 1);
    Double into_band = z_offset / interval - band;
   
//    printf("%g z_offset=%g interval=%g band=%d\n", into_band,
//            z_offset, interval, band);
    // FIXME: why do we need to clamp here?  Is it because the walls can
    // extend further up/down than the centre-line?
    if (into_band < 0.0) into_band = 0.0;
    if (into_band > 1.0) into_band = 1.0;
    assert(into_band >= 0.0);
    assert(into_band <= 1.0);
    
    pen1.Interpolate(pen2, into_band);
    
    SetColour(pen1, true, factor);
}

void GfxCore::PlaceVertexWithColour(Double x, Double y, Double z, Double factor)
{
    // Set the drawing colour based on an altitude, and then place a vertex.

    SetColourFromHeight(z, factor);
    PlaceVertex(x, y, z);
}

static void IntersectLineWithPlane(Double x0, Double y0, Double z0,
				   Double x1, Double y1, Double z1,
				   Double z, Double& x, Double& y)
{
    // Find the intersection point of the line (x0, y0, z0) -> (x1, y1, z1)
    // with the plane parallel to the xy-plane with z-axis intersection z.
    assert(z1 - z0 != 0.0);

    Double t = (z - z0) / (z1 - z0);
//    assert(0.0 <= t && t <= 1.0);		FIXME: rounding problems!

    x = x0 + t * (x1 - x0);
    y = y0 + t * (y1 - y0);
}

void GfxCore::SplitLineAcrossBands(int band, int band2,
				   const Vector3 &p, const Vector3 &p2,
				   Double factor)
{
    int step = (band < band2) ? 1 : -1;
    for (int i = band; i != band2; i += step) {
	Double x, y, z;
	z = GetDepthBoundaryBetweenBands(i, i + step);
	IntersectLineWithPlane(p.getX(), p.getY(), p.getZ(),
			       p2.getX(), p2.getY(), p2.getZ(),
			       z, x, y);
	PlaceVertexWithColour(x, y, z, factor);
    }
}

int GfxCore::GetDepthColour(Double z) const
{
    // Return the (0-based) depth colour band index for a z-coordinate.
    Double z_ext = m_Parent->GetZExtent();
    z += z_ext / 2;
    return int((z / (z_ext == 0.0 ? 1.0 : z_ext)) * (m_Bands - 1));
}

Double GfxCore::GetDepthBoundaryBetweenBands(int a, int b) const
{
    // Return the z-coordinate of the depth colour boundary between
    // two adjacent depth colour bands (specified by 0-based indices).

    assert((a == b - 1) || (a == b + 1));

    int band = (a > b) ? a : b; // boundary N lies on the bottom of band N.
    if (m_Bands == 1) return 0;
    Double z_ext = m_Parent->GetZExtent();
    return (z_ext * band / (m_Bands - 1)) - z_ext / 2;
}

void GfxCore::AddQuadrilateral(const Vector3 &a, const Vector3 &b, 
			       const Vector3 &c, const Vector3 &d)
{
    Vector3 normal = (a - c) * (d - b);
    normal.normalise();
    Double factor = dot(normal, light) * .3 + .7;
    int a_band, b_band, c_band, d_band;
    a_band = GetDepthColour(a.getZ());
    a_band = min(max(a_band, 0), m_Bands);
    b_band = GetDepthColour(b.getZ());
    b_band = min(max(b_band, 0), m_Bands);
    c_band = GetDepthColour(c.getZ());
    c_band = min(max(c_band, 0), m_Bands);
    d_band = GetDepthColour(d.getZ());
    d_band = min(max(d_band, 0), m_Bands);
    BeginPolygon();
////    PlaceNormal(normal.getX(), normal.getY(), normal.getZ());
    PlaceVertexWithColour(a.getX(), a.getY(), a.getZ(), factor);
    if (a_band != b_band) {
	SplitLineAcrossBands(a_band, b_band, a, b, factor);
    }
    PlaceVertexWithColour(b.getX(), b.getY(), b.getZ(), factor);
    if (b_band != c_band) {
	SplitLineAcrossBands(b_band, c_band, b, c, factor);
    }
    PlaceVertexWithColour(c.getX(), c.getY(), c.getZ(), factor);
    if (c_band != d_band) {
	SplitLineAcrossBands(c_band, d_band, c, d, factor);
    }
    PlaceVertexWithColour(d.getX(), d.getY(), d.getZ(), factor);
    if (d_band != a_band) {
	SplitLineAcrossBands(d_band, a_band, d, a, factor);
    }
    EndPolygon();
}

void GfxCore::DrawPolylines(bool depth_colour, bool tubes, int num_polylines,
			    const int* num_points, const Point* vertices)
{
    // Draw a set of polylines.
    
//    EnableSmoothPolygons();
    
    for (int polyline = 0; polyline < num_polylines; polyline++) {
        int length = num_points[polyline];
        const Point* vertices_start = vertices;

        assert(length > 1);

        SetColour(m_Pens[7]);

	BeginPolyline();
	
	if (depth_colour) {
	    PlaceVertexWithColour(vertices->x, vertices->y, vertices->z, 1.0);

	    int band = GetDepthColour(vertices->z);
	    ++vertices;

	    for (int segment = 1; segment < length; segment++) {
		int band2 = GetDepthColour(vertices->z);
		if (band != band2) {
		    int step = (band < band2) ? 1 : -1;
		    for (int i = band; i != band2; i += step) {
			Double x, y, z;
			z = GetDepthBoundaryBetweenBands(i, i + step);
			IntersectLineWithPlane(vertices[-1].x,
					       vertices[-1].y,
					       vertices[-1].z,
					       vertices->x,
					       vertices->y,
					       vertices->z,
					       z, x, y);
			PlaceVertexWithColour(x, y, z, 1.0);
		    }
		}
		PlaceVertexWithColour(vertices->x, vertices->y, vertices->z,
                                      1.0);
		band = band2;
		++vertices;
	    }
	} else {
	    for (int segment = 0; segment < length; segment++) {
	        SetColour(m_Parent->GetSurfacePen());
		PlaceVertex(vertices->x, vertices->y, vertices->z);
		++vertices;
	    }
	}

	EndPolyline();

	if (!tubes) continue;

        Double size = 1;
        
        Vector3 u[4];
        Vector3 prev_pt_v;

	Vector3 right, up;

	const Vector3 up_v(0.0, 0.0, 1.0);

	Vector3 last_right(1.0, 0.0, 0.0);
	
        for (int segment = 0; segment < length; segment++) {
            // get the coordinates of this vertex
            Vector3 pt_v(vertices_start->x, vertices_start->y,
                         vertices_start->z);
            vertices_start++;

            if (segment == 0) {
		Double h = sqrd(vertices_start->x - pt_v.getX()) +
			   sqrd(vertices_start->y - pt_v.getY());
		Double v = sqrd(vertices_start->z - pt_v.getZ());
		if (h + v > 30.0 * 30.0) {
		    Double scale = 30.0 / sqrt(h + v);
		    h *= scale;
		    v *= scale;
		}
                size = sqrt(h + v / 9);
            } else if (segment == length - 1) {
		Double h = sqrd(prev_pt_v.getX() - pt_v.getX()) +
			   sqrd(prev_pt_v.getY() - pt_v.getY());
		Double v = sqrd(prev_pt_v.getZ() - pt_v.getZ());
		if (h + v > 30.0 * 30.0) {
		    Double scale = 30.0 / sqrt(h + v);
		    h *= scale;
		    v *= scale;
		}
                size = sqrt(h + v / 9);
            } else {
		Double h = sqrd(vertices_start->x - pt_v.getX()) +
			   sqrd(vertices_start->y - pt_v.getY());
		Double v = sqrd(vertices_start->z - pt_v.getZ());
		if (h + v > 30.0 * 30.0) {
		    Double scale = 30.0 / sqrt(h + v);
		    h *= scale;
		    v *= scale;
		}
                size = sqrt(h + v / 9);
		h = sqrd(prev_pt_v.getX() - pt_v.getX()) +
		    sqrd(prev_pt_v.getY() - pt_v.getY());
		v = sqrd(prev_pt_v.getZ() - pt_v.getZ());
		if (h + v > 30.0 * 30.0) {
		    Double scale = 30.0 / sqrt(h + v);
		    h *= scale;
		    v *= scale;
		}
                size += sqrt(h + v / 9);
                size /= 2;
            }
	    size /= 4;

	    double z_pitch_adjust = 0.0;
            bool cover_end = false;
            
            if (segment == 0) {
                // first segment
        
                // get the coordinates of the next vertex
                Vector3 next_pt_v(vertices_start->x, vertices_start->y, vertices_start->z);
                
                // calculate vector from this pt to the next one
                Vector3 leg_v = pt_v - next_pt_v;
                
                // obtain a vector in the LRUD plane
  		right = leg_v * up_v;
		if (right.magnitude() == 0) {
		    right = last_right;
		    // obtain a second vector in the LRUD plane, perpendicular
		    // to the first
		    up = right * leg_v;
		} else {
		    last_right = right;
		    up = up_v;
		}

                cover_end = true;
            }
            else if (segment == length - 1) {
                // last segment
                
                // calculate vector from the previous pt to this one
                Vector3 leg_v = prev_pt_v - pt_v;

                // obtain a horizontal vector in the LRUD plane
                right = leg_v * up_v;
		if (right.magnitude() == 0) {
		    right = Vector3(last_right.getX(), last_right.getY(), 0.0);
		    // obtain a second vector in the LRUD plane, perpendicular
		    // to the first
		    up = right * leg_v;
		} else {
		    last_right = right;
		    up = up_v;
		}

                cover_end = true;
            }
            else {
                // intermediate segment

                // get the coordinates of the next vertex
                Vector3 next_pt_v(vertices_start->x, vertices_start->y, vertices_start->z);

                // calculate vectors from this vertex to the next vertex, and
                // from the previous vertex to this one
                Vector3 leg1_v = prev_pt_v - pt_v;
                Vector3 leg2_v = pt_v - next_pt_v;

		// obtain horizontal vectors perpendicular to both legs,
		// then normalise and average to get a horizontal bisector
		Vector3 r1 = leg1_v * up_v;
		Vector3 r2 = leg2_v * up_v;
		r1.normalise();
		r2.normalise();
		right = r1 + r2;
		if (right.magnitude() == 0) {
		    right = last_right;
		    up = right * leg1_v;
		} else {
		    if (r1.magnitude() == 0) {
			Vector3 n = leg1_v;
			n.normalise();
			z_pitch_adjust = n.getZ();
			up = Vector3(0, 0, leg1_v.getZ());
			up = right * up;
			// Rotate pitch section to minimise the "tortional
			// stress" - FIXME: use triangles instead of
			// rectangles?
			int shift = 0;
			Double maxdotp, dotp;
			maxdotp = dot(up - right, u[0]);
			dotp = dot(up - right, u[1]);
			if (dotp > maxdotp) { maxdotp = dotp; shift = 1; }
			dotp = dot(up - right, u[2]);
			if (dotp > maxdotp) { maxdotp = dotp; shift = 2; }
			dotp = dot(up - right, u[3]);
			if (dotp > maxdotp) { maxdotp = dotp; shift = 3; }
			if (shift) {
			    Vector3 temp(u[0]);
			    int j = 0;
			    for (int i = 0; i < 3; ++i) {
				int k = (j + shift) % 4;
				u[j] = u[k];
				j = k;
			    }
			    u[j] = temp;
			}
		    } else if (r2.magnitude() == 0) {
			Vector3 n = leg2_v;
			n.normalise();
			z_pitch_adjust = n.getZ();
			up = Vector3(0, 0, leg2_v.getZ());
			up = right * up;
		    } else {
			up = up_v;
		    }
		    last_right = right;
		}
            }

	    // scale the vectors in the LRUD plane appropriately
	    right.normalise();
	    up.normalise();
            
	    if (z_pitch_adjust != 0) up = up + Vector3(0, 0, z_pitch_adjust);
	    right *= size;
	    up *= size;

	    // produce coordinates of the corners of the LRUD "plane"
	    Vector3 v[4];
	    v[0] = pt_v - right + up;
	    v[1] = pt_v + right + up;
	    v[2] = pt_v + right - up;
	    v[3] = pt_v - right - up;
            
            if (segment > 0) {
#if 0 // pre-Mark
		AddQuadrilateral(u[0], u[1], v[1], v[0]);
		AddQuadrilateral(v[3], v[2], u[2], u[3]);
		AddQuadrilateral(u[1], u[2], v[2], v[1]);
		AddQuadrilateral(u[3], u[0], v[0], v[3]);
#else
                AddQuadrilateral(v[0], v[1], u[1], u[0]);
                AddQuadrilateral(u[2], u[3], v[3], v[2]);
                AddQuadrilateral(v[1], v[2], u[2], u[1]);
                AddQuadrilateral(v[3], v[0], u[0], u[3]);
#endif
            }

            if (cover_end) {
#if 0 // pre-Mark
                AddQuadrilateral(v[0], v[1], v[2], v[3]);
#else
                AddQuadrilateral(v[3], v[2], v[1], v[0]);
#endif
            }

            prev_pt_v = pt_v;
            u[0] = v[0];
            u[1] = v[1];
            u[2] = v[2];
            u[3] = v[3];
        }
    }

  //  DisableSmoothPolygons();
}

void GfxCore::FullScreenMode()
{
    // Toggle full-screen mode.
    wxWindow * old_parent = GetParent();
    if (old_parent != m_ParentWin) {
	Reparent(m_ParentWin);
	old_parent->Destroy();
    } else {
	// Determine the dimensions of the screen.
	int width;
	int height;
	wxDisplaySize(&width, &height);

	// Create a window covering the entire screen, with no decorations.
	wxFrame * full_screen;
	full_screen = new wxFrame(m_Parent, 100, "Aven",
				  wxPoint(0, 0), wxSize(width, height),
				  wxSTAY_ON_TOP | wxFRAME_FLOAT_ON_PARENT);
	full_screen->Show(true);
	
	// Reparent ourselves to be inside the new frame.
	Reparent(full_screen);
    }
}

