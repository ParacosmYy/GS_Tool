#include "l25031/m25031.h"
QVector<double> m25031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
