#include "d35483/m35483.h"
QVector<double> m35483::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
