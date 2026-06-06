#include "i28068/m28068.h"
QVector<double> m28068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
