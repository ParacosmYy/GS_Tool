#include "m25832/m25832.h"
QVector<double> m25832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
