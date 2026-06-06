#include "c24102/m24102.h"
QVector<double> m24102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
