#include "k12110/m12110.h"
QVector<double> m12110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
