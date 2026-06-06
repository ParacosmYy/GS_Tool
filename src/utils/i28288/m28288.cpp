#include "i28288/m28288.h"
QVector<double> m28288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
