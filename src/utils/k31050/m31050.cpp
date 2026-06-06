#include "k31050/m31050.h"
QVector<double> m31050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
