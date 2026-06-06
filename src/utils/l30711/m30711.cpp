#include "l30711/m30711.h"
QVector<double> m30711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
