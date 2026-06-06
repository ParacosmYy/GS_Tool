#include "k29430/m29430.h"
QVector<double> m29430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
