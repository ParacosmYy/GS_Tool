#include "p16175/m16175.h"
QVector<double> m16175::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
