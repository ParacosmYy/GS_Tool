#include "s9138/m9138.h"
QVector<double> m9138::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
