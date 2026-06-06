#include "i37328/m37328.h"
QVector<double> m37328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
