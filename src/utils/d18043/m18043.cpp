#include "d18043/m18043.h"
QVector<double> m18043::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
