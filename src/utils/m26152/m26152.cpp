#include "m26152/m26152.h"
QVector<double> m26152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
