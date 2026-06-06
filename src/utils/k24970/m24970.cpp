#include "k24970/m24970.h"
QVector<double> m24970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
