/***************************************************************************
                          kstars.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Mon Feb  5 01:11:45 PST 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//JH 11.06.2002: replaced infoPanel with infoBoxes
//JH 24.08.2001: reorganized infoPanel
//JH 25.08.2001: added toolbar, converted menu items to KAction objects
//JH 25.08.2001: main window now resizable, window size saved in config file


#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
#include <kdebug.h>
#include <qpalette.h>

#include "Options.h"
#include "kstars.h"
#include "simclock.h"
#include "finddialog.h"
#include "ksutils.h"
#include "infoboxes.h"

// to remove warnings
#include "indimenu.h"
#include "indidriver.h"

KStars::KStars( bool doSplash ) :
	DCOPObject("KStarsInterface"), KMainWindow(),
	skymap(0), findDialog(0), centralWidget(0),
	AAVSODialog(0), DialogIsObsolete(false)
{
	pd = new privatedata(this);

	// we're nowhere near ready to take dcop calls
	kapp->dcopClient()->suspend();

	if ( doSplash ) {
		pd->kstarsData = new KStarsData();
		QObject::connect(pd->kstarsData, SIGNAL( initFinished(bool) ),
				this, SLOT( datainitFinished(bool) ) );

		pd->splash = new KStarsSplash(0, "Splash");
		QObject::connect(pd->splash, SIGNAL( closeWindow() ), this, SLOT( closeWindow() ) );
		QObject::connect(pd->kstarsData, SIGNAL( progressText(QString) ),
				pd->splash, SLOT( setMessage(QString) ));
		pd->splash->show();
	}
	pd->kstarsData->initialize();

	//Set Geographic Location
	pd->kstarsData->setLocationFromOptions();

	//set up Dark color scheme for application windows
	DarkPalette = QPalette(QColor("red4"), QColor("DarkRed"));
	DarkPalette.setColor( QPalette::Normal, QColorGroup::Base, QColor( "black" ) );
	DarkPalette.setColor( QPalette::Normal, QColorGroup::Text, QColor( "red2" ) );
	DarkPalette.setColor( QPalette::Normal, QColorGroup::Highlight, QColor( "red2" ) );
	DarkPalette.setColor( QPalette::Normal, QColorGroup::HighlightedText, QColor( "black" ) );
	
	//set color scheme
	OriginalPalette = QApplication::palette();
	pd->kstarsData->colorScheme()->loadFromConfig( kapp->config() );
	if ( Options::darkAppColors() ) {
		QApplication::setPalette( DarkPalette, true );
	}

	#if ( __GLIBC__ >= 2 &&__GLIBC_MINOR__ >= 1 )
	kdDebug() << "glibc >= 2.1 detected.  Using GNU extension sincos()" << endl;
	#else
	kdDebug() << "Did not find glibc >= 2.1.  Will use ANSI-compliant sin()/cos() functions." << endl;
	#endif
}

KStars::KStars( KStarsData* kd )
	: DCOPObject("KStarsInterface"), KMainWindow( )
{
	// The assumption is that kstarsData is fully initialized

	pd = new privatedata(this);
	pd->kstarsData = kd;
	pd->buildGUI();

	kd->clock()->start();

	show();
}

KStars::~KStars()
{
	//store focus values in Options
	Options::setFocusRA( skymap->focus()->ra()->Hours() );
	Options::setFocusDec( skymap->focus()->dec()->Degrees() );

	//Store Window geometry in Options object
	Options::setWindowWidth( width() );
	Options::setWindowHeight( height() );

	//We need to explicitly save the colorscheme data to the config file
	data()->colorScheme()->saveToConfig( kapp->config() );

	//synch the config file with the Config object
	Options::writeConfig();

	clearCachedFindDialog();

	if (skymap) delete skymap;
	if (pd) delete pd;
	if (centralWidget) delete centralWidget;
	if (AAVSODialog) delete AAVSODialog;
	if (indimenu) delete indimenu;
	if (indidriver) delete indidriver;
}

void KStars::clearCachedFindDialog() {
	if ( findDialog  ) {  // dialog is cached
/**
	*Delete findDialog only if it is not opened
	*/
		if ( findDialog->isHidden() ) {
			delete findDialog;
			findDialog = 0;
			DialogIsObsolete = false;
		}
		else
			DialogIsObsolete = true;  // dialog was opened so it could not deleted
   }
}

void KStars::updateTime( const bool automaticDSTchange ) {
	dms oldLST( LST()->Degrees() );
	// Due to frequently use of this function save data and map pointers for speedup.
	// Save options and geo() to a pointer would not speedup because most of time options
	// and geo will accessed only one time.
	KStarsData *Data = data();
	SkyMap *Map = map();

	Data->updateTime( geo(), Map, automaticDSTchange );
	if ( infoBoxes()->timeChanged(Data->UTime, Data->LTime, LST(), Data->CurrentDate) )
	        Map->update();

	//We do this outside of kstarsdata just to get the coordinates
	//displayed in the infobox to update every second.
//	if ( !Options::isTracking() && LST()->Degrees() > oldLST.Degrees() ) {
//		int nSec = int( 3600.*( LST()->Hours() - oldLST.Hours() ) );
//		Map->focus()->setRA( Map->focus()->ra()->Hours() + double( nSec )/3600. );
//		if ( Options::useAltAz() ) Map->focus()->EquatorialToHorizontal( LST(), geo()->lat() );
//		Map->showFocusCoords();
//	}

	//If time is accelerated beyond slewTimescale, then the clock's timer is stopped,
	//so that it can be ticked manually after each update, in order to make each time
	//step exactly equal to the timeScale setting.
	//Wrap the call to manualTick() in a singleshot timer so that it doesn't get called until
	//the skymap has been completely updated.
	if ( Data->clock()->isManualMode() && Data->clock()->isActive() ) {
		QTimer::singleShot( 0, Data->clock(), SLOT( manualTick() ) );
	}
}

KStarsData* KStars::data( void ) { return pd->kstarsData; }

#include "kstars.moc"
