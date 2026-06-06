#include "m18672/m18672.h"
QVector<double> m18672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
