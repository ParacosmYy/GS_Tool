#include "p16315/m16315.h"
QVector<double> m16315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
