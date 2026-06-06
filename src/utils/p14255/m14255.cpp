#include "p14255/m14255.h"
QVector<double> m14255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
