#include "l8751/m8751.h"
QVector<double> m8751::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
