#include "m8672/m8672.h"
QVector<double> m8672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
