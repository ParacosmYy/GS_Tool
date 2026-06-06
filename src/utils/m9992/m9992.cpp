#include "m9992/m9992.h"
QVector<double> m9992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
