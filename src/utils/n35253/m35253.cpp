#include "n35253/m35253.h"
QVector<double> m35253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
