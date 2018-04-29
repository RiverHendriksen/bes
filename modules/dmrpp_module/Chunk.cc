// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of the BES

// Copyright (c) 2016 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
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
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <sstream>
#include <cstdlib>
#include <cassert>

#include <BESDebug.h>
#include <BESInternalError.h>
#include <BESContextManager.h>

#include "Chunk.h"
#include "DmrppUtil.h"
#include "DmrppRequestHandler.h"

const string debug = "dmrpp";

using namespace std;

namespace dmrpp {

/**
 * @brief parse the chunk position string
 *
 * Extract information from the chunk position string and store as a
 * vector of integers in the instance
 *
 * @note If we can change the DMR++ syntax to be less verbose and use
 * a list of ints with whitespace as a separator, then the parsing code
 * will be much simpler since istringstream is designed to deal with exactly
 * that form of input.
 *
 * @param pia The chunk position string.
 */
void Chunk::set_position_in_array(const string &pia)
{
    if (pia.empty()) return;

    if (d_chunk_position_in_array.size()) d_chunk_position_in_array.clear();

    // Assume input is [x,y,...,z] where x, ..., are integers; modest syntax checking
    // [1] is a minimal 'position in array' string.
    if (pia.find('[') == string::npos || pia.find(']') == string::npos || pia.length() < 3)
        throw BESInternalError("while parsing a DMR++, chunk position string malformed", __FILE__, __LINE__);

    if (pia.find_first_not_of("[]1234567890,") != string::npos)
        throw BESInternalError("while parsing a DMR++, chunk position string illegal character(s)", __FILE__, __LINE__);

    // string off []; iss holds x,y,...,z
    istringstream iss(pia.substr(1, pia.length()-2));

    char c;
    unsigned int i;
    while (!iss.eof() ) {
        iss >> i; // read an integer

        d_chunk_position_in_array.push_back(i);

        iss >> c; // read a separator (,)
    }
}

/**
 * @brief Returns a curl range argument.
 * The libcurl requires a string argument for range-ge activitys, this method
 * constructs one in the required syntax from the offset and size information
 * for this byteStream.
 *
 */
std::string Chunk::get_curl_range_arg_string()
{
    ostringstream range;   // range-get needs a string arg for the range
    range << d_offset << "-" << d_offset + d_size - 1;
    return range.str();
}

/**
 * @brief Modify the \arg data_access_url so that it include tracking info
 *
 * The tracking info is the value of the BESContext "cloudydap".
 *
 * @param data_access_url The URL to hack
 */
void Chunk::add_tracking_query_param(string &data_access_url)
{
    /** - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Cloudydap test hack where we tag the S3 URLs with a query string for the S3 log
     * in order to track S3 requests. The tag is submitted as a BESContext with the
     * request. Here we check to see if the request is for an AWS S3 object, if
     * it is AND we have the magic BESContext "cloudydap" then we add a query
     * parameter to the S3 URL for tracking purposes.
     *
     * Should this be a function? FFS why? This is the ONLY place where this needs
     * happen, as close to the curl call as possible and we can just turn it off
     * down the road. - ndp 1/20/17 (EOD)
     */
    std::string aws_s3_url("https://s3.amazonaws.com/");
    // Is it an AWS S3 access?
    if (!data_access_url.compare(0, aws_s3_url.size(), aws_s3_url)) {
        // Yup, headed to S3.
        string cloudydap_context("cloudydap");
        bool found;
        string cloudydap_context_value;
        cloudydap_context_value = BESContextManager::TheManager()->get_context(cloudydap_context, found);
        if (found) {
            data_access_url += "?cloudydap=" + cloudydap_context_value;
        }
    }
}

#if 0
void Chunk::add_to_multi_read_queue(CURLM *multi_handle)
{
    BESDEBUG(debug,"Chunk::"<< __func__ <<"() - BEGIN  " << to_string() << endl);

    if (d_is_read || d_is_in_multi_queue) {
        BESDEBUG("dmrpp", "Chunk::"<< __func__ <<"() - Chunk has been " << (d_is_in_multi_queue?"queued to be ":"") << "read! Returning." << endl);
        return;
    }

    // This call uses the internal size param and allocates the buffer's memory
    set_rbuf_to_size();

    string data_access_url = get_data_url();

    BESDEBUG(debug,"Chunk::"<< __func__ <<"() - data_access_url "<< data_access_url << endl);

    add_tracking_query_param(data_access_url);

    string range = get_curl_range_arg_string();

    BESDEBUG(debug,
        __func__ <<" - Retrieve  " << get_size() << " bytes " "from "<< data_access_url << ": " << range << endl);

    CURL* curl = curl_easy_init();
    if (!curl) {
        throw BESInternalError("Unable to initialize curl handle", __FILE__, __LINE__);
    }

    CURLcode res = curl_easy_setopt(curl, CURLOPT_URL, data_access_url.c_str());
    if (res != CURLE_OK) throw BESInternalError("string(curl_easy_strerror(res))", __FILE__, __LINE__);

    // Use CURLOPT_ERRORBUFFER for a human-readable message
    //
    res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, d_curl_error_buf);
    if (res != CURLE_OK) throw BESInternalError("string(curl_easy_strerror(res))", __FILE__, __LINE__);

