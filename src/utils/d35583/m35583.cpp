#include "d35583/m35583.h"
QVector<double> m35583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
