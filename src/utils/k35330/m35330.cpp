#include "k35330/m35330.h"
QVector<double> m35330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
