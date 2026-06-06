#include "k10910/m10910.h"
QVector<double> m10910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
