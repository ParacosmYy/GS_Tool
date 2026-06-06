#include "g25106/m25106.h"
QVector<double> m25106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
