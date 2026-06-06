#include "m28052/m28052.h"
QVector<double> m28052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
