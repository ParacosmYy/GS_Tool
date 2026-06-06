#include "s16738/m16738.h"
QVector<double> m16738::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
