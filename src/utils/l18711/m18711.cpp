#include "l18711/m18711.h"
QVector<double> m18711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
