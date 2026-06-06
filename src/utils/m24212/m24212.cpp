#include "m24212/m24212.h"
QVector<double> m24212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
