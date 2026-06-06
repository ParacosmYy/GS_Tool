#include "k33250/m33250.h"
QVector<double> m33250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
