#include "k18150/m18150.h"
QVector<double> m18150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
