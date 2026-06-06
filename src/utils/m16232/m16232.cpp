#include "m16232/m16232.h"
QVector<double> m16232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
