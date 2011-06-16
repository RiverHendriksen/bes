// BESGlobalIQ.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004-2009 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "BESGlobalIQ.h"
#include "BESInitList.h"
#include "BESInitializer.h"

bool BESGlobalIQ::is_initialized = false ;

/** @brief initialize global objects in an orderly fashion.
 *
 * There are multiple levels of initialization, which allows for the
 * ordering of global objects to be created, functions to be called,
 * whatever initialization needs to take place for your application. This
 * function traverses the different levels, in order, calling the first
 * initialization function for that level. The initialization object is
 * BESGlobalInit, which contains an initialization function, a termination
 * function (if one is provided), and a pointer to the next BESGlobalInit
 * object. So there can be multiple initialization routines called per
 * level.
 *
 * @param argc number of arguments being passed in the argv parameter. This
 * is the same as the argc command line argument passed to main.
 * @param argv the actual arguments being passed. This is the same as the
 * command line arguments passed to main.
 * @return returns true if initialization is successful, false if
 * initialization fails and the application should terminate. Initialization
 * stops after the first initialization function returns false.
 * @see BESGlobalInit
 */
bool BESGlobalIQ::BESGlobalInit(int argc, char **argv)
{
    bool retVal = true;
    if (BESGlobalIQ::is_initialized == false)
    {
        for (int i = 0; i < 5; i++)
        {
            if (BESGlobalInitList[i])
            {
                retVal = BESGlobalInitList[i]->initialize(argc, argv);
                if (retVal != true)
                {
                    break;
                }
            }
        }
        BESGlobalIQ::is_initialized = true;
    }

    return retVal;
}

/** @brief Runs the termination functions in reverse order of
 * initialization, providing the application to clean up its global objects.
 *
 * This function traverses the levels in reverse order calling the first
 * termination function in that level.
 *
 * @return Returns true at this time. If termination fails for some reason
 * termination continues on.
 * @see BESGlobalInit
 */
bool
BESGlobalIQ::BESGlobalQuit()
{
    if( BESGlobalIQ::is_initialized == true )
    {
	for(int i = 4; i >= 0; i--)
	{
	    if(BESGlobalInitList[i])
	    {
		BESGlobalInitList[i]->terminate();
	    }
	}
	BESGlobalIQ::is_initialized = false ;
    }

    return true ;
}

