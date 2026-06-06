#include "k25310/m25310.h"
QVector<double> m25310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
