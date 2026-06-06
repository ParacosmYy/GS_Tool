#include "k15650/m15650.h"
QVector<double> m15650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
