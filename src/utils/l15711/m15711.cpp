#include "l15711/m15711.h"
QVector<double> m15711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
