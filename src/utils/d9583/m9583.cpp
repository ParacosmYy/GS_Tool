#include "d9583/m9583.h"
QVector<double> m9583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
