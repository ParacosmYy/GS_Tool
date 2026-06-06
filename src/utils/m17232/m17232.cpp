#include "m17232/m17232.h"
QVector<double> m17232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
