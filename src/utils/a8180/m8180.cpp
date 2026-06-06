#include "a8180/m8180.h"
QVector<double> m8180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
