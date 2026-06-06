#include "k25250/m25250.h"
QVector<double> m25250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
