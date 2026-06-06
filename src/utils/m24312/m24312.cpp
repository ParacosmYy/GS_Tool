#include "m24312/m24312.h"
QVector<double> m24312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
