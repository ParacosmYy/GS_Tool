#include "k24010/m24010.h"
QVector<double> m24010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
