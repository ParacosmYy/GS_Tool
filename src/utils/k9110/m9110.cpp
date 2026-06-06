#include "k9110/m9110.h"
QVector<double> m9110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
