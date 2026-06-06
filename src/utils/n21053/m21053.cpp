#include "n21053/m21053.h"
QVector<double> m21053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
