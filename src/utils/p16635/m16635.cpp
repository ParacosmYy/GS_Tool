#include "p16635/m16635.h"
QVector<double> m16635::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
