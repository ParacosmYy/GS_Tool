#include "l26911/m26911.h"
QVector<double> m26911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
