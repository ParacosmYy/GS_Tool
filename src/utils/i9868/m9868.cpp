#include "i9868/m9868.h"
QVector<double> m9868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
