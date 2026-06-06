#include "q8656/m8656.h"
QVector<double> m8656::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
