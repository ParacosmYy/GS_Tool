#include "h8847/m8847.h"
QVector<double> m8847::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
