#include "d16263/m16263.h"
QVector<double> m16263::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
