#include "i17568/m17568.h"
QVector<double> m17568::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
