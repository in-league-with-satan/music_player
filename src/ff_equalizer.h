/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef FF_EQUALIZER_H
#define FF_EQUALIZER_H

#include <vector>

#include "ff_tools.h"

struct FFEqualizerContext;

struct EQParam
{
    int frequency;
    int width;
    double gain;
};

typedef std::vector <EQParam> EQParams;

class FFEqualizer
{
public:
    FFEqualizer();
    ~FFEqualizer();

    void setup(EQParams params);

    bool proc(AVFrame *frame);

private:
    int init();
    void free();

    FFEqualizerContext *d;
};

Q_DECLARE_METATYPE(EQParam)
Q_DECLARE_METATYPE(EQParams)

#endif // FF_EQUALIZER_H
