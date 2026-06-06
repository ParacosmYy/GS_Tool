#include "l8031/m8031.h"
QVector<double> m8031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
