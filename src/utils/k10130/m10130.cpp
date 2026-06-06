#include "k10130/m10130.h"
QVector<double> m10130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
