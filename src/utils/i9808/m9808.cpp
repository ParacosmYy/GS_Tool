#include "i9808/m9808.h"
QVector<double> m9808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
