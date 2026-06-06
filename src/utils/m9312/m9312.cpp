#include "m9312/m9312.h"
QVector<double> m9312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
