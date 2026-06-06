#include "l31711/m31711.h"
QVector<double> m31711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
