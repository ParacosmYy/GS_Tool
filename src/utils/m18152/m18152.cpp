#include "m18152/m18152.h"
QVector<double> m18152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
