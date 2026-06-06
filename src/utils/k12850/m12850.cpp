#include "k12850/m12850.h"
QVector<double> m12850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
