#include "m9252/m9252.h"
QVector<double> m9252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
