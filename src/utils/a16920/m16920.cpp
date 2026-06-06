#include "a16920/m16920.h"
QVector<double> m16920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
