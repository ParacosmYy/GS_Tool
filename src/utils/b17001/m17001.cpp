#include "b17001/m17001.h"
QVector<double> m17001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
