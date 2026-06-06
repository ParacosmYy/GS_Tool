#include "h7847/m7847.h"
QVector<double> m7847::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
