#include "m28672/m28672.h"
QVector<double> m28672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
