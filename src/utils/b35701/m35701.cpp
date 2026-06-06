#include "b35701/m35701.h"
QVector<double> m35701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
