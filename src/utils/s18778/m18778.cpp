#include "s18778/m18778.h"
QVector<double> m18778::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