    // get the offset to offset + size bytes
    if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str() /*"0-199"*/))
    throw BESInternalError(string("HTTP Error: ").append(d_curl_error_buf), __FILE__, __LINE__);

    // Pass all data to the 'write_data' function
    if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chunk_write_data))
    throw BESInternalError(string("HTTP Error: ").append(d_curl_error_buf), __FILE__, __LINE__);

    // Pass this to write_data as the fourth argument
    if (CURLE_OK != curl_easy_setopt(curl, CURLOPT_WRITEDATA, this))
    throw BESInternalError(string("HTTP Error: ").append(d_curl_error_buf), __FILE__, __LINE__);

    /* add the individual transfers */
    curl_multi_add_handle(multi_handle, curl);

    BESDEBUG(debug, "Chunk::"<< __func__ <<"() - Added to multi_handle: "<< to_string() << endl);

    /* we start some action by calling perform right away */
    // int still_running;
    //  curl_multi_perform(multi_handle, &still_running);
    d_curl_handle = curl;
    d_is_in_multi_queue = true;

    BESDEBUG(debug, __func__ <<"() - END  "<< to_string() << endl);
}
#endif


void Chunk::inflate_chunk(bool deflate, bool shuffle, unsigned int chunk_size, unsigned int elem_width)
{
    // If data are compressed/encoded, then decode them here.
    // At this point, the bytes_read property would be changed.
    // The file that implements the deflate filter is H5Zdeflate.c in the hdf5 source.
    // The file that implements the shuffle filter is H5Zshuffle.c.

    // TODO This code is pretty naive - there are apparently a number of
    // different ways HDF5 can compress data, and it does also use a scheme
    // where several algorithms can be applied in sequence. For now, get
    // simple zlib deflate working.jhrg 1/15/17
    // Added support for shuffle. Assuming unshuffle always is applied _after_
    // inflating the data (reversing the shuffle --> deflate process). It is
    // possible that data could just be deflated or shuffled (because we
    // have test data are use only shuffle). jhrg 1/20/17

    chunk_size *= elem_width;

    if (deflate) {
        char *dest = new char[chunk_size];  // TODO unique_ptr<>. jhrg 1/15/17
        try {
            inflate(dest, chunk_size, get_rbuf(), get_rbuf_size());
            // This replaces (and deletes) the original read_buffer with dest.
            set_rbuf(dest, chunk_size);
        }
        catch (...) {
            delete[] dest;
            throw;
        }
    }

    if (shuffle) {
        // The internal buffer is chunk's full size at this point.
        char *dest = new char[get_rbuf_size()];
        try {
            unshuffle(dest, get_rbuf(), get_rbuf_size(), elem_width);
            set_rbuf(dest, get_rbuf_size());
        }
        catch (...) {
            delete[] dest;
            throw;
        }
    }

#if 0 // This was handy during development for debugging. Keep it for awhile (year or two) before we drop it ndp - 01/18/17
    if(BESDebug::IsSet("dmrpp")) {
        unsigned long long chunk_buf_size = get_rbuf_size();
        dods_float32 *vals = (dods_float32 *) get_rbuf();
        ostream *os = BESDebug::GetStrm();
        (*os) << std::fixed << std::setfill('_') << std::setw(10) << std::setprecision(0);
        (*os) << "DmrppArray::"<< __func__ <<"() - Chunk[" << i << "]: " << endl;
        for(unsigned long long k=0; k< chunk_buf_size/prototype()->width(); k++) {
            (*os) << vals[k] << ", " << ((k==0)|((k+1)%10)?"":"\n");
        }
    }
#endif
}

/**
 * @brief Read the chunk associated with this Chunk
 *
 * @param deflate True if we should deflate the data
 * @param shuffle_chunk True if the chunk was shuffled.
 * @param chunk_size The size of the chunk once deflated in elements; ignored when deflate is false
 * @param elem_width Number of bytes in an element; ignored when shuffle_chunk is false
 */
