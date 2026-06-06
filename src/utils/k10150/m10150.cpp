#include "k10150/m10150.h"
QVector<double> m10150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
