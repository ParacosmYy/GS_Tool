#include "k25370/m25370.h"
QVector<double> m25370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
