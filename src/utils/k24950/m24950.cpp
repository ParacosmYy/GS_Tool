#include "k24950/m24950.h"
QVector<double> m24950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
