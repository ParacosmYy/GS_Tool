#include "m9952/m9952.h"
QVector<double> m9952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
