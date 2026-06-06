#include "a16240/m16240.h"
QVector<double> m16240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
