#include "m9912/m9912.h"
QVector<double> m9912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
