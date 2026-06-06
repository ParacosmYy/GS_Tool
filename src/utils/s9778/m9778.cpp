#include "s9778/m9778.h"
QVector<double> m9778::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
