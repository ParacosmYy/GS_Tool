#include "k32890/m32890.h"
QVector<double> m32890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
