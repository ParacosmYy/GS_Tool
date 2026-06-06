#include "g16586/m16586.h"
QVector<double> m16586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
