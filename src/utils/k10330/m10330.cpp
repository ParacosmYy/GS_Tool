#include "k10330/m10330.h"
QVector<double> m10330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
