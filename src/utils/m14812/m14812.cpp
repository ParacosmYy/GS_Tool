#include "m14812/m14812.h"
QVector<double> m14812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
