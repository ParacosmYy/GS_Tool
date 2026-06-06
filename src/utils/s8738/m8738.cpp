#include "s8738/m8738.h"
QVector<double> m8738::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
