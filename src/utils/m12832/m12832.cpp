#include "m12832/m12832.h"
QVector<double> m12832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
