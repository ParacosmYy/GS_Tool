#include "s8058/m8058.h"
QVector<double> m8058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
