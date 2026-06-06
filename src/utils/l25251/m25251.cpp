#include "l25251/m25251.h"
QVector<double> m25251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
