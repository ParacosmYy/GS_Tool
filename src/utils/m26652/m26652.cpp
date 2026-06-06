#include "m26652/m26652.h"
QVector<double> m26652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
