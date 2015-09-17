// BESXMLDefineCommand.h

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#ifndef A_BESXMLDefineCommand_h
#define A_BESXMLDefineCommand_h 1

#include <vector>

using std::vector;

#include "BESXMLCommand.h"
#include "BESDataHandlerInterface.h"

class BESXMLDefineCommand: public BESXMLCommand {
private:
    string _default_constraint;
    string _default_dap4_constraint;
    string _default_dap4_function;
    vector<string> _containers;
    map<string, string> _stores;
    map<string, string> _constraints;
    map<string, string> _dap4constraints;
    map<string, string> _dap4functions;
    map<string, string> _attributes;
    void handle_container_element(const string &action, xmlNode *node, const string &vallues,
        map<string, string> &props);
    void handle_aggregate_element(const string &action, xmlNode *node, const string &vallues,
        map<string, string> &props);
public:
    BESXMLDefineCommand(const BESDataHandlerInterface &base_dhi);
    virtual ~BESXMLDefineCommand()
    {
    }

    virtual void parse_request(xmlNode *node);

    virtual bool has_response()
    {
        return false;
    }

    virtual void prep_request();

    virtual void dump(ostream &strm) const;

    static BESXMLCommand * CommandBuilder(const BESDataHandlerInterface &base_dhi);
};

#endif // A_BESXMLDefineCommand_h

