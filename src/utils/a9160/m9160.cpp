#include "a9160/m9160.h"
QVector<double> m9160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
