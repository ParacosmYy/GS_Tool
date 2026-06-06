#include "k10390/m10390.h"
QVector<double> m10390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
