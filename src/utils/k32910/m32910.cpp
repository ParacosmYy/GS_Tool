#include "k32910/m32910.h"
QVector<double> m32910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
