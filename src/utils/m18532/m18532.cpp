#include "m18532/m18532.h"
QVector<double> m18532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
