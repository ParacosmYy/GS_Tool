#include "f35725/m35725.h"
QVector<double> m35725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
