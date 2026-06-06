#include "a9180/m9180.h"
QVector<double> m9180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
