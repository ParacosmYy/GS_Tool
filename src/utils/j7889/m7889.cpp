#include "j7889/m7889.h"
QVector<double> m7889::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
