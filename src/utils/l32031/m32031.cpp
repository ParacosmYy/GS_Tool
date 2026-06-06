#include "l32031/m32031.h"
QVector<double> m32031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
