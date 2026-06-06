#include "t35339/m35339.h"
QVector<double> m35339::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
