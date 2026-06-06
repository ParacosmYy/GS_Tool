#include "l13711/m13711.h"
QVector<double> m13711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
