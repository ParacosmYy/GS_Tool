#include "k18650/m18650.h"
QVector<double> m18650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
