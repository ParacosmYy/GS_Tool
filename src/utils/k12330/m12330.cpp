#include "k12330/m12330.h"
QVector<double> m12330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
