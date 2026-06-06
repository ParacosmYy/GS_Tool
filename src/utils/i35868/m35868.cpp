#include "i35868/m35868.h"
QVector<double> m35868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