void Chunk::read(bool deflate, bool shuffle, unsigned int chunk_size, unsigned int elem_width)
{
    if (d_is_read) {
        BESDEBUG("dmrpp", "Chunk::"<< __func__ <<"() - Already been read! Returning." << endl);
        return;
    }

#if 0
    if (!d_is_in_multi_queue) {
#endif

        // This call uses the internal size param and allocates the buffer's memory
        set_rbuf_to_size();

        CURL *curl = DmrppRequestHandler::curl_handle_pool->get_easy_handle(this);
        if (!curl) throw BESInternalError("Error getting curl handle.", __FILE__, __LINE__);

        // Perform the request
        CURLcode curl_code = curl_easy_perform(curl);
        if (CURLE_OK != curl_code) {
            throw BESInternalError(string("Data transfer error: ").append(curl_easy_strerror(curl_code)), __FILE__, __LINE__);
        }

        ostringstream oss;
        // For HTTP, check the return code, for the file protocol, if curl_code is OK, that's good enough
        string http_url("http://");
        if (get_data_url().compare(0, http_url.size(), http_url) == 0 /*equal*/) {
            long http_code = 0;
            curl_code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (CURLE_OK != curl_code) {
                throw BESInternalError(string("Error getting HTTP response code: ").append(curl_easy_strerror(curl_code)), __FILE__, __LINE__);
            }

            if (http_code != 200) {
                oss << "HTTP status error. Expected an OK status, but got: ";
                oss << http_code;
                throw BESInternalError(oss.str(), __FILE__, __LINE__);
            }
        }

        DmrppRequestHandler::curl_handle_pool->release_handle(curl);

        // If the expected byte count was not read, it's an error.
        if (get_size() != get_bytes_read()) {
            oss << "Wrong number of bytes read for chunk; read: " << get_bytes_read() << ", expected: " << get_size();
            throw BESInternalError(oss.str(), __FILE__, __LINE__);
        }
#if 0
}
#endif


    if (deflate || shuffle)
        inflate_chunk(deflate, shuffle, chunk_size, elem_width);

#if 0
    d_is_in_multi_queue = false;
#endif


    d_is_read = true;
}

/**
 * Version of Chunk::read() for use with DmrppArray::read_chunk_serial()
 *
 * This method is for reading one chunk after the other, using a CURL handle
 * from the CurlHandlePool.
 *
 * @param deflate
 * @param shuffle
 * @param chunk_size
 * @param elem_width
 */
void Chunk::read_serial(bool deflate, bool shuffle, unsigned int chunk_size, unsigned int elem_width)
{
#if 0
    assert(!d_is_in_multi_queue);   // Never call this code when using the multi handles
#endif


    if (d_is_read) {
        BESDEBUG("dmrpp", "Chunk::"<< __func__ <<"() - Already been read! Returning." << endl);
        return;
    }

    set_rbuf_to_size();

    CURL *curl = DmrppRequestHandler::curl_handle_pool->get_easy_handle(this);
    if (!curl) throw BESInternalError("Error getting curl handle.", __FILE__, __LINE__);

    // Perform the request
    CURLcode curl_code = curl_easy_perform(curl);
    if (CURLE_OK != curl_code) {
        throw BESInternalError(string("Data transfer error: ").append(curl_easy_strerror(curl_code)), __FILE__,
            __LINE__);
    }

    ostringstream oss;
    // For HTTP, check the return code, for the file protocol, if curl_code is OK, that's good enough
    string http_url("http://");
    if (get_data_url().compare(0, http_url.size(), http_url) == 0 /*equal*/) {
        long http_code = 0;
        curl_code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (CURLE_OK != curl_code) {
            throw BESInternalError(string("Error getting HTTP response code: ").append(curl_easy_strerror(curl_code)),
                __FILE__, __LINE__);
        }

        if (http_code != 200) {
            oss << "HTTP status error. Expected an OK status, but got: ";
            oss << http_code;
            throw BESInternalError(oss.str(), __FILE__, __LINE__);
        }
    }

    DmrppRequestHandler::curl_handle_pool->release_handle(curl);

    // If the expected byte count was not read, it's an error.
    if (get_size() != get_bytes_read()) {
        oss << "Wrong number of bytes read for chunk; read: " << get_bytes_read() << ", expected: " << get_size();
        throw BESInternalError(oss.str(), __FILE__, __LINE__);
    }

    if (deflate || shuffle) inflate_chunk(deflate, shuffle, chunk_size, elem_width);

    d_is_read = true;
}

/**
 *
 *  unsigned long long d_size;
 *  unsigned long long d_offset;
 *  std::string d_md5;
 *  std::string d_uuid;
 *  std::string d_data_url;
 *  std::vector<unsigned int> d_chunk_position_in_array;
 *
 */
void Chunk::dump(ostream &oss) const
{
    oss << "Chunk";
    oss << "[ptr='" << (void *)this << "']";
    oss << "[data_url='" << d_data_url << "']";
    oss << "[offset=" << d_offset << "]";
    oss << "[size=" << d_size << "]";
    oss << "[chunk_position_in_array=(";
    for (unsigned long i = 0; i < d_chunk_position_in_array.size(); i++) {
        if (i) oss << ",";
        oss << d_chunk_position_in_array[i];
    }
    oss << ")]";
    oss << "[is_read=" << d_is_read << "]";
#if 0
    oss << "[is_in_multi_queue=" << d_is_in_multi_queue << "]";
#endif

}

string Chunk::to_string() const
{
    std::ostringstream oss;
    dump(oss);
    return oss.str();
}

} // namespace dmrpp

