#include "k10410/m10410.h"
QVector<double> m10410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
