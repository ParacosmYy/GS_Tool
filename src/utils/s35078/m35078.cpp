#include "s35078/m35078.h"
QVector<double> m35078::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
