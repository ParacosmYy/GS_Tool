#include "f35825/m35825.h"
QVector<double> m35825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
