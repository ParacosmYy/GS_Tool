#include "k12650/m12650.h"
QVector<double> m12650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
