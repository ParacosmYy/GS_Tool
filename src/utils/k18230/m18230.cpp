#include "k18230/m18230.h"
QVector<double> m18230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
