#include "s9318/m9318.h"
QVector<double> m9318::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
