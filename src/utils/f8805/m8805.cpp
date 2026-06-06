#include "f8805/m8805.h"
QVector<double> m8805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
