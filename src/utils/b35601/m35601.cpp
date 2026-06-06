#include "b35601/m35601.h"
QVector<double> m35601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
