#include "k27110/m27110.h"
QVector<double> m27110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
