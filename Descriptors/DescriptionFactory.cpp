/*

Copyright (c) 2013, Project OSRM, Dennis Luxen, others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "DescriptionFactory.h"

DescriptionFactory::DescriptionFactory() : entireLength(0) { }

DescriptionFactory::~DescriptionFactory() { }

inline double DescriptionFactory::DegreeToRadian(const double degree) const {
        return degree * (M_PI/180);
}

inline double DescriptionFactory::RadianToDegree(const double radian) const {
        return radian * (180/M_PI);
}

double DescriptionFactory::GetBearing(
    const FixedPointCoordinate & A,
    const FixedPointCoordinate & B
) const {
    double delta_long = DegreeToRadian(B.lon/COORDINATE_PRECISION - A.lon/COORDINATE_PRECISION);

    const double lat1 = DegreeToRadian(A.lat/COORDINATE_PRECISION);
    const double lat2 = DegreeToRadian(B.lat/COORDINATE_PRECISION);

    const double y = sin(delta_long) * cos(lat2);
    const double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(delta_long);
    double result = RadianToDegree(atan2(y, x));
    while(result < 0.) {
        result += 360.;
    }
    while(result >= 360.) {
        result -= 360.;
    }
    return result;
}

void DescriptionFactory::SetStartSegment(const PhantomNode & start) {
    start_phantom = start;
    AppendSegment(
        start.location,
        PathData(0, start.nodeBasedEdgeNameID, 10, start.weight1)
    );
}

void DescriptionFactory::SetEndSegment(const PhantomNode & target) {
    target_phantom = target;
    pathDescription.push_back(
        SegmentInformation(
            target.location,
            target.nodeBasedEdgeNameID,
            0,
            target.weight1,
            0,
            true
        )
    );
}

void DescriptionFactory::AppendSegment(
    const FixedPointCoordinate & coordinate,
    const PathData & data
) {
    if(
        ( 1 == pathDescription.size())                  &&
        ( pathDescription.back().location == coordinate)
    ) {
        pathDescription.back().name_id = data.name_id;
    } else {
        pathDescription.push_back(
            SegmentInformation(
                coordinate,
                data.name_id,
                data.durationOfSegment,
                0,
                data.turnInstruction
            )
        );
    }
}

void DescriptionFactory::AppendEncodedPolylineString(
    const bool return_encoded,
    std::vector<std::string> & output
) {
    std::string temp;
    if(return_encoded) {
        polyline_compressor.printEncodedString(pathDescription, temp);
    } else {
        polyline_compressor.printUnencodedString(pathDescription, temp);
    }
    output.push_back(temp);
}

void DescriptionFactory::AppendEncodedPolylineString(
    std::vector<std::string> &output
) const {
    std::string temp;
    polyline_compressor.printEncodedString(pathDescription, temp);
    output.push_back(temp);
}

void DescriptionFactory::AppendUnencodedPolylineString(
    std::vector<std::string>& output
) const {
    std::string temp;
    polyline_compressor.printUnencodedString(pathDescription, temp);
    output.push_back(temp);
}

void DescriptionFactory::BuildRouteSummary(
    const double distance,
    const unsigned time
) {
    summary.startName = start_phantom.nodeBasedEdgeNameID;
    summary.destName = target_phantom.nodeBasedEdgeNameID;
    summary.BuildDurationAndLengthStrings(distance, time);
}
