#include "l7911/m7911.h"
QVector<double> m7911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
