#include "s9578/m9578.h"
QVector<double> m9578::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
