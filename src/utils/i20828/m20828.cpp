#include "i20828/m20828.h"
QVector<double> m20828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
