#include "m14532/m14532.h"
QVector<double> m14532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
