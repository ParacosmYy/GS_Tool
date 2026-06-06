#include "k24150/m24150.h"
QVector<double> m24150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
