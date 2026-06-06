#include "m25312/m25312.h"
QVector<double> m25312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
