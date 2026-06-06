#include "k15710/m15710.h"
QVector<double> m15710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
