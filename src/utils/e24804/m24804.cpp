#include "e24804/m24804.h"
QVector<double> m24804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
