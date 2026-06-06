#include "o35214/m35214.h"
QVector<double> m35214::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
