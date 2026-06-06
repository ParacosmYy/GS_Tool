#include "k33310/m33310.h"
QVector<double> m33310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
